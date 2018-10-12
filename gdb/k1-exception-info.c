#include "config.h"
#include "defs.h"
#include <stdio.h>
#include <inttypes.h>
#include "inferior.h"
#include "k1-common-tdep.h"
#include "k1-target.h"
#include "k1-exception-info.h"
#include "ptid.h"
#include "gdbcore.h"
#include "gdbthread.h"
#include "common-types.h"

#define ARRAY_SIZE(a) (sizeof (a) / sizeof ((a)[0]))
#define MSG_FROM_ARRAY(a,i) ((i >= ARRAY_SIZE(a) || !a[i]) ? "[unknown]" : a[i])

#define MB (1024ULL * 1024)
#define GB (1024ULL * 1024 * 1024)

struct addr_range_s
{
  const char *name;
  uint64_t size;
} cool_addr_map[] = {
  {"Local SMEM", 4 * MB},
  {"Local Meta", 4 * MB},
  {"Local periph", 8 * MB},
  {"Cluster 0 SMEM", 4 * MB},
  {"Cluster 0 Meta", 4 * MB},
  {"Cluster 0 periph", 8 * MB},
  {"Cluster 1 SMEM", 4 * MB},
  {"Cluster 1 Meta", 4 * MB},
  {"Cluster 1 periph", 8 * MB},
  {"Cluster 2 SMEM", 4 * MB},
  {"Cluster 2 Meta", 4 * MB},
  {"Cluster 2 periph", 8 * MB},
  {"Cluster 3 SMEM", 4 * MB},
  {"Cluster 3 Meta", 4 * MB},
  {"Cluster 3 periph", 8 * MB},
  {"Cluster 4 SMEM", 4 * MB},
  {"Cluster 4 Meta", 4 * MB},
  {"Cluster 4 periph", 8 * MB},
  {"[reserved]", 32 * MB},
  {"AXI config", 256 * MB},
  {"AXI periphs", 256 * MB},
  {"PCIE alias", (2 * GB - 640 * MB)},
  {"DDR alias", 2 * GB},
  {"DDR", 64 * GB},
  {"PCIE", (256 * GB - 68 * GB)},
};

enum {
  EN_REG_OCEC_DES_SW_BREAK = 0,
  EN_REG_OCEC_DES_HW_BREAK = 1,
  EN_REG_OCEC_DES_WATCHPOINT = 2,
  EN_REG_OCEC_DES_HWTRAP = 5,
  EN_REG_OCEC_DES_SCALL = 7,
  EN_REG_OCEC_DES_DE_STEPI = 8,
  EN_REG_OCEC_DES_DE_HWTRAP = 9,
  EN_REG_OCEC_DES_DE_INT = 10,
  EN_REG_OCEC_DES_DE_SCALL = 11,
};
#define EN_REG_OCEC_DES_FIRST_DE EN_REG_OCEC_DES_DE_STEPI

const char *reg_ocec_des_msg[] = {
  "software break", // 0
  "hardware break", // 1
  "watchpoint", // 2
  NULL, // 3
  NULL, // 4
  "hardware trap interception", // 5
  NULL, // 6
  "scall trap interception", // 7
  "double exception interception on system_stepi", // 8
  "double exception interception on hardware trap", // 9
  "double exception interception on interrupt", // 10
  "double exception interception on scall", // 11
};

const char *trap_msg[] = {
  "RESET - reset sequence", // 0
  "OPCODE - malformed bundle", // 1
  "PRIVILEGE - attempt to execute an privilege instruction", // 2
  "DMISALIGN - effective address for data memory access is not aligned in areas or "
    "on instructions that do not support misalignment", // 3
  "PSYSERROR - system error to an external program fetch request", // 4
  "DSYSERROR - system error to an external data access", // 5
  "PDECCERROR - double ECC error to an external program fetch request", // 6
  "DDECCERROR - double ECC error to an external data access", // 7
  "PPARERROR - parity error to an external program fetch request", // 8
  "DPARERROR - parity error to an external data access", // 9
  "PSECCERROR - single ECC error to an external program fetch request", // 10
  "DSECCERROR - single ECC error to an external data access", // 11
  "NOMAPPING - memory translation trap", // 12
  "PROTECTION - memory translation trap", // 13
  "WRITETOCLEAN - memory translation trap", // 14
  "ATOMICTOCLEAN - memory translation trap", // 15
};

#define ES_RWS_FIRST_DATA 2
const char *es_rwx_msg[] = {
   "not a memory trap", // 0
   "fetch side memory trap", // 1
   "data side memory trap on a write access (dzerol is considered as a write access)", // 2 = ES_RWS_FIRST_DATA
   NULL, // 3
   "data side memory trap on a read access (including DINVALL, DTOUCHL, DINVAL, WPURGE)", // 4
   NULL, // 5
   "data side memory trap on an atomic access", // 6
};

