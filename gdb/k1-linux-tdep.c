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
#include <limits.h>
#include <stdlib.h>

#include "k1-common-tdep.h"
#include "k1-linux-mux.h"
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
#include "cli/cli-cmds.h"
#include "remote.h"
#include "cli/cli-decode.h"

#define NCLUSTERS 20
struct cluster_list
{
  int32_t cluster_id;
  int32_t pid_spawner;
  int32_t pid_gdbserver;
  int32_t own;
};

static struct cmd_list_element *kalray_set_cmdlist = NULL;
static struct cmd_list_element *kalray_show_cmdlist = NULL;
static struct cmd_list_element *spawned_clusters_cmdlist = NULL, *spawned_clusters_cmdlist_1 = NULL;
static int opt_debug_spawned_clusters = 0, debug_spawned_clusters = 0;
static int use_comm_mux = 0;
char *sysroot_path = NULL;

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
  char *file, *comm, *k1_comm, *pargs, cmd[PATH_MAX + 50], file_dir[PATH_MAX];
  struct stat vstat;
  int ret;

  if (!ptid_equal (inferior_ptid, null_ptid))
  {
    fprintf (stderr, "Gdb already attached!\n");
    return;
  }

  pargs = args;
  k1_comm = comm = extract_arg (&pargs);
  if (!comm)
  {
    fprintf (stderr, "Error: the comm was not specified!\n%s", syntax);
    return;
  }
  file = extract_arg (&pargs);
  if (!file)
  {
    fprintf (stderr, "Error: the K1 Linux program was not specified!\n%s", syntax);
    return;
  }
  if (stat (file, &vstat) || S_ISDIR (vstat.st_mode) || realpath (file, file_dir) == NULL)
  {
    fprintf (stderr, "Error: Cannot find file %s!\n", file);
    return;
  }

  execute_command ("set pagination off", 0);

  // set file
  sprintf (cmd, "file %s", file);
  execute_command (cmd, 0);

  // set sysroot
  if (sysroot_path)
  {
    free (sysroot_path);
    sysroot_path = NULL;
  }
  strcpy (file_dir, dirname (file_dir));
  while (*file_dir && (file_dir[0] != '/' || file_dir[1] != 0))
  {
    sprintf (cmd, "%s/etc/inittab", file_dir);
    if (stat (cmd, &vstat) == 0)
    {
      // found the root of k1 filesystem
      sysroot_path = strdup (file_dir);
      sprintf (cmd, "set sysroot %s", file_dir);
      fprintf (stderr, "Set sysroot to %s\n", file_dir);
      execute_command (cmd, 0);
      break;
    }

    strcpy (file_dir, dirname (file_dir));
  }

  if (!getenv ("NO_MUX"))
  {
    int port;
    use_comm_mux = 1;
    if ((port = create_con_mux (comm)) < 0)
      return;
    comm = cmd + 100;
    sprintf (comm, "127.0.0.1:%d", port);
  }

  ret = 0;
  TRY
  {
    sprintf (cmd, "target remote %s", comm);
    execute_command (cmd, 0);
  }
  CATCH (ex, RETURN_MASK_ALL)
  {
    printf ("Error while trying to connect (%s).\n", ex.message ?: "");
    ret = -1;
  }
  END_CATCH

  if (ret)
    return;

  fprintf (stderr, "Attached to K1 linux user debug using %s.\n", k1_comm);
}

static int
spawned_clusters_get_list (int *pid_gdbserver, struct cluster_list *lst)
{
  char *buf, *parg;
  int i;
  long size = 500;

  if (ptid_equal (inferior_ptid, null_ptid))
  {
    fprintf  (stderr, "Cannot list spawned clusters without a selected thread\n");
    return -1;
  }

  buf = (char *) malloc (size);

  sprintf (buf, "Qk1.spawned_list:");
  putpkt (buf);
  getpkt (&buf, &size, 0);
  if (strncmp (buf, "OK", 2))
  {
    fprintf (stderr, "Error returned by K1 for list spawned clusters: %s\n", buf);
    free (buf);
    return -1;
  }
  buf[size] = 0;

  *pid_gdbserver = strtol (buf + 2, &parg, 10);
  for (i = 0; parg && *parg == ':'; i++, parg = strchr (parg, ':'))
  {
    *parg++ = 0;
    sscanf (parg, "%d %d %d %d", &lst[i].cluster_id, &lst[i].pid_spawner, &lst[i].pid_gdbserver, &lst[i].own);
  }

  free (buf);

  return i;
}

