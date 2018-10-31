#include "defs.h"
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <signal.h>
#include <poll.h>
#include "serial.h"
#include <errno.h>
#include <libgen.h>
#include <signal.h>
#include "k1-linux-mux.h"
#include "target.h"
#include "../../oce/cnoc_debug/host_linux/mppa_debug_server_common.h"

#define CHAR_MUX_GDB 'G'
#define CHAR_MUX_CLUSTERS 'C'
#define EXE_DEBUG_CLUSTERS "k1-linux-debug"

static struct serial con_k1_scb;
static int fd_clusters_mux[2], fd_gdb_mux;
static pthread_t th_mux = 0;
static int fd_listen, exit_received, first_read_from_mux;
static int nmuxed;
static void *h_debug_clusters = NULL;
static int pid_clusters;

static void
atexit_inform_peer (void)
{
  if (pid_clusters >= 0)
  {
    sleep (1);
    kill (pid_clusters, SIGUSR1);
  }
  pid_clusters = -1;
}

static void
close_fds (int close_cluster_to_mux)
{
  if (con_k1_scb.ops)
  {
    con_k1_scb.ops->close (&con_k1_scb);
    con_k1_scb.ops = NULL;
  }
  if (con_k1_scb.fd)
  {
    close (con_k1_scb.fd);
    con_k1_scb.fd = -1;
  }
  if (fd_clusters_mux[0] >= 0)
  {
    close (fd_clusters_mux[0]);
    fd_clusters_mux[0] = -1;
  }
  if (close_cluster_to_mux && fd_clusters_mux[1] >= 0)
  {
    close (fd_clusters_mux[1]);
    fd_clusters_mux[1] = -1;
  }
  if (fd_listen >= 0)
  {
    close (fd_listen);
    fd_listen = -1;
  }
  if (fd_gdb_mux >= 0)
  {
    close (fd_gdb_mux);
    fd_gdb_mux = -1;
  }
}

