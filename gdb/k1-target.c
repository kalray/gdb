/* Target ops to connect to the K1 simulator.

   Copyright (C) 2010, Kalray
*/

#include "config.h"
#include "defs.h"
#include "target.h"

#include <assert.h>
#include <libgen.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/stat.h>

#include "environ.h"
#include "gdbcmd.h"
#include "gdbcore.h"
#include "gdbthread.h"
#include "inferior.h"
#include "infrun.h"
#include "observer.h"
#include "osdata.h"
#include "main.h"
#include "symfile.h"
#include "top.h"
#include "arch-utils.h"
#include "cli/cli-decode.h"
#include "cli/cli-setshow.h"
#include "cli/cli-utils.h"

#include "event-top.h"
#include "regcache.h"
#include "event-loop.h"
#include "location.h"

#include "regcache.h"
#include "elf/k1b.h"
#include "elf-bfd.h"

#include "elf/k1b.h"
#include "k1-target.h"

#ifndef MAX
# define MAX(a, b) ((a < b) ? (b) : (a))
#endif

extern int remote_hw_breakpoint_limit;
extern int remote_hw_watchpoint_limit;
extern int print_stopped_thread;

static cmd_cfunc_ftype *real_run_command;
static struct target_ops k1_target_ops;
static char *da_options = NULL;

static const char *simulation_vehicles[] = { "k1-cluster", NULL };
static const char *simulation_vehicle;
static const char *scluster_debug_levels[] = {"system", "kernel-user", "user", "inherited", NULL};
static const char *scluster_debug_level;
static const char *sglobal_debug_levels[] = {"system", "kernel-user", "user", NULL};
static const char *sglobal_debug_level;
int idx_global_debug_level, global_debug_level_set;
int opt_hide_threads = 1;
int opt_break_on_spawn = 0;
int in_info_thread = 0;

static pid_t server_pid;
int after_first_resume = 0;
int inf_created_change_th = 0;

static const struct inferior_data *k1_attached_inf_data;

const char *get_str_debug_level (int level)
{
  return scluster_debug_levels[level];
}

static struct inferior_data*
mppa_init_inferior_data (struct inferior *inf)
{
    struct inferior_data *data = xcalloc (1, sizeof (struct inferior_data));
    char *endptr;
    struct osdata *osdata;
    struct osdata_item *last;
    struct osdata_item *item;
    int ix_items;

    data->cluster_debug_level = DBG_LEVEL_INHERITED;
    data->os_supported_debug_level = -1;
    data->cluster_break_on_spawn = 0;
    set_inferior_data (inf, k1_attached_inf_data, data);

    /* Cluster name */
    data->cluster = "Cluster ?";

    osdata = get_osdata (NULL);

    for (ix_items = 0;
         VEC_iterate (osdata_item_s, osdata->items,
                      ix_items, item);
         ix_items++) {
        unsigned long pid = strtoul (get_osdata_column (item, "pid"), &endptr, 10);
        const char *cluster = get_osdata_column (item, "cluster");
        
        if (pid != inf->pid) continue;

        data->cluster = cluster;
    }

    return data;
}

struct inferior_data*
mppa_inferior_data (struct inferior *inf)
{
    struct inferior_data *data;

    data = inferior_data (inf, k1_attached_inf_data);

    if (!data)
        data = mppa_init_inferior_data (inf);

    return data;
}

static void
k1_push_arch_stratum (struct target_ops *ops, int from_tty)
{   
    if (find_target_beneath (&current_target) != &k1_target_ops) {
        push_target (&k1_target_ops);
    }
}

static void
k1_target_new_thread (struct thread_info *t)
{
    if (!ptid_equal(inferior_ptid, null_ptid))
      current_inferior ()->control.stop_soon = STOP_QUIETLY;

    k1_push_arch_stratum (NULL, 0);
}

static void k1_target_mourn_inferior (struct target_ops *target)
{
  struct target_ops *remote_target = find_target_beneath(target);

  gdb_assert (target == &k1_target_ops);
  remote_target->to_mourn_inferior (remote_target);
  /* Force disconnect even if we are in extended mode */
  unpush_target (remote_target);
  unpush_target (&k1_target_ops);

  if (server_pid)
  {
    kill (server_pid, 9);
    server_pid = 0;
  }
}

static void k1_target_create_inferior (struct target_ops *ops, 
				       char *exec_file, char *args,
				       char **env, int from_tty);

static int 
k1_region_ok_for_hw_watchpoint (struct target_ops *ops, CORE_ADDR addr, int len)
{
    return 1;
}

static void 
k1_target_open (const char *name, int from_tty)
{

}

static void k1_target_close (struct target_ops *ops)
{

}