#define ES_RWS_STEPI_FIRST_DATA 2
const char *es_rwx_stepi_msg[] = {
   "no data memory access in the stepped bundle (canceled non-trapping loads and "
     "non executed conditional ld/st are considered as such)", // 0
   NULL, // 1
   "data write memory access in the stepped bundle", // 2 = ES_RWS_STEPI_FIRST_DATA
   NULL, // 3
   "data read memory access in the stepped bundle (DINVALL, DTOUCHL, DINVAL, WPURGE "
     "are considered as read accesses)", // 4
   NULL, // 5
   "data atomic memory access in the stepped bundle", // 6
};

const char *es_nta_msg[] = {
  "trap was caused by a regular data memory access", // 0
  "trap was caused by a non-trapping data memory access", // 1
};

const char *es_nta_stepi_msg[] = {
  "the stepped bundle contains a regular data memory access", // 0
  "the stepped bundle contains a non-trapping data memory access", // 1
};

const char *es_as_msg[] = {
  [1] = "byte access",
  [2] = "half-word access",
  [4] = "word access (including atomic word size instructions)",
  [8] = "double-word access (including atomic double-word size instructions)",
  [16] = "quad-word access",
  [17] = "cache maintenance access (DINVALL, DTOUCHL, DINVAL)",
  [31] = "cache line-size access (DZEROL)",
};

enum {
  EN_ES_EC_RESET, // 0
  EN_ES_EC_HWTRAP, // 1
  EN_ES_EC_INT, // 2
  EN_ES_EC_SCALL, // 3
  EN_ES_EC_SYSTEM_STEPI // 4
};

const char *es_ec_msg[] = {
  "none", // 0
  "hardware trap", // 1
  "interrupt", // 2
  "scall", // 3
  "system stepi", // 4
};

#define PS_ET_BIT 2
#define PS_MME_BIT 11

union reg_ocec_s
{
  uint64_t reg;
  struct {
    uint64_t des:4;   // 0 - 3
    uint64_t wpe:1;   // 4 - 4
    uint64_t wpr:6;   // 5 - 10
    uint64_t htim:17; // 11 - 27
    uint64_t sim:2;   // 28 - 29
  } _;
};

union reg_es_s
{
  uint64_t reg;
  union
  {
    struct
    {
      uint64_t ec:3;  // 0 - 2
    } common;
    struct
    {
      uint64_t ec:3;  // 0 - 2
      uint64_t itn:5; // 3 - 7
      uint64_t itl:4; // 8 - 11
      uint64_t iti:10; // 12 - 21
    } irq;
    struct
    {
      uint64_t ec:3;  // 0 - 2
      uint64_t sn:12; // 3 - 14
    } scall;
    struct
    {
      uint64_t ec:3;  // 0 - 2
      uint64_t htc:5; // 3 - 7
      uint64_t rwx:3; // 8 - 10
      uint64_t nta:1; // 11- 11
      uint64_t uca:1; // 12 - 12
      uint64_t as:5;  // 13 - 17
      uint64_t bs:4;  // 18 - 21
      uint64_t ri:6;  // 22 - 27
    } trap;
    struct
    {
      uint64_t ec:3;  // 0 - 2
      uint64_t :5;    // 3 - 7
      uint64_t rwx:3; // 8 - 10
      uint64_t nta:1; // 11- 11
      uint64_t uca:1; // 12 - 12
      uint64_t as:5;  // 13 - 17
      uint64_t bs:4;  // 18 - 21
      uint64_t ri:6;  // 22 - 27
    } sys_si;
  };
};

static void
show_addr_info (uint64_t addr, int mme)
{
  const char *sym_name = NULL;
  struct bound_minimal_symbol min_sym;
  const char *sym_type;

  sym_type = "[unknown symbol type]";
  min_sym = lookup_minimal_symbol_by_pc_section (addr, NULL);
  if (min_sym.minsym)
  {
    int mst = MSYMBOL_TYPE(min_sym.minsym);
    if (mst == mst_text || mst == mst_text_gnu_ifunc || mst == mst_solib_trampoline || mst == mst_file_text)
      sym_type = "function";
    else if (mst == mst_slot_got_plt)
      sym_type = "got_plt section variable";
    else if (mst == mst_data || mst == mst_file_data)
      sym_type = "data section variable";
    else if (mst == mst_bss || mst == mst_file_bss)
      sym_type = "bss section variable";
    else if (mst == mst_abs)
      sym_type = "nonrelocatable variable";
    sym_name = MSYMBOL_PRINT_NAME (min_sym.minsym);
  }

  if (sym_name)
    printf (" in %s %s", sym_type, sym_name);
  else if (!mme)
  {
    int i;
    for (i = 0; i < ARRAY_SIZE(cool_addr_map); i++)
    {
      if (addr < cool_addr_map[i].size)
        break;
      addr -= cool_addr_map[i].size;
    }
    if (i < ARRAY_SIZE(cool_addr_map))
      printf (" in %s memory map range", cool_addr_map[i].name);
  }
}

