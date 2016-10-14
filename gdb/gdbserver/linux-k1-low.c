/* GNU/Linux/K1 specific low level interface, for the remote server for GDB.
   Copyright 2007-2012 Free Software Foundation, Inc.
   Copyright 2013-14 Kalray

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


#include "server.h"
#include "linux-low.h"
#include "regdef.h"
#include "tdesc.h"

#include <sys/ptrace.h>

static const char *xmltarget_k1_linux = 0;

void init_registers_k1 (void);
extern const struct target_desc *tdesc_k1;

enum regnum {R0 = 0, R_PC = 64, R_PS, R_CS, R_RA, R_LC, R_LE, R_LS};

static void
k1_fill_gregset (struct regcache *regcache, void *buf)
{
  elf_greg_t* rset = (elf_greg_t *) buf;
  const struct target_desc *tdesc = regcache->tdesc;
  int r0_regnum;
  char *ptr;
  int i;

  // R0 - R63
  r0_regnum = find_regno (tdesc, "r0");
  ptr = (char *) &rset[R0];

  for (i = r0_regnum; i < r0_regnum + 64; i++)
  {
    collect_register (regcache, i, ptr);
    ptr += register_size (tdesc, i);
  }

  collect_register_by_name (regcache, "pc", (char*) &rset[R_PC]);
  collect_register_by_name (regcache, "ps", (char*) &rset[R_PS]);
  collect_register_by_name (regcache, "cs", (char*) &rset[R_CS]);
  collect_register_by_name (regcache, "ra", (char*) &rset[R_RA]);
  collect_register_by_name (regcache, "lc", (char*) &rset[R_LC]);
  collect_register_by_name (regcache, "le", (char*) &rset[R_LE]);
  collect_register_by_name (regcache, "ls", (char*) &rset[R_LS]);
}

static void
k1_store_gregset (struct regcache *regcache, const void *buf)
{
  const elf_greg_t* rset = (const elf_greg_t*)buf;
  const struct target_desc *tdesc = regcache->tdesc;
  int r0_regnum;
  char *ptr;
  int i;

  // R0 - R63
  r0_regnum = find_regno (tdesc, "r0");
  ptr = (char *) &rset[R0];

  for (i = r0_regnum; i < r0_regnum + 64; i++)
  {
    supply_register (regcache, i, ptr);
    ptr += register_size (tdesc, i);
  }

  supply_register_by_name (regcache, "pc", (char*) &rset[R_PC]);
  supply_register_by_name (regcache, "ps", (char*) &rset[R_PS]);
  supply_register_by_name (regcache, "cs", (char*) &rset[R_CS]);
  supply_register_by_name (regcache, "ra", (char*) &rset[R_RA]);
  supply_register_by_name (regcache, "lc", (char*) &rset[R_LC]);
  supply_register_by_name (regcache, "le", (char*) &rset[R_LE]);
  supply_register_by_name (regcache, "ls", (char*) &rset[R_LS]);
}

struct regset_info k1_regsets[] = {
  { PTRACE_GETREGS, PTRACE_SETREGS, 0, sizeof (elf_gregset_t),
    GENERAL_REGS,
    k1_fill_gregset, k1_store_gregset },
  { 0, 0, 0, -1, -1, NULL, NULL }
};

#define K1_BREAKPOINT {0xF9,0x00, 0x10, 0x02}

static const gdb_byte k1_breakpoint[] = K1_BREAKPOINT;
#define k1_breakpoint_len 4

static CORE_ADDR
k1_get_pc (struct regcache *regcache)
{
  unsigned long pc;

  collect_register_by_name (regcache, "pc", &pc);
  return pc;
}

static void
k1_set_pc (struct regcache *regcache, CORE_ADDR pc)
{
  unsigned long newpc = pc;
  supply_register_by_name (regcache, "pc", &newpc);
}

static int
k1_breakpoint_at (CORE_ADDR where)
{
  unsigned long insn;

  (*the_target->read_memory) (where, (unsigned char *) &insn, k1_breakpoint_len);
  return memcmp((char *) &insn, k1_breakpoint, k1_breakpoint_len) == 0;
}

static struct regsets_info k1_regsets_info =
{
  k1_regsets, /* regsets */
  0, /* num_regsets */
  NULL, /* disabled_regsets */
};

static struct regs_info vregs_info =
{
  NULL, /* regset_bitmap */
  NULL, /* usrregs_info */
  &k1_regsets_info
};

static void
k1_arch_setup (void)
{
  current_process ()->tdesc = tdesc_k1;
}

static const struct regs_info *
k1_regs_info (void)
{
  return &vregs_info;
}

int k1_supports_z_point_type (char z_type)
{
  switch (z_type)
  {
  case Z_PACKET_SW_BP:
    return 1;
  }

  return 0;
}