static char *
mppa_pid_to_str (struct target_ops *ops, ptid_t ptid)
{
  struct thread_info *ti = find_thread_ptid (ptid);
  struct target_ops *remote_target = find_target_beneath (ops);

  if (ti)
  {
    const char *name = remote_target->to_thread_name (ops, ti);

    if (in_info_thread)
    {
      in_info_thread = 0;
      return (char *) "Thread";
    }

    return (char *) name;
  }

  return find_target_beneath (ops)->to_pid_to_str (find_target_beneath (ops), ptid);
}

static char *
mppa_threads_extra_info (struct target_ops *ops, struct thread_info *tp)
{
    return NULL;
}

static void k1_target_attach (struct target_ops *ops, char *args, int from_tty)
{
    const char tar_remote_str[] = "target extended-remote";
    struct observer *new_thread_observer;
    int print_thread_events_save = print_thread_events;
    char *host, *port, *tar_remote_cmd;

    if (!args)
        args = "";

    port = strchr (args, ':');
    if (port) {
        *port = 0;
        port++;
        host = args;
    } else {
        port = args;
        host = "";
    }

    tar_remote_cmd = alloca (strlen(host) + strlen(port) + strlen(tar_remote_str) + 4);
    parse_pid_to_attach (port);

    print_thread_events = 0;
    sprintf (tar_remote_cmd, "%s %s:%s", tar_remote_str, host, port);

    /* Load the real debug target by issuing 'target remote'. Of
       course things aren't that simple because it's not meant to be
       used that way. One issue is that connecting to the gdb_stub
       will emit MI stop notifications and will print the initial
       frame at the connection point. The observer
       makes infrun believe that we're in a series of steps and thus
       inhibits the emission of the new_thread observer notification
       that prints the initial MI *stopped message. */
    new_thread_observer = observer_attach_new_thread (k1_target_new_thread);
    /* tar remote */

    async_disable_stdin ();
    TRY
    {
        execute_command (tar_remote_cmd, 0);
    }
    CATCH (ex, RETURN_MASK_ALL)
    {
        observer_detach_new_thread (new_thread_observer);
        print_thread_events = print_thread_events_save;
        throw_exception (ex);
    }
    END_CATCH

    /* We need to tell the debugger to fake a synchronous
       command. This has already been done at the upper level when the
       main loop executes the "run" command, but the execute_command
       call we did just above reseted to async handling when it
       terminated. */ 
    async_disable_stdin ();
    k1_push_arch_stratum (NULL, 0);

    /* Remove hacks*/
    observer_detach_new_thread (new_thread_observer);
    current_inferior ()->control.stop_soon = NO_STOP_QUIETLY;
    print_thread_events = print_thread_events_save;
}

static void k1_target_create_inferior (struct target_ops *ops, 
				       char *exec_file, char *args,
				       char **env, int from_tty)
{
    char set_non_stop_cmd[] = "set non-stop";
    char set_pagination_off_cmd[] = "set pagination off";
    char **argv_args = gdb_buildargv (args);
    char **da_args = gdb_buildargv (da_options);
    char **stub_args;
    char **arg;
    int nb_args = 0, nb_da_args = 0;
    int pipefds[2];
    int no_march = 0;
    int no_mcluster = 0;
    int core;
    int argidx = 0;
    int saved_async_execution = !sync_execution;

    if (exec_file == NULL)
      error (_("No executable file specified.\nUse the \"file\" or \"exec-file\" command."));

    k1_push_arch_stratum (NULL, 0);
    execute_command (set_non_stop_cmd, 0);
    execute_command (set_pagination_off_cmd, 0);
    remote_hw_breakpoint_limit = 0;
    remote_hw_watchpoint_limit = 1;

    arg = argv_args;
    while (arg && *arg++) nb_args++;
    if (nb_args && !*argv_args[nb_args - 1])
      nb_args--;

    arg = da_args;
    while (arg && *arg++) nb_da_args++;

    stub_args = xmalloc ((nb_args + nb_da_args + 7) * sizeof (char *));
    stub_args[argidx++] = (char *) simulation_vehicle;

    core = (elf_elfheader(exec_bfd)->e_flags & ELF_K1_CORE_MASK);

    if (nb_da_args && strlen (da_options)) {
	arg = da_args;
	while (*arg) {
	    if (strncmp (*arg, "--mcluster=", 11) == 0)
		no_mcluster = 1;
	    if (strncmp (*arg, "--march=", 8) == 0)
		no_march = 1;


	    stub_args[argidx++] = *arg++;
	}
    }

    if(!no_march)
      switch(core) {
	case ELF_K1_CORE_B_DP:
	case ELF_K1_CORE_B_IO:
	    stub_args[argidx++] = "--march=bostan";
	    break;
	default:
	    error (_("The K1 binary is compiled for an unknown core."));
      }

    if (!no_mcluster)
	switch (core) {
	case ELF_K1_CORE_B_DP:
	    stub_args[argidx++] = "--mcluster=node";
	    break;

	case ELF_K1_CORE_B_IO:
	    stub_args[argidx++] = "--mcluster=ioddr";
	    break;

	default:
	    error (_("The K1 binary is compiled for an unknown core."));
	}

    stub_args[argidx++] = "--gdb";
    stub_args[argidx++] = "--";
    stub_args[argidx++] = exec_file;
    if (nb_args)
    {
      memcpy (stub_args + argidx,  argv_args, nb_args * sizeof (char *));
      argidx += nb_args;
    }
    stub_args[argidx++] = NULL;

    /* Check that we didn't overflow the allocation above. */
    gdb_assert (argidx <= nb_args + nb_da_args + 7);

    if (server_pid != 0) {
	kill (server_pid, 9);
	waitpid (server_pid, NULL, 0);
    }

    pipe (pipefds);
    server_pid = fork ();
    
    if (server_pid < 0)
	error ("Couldn't fork to launch the server.");

    if (server_pid == 0) {
	char path[PATH_MAX];
	char tmp[PATH_MAX] = { 0 };
	char *dir;

	close (pipefds[0]);
	dup2(pipefds[1], 500);
	close (pipefds[1]);

	setsid ();

	/* Child */
	if (env)
	    environ = env;
	execvp (simulation_vehicle, stub_args);
	
	/* Not in PATH */
	if (readlink ("/proc/self/exe", tmp, PATH_MAX) != -1) {
	    dir = dirname (tmp);
	    snprintf (path, PATH_MAX, "%s/%s", dir, simulation_vehicle);
	    execvp (path, stub_args);
	}

	printf_unfiltered ("Could not find %s in you PATH\n", simulation_vehicle);
	exit (1);
  }
  else
  {
    int port;
    char cmd_port[10];

    close (pipefds[1]);
    read (pipefds[0], &port, sizeof(port));
    close (pipefds[0]);

    sprintf (cmd_port, "%i", port);
    print_stopped_thread = 0;
    k1_target_attach (ops, cmd_port, from_tty);
    print_stopped_thread = 1;
    if (saved_async_execution)
      async_enable_stdin ();
  }

}

