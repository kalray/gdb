/* Target-dependent code for the Kalray K1 for GDB, the GNU debugger.

   Copyright (C) 2010 Kalray

   This file is part of GDB.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#include "defs.h"
#include "arch-utils.h"
#include "cli/cli-decode.h"
#include "frame-unwind.h"
#include "gdbcmd.h"
#include "gdbcore.h"
#include "objfiles.h"
#include "observer.h"
#include "reggroups.h"
#include "target-descriptions.h"
#include "remote.h"
#include "event-loop.h"

#include "k1-target.h"
#include "gdbthread.h"
#include "k1-common-tdep.h"

struct k1_inferior_data
{
  CORE_ADDR step_pad_area;
  CORE_ADDR step_pad_area_lma;
  int has_step_pad_area_p;
  int has_step_pad_area_lma_p;
};

struct displaced_step_closure
{
  /* Take into account that ALUs might have extensions. */
  uint32_t insn_words[8];
  int num_insn_words;

  unsigned branchy : 1;
  unsigned scall_jump : 1;
  unsigned rewrite_RA : 1;
  unsigned rewrite_LE : 1;
  unsigned rewrite_reg : 1;

  /* The destination address when the branch is taken. */
  unsigned long long dest;
  int reg;
};

static const struct k1_inferior_data *_k1_inferior_data;
static const struct inferior_data *k1_inferior_data_token;

static struct k1_inferior_data*
k1_inferior_data (struct inferior *inf)
{
  struct k1_inferior_data *res;

  res = inferior_data (inf, k1_inferior_data_token);
  if (!res)
  {
    res = xcalloc (1, sizeof (*res));
    set_inferior_data (inf, k1_inferior_data_token, res);
  }

  return res;
}

static CORE_ADDR k1_fetch_tls_load_module_address (struct objfile *objfile)
{
  struct regcache *regs = get_current_regcache ();
  ULONGEST val;

  regcache_raw_read_unsigned (regs, gdbarch_tdep (target_gdbarch ())->local_regnum, &val);
  return val;
}

int
get_is_hot_attached (struct inferior *inf)
{
  int ret;
  char *buf = (char *) malloc (256);
  long size = 256;

  strcpy (buf, "ka");
  putpkt (buf);
  getpkt (&buf, &size, 0);

  ret = *buf - '0';
  free (buf);

  return ret;
}

static void
k1_inferior_created (struct target_ops *target, int from_tty)
{
  int os_debug_level, cont = 0;
  struct inferior *inf;

  k1_current_arch = K1_NUM_ARCHES;

  if (after_first_resume)
    return;

  if (!(inf = current_inferior ()))
    return;

  os_debug_level = get_os_supported_debug_levels (inf);
  if (os_debug_level == DBG_LEVEL_SYSTEM)
    return;

  if (idx_global_debug_level)
  {
    set_cluster_debug_level_no_check (inf, idx_global_debug_level);
    cont = 1;
  }
  else if (!global_debug_level_set)
  {
    //set global debug level if not set before attach and if we boot with gdb
    struct regcache *rc = get_current_regcache ();
    int is_hot_attached = get_is_hot_attached (inf);
    CORE_ADDR pc = regcache_read_pc (rc);

    if (rc && !is_hot_attached)
    {
      apply_global_debug_level (os_debug_level);
      if (pc == 0 || os_debug_level == DBG_LEVEL_USER)
        cont = 1;
    }
  } // !global_debug_level_set

  if (cont)
  {
    inf_created_change_th = 1;
    continue_1 (0);
  }
}