static void
show_trap_info (union reg_es_s es)
{
  printf ("\ttrap cause: %s (es.htc=0x%x)\n",
    es.trap.htc >= ARRAY_SIZE(trap_msg) ? "[unknown]" : trap_msg[es.trap.htc], es.trap.htc);
  printf ("\tread write execute: %s (es.rwx=0x%x)\n", MSG_FROM_ARRAY (es_rwx_msg, es.trap.rwx), es.trap.rwx);
  if (es.trap.rwx >= ES_RWS_FIRST_DATA)
  {
    printf ("\tnon trapping access: %s (es.nta=0x%x)\n", MSG_FROM_ARRAY (es_nta_msg, es.trap.nta), es.trap.nta);
    printf ("\ttrap caused by an %scached memory access instruction (es.uca=0x%x)\n",
      es.trap.uca ? "un" : "", es.trap.uca);
    printf ("\taccess size: %s (es.as=0x%x)\n", MSG_FROM_ARRAY (es_as_msg, es.trap.as), es.trap.as);
    printf ("\tsource or destination operand register "
      "of the data memory access that caused the trap: r%d (es.ri=%u)\n", es.trap.ri, es.trap.ri);
  }
  printf ("\tbundle size: %d bits (%d syllable%s) (es.bs=%d)\n", es.trap.bs * 32, es.trap.bs,
    es.trap.bs > 1 ? "s" : "", es.trap.bs);
}

static void
show_stepi_trap_info (union reg_es_s es)
{
  printf ("\tread write execute: %s (es.rwx=0x%x\n)",
    MSG_FROM_ARRAY (es_rwx_stepi_msg, es.sys_si.rwx), es.sys_si.rwx);
  if (es.sys_si.rwx >= ES_RWS_STEPI_FIRST_DATA)
  {
    printf ("\tnon trapping access: %s (es.nta=0x%x)\n",
      MSG_FROM_ARRAY (es_nta_stepi_msg, es.sys_si.nta), es.sys_si.nta);
    printf ("\tthe stepped bundle %s a *U LSU instruction (es.uca=0x%x)\n",
      es.sys_si.uca ? "contains" : "does not contain", es.sys_si.uca);
    printf ("\tdata access size of the stepped bundle: %s (es.as=0x%x)\n",
      MSG_FROM_ARRAY (es_as_msg, es.sys_si.as), es.sys_si.as);
    printf ("\tsource or destination operand register of the data "
      "memory access in the stepped bundle: r%d (es.ri=%u)\n", es.sys_si.ri, es.sys_si.ri);
  }
  printf ("\tbundle size of the stepped bundle: %d bits (%d syllable%s) (es.bs=%d)\n", es.sys_si.bs * 32,
    es.sys_si.bs, es.sys_si.bs > 1 ? "s" : "", es.sys_si.bs);
}