static int
mppa_mark_clusters_booted (struct inferior *inf, void *_ptid)
{
    struct thread_info *thread;
    ptid_t *ptid = _ptid;

    if (!inf->pid)
      return 0;

    thread = any_live_thread_of_process (inf->pid);

    /* Newer GDBs mark the thread as running before passing it to
       target_resume. However, if we are resuming the thread, it must
       have been stooped before... */
    if (thread && (is_stopped (thread->ptid)
	           || ptid_match (thread->ptid, *ptid))) {
        mppa_inferior_data (inf)->booted = 1;
    }
    return 0;
}

static void
mppa_target_resume (struct target_ops *ops,
                    ptid_t ptid, int step, enum gdb_signal siggnal)
{
    struct target_ops *remote_target = find_target_beneath(ops);

    if (!after_first_resume && !inf_created_change_th) {
        after_first_resume = 1;
        iterate_over_inferiors (mppa_mark_clusters_booted, &ptid);
    }
    
    return remote_target->to_resume (remote_target, ptid, step, siggnal);
}

static void
k1_change_file (const char *file_path)
{
  struct stat st;
  if (stat (file_path, &st))
  {
    printf ("Cannot stat K1 executable file %s\n", file_path);
    return;
  }

  if (st.st_mode & S_IFDIR)
  {
    printf ("%s is a directory, not a K1 executable!\n", file_path);
    return;
  }

  TRY
  {
    current_inferior ()->symfile_flags |= SYMFILE_DEFER_BP_RESET;
    exec_file_attach ((char *) file_path, 0);
    symbol_file_add_main (file_path, 0);
  }
  CATCH (ex, RETURN_MASK_ALL)
  {
    // exception
  }
  END_CATCH
}

static ptid_t
k1_target_wait (struct target_ops *target,
                ptid_t ptid, struct target_waitstatus *status, int options)
{
    struct target_ops *remote_target = find_target_beneath(target);
    ptid_t res;
    struct inferior *inferior;
    int ix_items;

    res = remote_target->to_wait (remote_target, ptid, status, options);

    inferior = find_inferior_pid (ptid_get_pid (res));

    if (inferior && !mppa_inferior_data (inferior)->booted) {
        char *endptr;
        struct osdata *osdata;
        struct osdata_item *last;
        struct osdata_item *item;

        osdata = get_osdata (NULL);
        
        for (ix_items = 0;
             VEC_iterate (osdata_item_s, osdata->items,
                          ix_items, item);
             ix_items++) {
            unsigned long pid = strtoul (get_osdata_column (item, "pid"), &endptr, 10);
            const char *file = get_osdata_column (item, "command");
            const char *cluster = get_osdata_column (item, "cluster");
            int os_debug_level;
            struct inferior_data *data;
            
            if (pid != ptid_get_pid (res))
                continue;

            data = mppa_inferior_data (inferior);
            data->booted = 1;

            if (file && file[0] != 0 && !data->sym_file_loaded)
            {
              ptid_t save_ptid = inferior_ptid;
              data->sym_file_loaded = 1;
              switch_to_thread (res);
              k1_change_file (file);
              switch_to_thread (save_ptid);
            }
            
            if (!after_first_resume)
            {
              status->value.sig = GDB_SIGNAL_TRAP;
            }
            else
            {
              if (data->cluster_break_on_spawn)
              {
                status->kind = TARGET_WAITKIND_STOPPED;
                status->value.sig = GDB_SIGNAL_TRAP;
              }
              else
                status->kind = TARGET_WAITKIND_SPURIOUS;

              //if os support a debug level, set it to the cluster
              os_debug_level = get_os_supported_debug_levels (inferior);
              if (os_debug_level > DBG_LEVEL_SYSTEM && !global_debug_level_set &&
                data->cluster_debug_level == DBG_LEVEL_INHERITED)
              {
                set_cluster_debug_level_no_check (inferior, os_debug_level);
              }
            }
            break;                
        }

    }

    return res;
}