static CORE_ADDR
k1_displaced_step_location (struct gdbarch *gdbarch)
{
  struct gdbarch_tdep *tdep = gdbarch_tdep (gdbarch);
  struct k1_inferior_data *data = k1_inferior_data (current_inferior ());
  struct regcache *regs = get_current_regcache ();
  ULONGEST ps;

  if (!data->has_step_pad_area_p)
  {
    struct bound_minimal_symbol msym = lookup_minimal_symbol ("_debug_start", NULL, NULL);
    if (msym.minsym == NULL)
      error ("Can not locate a suitable step pad area.");
    if (BMSYMBOL_VALUE_ADDRESS (msym) % 4)
      warning ("Step pad area is not 4-byte aligned.");
    data->step_pad_area = (BMSYMBOL_VALUE_ADDRESS (msym) + 3) & ~0x3;
    data->has_step_pad_area_p = 1;

    msym = lookup_minimal_symbol ("_debug_start_lma", NULL, NULL);
    if (msym.minsym != NULL)
    {
      if (BMSYMBOL_VALUE_ADDRESS (msym) % 4)
        warning ("Physical step pad area is not 4-byte aligned.");
      data->step_pad_area_lma = (BMSYMBOL_VALUE_ADDRESS (msym) + 3) & ~0x3;
      data->has_step_pad_area_lma_p = 1;
    }
  }

  if (data->has_step_pad_area_lma_p)
  {
    regcache_raw_read_unsigned (regs, tdep->ps_regnum, &ps);
    /* MMU active ? */
    if ((ps & (1 << 11)) != 0)
      return data->step_pad_area_lma;
  }

  return data->step_pad_area;
}

static int
extract_mds_bitfield (k1opc_t *op, uint32_t syllab, int bitfield)
{
  k1_bitfield_t *bfield;
  int res;

  bfield = &op->format[bitfield]->bfield[0];
  res = (syllab >> bfield->to_offset) & ((1 << bfield->size) - 1);

  if (res & (1 << (bfield->size - 1)))
    res |= (0xffffffff << bfield->size);

  return res;
}

static void
patch_mds_bitfield (k1opc_t *op, uint32_t *syllab, int bitfield, int value)
{
  k1_bitfield_t *bfield;
  uint32_t mask;

  bfield = &op->format[bitfield]->bfield[0];
  mask = ~(((1 << bfield->size) - 1) << bfield->to_offset);
  *syllab &= mask;
  *syllab |= (value << bfield->to_offset) & ~mask;
}

void
send_stop_at_main (int bstop)
{
  char *buf;
  long size;

  if (ptid_equal (inferior_ptid, null_ptid))
    return;

  size = 256;
  buf = (char *) malloc (size);
  sprintf (buf, "ks%d", bstop);
  putpkt (buf);
  getpkt (&buf, &size, 0);
  free (buf);
}

void
send_cluster_break_on_spawn (struct inferior *inf, int v)
{
  char *buf;
  long size = 256;

  buf = (char *) malloc (size);
  sprintf (buf, "kS%dp%x.1", v, inf->pid);
  putpkt (buf);
  getpkt (&buf, &size, 0);
  free (buf);
}

void
send_cluster_debug_level (int level)
{
  char *buf;
  long size;

  if (ptid_equal (inferior_ptid, null_ptid))
    return;

  size = 256;
  buf = (char *) malloc (size);
  sprintf (buf, "kD%d", level);
  putpkt (buf);
  getpkt (&buf, &size, 0);
  free (buf);
}

void
send_cluster_postponed_debug_level (struct inferior *inf, int level)
{
  char *buf;
  long size;

  size = 256;
  buf = (char *) malloc (size);
  sprintf (buf, "kP%dp%x.1", level, inf->pid);
  putpkt (buf);
  getpkt (&buf, &size, 0);
  free (buf);
}

int
get_os_supported_debug_levels (struct inferior *inf)
{
  char *buf = (char *) malloc (256);
  long size = 256;
  struct inferior_data *data = mppa_inferior_data (inf);
  int ret = data->os_supported_debug_level;

  if (data->os_supported_debug_level == -1)
  {
    sprintf (buf, "kdp%x.1", (unsigned int) inf->pid);
    putpkt (buf);
    getpkt (&buf, &size, 0);

    ret = *buf - '0';
    free (buf);

    if (ret < DBG_LEVEL_SYSTEM || ret >= DBG_LEVEL_MAX)
      ret = DBG_LEVEL_SYSTEM;

    data->os_supported_debug_level = ret;
  }

  return ret;
}

static void
inform_dsu_stepi_bkp (void)
{
  char *buf = (char *) malloc (256);
  long size = 256;
  strcpy (buf, "kB");
  putpkt (buf);
  getpkt (&buf, &size, 0);
  free (buf);
}

