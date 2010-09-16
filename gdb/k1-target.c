/* Target ops to connect to the K1 simulator. 

   Copyright (C) 2010, Kalray
*/

#include "defs.h"

#include <assert.h>
#include <libgen.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include "environ.h"
#include "gdbcmd.h"
#include "gdbcore.h"
#include "gdbthread.h"
#include "inferior.h"
#include "observer.h"
#include "main.h"
#include "target.h"
#include "top.h"

#include "cli/cli-decode.h"
#include "cli/cli-setshow.h"

static cmd_cfunc_ftype *real_run_command;
static struct observer *exec_file_observer;

static struct target_ops k1_target_ops;
static char *da_options = NULL;

static const char *simulation_vehicles[] = { "mppa_tlm", "runner", NULL };
static const char *simulation_vehicle;

pid_t server_pid;

static void 
k1_target_open (char *name, int from_tty)
{
    push_target (&k1_target_ops);
}

static void k1_target_close (int quitting)
{

}

static void
k1_target_new_thread (struct thread_info *t)
{
    if (!ptid_equal(inferior_ptid, null_ptid))
	inferior_thread ()->step_multi = 1;

    /* When we attach, don't wait... the K1 is already stopped. */
    current_target.to_attach_no_wait = 1;
}

static void k1_target_mourn_inferior (struct target_ops *target)
{
    unpush_target (target);
    generic_mourn_inferior ();
    kill (server_pid, 9);
}

static void k1_target_create_inferior (struct target_ops *ops, 
				       char *exec_file, char *args,
				       char **env, int from_tty);

void k1_target_attach (struct target_ops *ops, char *args, int from_tty)
{
    char tar_remote_cmd[] = "target extended-remote :        ";
    int saved_batch_silent = batch_silent;
    struct observer *new_thread_observer;
    char *cmd_port;

    parse_pid_to_attach (args);

    cmd_port = strchr(tar_remote_cmd, ':');
    sprintf (cmd_port + 1, "%s", args);

    /* Load the real debug target by issuing 'target remote'. Of
       course things aren't that simple because it's not meant to be
       used that way. One issue is that connecting to the gdb_stub
       will emit MI stop notifications and will print the initial
       frame at the connection point. BATCH_SILENT removes the frame
       display (see infrun.c) and the ugly hack with the observer
       makes infrun believe that we're in a series of steps and thus
       inhibits the emission of the new_thread observer notification
       that prints the initial MI *stopped message. */
    batch_silent = 1;
    new_thread_observer = observer_attach_new_thread (k1_target_new_thread);
    /* tar remote */
    execute_command (tar_remote_cmd, 0);
    /* Remove hacks*/
    observer_detach_new_thread (new_thread_observer);
    batch_silent = saved_batch_silent;
    inferior_thread ()->step_multi = 0;
    current_inferior ()->stop_soon = NO_STOP_QUIETLY;

    /* Our target vector has been poped from the target stack by
       'target remote'. If we want to keep the ability to 'run', we
       need to equip the remote target vector with our create_inferior
       implementation. */
    ops = find_target_beneath(&current_target);
    ops->to_create_inferior = k1_target_create_inferior;
    ops->to_attach = k1_target_attach;
    ops->to_mourn_inferior = k1_target_mourn_inferior;
}

static void k1_target_create_inferior (struct target_ops *ops, 
				       char *exec_file, char *args,
				       char **env, int from_tty)
{
    char **argv_args = gdb_buildargv (args);
    char **da_args = gdb_buildargv (da_options);
    char **stub_args;
    char **arg;
    int nb_args = 0, nb_da_args = 0;
    int pipefds[2];
    int port;
    int argidx = 0;

    if (exec_file == NULL)
	error (_("No executable file specified.\n\
Use the \"file\" or \"exec-file\" command."));

    arg = argv_args;
    while (arg && *arg++) nb_args++;
    arg = da_args;
    while (arg && *arg++) nb_da_args++;

    stub_args = xmalloc ((nb_args+7)*sizeof (char*));
    stub_args[argidx++] = simulation_vehicle;
    if (nb_da_args && strlen (da_options)) {
	arg = da_args;
	while (*arg) {
	    stub_args[argidx++] = *arg++;
	}
    }
    stub_args[argidx++] = "--gdb";
    stub_args[argidx++] = "--";
    stub_args[argidx++] = exec_file;
    memcpy (stub_args + argidx,  argv_args, (nb_args+1)*sizeof (char*));

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
	dup2(pipefds[1], 100);
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
    } else {
	int port;
	char cmd_port[10];
	
	close (pipefds[1]);
	read (pipefds[0], &port, sizeof(port));
	close (pipefds[0]);
	
	sprintf (cmd_port, "%i", port);
	k1_target_attach (ops, cmd_port, from_tty);
    }

}