static void
stopped_cpu_status (void)
{
  struct gdbarch_tdep *tdep = gdbarch_tdep (target_gdbarch ());
  struct regcache *reg_cache = get_current_regcache ();
  union reg_ocec_s ocec;
  union reg_es_s es;
  uint64_t pc, spc, ea, ps;
  int mme, ps_et;

  // read required registers
  regcache_raw_read_unsigned (reg_cache, tdep->ocec_regnum, &ocec.reg);
  regcache_raw_read_unsigned (reg_cache, tdep->es_regnum, &es.reg);
  regcache_raw_read_unsigned (reg_cache, tdep->ea_regnum, &ea);
  regcache_raw_read_unsigned (reg_cache, tdep->ps_regnum, &ps);

  mme = (ps & (1 << PS_MME_BIT)) != 0;
  ps_et = (ps & (1 << PS_ET_BIT)) != 0;

  printf ("The processor entered debug mode because of a %s (ocec.des=0x%x).\n",
    MSG_FROM_ARRAY (reg_ocec_des_msg, ocec._.des), ocec._.des);

  if (ocec._.des <= EN_REG_OCEC_DES_HWTRAP)
  {
    if (ps_et && es.common.ec != 0 /*none*/)
    {
      printf ("Trap information (es=0x%" PRIx64 "):\n", es.reg);
      printf ("\ttrap type: %s (es.ec=%d)\n", MSG_FROM_ARRAY (es_ec_msg, es.common.ec), es.common.ec);
      printf ("\texception address (ea): 0x%" PRIx64, ea);
      show_addr_info (ea, mme);
      printf ("\n");
      show_trap_info (es);
    }
    else
    {
      printf ("Not in a trap (ps.et=%d, es.ec=%d)\n", ps_et, es.common.ec);
    }
  }
  else if (ocec._.des == EN_REG_OCEC_DES_SCALL)
  {
    printf ("Scall number: %d (es.sn=%d)\n", es.scall.sn, es.scall.sn);
  }
  else if (ocec._.des > EN_REG_OCEC_DES_FIRST_DE)
  {
    if (ocec._.des == EN_REG_OCEC_DES_DE_HWTRAP)
    {
      printf ("\texception address (ea): 0x%" PRIx64, ea);
      show_addr_info (ea, mme);
      printf ("\n");
    }
    printf ("First exception taken: %s (es.ec=%d)\n", MSG_FROM_ARRAY (es_ec_msg, es.common.ec), es.common.ec);
    if (es.common.ec == EN_ES_EC_HWTRAP)
      show_trap_info (es);
    else if (es.common.ec == EN_ES_EC_SCALL)
      printf ("\tscall number: %d (es.sn=%d)\n", es.scall.sn, es.scall.sn);
    else if (es.common.ec == EN_ES_EC_INT)
    {
      printf ("\tinterrupt number %d: (es.itn=%d)\n", es.irq.itn, es.irq.itn);
      printf ("\tinterrupt level: %d (es.itl=%d)\n", es.irq.itl, es.irq.itl);
      printf ("\tinterrupt info (copy of mes for memory related interrupts: 12 for SECCs, "
        "16 for Data asynchronous memory errors, 17 for I/D line invalidation): %x (es.iti=%d)\n",
        es.irq.iti, es.irq.iti);
    }
    else if (es.common.ec == EN_ES_EC_SYSTEM_STEPI)
    {
      printf ("\texception address (ea): 0x%" PRIx64, ea);
      show_addr_info (ea, mme);
      printf ("\n");
      show_stepi_trap_info (es);
    }
  }

  // PC information
  pc = regcache_read_pc (reg_cache);
  printf ("pc: 0x%" PRIx64, pc);
  show_addr_info (pc, mme);
  printf ("\n");

  // SPC info
  regcache_raw_read_unsigned (reg_cache, tdep->spc_regnum, &spc);
  printf ("spc: 0x%" PRIx64, spc);
  show_addr_info (spc, mme);
  printf ("\n");
}

static const char *cpu_status[] = {
  "sleeping", // 0
  "reseting", // 1
  "running", // 2
  "idle", // 3
};
#define CPU_FSM_IDLE 3

static void
show_pwr_cpu_status (void)
{
  uint64_t power_control_status_addr, status = 0;
  uint64_t rst, power_control_rst_addr = 0x941040; // &mppa_pwr_ctrl[0]->vector_proc_control.reset_on_wakeup.write
  int idle, fsm, wd;
  int vehicle = inferior_ptid.lwp - 1;

  if (vehicle < 16) // pe
    power_control_status_addr = 0x940400 + vehicle * 16; // &mppa_pwr_ctrl[0]->pe_status[vehicle].proc_sts
  else
    power_control_status_addr = 0x944100; // &mppa_pwr_ctrl[0]->rm_status.proc_sts

  if (!read_memory_no_dcache (power_control_status_addr, (gdb_byte *) &status, 8))
		error (_("[cannot read power controller status]\n"));
  if (!read_memory_no_dcache (power_control_rst_addr, (gdb_byte *) &rst, 8))
		error (_("[cannot read power controller reset]\n"));

  wd = status & 1;
  idle = (status >> 1) & 3;
  fsm = (status >> 3) & 3;

  printf ("The processor state: %s", cpu_status[fsm]);
  if (fsm == CPU_FSM_IDLE)
    printf ("%d", idle);
  else
    printf (", last idle: %d", idle);
  printf (", watchdog %s", wd ? "set" : "not set");
  printf (", reset on wakeup %s\n", ((1 << vehicle) & rst) ? "set" : "not set");
}

void
mppa_cpu_status_command (char *args, int from_tty)
{
  if (ptid_equal (inferior_ptid, null_ptid))
  {
    printf (_ ("No thread selected.\n"));
    return;
  }

  if (is_stopped (inferior_ptid))
    stopped_cpu_status ();
  else
    printf ("The processor is not in debug mode.\n");

  show_pwr_cpu_status ();
}