static int
start_listen (int *port, int count)
{
  int fd = -1, port_end;
  socklen_t tmp = 1;
  struct sockaddr_in sockaddr;

  if ((fd = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
  {
    fprintf (stderr, "Error opening socket (err=%d, %s)\n", errno, strerror (errno));
    goto label_err;
  }

  setsockopt (fd, SOL_SOCKET, SO_REUSEADDR, (char *) &tmp, sizeof (tmp));

  for (port_end = *port + count; *port < port_end; (*port)++)
  {
    sockaddr.sin_family = PF_INET;
    sockaddr.sin_port = htons (*port);
    sockaddr.sin_addr.s_addr = INADDR_ANY;

    if (!bind (fd, (struct sockaddr *) &sockaddr, sizeof (sockaddr)))
      break;
  }

  if (*port >= port_end)
  {
    fprintf (stderr, "Error in bind (err=%d, %s)\n", errno, strerror (errno));
    goto label_err;
  }

  if (listen (fd, 1))
  {
    fprintf (stderr, "Error in listen (err=%d, %s)\n", errno, strerror (errno));
    goto label_err;
  }

  return fd;
label_err:
  if (fd >= 0)
    close (fd);
  return -1;
}

int start_clusters_debug (void);
int
start_clusters_debug (void)
{
  char *argv[20], exe_path[PATH_MAX], t[20], *t1;
  int argc, sz;

  if ((sz = readlink ("/proc/self/exe", exe_path, sizeof (exe_path) - strlen (EXE_DEBUG_CLUSTERS) - 2)) < 0)
  {
    fprintf (stderr, "Error: cannot get the path of the current executable or the path is too long\n");
    goto label_err;
  }
  exe_path[sz] = 0;
  strcpy (exe_path, dirname (exe_path));
  sprintf (exe_path + strlen (exe_path), "/%s", EXE_DEBUG_CLUSTERS);
  if ((t1 = realpath (exe_path, NULL)) == NULL || access (t1, R_OK) != 0)
  {
    fprintf (stderr, "Error: cannot find executable to debug clusters\n");
    goto label_err;
  }
  strcpy (exe_path, t1);
  free (t1);

  argc = 0;
  argv[argc++] = EXE_DEBUG_CLUSTERS;
  if (sysroot_path)
  {
    argv[argc++] = "--sysroot";
    argv[argc++] = sysroot_path;
  }
  argv[argc++] = "--con";
  sprintf (t, "<fd>:%d", fd_clusters_mux[1]);
  argv[argc++] = t;
  argv[argc++] = NULL;

  if ((pid_clusters = fork ()) == 0)
  {
    setpgrp ();
    close_fds (0);
    execv (exe_path, argv);
    fprintf (stderr, "Error: execv of %s returned\n", exe_path);
    exit (1);
  }

  atexit (atexit_inform_peer);

  if (fd_clusters_mux[1] >= 0)
  {
    close (fd_clusters_mux[1]);
    fd_clusters_mux[1] = -1;
  }

  return 0;
label_err:
  if (h_debug_clusters)
  {
    dlclose (h_debug_clusters);
    h_debug_clusters = NULL;
  }
  return -1;
}

static int
check_pollfd_err (short revents, int no_err_msg, char *fd_name)
{
  if ((revents & (POLLERR | POLLHUP | POLLNVAL)) == 0)
    return 0;

  if (!no_err_msg)
  {
    if (!strcmp (fd_name, "clusters") && (revents & POLLHUP))
      fprintf (stderr, "Connection to K1 closed\n");
    else
    {
      fprintf (stderr, "Error received from poll for %s:", fd_name);
      if (revents & POLLERR)
        fprintf (stderr, " POLLERR");
      if (revents & POLLHUP)
        fprintf (stderr, " POLLHUP");
      if (revents & POLLNVAL)
        fprintf (stderr, " POLLNVAL");
      fprintf (stderr, "\n");
    }
  }

  return -1;
}

static int
fd_write (int fd, unsigned char *buf, int nb_to_write, const char *from, const char *to, const char *msg)
{
  int nb, total_nb = 0, remain_nb = nb_to_write;

  while (remain_nb > 0)
  {
    if ((nb = write (fd, buf + total_nb, remain_nb)) == 0)
    {
      if (!exit_received)
        fprintf (stderr, "Connection between %s and %s closed (WR)\n", from, to);
      return -1;
    }
    if (nb < 0)
    {
      if (errno == EINTR)
        continue;
      if (!exit_received)
        fprintf (stderr, "Error: %d (%s) after writing to %s %d/%d bytes of %s from %s\n",
          errno, strerror (errno), to, total_nb, nb, msg, from);
      return -1;
    }
    total_nb += nb;
    remain_nb -= nb;
  }

  return 0;
}

static int
fd_read_write_fd_mux (int fd_src, unsigned char *buf, int buf_len, int fd_mux, int is_gdb)
{
  char buf1[20];
  int nb, nb_to_read, nb_total = 0, is_header = 1;
  struct mux_header *mux_h = (struct mux_header *) buf;
  struct inc_cmd *in = (struct inc_cmd *) (buf + SZ_MUXH);
  const char *src = is_gdb ? "GDB" : "clusters";

  nb_to_read = is_gdb ? buf_len - SZ_MUXH : SZ_INC;
  do
  {
    do
    {
      if ((nb = read (fd_src, buf  + SZ_MUXH + nb_total, nb_to_read)) <= 0)
      {
        if (nb < 0 && errno == EINTR)
          continue;
        if (!exit_received && !is_gdb)
          fprintf (stderr, "Connection between mux and %s closed (RD)\n", src);
        return -1;
      }
      nb_to_read -= nb;
      nb_total += nb;
    } while (!is_gdb && nb_to_read);
  } while (!is_gdb && is_header-- && (nb_to_read = in->len_buf) > 0);

  memset (mux_h, 'X', SZ_MUXH);
  mux_h->id = is_gdb ? CHAR_MUX_GDB : CHAR_MUX_CLUSTERS;
  mux_h->len = nb_total;
  if (fd_write (fd_mux, buf, SZ_MUXH + nb_total, src, "mux", "package") < 0)
    return -1;

  return 0;
}

static int
fd_mux_read_dispatch (int fd_mux, unsigned char *buf, int buf_len, int fd_gdb, int fd_clusters)
{
  enum {MUX_ST_HEADER = 0, MUX_ST_PKG};
  int already_polled, nb, buf_pos, read_state, nb_to_read, fd_dest, remain_pkg, ret = 0;
  struct mux_header *mux_h = (struct mux_header *) buf;
  const char *dest = "";
  char c;

  already_polled = 1;
  buf_pos = 0;
  read_state = MUX_ST_HEADER;
  nb_to_read = SZ_MUXH;

  while (1)
  {
    if ((nb = read (fd_mux, buf + buf_pos, nb_to_read)) <= 0)
    {
      if (nb < 0 && errno == EINTR)
        continue;

      while (nb == 0 && !exit_received && !already_polled)
      {
        struct pollfd pollfds = {fd_mux, POLLIN, 0};
        int active = poll (&pollfds, 1, 2000);
        if (active < 0 && errno == EINTR)
          continue;
        if (active <= 0 || check_pollfd_err (pollfds.revents, 1, "K1"))
          break;
        already_polled = 2;
      }
      if (already_polled == 2)
      {
        already_polled = 1;
        continue;
      }

      if (!exit_received)
        fprintf (stderr, "Connection to K1 closed\n");
      return -1;
    }
    already_polled = 0;

    buf_pos += nb;

    if (read_state == MUX_ST_HEADER)
    {
      c = mux_h->id;
      if ((c != CHAR_MUX_GDB && c != CHAR_MUX_CLUSTERS) || (buf_pos == SZ_MUXH && mux_h->x != 'X'))
      {
        fprintf (stderr, "Error: invalid package header: id=%d (should be %c or %c)",
          ret, CHAR_MUX_GDB, CHAR_MUX_CLUSTERS);
        if (first_read_from_mux)
          fprintf (stderr, "\nPlease make sure that NO_MUX env variable is not defined for K1 gdbserver\n");
        else
        {
          if (buf_pos == SZ_MUXH)
            fprintf (stderr, ", length=%d, x=%c (should be X)\n", mux_h->len, mux_h->x);
          else
            fprintf (stderr, "\n");
        }
        ret = -1;
        break;
      }

      if (buf_pos != SZ_MUXH)
        continue;

      if (c == CHAR_MUX_GDB)
      {
        fd_dest = fd_gdb;
        dest = "GDB";
      }
      else // CHAR_MUX_CLUSTERS
      {
        fd_dest = fd_clusters;
        dest = "clusters";
        if (nmuxed < 3)
        {
          if (start_clusters_debug () < 0)
          {
            ret = -1;
            break;
          }
          nmuxed++;
        }
      }

      remain_pkg = mux_h->len;
      buf_pos = 0;
      read_state = MUX_ST_PKG;
      nb_to_read = (remain_pkg <= buf_len) ? remain_pkg : buf_len;
    }
    else if (read_state == MUX_ST_PKG)
    {
      if ((ret = fd_write (fd_dest, buf, buf_pos, "mux", dest, "message")) < 0)
        break;

      buf_pos = 0;
      remain_pkg -= nb;
      if (remain_pkg == 0)
        break;

      nb_to_read = (remain_pkg <= buf_len) ? remain_pkg : buf_len;
    }
  } // while (1)

  return ret;
}

static void*
thread_mux (void *pvoid)
{
  struct pollfd pollfds[3];
  enum {MUX_K1_FD = 0, MUX_GDB_FD, MUX_CLUSTERS_FD};
  unsigned char *buf_gdb = con_k1_scb.buf;
  int active, buf_gdb_len = sizeof (con_k1_scb.buf);

  fd_gdb_mux = accept (fd_listen, NULL, NULL);
  close (fd_listen);
  fd_listen = -1;
  if (fd_gdb_mux < 0)
  {
    fprintf (stderr, "Error in accept (err=%d, %s)\n", errno, strerror (errno));
    return NULL;
  }
  else
  {
    socklen_t tmp = 1;
    setsockopt (fd_gdb_mux, SOL_SOCKET, SO_KEEPALIVE, (char *) &tmp, sizeof (tmp));
    tmp = 1;
    setsockopt (fd_gdb_mux, IPPROTO_TCP, TCP_NODELAY, (char *) &tmp, sizeof (tmp));
  }

  nmuxed = 2;
  memset (pollfds, 0, sizeof (pollfds));
  pollfds[MUX_K1_FD].fd = con_k1_scb.fd;
  pollfds[MUX_K1_FD].events = POLLIN;
  pollfds[MUX_GDB_FD].fd = fd_gdb_mux;
  pollfds[MUX_GDB_FD].events = POLLIN;
  pollfds[MUX_CLUSTERS_FD].fd = fd_clusters_mux[0];
  pollfds[MUX_CLUSTERS_FD].events = POLLIN;

  while (1)
  {
    pollfds[MUX_K1_FD].revents = pollfds[MUX_GDB_FD].revents = pollfds[MUX_CLUSTERS_FD].revents = 0;
    active = poll (pollfds, nmuxed, -1);
    if (active < 0 && errno == EINTR)
      continue;
    if (active < 0 || active == 0)
    {
      if (!exit_received)
        fprintf (stderr, "Error %d (%s): Poll mux returned %d\n", errno, strerror (errno), active);
      break;
    }

    if (check_pollfd_err (pollfds[MUX_K1_FD].revents, 0, "K1"))
      break;
    if (check_pollfd_err (pollfds[MUX_GDB_FD].revents, exit_received, "GDB"))
      break;
    if (nmuxed >= 3 && check_pollfd_err (pollfds[MUX_CLUSTERS_FD].revents, exit_received, "clusters"))
      break;

    if (pollfds[MUX_K1_FD].revents)
    {
      if (fd_mux_read_dispatch (pollfds[MUX_K1_FD].fd, buf_gdb, buf_gdb_len,
        pollfds[MUX_GDB_FD].fd, pollfds[MUX_CLUSTERS_FD].fd) < 0)
        break;
    }

    if (pollfds[MUX_GDB_FD].revents)
    {
      if (fd_read_write_fd_mux (pollfds[MUX_GDB_FD].fd, buf_gdb, buf_gdb_len, pollfds[MUX_K1_FD].fd, 1) < 0)
        break;
    }

    if (pollfds[MUX_CLUSTERS_FD].revents)
    {
      if (fd_read_write_fd_mux (pollfds[MUX_CLUSTERS_FD].fd, buf_gdb, buf_gdb_len, pollfds[MUX_K1_FD].fd, 0) < 0)
        break;
    }
  } // while (1) poll

  close_fds (1);
  return NULL;
}

int
create_con_mux (const char *name)
{
  int i, port, ret;
  extern const struct serial_ops *serial_interface_lookup (const char *name);

  // init connection structures
  memset (&con_k1_scb, 0, sizeof (con_k1_scb));
  con_k1_scb.name = (char *) "connection to K1";
  con_k1_scb.bufcnt = 0;
  con_k1_scb.bufp = con_k1_scb.buf;
  con_k1_scb.error_fd = -1;
  con_k1_scb.refcnt = 1;
  fd_gdb_mux = fd_clusters_mux[0] = fd_clusters_mux[1] = -1;
  exit_received = 0;
  fd_listen = -1;
  pid_clusters = -1;
  first_read_from_mux = 1;

  // search how to connect to k1
  if (strcmp (name, "pc") == 0)
    con_k1_scb.ops = serial_interface_lookup ("pc");
  else if (startswith (name, "lpt"))
    con_k1_scb.ops = serial_interface_lookup ("parallel");
  else if (strchr (name, ':'))
    con_k1_scb.ops = serial_interface_lookup ("tcp");
  else
    con_k1_scb.ops = serial_interface_lookup ("hardwire");
  if (!con_k1_scb.ops)
  {
    fprintf (stderr, "Error: unrecognized connection string %s!\n", name);
    goto label_err;
  }

  // open connection to k1
  TRY
  {
    if ((ret = con_k1_scb.ops->open (&con_k1_scb, name)) != 0)
      fprintf (stderr, "Error while trying to connect to K1 using %s.\n", name);
  }
  CATCH (ex, RETURN_MASK_ALL)
  {
    fprintf (stderr, "Error while trying to connect to K1 using %s (%s).\n", name, ex.message ?: "");
    ret = -1;
  }
  END_CATCH
  if (ret)
    goto label_err;

  if (baud_rate != -1 && con_k1_scb.ops->setbaudrate (&con_k1_scb, baud_rate))
    goto label_err;

  con_k1_scb.ops->setparity (&con_k1_scb, serial_parity);
  con_k1_scb.ops->go_raw (&con_k1_scb);
  con_k1_scb.ops->flush_input (&con_k1_scb);

  if (socketpair (AF_UNIX, SOCK_STREAM, 0, fd_clusters_mux) < 0)
    goto label_err;

  for (i = 0; i < 2; i++)
  {
    socklen_t tmp = 1;
    setsockopt (fd_clusters_mux[i], IPPROTO_TCP, TCP_NODELAY, (char *) &tmp, sizeof (tmp));
  }

  port = 2200;
  if ((fd_listen = start_listen (&port, 100)) < 0)
    goto label_err;

  if (pthread_create (&th_mux, NULL, &thread_mux, NULL))
  {
    fprintf (stderr, "Error creating the pthread for the connections management\n");
    goto label_err;
  }

  return port;
label_err:
  close_fds (1);
  return -1;
}