char *
send_get_dev_list_string (const char *full_name)
{
  long size = 4096, len, result_size = 0;
  char *result = strdup (""), *buf = (char *) malloc (size);

  while (1)
  {
    sprintf (buf, "kl:%lx,%lx:%s", result_size, size - 4, full_name);
    putpkt (buf);
    getpkt (&buf, &size, 0);

    len = strlen (buf);
    if (len <= 1)
      break;

    result = realloc (result, result_size + len);
    strcpy (result + result_size, buf + 1);
    result_size += len - 1;
  }

  return result;
}

int
send_set_kwatch (const char *full_name, int watch_type, int bset, char **err_msg)
{
  long len = strlen (full_name), size = len + 300;
  char *buf = (char *) malloc (size);
  int ret;

  sprintf (buf, "kW%d%d:%s", watch_type, bset, full_name);
  putpkt (buf);
  getpkt (&buf, &size, 0);

  ret = strcmp (buf, "OK");
  if (ret && err_msg)
    *err_msg = strdup (buf + 3);

  free (buf);

  return ret;
}

int
get_cluster_debug_level (int pid)
{
  struct inferior *inf;
  int ret;

  if (pid == -1)
    inf = current_inferior ();
  else
    inf = find_inferior_pid (pid);

  ret = mppa_inferior_data (inf)->cluster_debug_level;
  return ret;
}

int
get_debug_level (int pid)
{
  int ret = get_cluster_debug_level (pid);

  if (ret == DBG_LEVEL_INHERITED)
    ret = idx_global_debug_level;

  return ret;
}

static void
patch_bcu_instruction (struct gdbarch *gdbarch, CORE_ADDR from, CORE_ADDR to, struct regcache *regs,
  struct displaced_step_closure *dsc)
{
  struct op_list *insn = branch_insns[k1_arch ()];
  struct gdbarch_tdep *tdep = gdbarch_tdep (gdbarch);

  if (debug_displaced)
    printf_filtered ("displaced: Looking at BCU instruction\n");

  /* In order to limit side effects, we patch every instruction or
     register in order to make branches target the step pad. This
     way we have a simple way to check if branches were taken or
     not. 'scalls' are an exception to this rule, because we don't
     want to change the 'ev' register that might influence other
     things than syscalls.
   */

