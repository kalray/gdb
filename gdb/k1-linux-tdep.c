/* Target-dependent code for the Kalray K1 for GDB, the GNU debugger.

   Copyright (C) 2016 Kalray

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

#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "k1-common-tdep.h"
#include "osabi.h"
#include "linux-tdep.h"
#include "solib-svr4.h"
#include "symtab.h"
#include "gdbthread.h"
#include "top.h"
#include "gdbarch.h"
#include "target-descriptions.h"
#include "frame-unwind.h"
#include "arch-utils.h"
#include "observer.h"

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

  pc_name = k1b_pc_name (gdbarch);
  sp_name = k1b_sp_name (gdbarch);
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

  set_gdbarch_num_pseudo_regs (gdbarch, k1b_num_pseudos (gdbarch));

  set_tdesc_pseudo_register_name (gdbarch, k1b_pseudo_register_name);
  set_tdesc_pseudo_register_type (gdbarch, k1b_pseudo_register_type);
  set_tdesc_pseudo_register_reggroup_p (gdbarch,
    k1b_pseudo_register_reggroup_p);

  set_gdbarch_pseudo_register_read (gdbarch, k1b_pseudo_register_read);
  set_gdbarch_pseudo_register_write (gdbarch, k1b_pseudo_register_write);
  set_gdbarch_dwarf2_reg_to_regnum (gdbarch, k1b_dwarf2_reg_to_regnum);
  dwarf2_frame_set_init_reg (gdbarch, k1_dwarf2_frame_init_reg);

  set_gdbarch_return_value (gdbarch, k1_return_value);
  set_gdbarch_push_dummy_call (gdbarch, k1_push_dummy_call);
  set_gdbarch_dummy_id (gdbarch, k1_dummy_id);

  set_gdbarch_skip_prologue (gdbarch, k1_skip_prologue);
  set_gdbarch_unwind_pc (gdbarch, k1_unwind_pc);
  dwarf2_append_unwinders (gdbarch);
  frame_unwind_append_unwinder (gdbarch, &k1_frame_unwind);

  set_gdbarch_breakpoint_from_pc (gdbarch, k1_breakpoint_from_pc);
  set_gdbarch_adjust_breakpoint_address (gdbarch,
    k1_adjust_breakpoint_address);
  /* Settings that should be unnecessary.  */
  set_gdbarch_inner_than (gdbarch, core_addr_lessthan);
  set_gdbarch_print_insn (gdbarch, k1_print_insn);

  set_gdbarch_max_insn_length (gdbarch, 8 * 4);

  set_gdbarch_get_longjmp_target (gdbarch, k1_get_longjmp_target);

  if (tdesc_has_registers (tdesc))
  {
    set_solib_svr4_fetch_link_map_offsets (gdbarch, svr4_ilp32_fetch_link_map_offsets);
    /* Hook in the ABI-specific overrides, if they have been registered.  */
    gdbarch_init_osabi (info, gdbarch);
  }

  return gdbarch;
}

static void
attach_user_command (char *args, int from_tty)
{
  static const char *syntax = "Syntax: attach-user <comm> [<path_in_initrd_k1_linux_program>]\n";
  char *file, *comm, *pargs;
  struct stat vstat;

  if (!ptid_equal (inferior_ptid, null_ptid))
  {
    fprintf (stderr, "Gdb already attached!\n");
    return;
  }

  pargs = args;
  comm = extract_arg (&pargs);
  if (!comm)
  {
    fprintf (stderr, "Error: the comm was not specified!\n%s", syntax);
    return;
  }
  file = extract_arg (&pargs);

  execute_command ("set pagination off", 0);

  if (file)
  {
    int file_len = strlen (file);
    char cmd[file_len + 50], file_dir[file_len + 1], file_base[file_len + 1];
    if (stat (file, &vstat))
    {
      fprintf (stderr, "Error: Cannot find file %s!\n", file);
      return;
    }

    // set file
    sprintf (cmd, "file %s", file);
    execute_command (cmd, 0);

    // set sysroot
    strcpy (file_dir, file);
    dirname (file_dir);
    while (*file_dir && ((file_dir[0] != '.' && file_dir[0] != '/') || file_dir[1] != 0))
    {
      strcpy (file_base, file_dir);
      if (strcmp (basename (file_base), "target") == 0)
      {
        sprintf (cmd, "%s/THIS_IS_NOT_YOUR_ROOT_FILESYSTEM", file_dir);
        if (stat (file, &vstat) == 0)
        {
          // found the root of k1 filesystem
          sprintf (cmd, "set sysroot %s", file_dir);
          fprintf (stderr, "Set sysroot to %s\n", file_dir);
          execute_command (cmd, 0);
          break;
        }
      }
      dirname (file_dir);
    }
  }

  TRY
  {
    char cmd[strlen (comm) + 20];
    sprintf (cmd, "target remote %s", comm);
    execute_command (cmd, 0);
  }
  CATCH (ex, RETURN_MASK_ALL)
  {
    printf ("Error while trying to connect (%s).\n", ex.message ?: "");
    return;
  }
  END_CATCH

  fprintf (stderr, "Attached to K1 linux user debug using %s.\n", comm);
}

static void
k1_inferior_created (struct target_ops *target, int from_tty)
{
  k1_current_arch = K1_NUM_ARCHES;
}

/* OS specific initialization of gdbarch.  */
static void
k1_linux_init_abi (struct gdbarch_info info, struct gdbarch *gdbarch)
{
  linux_init_abi (info, gdbarch);

  set_solib_svr4_fetch_link_map_offsets (gdbarch, svr4_ilp32_fetch_link_map_offsets);

  /* Enable TLS support.  */
  set_gdbarch_fetch_tls_load_module_address (gdbarch, svr4_fetch_objfile_link_map);
}

extern initialize_file_ftype _initialize_k1_linux_tdep;
void
_initialize_k1_linux_tdep (void)
{
  gdbarch_register_osabi (bfd_arch_k1, bfd_mach_k1bio_usr, GDB_OSABI_LINUX, k1_linux_init_abi);

  add_com ("attach-user", class_run, attach_user_command,
    _("Connect to gdbserver running on MPPA.\n"
    "Usage is `attach-user <comm> [<path_in_initrd_k1_linux_program>]'."));
}

extern initialize_file_ftype _initialize_k1_tdep;
void
_initialize_k1_tdep (void)
{
  k1_look_for_insns ();
  gdbarch_register (bfd_arch_k1, k1_gdbarch_init, NULL);
  observer_attach_inferior_created (k1_inferior_created);
}