static void
mppa_attach (struct target_ops *ops, const char *args, int from_tty)
{
    struct target_ops *remote_target, *k1_ops = find_target_beneath(&current_target);

    if (k1_ops != &k1_target_ops)
        error ("Don't know how to attach.  Try \"help target\".");

    remote_target = find_target_beneath(&k1_target_ops);
    return remote_target->to_attach (remote_target, args, from_tty);
}

static int
k1_target_can_run (struct target_ops *ops)
{
    return 1;
}

static int
k1_target_supports_non_stop (struct target_ops *ops)
{
  return 1;
}

static int
k1_target_can_async (struct target_ops *ops)
{
  return target_async_permitted;
}

static void change_thread_cb (struct inferior *inf)
{
  struct thread_info *th;
  
  //printf ("change thread_cb pid=%d, lpw=%d\n", inferior_ptid.pid, inferior_ptid.lwp);

  if (is_stopped (inferior_ptid))
    return;
  
  th = any_live_thread_of_process (inf->pid);
  if (!is_stopped (th->ptid))
      return;

  switch_to_thread (th->ptid);
}

static int
is_crt_cpu_in_user_mode (struct regcache *regcache)
{
  uint64_t ps;
  regcache_raw_read_unsigned (regcache, 65, &ps);

  return ((ps & 1) == 0); //PS.PM == 0 (user mode)
}

static void
k1_fetch_registers (struct target_ops *target, struct regcache *regcache, int regnum)
{
  // don't use current_inferior () & current_inferior_
  // our caller (regcache_raw_read) changes only inferior_ptid
  
  struct target_ops *remote_target;
  int crt_thread_mode_used, new_mode;
  struct inferior_data *data;
  struct inferior *inf;
  
  // get the registers of the current thread (CPU) in the usual way
  remote_target = find_target_beneath (target);
  remote_target->to_fetch_registers (target, regcache, regnum);

  // first time we see a cluster, set the debug level
  inf = find_inferior_pid (inferior_ptid.pid);
  data = mppa_inferior_data (inf);
  if (!data)
    return;

  if (data->cluster_debug_level_postponed)
  {
    data->cluster_debug_level_postponed = 0;
    send_cluster_debug_level (get_debug_level (inferior_ptid.pid));
  }

  // after attach, if "c -a" executed to skip system code because of the
  // debug level, change the thread to a stopped one
  if (inf_created_change_th)
  {
    if (is_crt_cpu_in_user_mode (regcache))
    {
      inf_created_change_th = 0;
      create_timer (0, (void (*)(void*)) change_thread_cb, inf);
    }
  }
}

static struct cmd_list_element *kalray_set_cmdlist;
static struct cmd_list_element *kalray_show_cmdlist;

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

static int str_debug_level_to_idx (const char *slevel)
{
  int level;
  for (level = 0; level < DBG_LEVEL_MAX; level++)
    if (!strcmp (slevel, scluster_debug_levels[level]))
      return level;

  return 0;
}

static void apply_cluster_debug_level (struct inferior *inf)
{
  int level;
  int os_supported_level;
  struct thread_info *th;
  ptid_t save_ptid, th_ptid;

  level = get_debug_level (inf->pid);
  
  th = any_live_thread_of_process (inf->pid);
  //the CPU must be stopped
  if (th == NULL || th->state != THREAD_STOPPED)
  {
    struct inferior_data *data = mppa_inferior_data (inf);
    //printf ("Info: No stopped CPU found for %s. "
    //  "Postpone the setting of the cluster debug level.\n", data->cluster);
    data->cluster_debug_level_postponed = 1;
    send_cluster_postponed_debug_level (inf, level);
    return;
  }

  os_supported_level = get_os_supported_debug_levels (inf);
  if (level > os_supported_level)
    return;

  save_ptid = inferior_ptid;
  th_ptid = th->ptid;
  switch_to_thread (th_ptid);
  set_general_thread (th_ptid);

  send_cluster_debug_level (level);

  switch_to_thread (save_ptid);
  set_general_thread (save_ptid);
}