int k1_insert_point (enum raw_bkpt_type type, CORE_ADDR addr, int size, struct raw_breakpoint *bp)
{
  if (type == raw_bkpt_type_sw)
  {
    return insert_memory_breakpoint (bp);
  }
  
  return 1;
}

int k1_remove_point (enum raw_bkpt_type type, CORE_ADDR addr, int size, struct raw_breakpoint *bp)
{
  if (type == raw_bkpt_type_sw)
  {
    return remove_memory_breakpoint (bp);
  }
  
  return 1;
}

static const gdb_byte * k1_sw_breakpoint_from_kind (int kind, int *size)
{
  *size = k1_breakpoint_len;
  return k1_breakpoint;
}

struct linux_target_ops the_low_target = {
  k1_arch_setup, /* void (*arch_setup) (void) */
  k1_regs_info, /* const struct regs_info *(*regs_info) (void) */
  NULL, /* int (*cannot_fetch_register) (int) */
  NULL, /* int (*cannot_store_register) (int) */
  NULL, /* int (*fetch_register) (struct regcache *regcache, int regno) */
  k1_get_pc, /* CORE_ADDR (*get_pc) (struct regcache *regcache) */
  k1_set_pc, /* void (*set_pc) (struct regcache *regcache, CORE_ADDR newpc) */
  NULL, /* int (*breakpoint_kind_from_pc) (CORE_ADDR *pcptr) */
  k1_sw_breakpoint_from_kind, /* const gdb_byte *(*sw_breakpoint_from_kind) (int kind, int *size) */
  NULL, /* VEC (CORE_ADDR) *(*get_next_pcs) (struct regcache *regcache) */
  0, /* int decr_pc_after_break */
  k1_breakpoint_at, /* int (*breakpoint_at) (CORE_ADDR pc) */
  k1_supports_z_point_type, /* int (*supports_z_point_type) (char z_type) */
  k1_insert_point, /* int (*insert_point) (enum raw_bkpt_type type, 
                    *   CORE_ADDR addr, int size, struct raw_breakpoint *bp) */
  k1_remove_point, /* int (*remove_point) (enum raw_bkpt_type type,
                    *   CORE_ADDR addr, int size, struct raw_breakpoint *bp) */
};

static char *create_xml (void)
{
  char *ret, *buf, reg_type[50];
  struct reg *r;
  int i, nr, pos;

  nr = tdesc_k1->num_registers;
  buf   = malloc (nr * 150 + 1000);
  strcpy (buf, "@<target><architecture>k1bio_usr</architecture>"
    "<feature name=\"eu.kalray.core.k1b\">");
  pos = strlen (buf);
  
  for (i = 0; i < nr; i++)
  {
    r = &tdesc_k1->reg_defs[i];
    if (!strcmp (r->name, "cs"))
    {
      pos += sprintf (buf + pos, "<struct id=\"cs_type\" size=\"4\">"
        "<field name=\"ic\" start=\"0\" end=\"0\" />"
        "<field name=\"io\" start=\"1\" end=\"1\" />"
        "<field name=\"dz\" start=\"2\" end=\"2\" />"
        "<field name=\"ov\" start=\"3\" end=\"3\" />"
        "<field name=\"un\" start=\"4\" end=\"4\" />"
        "<field name=\"in\" start=\"5\" end=\"5\" />"
        "<field name=\"rm\" start=\"8\" end=\"9\" />"
        "<field name=\"wu\" start=\"15\" end=\"15\" />"
        "<field name=\"cc\" start=\"16\" end=\"31\" /></struct>");
      strcpy (reg_type, "cs_type");
    }
    else if (!strcmp (r->name, "pc") || !strcmp (r->name, "ra"))
      strcpy (reg_type, "code_ptr");
    else if (!strcmp (r->name, "ps"))
    {
      pos += sprintf (buf + pos, "<struct id=\"ps_type\" size=\"4\">"
        "<field name=\"pm\" start=\"0\" end=\"0\" /></struct>");
      strcpy (reg_type, "ps_type");
    }
    else
      sprintf (reg_type, "int%d", r->size);

    pos += sprintf (buf + pos,
      "<reg name=\"%s\" regnum=\"%d\" bitsize=\"%d\" type=\"%s\"/>",
      r->name, i, r->size, reg_type);
  } 

  strcpy (buf + pos, "</feature></target>");

  ret = strdup (buf);
  free (buf);
  
  return ret;
}


void
initialize_low_arch (void)
{
  /* Initialize the Linux target descriptions.  */
  init_registers_k1 ();

  if (!tdesc_k1->xmltarget)
  {
    xmltarget_k1_linux = create_xml ();
    ((struct target_desc *) tdesc_k1)->xmltarget = xmltarget_k1_linux;
  }

  initialize_regsets_info (&k1_regsets_info);
}