static int
k1_target_can_run (void)
{
    return 1;
}

static int
k1_target_supports_non_stop (void)
{
  return 1;
}

static int
k1_target_can_async (void)
{
  if (!target_async_permitted)
    /* We only enable async when the user specifically asks for it.  */
    return 0;

  return 1;
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


static void
attach_mppa_command (char *args, int from_tty)
{
    char set_target_async_cmd[] = "set target-async";
    char set_non_stop_cmd[] = "set non-stop";
    char set_pagination_off_cmd[] = "set pagination off";
    dont_repeat ();
    
    execute_command (set_target_async_cmd, 0);
    execute_command (set_non_stop_cmd, 0);
    execute_command (set_pagination_off_cmd, 0);
    k1_target_attach (&current_target, args, from_tty);
}

static void
run_mppa_command_continuation (void *args)
{
      clear_proceed_status ();
      proceed ((CORE_ADDR) -1, TARGET_SIGNAL_DEFAULT, 0);
}

/* This command is run instead of run when the K1 target is
   loaded. See k1-tdep.c. */
void
run_mppa_command (char *args, int from_tty)
{
    char set_target_async_cmd[] = "set target-async";
    char set_non_stop_cmd[] = "set non-stop";
    char set_pagination_off_cmd[] = "set pagination off";
    dont_repeat ();

    if (exec_file_observer) {
	observer_detach_executable_changed (exec_file_observer);
	exec_file_observer = NULL;
    }

    if (lookup_minimal_symbol_text ("pthread_create", NULL)) {
	execute_command (set_target_async_cmd, 0);
	execute_command (set_non_stop_cmd, 0);
	execute_command (set_pagination_off_cmd, 0);
	target_create_inferior (get_exec_file (0), get_inferior_args (),
				environ_vector (current_inferior ()->environment),
				from_tty);
	/* Prevent the stop notification from the freshly connected RM
	   to show up in the CLI. We restart this core right after
	   that. */
	current_inferior ()->stop_soon = STOP_QUIETLY_NO_SIGSTOP;
	/* We use a continuation, because we want that to run after
	   the first stop notification has been handled. Note that an
	   inferior continuation wouldn't do as this is also used by
	   notice_new_inferior () and we want to run after that. */
	add_continuation (inferior_thread (),
			  run_mppa_command_continuation, NULL, NULL);
    } else {
	real_run_command (args, from_tty);
    }
}

static void
k1_override_run (void)
{
    if (real_run_command == 0) {
	/* Target files are linked first, thus we can't override a
	   generic function like 'run' in the _initialize_... function
	   of a targetting file. We do that here. It's ugly, I know. */
	char *run_cmd_name = xstrdup ("run");
	struct cmd_list_element *run;
	
	run = lookup_cmd (&run_cmd_name, cmdlist, "", 0, 0);
	real_run_command = run->function.cfunc;
	run->function.cfunc = run_mppa_command;
    }
}


void
_initialize__k1_target (void)
{
    simulation_vehicle = simulation_vehicles[0];
    
    k1_target_ops.to_shortname = "k1-iss";
    k1_target_ops.to_longname = "K1 target runinng on simulation";
    k1_target_ops.to_doc = 
	"Use the K1 simulation environment to debug your application.";
    k1_target_ops.to_stratum = arch_stratum + 1;

    k1_target_ops.to_open = k1_target_open;
    k1_target_ops.to_close = k1_target_close;

    k1_target_ops.to_attach = k1_target_attach;
    k1_target_ops.to_attach_no_wait = 1;

    k1_target_ops.to_create_inferior = k1_target_create_inferior;
    k1_target_ops.to_mourn_inferior = k1_target_mourn_inferior;

    k1_target_ops.to_supports_non_stop = k1_target_supports_non_stop;
    k1_target_ops.to_can_async_p = k1_target_can_async;
    k1_target_ops.to_can_run = k1_target_can_run;
    k1_target_ops.to_magic = OPS_MAGIC;
    
    add_target (&k1_target_ops);

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

    add_com ("attach-mppa", class_run, attach_mppa_command, _("\
Connect to a MPPA TLM platform and start debugging it.\n\
Usage is `attach-mppa PORT'."));

    add_com ("run-mppa", class_run, run_mppa_command, _("\
Connect to a MPPA TLM platform and start debugging it."));

    exec_file_observer = observer_attach_executable_changed (k1_override_run);
}