void set_cluster_debug_level_no_check (struct inferior *inf, int debug_level)
{
  struct inferior_data *data;

  data = mppa_inferior_data (inf);
  data->cluster_debug_level = debug_level; 
  apply_cluster_debug_level (inf);
}

static void set_cluster_debug_level (char *args, int from_tty, struct cmd_list_element *c)
{
  int new_level, prev_level;
  struct inferior *inf;

  if (ptid_equal (inferior_ptid, null_ptid))
    error (_("Cannot set debug level without a selected thread."));

  inf = current_inferior ();
  prev_level = get_cluster_debug_level (inf->pid);
  new_level = str_debug_level_to_idx (scluster_debug_level);

  if (new_level != prev_level)
  {
    int os_supported_level = get_os_supported_debug_levels (inf);
    if (os_supported_level < new_level)
    {
      struct inferior_data *data = mppa_inferior_data (inf);
      printf ("Cannot set debug level %s for %s (highest level supported by os is %s).\n",
        scluster_debug_levels[new_level], data->cluster,
        scluster_debug_levels[os_supported_level]);
    }
    else
      set_cluster_debug_level_no_check (inf, new_level);
  }
}

static int set_cluster_debug_level_iter (struct inferior *inf, void *not_used)
{
  apply_cluster_debug_level (inf);
  return 0;
}

void apply_global_debug_level (int level)
{
  idx_global_debug_level = level;
  if (have_inferiors ())
    iterate_over_inferiors (set_cluster_debug_level_iter, NULL);
  else
  {
    printf ("Info: No cluster found. Action postponed for attach.\n");
  }
}

static void set_global_debug_level (char *args, int from_tty, struct cmd_list_element *c)
{
  int new_level;

  new_level = str_debug_level_to_idx (sglobal_debug_level);
  global_debug_level_set = 1;
  if (new_level != idx_global_debug_level) 
  {
    apply_global_debug_level (new_level);
  }
}

static void
show_cluster_debug_level (struct ui_file *file, int from_tty, 
  struct cmd_list_element *c, const char *value)
{
  struct inferior_data *data;
  int level;

  if (ptid_equal (inferior_ptid, null_ptid) || is_exited (inferior_ptid))
  {
    printf (_("Cannot show debug level without a live selected thread."));
    return;
  }

  data = mppa_inferior_data (find_inferior_pid (inferior_ptid.pid));
  
  level = get_cluster_debug_level (-1);
  fprintf_filtered (file, "The cluster debug level is \"%s\"%s.\n",
    scluster_debug_levels[level], 
    data->cluster_debug_level_postponed ? " (setting postponed)" : "");
}

static void
show_global_debug_level (struct ui_file *file, int from_tty, 
  struct cmd_list_element *c, const char *value)
{
  fprintf_filtered (file, "The global debug level  is \"%s\".\n",
    scluster_debug_levels[idx_global_debug_level]);
}

static void
set_cluster_break_on_spawn (char *args, int from_tty, struct cmd_list_element *c)
{
  struct inferior *inf;
  struct inferior_data *data;

  if (ptid_equal (inferior_ptid, null_ptid))
    error (_("Cannot set break on reset without a selected thread."));

  inf = current_inferior ();
  data = mppa_inferior_data (inf);
  data->cluster_break_on_spawn = opt_break_on_spawn;
  send_cluster_break_on_spawn (inf, opt_break_on_spawn);
}

static void
show_cluster_break_on_spawn (struct ui_file *file, int from_tty, 
  struct cmd_list_element *c, const char *value)
{
  struct inferior_data *data;

  if (ptid_equal (inferior_ptid, null_ptid))
  {
    printf (_("Cannot show break on reset without a live selected thread."));
    return;
  }

  data = mppa_inferior_data (find_inferior_pid (inferior_ptid.pid));
  fprintf_filtered (file, "The cluster break on reset is %d.\n", data->cluster_break_on_spawn);
}

extern int remote_hw_breakpoint_limit;
extern int remote_hw_watchpoint_limit;