static int
spawned_clusters_start_debug_cluster (int cluster_id)
{
  char *buf;
  long size = 200;

  buf = (char *) malloc (size);
  sprintf (buf, "Qk1.start_debug:%d", cluster_id);
  putpkt (buf);
  getpkt (&buf, &size, 0);
  if (strncmp (buf, "OK", 2))
  {
    fprintf (stderr, "Error returned by K1 for start debug spawned cluster %d: %s\n", cluster_id, buf);
    free (buf);
    return -1;
  }
  free (buf);

  return 0;
}

static void
spawned_clusters_list_command (char *args, int from_tty)
{
  char *parg, *arg;
  int i, n, all, pid_gdbserver;
  struct cluster_list lst[NCLUSTERS];

  parg = args;
  arg = extract_arg (&parg);
  if (!arg)
    arg = "own";
  else if (strcmp (arg, "own") && strcmp (arg, "all"))
    error ("Invalid option %s for list spawned clusters", arg);
  all = strcmp (arg, "all") == 0;

  if (extract_arg (&parg))
    error ("List spawned clusters accepts a single argument");

  n = spawned_clusters_get_list (&pid_gdbserver, lst);
  if (n < 0)
    return;
  if (n == 0)
  {
    fprintf (stdout, "No spawned clusters\n");
    return;
  }

  fprintf (stdout, "List %s spawned cluster (own pid_gdbserver = %d):\n", arg, pid_gdbserver);
  fprintf (stdout, "cluster_id | pid_spawner | pid_gdbserver | own\n");
  fprintf (stdout, "-----------|-------------|---------------|----\n");

  for (i = 0; i < n; i++)
  {
    if (!all && !lst[i].own)
      continue;
    fprintf (stdout, "%10d | %11d | ", lst[i].cluster_id, lst[i].pid_spawner);
    if (lst[i].pid_gdbserver == -1)
      fprintf (stdout, "%13s", "none");
    else
      fprintf (stdout, "%13d", lst[i].pid_gdbserver);
    fprintf (stdout, " | %s\n", lst[i].own ? "yes" : "no");
  }
}

static void
spawned_clusters_debug_command (char *args, int from_tty)
{
  char *parg, *arg, *endp;
  int i, n, all, own, pid_gdbserver, arg_to_debug[NCLUSTERS], narg_to_debug;
  struct cluster_list lst[NCLUSTERS];

  parg = args;
  arg = extract_arg (&parg);
  if (!arg)
    error ("The spawned cluster(s) to be debugged is not specified");

  all = own = 0;
  narg_to_debug = 0;
  do
  {
    if (!strcmp (arg, "all") || !strcmp (arg, "own"))
    {
      if (all || own)
        error ("More than one all/own options specified");
      if (!strcmp (arg, "all"))
        all = 1;
      else
        own = 1;
      continue;
    }
    n = strtol (arg, &endp, 10);
    if (*endp || !((n >= 0 && n <= 15) || n == 128 || n == 192))
      error ("Invalid cluster id %s to debug", arg);
    arg_to_debug[narg_to_debug++] = n;
  } while ((arg = extract_arg (&parg)) != NULL);

  n = spawned_clusters_get_list (&pid_gdbserver, lst);
  for (i = 0; i < n; i++)
  {
    int j, debug = 0, cluster_id = lst[i].cluster_id;
    if (all || (own && lst[i].own))
      debug = 1;

    for (j = 0; j < narg_to_debug; j++)
    {
      if (arg_to_debug[j] == cluster_id)
      {
        debug = 1;
        break;
      }
    }

    if (!debug)
      continue;

    if (j < narg_to_debug)
      arg_to_debug[j] = -1;

    if (lst[i].pid_gdbserver != -1)
    {
      fprintf (stdout, "Cluster %d is already debugged\n", cluster_id);
      continue;
    }

    fprintf (stdout, "Start debugging cluster %d ...\n", cluster_id);
    if (!spawned_clusters_start_debug_cluster (cluster_id))
      fprintf (stdout, "  OK\n");
  }

  for (i = 0; i < narg_to_debug; i++)
    if (arg_to_debug[i] != -1)
      fprintf (stdout, "Cannot start debug cluster %d because it is not spawned yet\n", arg_to_debug[i]);
}