  while (insn)
  {
    k1opc_t *op = insn->op;

    if ((dsc->insn_words[0] & op->codeword[0].mask) != op->codeword[0].opcode)
    {
      insn = insn->next;
      continue;
    }

    if (debug_displaced)
      printf_filtered ("displaced: found branchy BCU insn: %s\n", op->as_op);

    dsc->branchy = 1;

    if (strcmp ("call", op->as_op) == 0)
    {
      dsc->rewrite_RA = 1;
      dsc->dest = from + extract_mds_bitfield (op, dsc->insn_words[0], 0) * 4;
      patch_mds_bitfield (op, &dsc->insn_words[0], 0, 0);
    }
    else if (strcmp ("goto", op->as_op) == 0)
    {
      dsc->dest = from + extract_mds_bitfield (op, dsc->insn_words[0], 0) * 4;
      patch_mds_bitfield (op, &dsc->insn_words[0], 0, 0);
    }
    else if (strncmp ("cjl.", op->as_op, 4) == 0)
    {
      ULONGEST ra;

      dsc->rewrite_RA = 1;
      regcache_raw_read_unsigned (regs, tdep->ra_regnum, &ra);
      dsc->dest = ra;
      regcache_raw_write_unsigned (regs, tdep->ra_regnum, to);
    }
    else if (strncmp ("cb.", op->as_op, 3) == 0)
    {
      dsc->dest = from + extract_mds_bitfield (op, dsc->insn_words[0], 1) * 4;
      patch_mds_bitfield (op, &dsc->insn_words[0], 1, 0);
    }
    else if (strcmp ("icall", op->as_op) == 0)
    {
      ULONGEST reg_value;

      dsc->rewrite_RA = 1;
      dsc->rewrite_reg = 1;
      dsc->reg = extract_mds_bitfield (op, dsc->insn_words[0], 0);
      regcache_raw_read_unsigned (regs, dsc->reg, &reg_value);
      dsc->dest = reg_value;
      regcache_raw_write_unsigned (regs, dsc->reg, to);
    }
    else if (strcmp ("igoto", op->as_op) == 0)
    {
      ULONGEST reg_value;

      dsc->rewrite_reg = 1;
      dsc->reg = extract_mds_bitfield (op, dsc->insn_words[0], 0);
      regcache_raw_read_unsigned (regs, dsc->reg, &reg_value);
      dsc->dest = reg_value;
      regcache_raw_write_unsigned (regs, dsc->reg, to);
    }
    else if (strcmp ("ret", op->as_op) == 0)
    {
      ULONGEST reg_value;

      dsc->rewrite_reg = 1;
      dsc->reg = tdep->ra_regnum;
      regcache_raw_read_unsigned (regs, dsc->reg, &reg_value);
      dsc->dest = reg_value;
      regcache_raw_write_unsigned (regs, dsc->reg, to);
    }
    else if (strcmp ("rfe", op->as_op) == 0)
    {
      ULONGEST reg_value;

      dsc->rewrite_reg = 1;
      dsc->reg = tdep->spc_regnum;
      regcache_raw_read_unsigned (regs, dsc->reg, &reg_value);
      dsc->dest = reg_value;
      regcache_raw_write_unsigned (regs, dsc->reg, to);
    }
    else if (strcmp ("scall", op->as_op) == 0 || strcmp ("trapa", op->as_op) == 0 || strcmp ("trapo", op->as_op) == 0)
    {
      ULONGEST reg_value;

      dsc->scall_jump = 1;
      regcache_raw_read_unsigned (regs, tdep->ev_regnum, &reg_value);
      dsc->dest = (reg_value & ~0xFFF) | ((reg_value & 0xFFF) << 1) | (reg_value & 0xFFF);
    }
    else if (strcmp ("loopdo", op->as_op) == 0 || strcmp ("loopgtz", op->as_op) == 0
      || strcmp ("loopnez", op->as_op) == 0)
    {
      ULONGEST reg_value;

      dsc->rewrite_LE = 1;
      dsc->dest = from + extract_mds_bitfield (op, dsc->insn_words[0], 1) * 4;
      patch_mds_bitfield (op, &dsc->insn_words[0], 1, 0);
    }
    else if (strcmp ("get", op->as_op) == 0 && op->format[1]->reg_nb != 64)
    {
      /* get version with immediate */
      if (extract_mds_bitfield (op, dsc->insn_words[0], 1) != (gdbarch_pc_regnum (gdbarch) - 64))
        break;

      dsc->branchy = 0;
      dsc->rewrite_reg = 1;
      dsc->reg = extract_mds_bitfield (op, dsc->insn_words[0], 0) & 0x3F;
      dsc->dest = from;
    }
    else if (strcmp ("get", op->as_op) == 0)
    {
      ULONGEST reg;

      /* indirect get instruction */
      reg = extract_mds_bitfield (op, dsc->insn_words[0], 1);
      regcache_raw_read_unsigned (regs, reg, &reg);

      if (reg != (gdbarch_pc_regnum (gdbarch) - 64))
        break;

      dsc->branchy = 0;
      dsc->rewrite_reg = 1;
      dsc->reg = extract_mds_bitfield (op, dsc->insn_words[0], 0) & 0x3F;
      dsc->dest = from;
    }
    else
    {
      internal_error (__FILE__, __LINE__, "Unknwon BCU insn");
    }

    break;
  }
}