static void
attach_mppa_command (char *args, int from_tty)
{
    char set_non_stop_cmd[] = "set non-stop";
    char set_pagination_off_cmd[] = "set pagination off";
    struct osdata *osdata;
    struct osdata_item *last;
    struct osdata_item *item;
    int ix_items,cur_pid, bstopped, bcur_inf_stopped;
    struct inferior *cur_inf;
    ptid_t cur_ptid, stopped_ptid;
    int saved_async_execution = !sync_execution;    

    dont_repeat ();

    print_thread_events = 0;
    after_first_resume = 0;

    k1_push_arch_stratum (NULL, 0);
    execute_command (set_non_stop_cmd, 0);
    execute_command (set_pagination_off_cmd, 0);

    k1_target_attach (&current_target, args, from_tty);

    remote_hw_breakpoint_limit = 0;
    remote_hw_watchpoint_limit = 1;

	bstopped = 0;
	bcur_inf_stopped = 0;
    osdata = get_osdata (NULL);
    
    cur_inf = current_inferior();    
    cur_ptid = inferior_ptid;
    cur_pid = cur_inf->pid;

    for (ix_items = 0;
         VEC_iterate (osdata_item_s, osdata->items,
                      ix_items, item);
         ix_items++) {
        char *endptr;
        unsigned long pid = strtoul (get_osdata_column (item, "pid"), &endptr, 10);
        struct inferior *inf;
        char attach_cmd[25];
        
        if (pid == cur_inf->pid) {
            continue;
        }
        
        inf = add_inferior_with_spaces ();
        set_current_inferior (inf);
        switch_to_thread (null_ptid);
        set_current_program_space (inf->pspace);
        sprintf (attach_cmd, "attach %li&", pid);
        execute_command (attach_cmd, 0);
        inf->control.stop_soon = NO_STOP_QUIETLY;
        inf->removable = 1;
    }

    bstopped = 0;
    bcur_inf_stopped = 0;
    osdata = get_osdata (NULL);

    for (ix_items = 0;
         VEC_iterate (osdata_item_s, osdata->items,
                      ix_items, item);
         ix_items++) {
        char *endptr;
        struct thread_info *live_th;
        unsigned long pid = strtoul (get_osdata_column (item, "pid"), &endptr, 10);
        const char *file = get_osdata_column (item, "command");
        const char *running = get_osdata_column (item, "running");

        if (strcmp (running, "yes"))
            continue;

        live_th = any_live_thread_of_process (pid);
        if (live_th && !live_th->stop_requested)
            set_stop_requested (live_th->ptid, 1);

        if (pid == cur_pid)
            bcur_inf_stopped = 1;

        if (!bstopped)
        {
            bstopped = 1;
            stopped_ptid = any_live_thread_of_process (pid)->ptid;
        }

        if (file && file[0])
        {
          struct inferior_data *data;
          
          switch_to_thread (any_live_thread_of_process (pid)->ptid);
          data = mppa_inferior_data (current_inferior ());
          if (!data->sym_file_loaded)
          {
            data->sym_file_loaded = 1;
            k1_change_file (file);
          }
        }
    }
    if (!bstopped)
        async_enable_stdin ();

    if (!bstopped || bcur_inf_stopped)
        switch_to_thread (cur_ptid);
    else
        switch_to_thread (stopped_ptid);

    /* This is a hack to populate the dwarf mapping tables now that we
       have the architecture at hand. Having these tables initialized
       from the debug reader routines will break as objfile_arch won't
       have the register descriptions */
    gdbarch_dwarf2_reg_to_regnum (get_current_arch(), 0);
    
    if (idx_global_debug_level)
    {
      apply_global_debug_level (idx_global_debug_level);
    }
    
  if (saved_async_execution)
    async_enable_stdin ();
}

static void
run_mppa_command (char *args, int from_tty)
{
    char set_non_stop_cmd[] = "set non-stop";
    char set_pagination_off_cmd[] = "set pagination off";
    char run_cmd[] = "run";

    dont_repeat ();

    k1_push_arch_stratum (NULL, 0);
    execute_command (set_non_stop_cmd, 0);
    execute_command (set_pagination_off_cmd, 0);
    execute_command (run_cmd, 0);
}

static void
mppa_inferior_data_cleanup (struct inferior *inf, void *data)
{
    xfree (data);
}

static void mppa_observer_breakpoint_created (struct breakpoint *b)
{
  const char *addr_string;
  
  if (b && b->location)
  {
    addr_string = event_location_to_string (b->location);
    if (b && addr_string && !strcmp (addr_string, "main"))
      send_stop_at_main (0);
  }

}

static void mppa_observer_breakpoint_deleted (struct breakpoint *b)
{
  const char *addr_string;

  if (b && b->location)
  {
    addr_string = event_location_to_string (b->location);
    if (b && addr_string && !strcmp (addr_string, "main"))
      send_stop_at_main (1);
  }
}

struct k1_dev_list
{
  int loaded;
  struct k1_dev_list *first_child;
  char *full_name;
  char *short_name; // points inside full_name, must not be freed
  struct k1_dev_list *next;
};
struct k1_dev_list *head_k1_dev_list = NULL;

static void
k1_fill_dev_list (struct k1_dev_list *dev)
{
  char *s, *crt_text, *sep;
  struct k1_dev_list **prev_dev, *crt_dev;
  int len;

  if (dev->loaded)
    return;

  s = send_get_dev_list_string (dev->full_name);
  if (!s)
    return;

  dev->loaded = 1;
  if (!*s)
    return;

  len = strlen (dev->full_name);
  for (crt_text = s, prev_dev = &dev->first_child; crt_text; crt_text = sep + 1, prev_dev = &crt_dev->next)
  {
    sep = strchr (crt_text, ';');
    if (sep)
      *sep = 0;

    crt_dev = (struct k1_dev_list *) calloc (1, sizeof (struct k1_dev_list));
    crt_dev->full_name = (char *) malloc (len + 1 + strlen (crt_text) + 1);
    if (len)
    {
      sprintf (crt_dev->full_name, "%s.%s", dev->full_name, crt_text);
      crt_dev->short_name = crt_dev->full_name + len + 1;
    }
    else
    {
      strcpy (crt_dev->full_name, crt_text);
      crt_dev->short_name = crt_dev->full_name;
    }

    *prev_dev = crt_dev;

    if (!sep)
      break;
  }

  free (s);
}

