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

#include "gdbcmd.h"
#include "gdbthread.h"
#include "inferior.h"
#include "observer.h"
#include "main.h"
#include "target.h"
#include "top.h"

#include "cli/cli-setshow.h"

static struct target_ops k1_target_ops;
static char *da_options = NULL;

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
    generic_mourn_inferior ();
    unpush_target (target);
    kill (server_pid, 9);
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
    char tar_remote_cmd[] = "target extended-remote :        ";
    int saved_batch_silent = batch_silent;
    struct observer *new_thread_observer;
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
    stub_args[argidx++] = "runner";
    if (nb_da_args && strlen (da_options)) {
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
	execvp ("runner", stub_args);
	
	/* Not in PATH */
	if (readlink ("/proc/self/exe", tmp, PATH_MAX) != -1) {
	    dir = dirname (tmp);
	    snprintf (path, PATH_MAX, "%s/runner", dir);
	    execvp (path, stub_args);
	}

	printf_unfiltered ("Could not find gdb_stub in you PATH\n");
	exit (1);
    } else {
	int port;
	char *cmd_port;

	close (pipefds[1]);
	read (pipefds[0], &port, sizeof(port));
	close (pipefds[0]);
	
	cmd_port = strchr(tar_remote_cmd, ':');
	sprintf (cmd_port + 1, "%i", port);
    }

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
    ops->to_mourn_inferior = k1_target_mourn_inferior;
}
   
void k1_target_attach (struct target_ops *ops, char *args, int from_tty)
{
  pid_t pid;

  parse_pid_to_attach (args);

  k1_target_create_inferior (ops, "-a", args, NULL, from_tty);
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


void
attach_mppa_command (char *args, int from_tty)
{
    char set_target_async_cmd[] = "set target-async";
    char set_non_stop_cmd[] = "set non-stop";
    char set_pagination_off_cmd[] = "set pagination off";
    dont_repeat ();
    
    execute_command (set_target_async_cmd, 0);
    execute_command (set_non_stop_cmd, 0);
    execute_command (set_pagination_off_cmd, 0);
    attach_command (args, from_tty);
}

void
_initialize_k1_target (void)
{
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

    add_com ("attach-mppa", class_run, attach_mppa_command, _("\
Connect to a MPPA TLM platform and start debugging it.\n\
Usage is `attach-mppa PORT[&]'."));
}