static struct displaced_step_closure *
k1_displaced_step_copy_insn (struct gdbarch *gdbarch, CORE_ADDR from, CORE_ADDR to, struct regcache *regs)
{
  struct displaced_step_closure *dsc = xzalloc (sizeof (struct displaced_step_closure));
  struct k1_inferior_data *data = k1_inferior_data (current_inferior ());

  if (debug_displaced)
    printf_filtered ("displaced: copying from %s\n", paddress (gdbarch, from));

  do
  {
    read_memory (from + dsc->num_insn_words * 4, (gdb_byte*) (dsc->insn_words + dsc->num_insn_words), 4);
  } while (dsc->insn_words[dsc->num_insn_words++] & (1 << 31));

  if (debug_displaced)
  {
    int i;
    printf_filtered ("displaced: copied a %i word(s)\n", dsc->num_insn_words);
    for (i = 0; i < dsc->num_insn_words; ++i)
      printf_filtered ("displaced: insn[%i] = %08x\n", i, dsc->insn_words[i]);
  }

  dsc->insn_words[0] = extract_unsigned_integer ((const gdb_byte*) dsc->insn_words, 4, gdbarch_byte_order (gdbarch));
  if (((dsc->insn_words[0] >> 29) & 0x3) == 0)
    patch_bcu_instruction (gdbarch, from, to, regs, dsc);
  store_unsigned_integer ((gdb_byte*) dsc->insn_words, 4, gdbarch_byte_order (gdbarch), dsc->insn_words[0]);

  write_memory (to, (gdb_byte*) dsc->insn_words, dsc->num_insn_words * 4);

  inform_dsu_stepi_bkp ();

  return dsc;
}

static void
stepi_to_skip_pm_cb (void *context)
{
  char si_cmd[25];
  strcpy (si_cmd, "si");
  execute_command (si_cmd, 0);
}

static void
k1_displaced_step_fixup (struct gdbarch *gdbarch, struct displaced_step_closure *dsc,
  CORE_ADDR from, CORE_ADDR to, struct regcache *regs)
{
  ULONGEST ps, lc, le, pc;
  struct gdbarch_tdep *tdep = gdbarch_tdep (gdbarch);
  int branched = 0;
  int exception = 0;
  struct thread_info *th;
  int cpu_level, debug_level;

  if (debug_displaced)
    printf_filtered ("displaced: Fixup\n");

  regcache_raw_read_unsigned (regs, tdep->ps_regnum, &ps);
  pc = regcache_read_pc (regs);
  if (debug_displaced)
    printf_filtered ("displaced: new pc %s\n", paddress (gdbarch, pc));
  if (pc - to == dsc->num_insn_words * 4)
  {
    pc = from + (pc - to);
    if (debug_displaced)
      printf_filtered ("displaced: Didn't branch\n");
  }
  else
  {
    ULONGEST spc;

    /* We branched. */
    branched = 1;
    if (debug_displaced)
      printf_filtered ("displaced: we branched (predicted dest: %llx) \n", dsc->dest);
    if (dsc->branchy && (pc == to || (dsc->scall_jump && pc == dsc->dest)))
    {
      /* The branchy instruction jumped to its destination. */
      pc = dsc->dest;

      /* Rewrite RA only if the brach executed correctly. */
      if (dsc->rewrite_RA)
      {
        regcache_raw_write_unsigned (regs, tdep->ra_regnum, from + dsc->num_insn_words * 4);
        if (debug_displaced)
          printf_filtered ("displaced: rewrite RA\n");
      }

      if (dsc->scall_jump)
      {
        regcache_raw_write_unsigned (regs, tdep->spc_regnum, from + dsc->num_insn_words * 4);
        if (debug_displaced)
          printf_filtered ("displaced: rewrite SPC\n");
      }
    }
    else
    {
      // Uh oh... seems we've taken some exceptional condition. This means interrupt or H/W trap
      regcache_raw_read_unsigned (regs, tdep->spc_regnum, &spc);
      if (debug_displaced)
        printf_filtered ("displaced: trapped SPC=%lx\n", (unsigned long) spc);
      gdb_assert (spc == to);
      spc = from;
      regcache_raw_write_unsigned (regs, tdep->spc_regnum, spc);
      exception = 1;
    }
  }

  /* Rewrite a patched reg unconditionnaly */
  if (dsc->rewrite_reg)
  {
    regcache_raw_write_unsigned (regs, dsc->reg, dsc->dest);
    if (debug_displaced)
      printf_filtered ("displaced: rewrite %i with %llx\n", dsc->reg, dsc->dest);
  }

  if (((ps >> 5) & 1) /* HLE */)
  {

    /* The loop setup is done only if H/W loops are actually
       enabled. */
    if (!exception && dsc->rewrite_LE)
    {
      regcache_raw_write_unsigned (regs, tdep->le_regnum, dsc->dest);
      regcache_raw_write_unsigned (regs, tdep->ls_regnum, from + dsc->num_insn_words * 4);
      if (debug_displaced)
        printf_filtered ("displaced: rewrite LE\n");
    }

    if (!branched)
    {
      regcache_raw_read_unsigned (regs, tdep->le_regnum, &le);
      if (debug_displaced)
        printf_filtered ("displaced: active loop pc(%llx) le(%llx)\n",
          (unsigned long long) pc, (unsigned long long) le);

      if (pc == le)
      {
        if (debug_displaced)
          printf_filtered ("displaced: at loop end\n");
        regcache_raw_read_unsigned (regs, tdep->lc_regnum, &lc);
        if (lc - 1 == 0)
          regcache_raw_read_unsigned (regs, tdep->le_regnum, &pc);
        else
          regcache_raw_read_unsigned (regs, tdep->ls_regnum, &pc);
        if (lc != 0)
          regcache_raw_write_unsigned (regs, tdep->lc_regnum, lc - 1);
      }
    }
  }

  regcache_write_pc (regs, pc);
  if (debug_displaced)
    printf_filtered ("displaced: writing PC %s\n", paddress (gdbarch, pc));

  th = find_thread_ptid (inferior_ptid);
  cpu_level = get_cpu_exec_level ();
  debug_level = get_debug_level (inferior_ptid.pid);

  if (debug_level > cpu_level)
  {
    if (th->control.step_range_start == 1 || th->control.step_range_end == 1)
    {
      printf ("[Schedule stepi after breakpoint to skip PM, exec_level = %s"
        ", debug_level = %s]\n", get_str_debug_level (cpu_level), get_str_debug_level (debug_level));

      create_timer (0, stepi_to_skip_pm_cb, NULL);
    }
  }
}

