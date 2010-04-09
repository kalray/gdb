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

#include "gdbthread.h"
#include "inferior.h"
#include "observer.h"
#include "main.h"
#include "target.h"
#include "top.h"

static struct target_ops k1_target_ops;

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
    inferior_thread ()->step_multi = 1;
}

static void k1_target_create_inferior (struct target_ops *ops, 
				       char *exec_file, char *args,
				       char **env, int from_tty)
{
    char **argv_args = gdb_buildargv (args);
    char **arg = argv_args;
    int nb_args = 0;
    char tar_remote_cmd[] = "target remote :1337";
    int saved_batch_silent = batch_silent;
    struct observer *new_thread_observer;

    if (exec_file == NULL)
	error (_("No executable file specified.\n\
Use the \"file\" or \"exec-file\" command."));

    while (*arg++) nb_args++;

    argv_args = xrealloc (argv_args, (nb_args+5)*sizeof (char*));
    memmove (argv_args + 4,  argv_args, (nb_args+1)*sizeof (char*));
    argv_args[0] = "gdb_stub";
    argv_args[1] = "-s";
    argv_args[2] = ":1337";
    argv_args[3] = exec_file;

    if (server_pid != 0) {
	kill (server_pid, 9);
	waitpid (server_pid, NULL, 0);
    }

    server_pid = fork ();
    
    if (server_pid < 0)
	error ("Couldn't fork to launch the server.");

    if (server_pid == 0) {
	char path[PATH_MAX];
	char tmp[PATH_MAX] = { 0 };
	char *dir;

	/* Child */
	environ = env;
	execvp ("gdb_stub", argv_args);
	
	/* Not in PATH */
	if (readlink ("/proc/self/exe", tmp, PATH_MAX) != -1) {
	    dir = dirname (tmp);
	    snprintf (path, PATH_MAX, "%s/gdb_stub", dir);
	    execvp (path, argv_args);
	}

	printf_unfiltered ("Could not find gdb_stub in you PATH\n");
	exit (1);
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
}
   
void k1_target_attach (struct target_ops *ops, char *args, int from_tty)
{
  pid_t pid;

  parse_pid_to_attach (args);
  error ("Attach not supported yet.");
}

static int
k1_target_can_run (void)
{
    return 1;
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

    k1_target_ops.to_create_inferior = k1_target_create_inferior;

    k1_target_ops.to_can_run = k1_target_can_run;
    k1_target_ops.to_magic = OPS_MAGIC;
    
    add_target (&k1_target_ops);
}