static void
k1_inferior_created (struct target_ops *target, int from_tty)
{
  k1_current_arch = K1_NUM_ARCHES;
  debug_spawned_clusters = 0;
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

static void
set_kalray_cmd (char *args, int from_tty)
{
  help_list (kalray_set_cmdlist, "set kalray ", -1, gdb_stdout);
}

static void
show_kalray_cmd (char *args, int from_tty)
{
  help_list (kalray_show_cmdlist, "show kalray ", -1, gdb_stdout);
}

static void
spawned_clusters_command (char *args, int from_tty)
{
  help_list (spawned_clusters_cmdlist, "spawned-clusters ", -1, gdb_stdout);
}

static void
set_debug_spawned_clusters (char *args, int from_tty, struct cmd_list_element *c)
{
  char *buf;
  long size = 256;

  if (ptid_equal (inferior_ptid, null_ptid))
    error ("Cannot set the debug spawned clusters option without a selected thread.\n");

  buf = (char *) malloc (size);

  sprintf (buf, "Qk1.spawn_debug:%d", opt_debug_spawned_clusters);
  putpkt (buf);
  getpkt (&buf, &size, 0);
  if (!strcmp (buf, "OK"))
    debug_spawned_clusters = opt_debug_spawned_clusters;
  else
  {
    printf ("Error: cannot debug spawned clusters: %s\n", buf);
  }

  free (buf);
}

static void
show_debug_spawned_clusters (struct ui_file *file, int from_tty,
  struct cmd_list_element *c, const char *value)
{
  if (ptid_equal (inferior_ptid, null_ptid))
  {
    printf ("Cannot show the debug spawned clusters option without a live selected thread.\n");
    return;
  }

  fprintf_filtered (file, "The debug spawned clusters option is %d.\n", debug_spawned_clusters);
}

static void
add_k1_commands (void)
{
  static const char *enum_spawned_clusters_list[] = {"own", "all", NULL};
  static const char *enum_spawned_clusters_debug[] = {"own", "all", "<cluster_id>", NULL};

  add_com ("attach-user", class_run, attach_user_command,
    "Connect to gdbserver running on MPPA.\n"
    "Usage is 'attach-user <comm> [<path_in_initrd_k1_linux_program>]'.");

  if (!getenv ("NO_MUX"))
  {
    struct cmd_list_element *c;

    add_prefix_cmd ("kalray", class_maintenance, set_kalray_cmd,
      "Configure various Kalray specific variables.",
      &kalray_set_cmdlist, "set kalray ", 0 /* allow-unknown */, &setlist);

    add_prefix_cmd ("kalray", class_maintenance, show_kalray_cmd,
      "Configure various Kalray specific variables.",
      &kalray_show_cmdlist, "show kalray ", 0 /* allow-unknown */, &showlist);

    add_setshow_boolean_cmd ("debug_spawned_clusters", class_maintenance, &opt_debug_spawned_clusters,
      "Set debug the spawned clusters.", "Show debug the spawned clusters.",
      NULL, &set_debug_spawned_clusters, &show_debug_spawned_clusters, &kalray_set_cmdlist, &kalray_show_cmdlist);

    // commands to list/start debug the clusters already spawned
    add_prefix_cmd ("spawned-clusters", class_run, spawned_clusters_command,
      "List/debug spawned MPPA clusters.",
      &spawned_clusters_cmdlist, "spawned-clusters ", 0 /* allow-unknown */, &cmdlist);

    c = add_cmd ("list", class_run, spawned_clusters_list_command,
      "List spawned MPPA clusters.\n"
      "Usage is 'spawned-clusters list [own|all]'.", &spawned_clusters_cmdlist);
    if (c)
      c->enums = enum_spawned_clusters_list;

    c = add_cmd ("debug", class_run, spawned_clusters_debug_command,
      "Start debug spawned MPPA cluster(s).\n"
      "Usage is 'spawned-clusters debug [own|all|<cluster_id>]'.", &spawned_clusters_cmdlist);
    if (c)
      c->enums = enum_spawned_clusters_debug;
  }
}

extern initialize_file_ftype _initialize_k1_linux_tdep;
void
_initialize_k1_linux_tdep (void)
{
  gdbarch_register_osabi (bfd_arch_k1, bfd_mach_k1bio_usr, GDB_OSABI_LINUX, k1_linux_init_abi);
  add_k1_commands ();
}

extern initialize_file_ftype _initialize_k1_tdep;
void
_initialize_k1_tdep (void)
{
  k1_look_for_insns ();
  gdbarch_register (bfd_arch_k1, k1_gdbarch_init, NULL);
  observer_attach_inferior_created (k1_inferior_created);
}