static void
k1_displaced_step_free_closure (struct gdbarch *gdbarch, struct displaced_step_closure *closure)
{
  xfree (closure);
}

static int
k1_register_reggroup_p (struct gdbarch *gdbarch, int regnum, struct reggroup *group)
{
  if (gdbarch_register_name (gdbarch, regnum) == NULL || *gdbarch_register_name (gdbarch, regnum) == '\0')
    return 0;

  if ((group == save_reggroup || group == restore_reggroup || group == all_reggroup)
    && strncmp (gdbarch_register_name (gdbarch, regnum), "oce", 3) == 0)
    return 0;

  return default_register_reggroup_p (gdbarch, regnum, group);
}

static struct gdbarch *
k1_gdbarch_init (struct gdbarch_info info, struct gdbarch_list *arches)
{
  struct gdbarch *gdbarch;
  struct gdbarch_tdep *tdep;
  const struct target_desc *tdesc;
  struct tdesc_arch_data *tdesc_data;
  int i;
  unsigned long mach;
  int has_pc = -1, has_sp = -1, has_le = -1, has_ls = -1, has_ps = -1;
  int has_ev = -1, has_lc = -1, has_local = -1, has_ra = -1, has_spc = -1;

  static const char k1_ev_name[] = "ev";
  static const char k1_lc_name[] = "lc";
  static const char k1_ls_name[] = "ls";
  static const char k1_le_name[] = "le";
  static const char k1_ps_name[] = "ps";
  static const char k1_ra_name[] = "ra";
  static const char k1_spc_name[] = "spc";
  static const char k1_local_name[] = "r13";

  const char *pc_name;
  const char *sp_name;

  /* If there is already a candidate, use it.  */
  arches = gdbarch_list_lookup_by_info (arches, &info);
  if (arches != NULL)
    return arches->gdbarch;

  tdep = xzalloc (sizeof (struct gdbarch_tdep));
  gdbarch = gdbarch_alloc (&info, tdep);

  pc_name = k1c_pc_name (gdbarch);
  sp_name = k1c_sp_name (gdbarch);
  mach = gdbarch_bfd_arch_info (gdbarch)->mach;

  /* This could (should?) be extracted from MDS */
  set_gdbarch_short_bit (gdbarch, 16);
  set_gdbarch_int_bit (gdbarch, 32);
  set_gdbarch_long_bit (gdbarch, 32);
  set_gdbarch_long_long_bit (gdbarch, 64);
  set_gdbarch_float_bit (gdbarch, 32);
  set_gdbarch_double_bit (gdbarch, 64);
  set_gdbarch_long_double_bit (gdbarch, 64);
  set_gdbarch_ptr_bit (gdbarch, 32);

  /* Get the k1 target description from INFO.  */
  tdesc = info.target_desc;
  if (tdesc_has_registers (tdesc))
  {
    set_gdbarch_num_regs (gdbarch, 0);
    tdesc_data = tdesc_data_alloc ();
    tdesc_use_registers (gdbarch, tdesc, tdesc_data);

    for (i = 0; i < gdbarch_num_regs (gdbarch); ++i)
    {
      if (strcmp (tdesc_register_name (gdbarch, i), pc_name) == 0)
        has_pc = i;
      else if (strcmp (tdesc_register_name (gdbarch, i), sp_name) == 0)
        has_sp = i;
      else if (strcmp (tdesc_register_name (gdbarch, i), k1_le_name) == 0)
        has_le = i;
      else if (strcmp (tdesc_register_name (gdbarch, i), k1_ls_name) == 0)
        has_ls = i;
      else if (strcmp (tdesc_register_name (gdbarch, i), k1_ps_name) == 0)
        has_ps = i;
      else if (strcmp (tdesc_register_name (gdbarch, i), k1_lc_name) == 0)
        has_lc = i;
      else if (strcmp (tdesc_register_name (gdbarch, i), k1_local_name) == 0)
        has_local = i;
      else if (strcmp (tdesc_register_name (gdbarch, i), k1_ra_name) == 0)
        has_ra = i;
      else if (strcmp (tdesc_register_name (gdbarch, i), k1_spc_name) == 0)
        has_spc = i;
      else if (strcmp (tdesc_register_name (gdbarch, i), k1_ev_name) == 0)
        has_ev = i;
    }

    if (has_pc < 0)
      error ("There's no '%s' register!", pc_name);
    if (has_sp < 0)
      error ("There's no '%s' register!", sp_name);
    if (has_le < 0)
      error ("There's no '%s' register!", k1_le_name);
    if (has_ls < 0)
      error ("There's no '%s' register!", k1_ls_name);
    if (has_lc < 0)
      error ("There's no '%s' register!", k1_lc_name);
    if (has_ps < 0)
      error ("There's no '%s' register!", k1_ps_name);
    if (has_local < 0)
      error ("There's no '%s' register!", k1_local_name);
    if (has_ra < 0)
      error ("There's no '%s' register!", k1_ra_name);
    if (has_spc < 0)
      error ("There's no '%s' register!", k1_spc_name);
    if (has_ev < 0)
      error ("There's no '%s' register!", k1_ev_name);

    tdep->ev_regnum = has_ev;
    tdep->le_regnum = has_le;
    tdep->ls_regnum = has_ls;
    tdep->lc_regnum = has_lc;
    tdep->ps_regnum = has_ps;
    tdep->ra_regnum = has_ra;
    tdep->spc_regnum = has_spc;
    tdep->local_regnum = has_local;
    set_gdbarch_pc_regnum (gdbarch, has_pc);
    set_gdbarch_sp_regnum (gdbarch, has_sp);
  }
  else
  {
    set_gdbarch_num_regs (gdbarch, 1);
    set_gdbarch_register_name (gdbarch, k1_dummy_register_name);
    set_gdbarch_register_type (gdbarch, k1_dummy_register_type);
  }

  set_gdbarch_register_reggroup_p (gdbarch, k1_register_reggroup_p);

  set_gdbarch_num_pseudo_regs (gdbarch, k1c_num_pseudos (gdbarch));

  set_tdesc_pseudo_register_name (gdbarch, k1c_pseudo_register_name);
  set_tdesc_pseudo_register_type (gdbarch, k1c_pseudo_register_type);
  set_tdesc_pseudo_register_reggroup_p (gdbarch,
    k1c_pseudo_register_reggroup_p);

  set_gdbarch_pseudo_register_read (gdbarch, k1c_pseudo_register_read);
  set_gdbarch_pseudo_register_write (gdbarch, k1c_pseudo_register_write);
  set_gdbarch_dwarf2_reg_to_regnum (gdbarch, k1c_dwarf2_reg_to_regnum);
  dwarf2_frame_set_init_reg (gdbarch, k1_dwarf2_frame_init_reg);

  set_gdbarch_return_value (gdbarch, k1_return_value);
  set_gdbarch_push_dummy_call (gdbarch, k1_push_dummy_call);
  set_gdbarch_dummy_id (gdbarch, k1_dummy_id);

  set_gdbarch_skip_prologue (gdbarch, k1_skip_prologue);
  set_gdbarch_unwind_pc (gdbarch, k1_unwind_pc);
  dwarf2_append_unwinders (gdbarch);
  frame_unwind_append_unwinder (gdbarch, &k1_frame_unwind);

  set_gdbarch_fetch_tls_load_module_address (gdbarch,
    k1_fetch_tls_load_module_address);

  set_gdbarch_breakpoint_from_pc (gdbarch, k1_breakpoint_from_pc);
  set_gdbarch_adjust_breakpoint_address (gdbarch,
    k1_adjust_breakpoint_address);
  /* Settings that should be unnecessary.  */
  set_gdbarch_inner_than (gdbarch, core_addr_lessthan);
  set_gdbarch_print_insn (gdbarch, k1_print_insn);

  /* Displaced stepping */
  set_gdbarch_displaced_step_copy_insn (gdbarch, k1_displaced_step_copy_insn);
  set_gdbarch_displaced_step_fixup (gdbarch, k1_displaced_step_fixup);
  set_gdbarch_displaced_step_free_closure (gdbarch, k1_displaced_step_free_closure);
  set_gdbarch_displaced_step_location (gdbarch, k1_displaced_step_location);
  set_gdbarch_max_insn_length (gdbarch, 8 * 4);

  set_gdbarch_get_longjmp_target (gdbarch, k1_get_longjmp_target);

  return gdbarch;
}