static void
k1_rec_destroy_dev_list (struct k1_dev_list *dev)
{
  struct k1_dev_list *child, *next;

  for (child = dev->first_child; child; child = next)
  {
    next = child->next; //get next child as the current child will be destroyed after the next line
    k1_rec_destroy_dev_list (child);
  }

  free (dev->full_name);
  free (dev);
}

static void
k1_destroy_dev_list (void)
{
  if (head_k1_dev_list)
    k1_rec_destroy_dev_list (head_k1_dev_list);
  head_k1_dev_list = NULL;
}

static void
kwatch_command (char *arg, int from_tty)
{
  char *err_msg = NULL;
  int id = 1;

  if (ptid_equal (inferior_ptid, null_ptid))
  {
    error ("Cannot add a kalray watchpoint without being attached");
    return;
  }

  if (send_set_kwatch (arg, 0 /*type=kwatch*/, 1 /*set*/, &err_msg))
  {
    printf ("Error adding kalray watchpoint %s: %s\n", arg, err_msg ? err_msg : "Unspecified error");
    if (err_msg)
      free (err_msg);
    return;
  }

  printf ("kwatchpoint %d: %s\n", id, arg);
}

static VEC (char_ptr) *
k1_dev_completer (struct cmd_list_element *ignore, const char *text, const char *word)
{
  VEC (char_ptr) *result = NULL;
  struct k1_dev_list *crt_dev;
  char *point_text, *copy_text, *crt_text, *stmp;

  if (ptid_equal (inferior_ptid, null_ptid))
    return NULL;

  if (!strcmp (ignore->name, "kwatch"))
    copy_text = strdup (text);
  else
  {
    // if multiple devs accepted by the command
    for (stmp = (char *) word; stmp > text && *stmp != ' ' && *stmp != '\t'; stmp--)
      ;
    if (stmp > text)
      stmp++;
    copy_text = strdup (stmp);
  }
  crt_text = copy_text;

  if (!head_k1_dev_list)
  {
    head_k1_dev_list = (struct k1_dev_list *) calloc (1, sizeof (struct k1_dev_list));
    head_k1_dev_list->full_name = head_k1_dev_list->short_name = strdup ("");
  }
  crt_dev = head_k1_dev_list;

  do
  {
    k1_fill_dev_list (crt_dev);

    point_text = strchr (crt_text, '.');
    if (point_text)
    {
      *point_text = 0;
      for (crt_dev = crt_dev->first_child; crt_dev && strcmp (crt_dev->short_name, crt_text); crt_dev = crt_dev->next)
        ;
      if (!crt_dev)
        break;
      crt_text = point_text + 1;
    }
    else
    {
      struct k1_dev_list *last_pushed_dev = NULL;
      int len = strlen (crt_text);

      for (crt_dev = crt_dev->first_child; crt_dev; crt_dev = crt_dev->next)
      {
        if (!strncmp (crt_dev->short_name, crt_text, len))
        {
          last_pushed_dev = crt_dev;
          VEC_safe_push (char_ptr, result, strdup (crt_dev->short_name));
        }
      }

      if (VEC_length (char_ptr, result) == 1)
      {
        k1_fill_dev_list (last_pushed_dev);
        if (last_pushed_dev->first_child)
        {
          free (VEC_pop (char_ptr, result));
          len = strlen (last_pushed_dev->short_name);
          stmp = (char *) malloc (len + 3);
          strcpy (stmp, last_pushed_dev->short_name);

          strcpy (stmp + len, ".");
          VEC_safe_push (char_ptr, result, strdup (stmp));
          strcpy (stmp + len, ".X"); // prevent readline to add a space
          VEC_safe_push (char_ptr, result, strdup (stmp));
          free (stmp);
        }
      }
    }
  } while (point_text);

  free (copy_text);

  return result;
}

void
_initialize__k1_target (void)
{
    simulation_vehicle = simulation_vehicles[0];
    scluster_debug_level = scluster_debug_levels[DBG_LEVEL_INHERITED];
    idx_global_debug_level = DBG_LEVEL_SYSTEM;
    global_debug_level_set = 0;
    sglobal_debug_level = sglobal_debug_levels[idx_global_debug_level];
    
    k1_target_ops.to_shortname = "mppa";
    k1_target_ops.to_longname = "Kalray MPPA connection";
    k1_target_ops.to_doc = 
	"Connect to a Kalray MPPA execution vehicle.";
    k1_target_ops.to_stratum = arch_stratum;

    k1_target_ops.to_open = k1_target_open;
    k1_target_ops.to_close = k1_target_close;

    k1_target_ops.to_create_inferior = k1_target_create_inferior;
    k1_target_ops.to_attach = mppa_attach;
    k1_target_ops.to_mourn_inferior = k1_target_mourn_inferior;
    k1_target_ops.to_wait = k1_target_wait;
    k1_target_ops.to_resume = mppa_target_resume;

    k1_target_ops.to_supports_non_stop = k1_target_supports_non_stop;
    k1_target_ops.to_can_async_p = k1_target_can_async;
    k1_target_ops.to_can_run = k1_target_can_run;
    k1_target_ops.to_attach_no_wait = 0;
    k1_target_ops.to_region_ok_for_hw_watchpoint = k1_region_ok_for_hw_watchpoint;

    k1_target_ops.to_pid_to_str = mppa_pid_to_str;
    k1_target_ops.to_extra_thread_info = mppa_threads_extra_info;

    k1_target_ops.to_fetch_registers = k1_fetch_registers;
    
    k1_target_ops.to_magic = OPS_MAGIC;
    
    add_target (&k1_target_ops);

    print_stopped_thread = 1;
    
    {
      extern int (*p_kalray_hide_thread) (struct thread_info *tp, ptid_t crt_ptid);
      p_kalray_hide_thread = kalray_hide_thread;
    }

    add_prefix_cmd ("kalray", class_maintenance, set_kalray_cmd, _("\
Kalray specific variables\n				    \
Configure various Kalray specific variables."),
		    &kalray_set_cmdlist, "set kalray ",
		    0 /* allow-unknown */, &setlist);
    add_prefix_cmd ("kalray", class_maintenance, show_kalray_cmd, _("\
Kalray specific variables\n				    \
Configure various Kalray specific variables."),
		    &kalray_show_cmdlist, "show kalray ",
		    0 /* allow-unknown */, &showlist);

    add_setshow_string_noescape_cmd ("debug_agent_options", class_maintenance,
				     &da_options,  _("\
Set the options passed to the debug agent."), _("\
Show the options passed to the debug agent."), NULL, NULL, NULL,
				   &kalray_set_cmdlist, &kalray_show_cmdlist);

    add_setshow_enum_cmd ("simulation_vehicle", class_maintenance,
			  simulation_vehicles, &simulation_vehicle, _("\
Set the simulation vehicle to use for execution."), _("\
Show the simulation vehicle to use for execution."), NULL, NULL, NULL,
			  &kalray_set_cmdlist, &kalray_show_cmdlist);

    add_setshow_enum_cmd ("cluster_debug_level", class_maintenance,
			  scluster_debug_levels, &scluster_debug_level,
        _("Set the cluster debug level."), _("Show the cluster debug level."), 
        NULL, set_cluster_debug_level, show_cluster_debug_level,
			  &kalray_set_cmdlist, &kalray_show_cmdlist);
    
    add_setshow_enum_cmd ("global_debug_level", class_maintenance,
			  sglobal_debug_levels, &sglobal_debug_level,
        _("Set the global debug level."), _("Show the global debug level."), 
        NULL, set_global_debug_level, show_global_debug_level,
			  &kalray_set_cmdlist, &kalray_show_cmdlist);
    
    add_setshow_boolean_cmd ("hide_threads", class_maintenance, &opt_hide_threads,
      _("Set hide threads in debug level."), _("Show hide threads in debug level."),
      NULL, NULL, NULL, &kalray_set_cmdlist, &kalray_show_cmdlist);

    add_setshow_boolean_cmd ("break_on_spawn", class_maintenance, &opt_break_on_spawn,
      _("Set break on reset."), _("Show break on reset."),
      NULL, set_cluster_break_on_spawn, show_cluster_break_on_spawn, &kalray_set_cmdlist, &kalray_show_cmdlist);
    
    add_com ("attach-mppa", class_run, attach_mppa_command, _("\
Connect to a MPPA TLM platform and start debugging it.\n\
Usage is `attach-mppa PORT'."));

    add_com ("run-mppa", class_run, run_mppa_command, _("\
Connect to a MPPA TLM platform and start debugging it."));

  {
    struct cmd_list_element *c = add_com ("kwatch", class_breakpoint, kwatch_command,
      _("Set watchpoint for a device registry (currently implemented only for simulator)."));
    set_cmd_completer (c, k1_dev_completer);
    atexit (k1_destroy_dev_list);
  }

    observer_attach_inferior_created (k1_push_arch_stratum);
    k1_attached_inf_data = register_inferior_data_with_cleanup (NULL, mppa_inferior_data_cleanup);
    
    observer_attach_breakpoint_created (mppa_observer_breakpoint_created);
    observer_attach_breakpoint_deleted (mppa_observer_breakpoint_deleted);
}