static void
k1_cleanup_inferior_data (struct inferior *inf, void *data)
{
  xfree (data);
}

extern initialize_file_ftype _initialize_k1_tdep;

void
_initialize_k1_tdep (void)
{
  k1_look_for_insns ();
  gdbarch_register (bfd_arch_k1, k1_gdbarch_init, NULL);

  observer_attach_inferior_created (k1_inferior_created);

  k1_inferior_data_token = register_inferior_data_with_cleanup (NULL, k1_cleanup_inferior_data);
}

int
get_thread_mode_used_for_ptid (ptid_t ptid)
{
  int ret;
  char *buf = (char *) malloc (256);
  long size = 256;
  sprintf (buf, "kmp%x.%x", (unsigned int) ptid.pid, (unsigned int) ptid.lwp);
  putpkt (buf);
  getpkt (&buf, &size, 0);
  ret = (int) (buf[0] - '0');
  free (buf);

  return ret;
}

int
get_cpu_exec_level (void)
{
  int ret;
  char *buf = (char *) malloc (256);
  long size = 256;
  strcpy (buf, "kc");
  putpkt (buf);
  getpkt (&buf, &size, 0);
  ret = (int) (buf[0] - '0');
  free (buf);

  return ret;
}

int
kalray_hide_thread (struct thread_info *tp, ptid_t crt_ptid)
{
  int debug_level;
  int th_mode_used;

  if (!opt_hide_threads) //don't hide if option hide_threads is off
  {
    in_info_thread = 1;
    return 0;
  }

  debug_level = get_debug_level (tp->ptid.pid);
  th_mode_used = get_thread_mode_used_for_ptid (tp->ptid);

  // don't hide a stopped thread and the current thread
  if (tp->state != THREAD_STOPPED && !ptid_equal (tp->ptid, crt_ptid))
  {
    // hide the thread if it was never seen in a exec level >= debug level
    // in debug_level system, don't hide anything
    if (debug_level && th_mode_used < (1 << debug_level))
      return 1;
  }

  in_info_thread = 1;
  return 0;
}
