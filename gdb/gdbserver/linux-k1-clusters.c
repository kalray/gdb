#include "common-defs.h"
#include "terminal.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <poll.h>
#include <errno.h>
#include <signal.h>

#include "linux-k1-low.h"
#include <sys/mppa_debug_server_common.h>

#define FILE_CLUSTERS "/dev/cnoc_debug_server"
#define CHAR_MUX_GDB 'G'
#define CHAR_MUX_CLUSTERS 'C'

int fd_host = -1, fd_clusters = -1, fd_gdbserver = -1;
int debug_mux = 0, exit_received = 0;
pid_t pid_gdb_gdbserver = -1, pid_gdbserver = -1;
unsigned char buf[MAX_CMD_BUF_LEN + ((SZ_INC > SZ_OUT) ? SZ_INC : SZ_OUT) + 32] __attribute__ ((aligned (8)));

void
sig_usr1_handler (int sig)
{
  _exit (sig);
}

void
atexit_inform_peer (void)
{
  sleep (1);
  kill (pid_gdb_gdbserver ?: pid_gdbserver, SIGUSR1);
}

int
check_pollfd_err (short revents, int no_err_msg, char *fd_name)
{
  if ((revents & (POLLERR | POLLHUP | POLLNVAL)) == 0)
    return 0;

  if (!no_err_msg)
  {
    if (!strcmp (fd_name, "host") && (revents & POLLHUP))
      fprintf (stderr, "Connection to host closed\n");
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
start_listen (int *port, int count, int acc)
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

    if (!bind (fd, (struct sockaddr *) &sockaddr, sizeof (sockaddr)) && !listen (fd, 1))
      break;
  }

  if (*port >= port_end)
  {
    fprintf (stderr, "Error in bind (err=%d, %s)\n", errno, strerror (errno));
    goto label_err;
  }

  fprintf (stderr, "Listening on port %d\n", *port);
  fflush (stderr);

  if (acc)
  {
    int tfd = accept (fd, NULL, NULL);
    close (fd);
    fd = tfd;
    if (fd < 0)
    {
      fprintf (stderr, "Error in accept (err=%d, %s)\n", errno, strerror (errno));
      goto label_err;
    }

    tmp = 1;
    setsockopt (fd, SOL_SOCKET, SO_KEEPALIVE, (char *) &tmp, sizeof (tmp));
    tmp = 1;
    setsockopt (fd, IPPROTO_TCP, TCP_NODELAY, (char *) &tmp, sizeof (tmp));
  }

  return fd;
label_err:
  if (fd >= 0)
    close (fd);
  return -1;
}

static int
connect_to_port (int port)
{
  struct sockaddr_in sa;
  int i, ntries = 30, fd;
  socklen_t tmp = 1;

  fd = socket (AF_INET, SOCK_STREAM, 0);
  if (fd < 0)
  {
    printf ("Error: cannot create socket.\n");
    return -1;
  }

  memset (&sa, 0, sizeof (sa));
  sa.sin_family = AF_INET;
  sa.sin_port = htons (port);
  sa.sin_addr.s_addr = INADDR_ANY;

  for (i = 0; i < ntries; i++)
  {
    if (!connect (fd, (struct sockaddr *) &sa, sizeof(sa)))
      break;
    //if (i == 2)
      fprintf (stderr, "Waiting for gdbserver\n");
    sleep (1);
  }
  if (i >= ntries)
  {
     printf ("Error: socket connect failed.\n");
     return -1;
  }

  tmp = 1;
  setsockopt (fd, IPPROTO_TCP, TCP_NODELAY, (char *) &tmp, sizeof (tmp));

  return fd;
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
fd_read_write_fd_mux (int fd_src, unsigned char *buf, int buf_len, int fd_mux, int is_gdbserver)
{
  int nb;
  static int cnt = 0;
  const char *src = is_gdbserver ? "gdbserver" : "clusters";
  struct mux_header *mux_h = (struct mux_header *) buf;
  struct out_cmd *out = (struct out_cmd *) (buf + SZ_MUXH);

  while ((nb = read (fd_src, buf + SZ_MUXH, buf_len - SZ_MUXH)) <= 0)
  {
    if (nb < 0 && errno == EINTR)
      continue;
    if (!exit_received && !is_gdbserver)
      fprintf (stderr, "Connection between %s and mux closed (RD)\n", src);
    return -1;
  }

  if (!is_gdbserver && (nb < SZ_OUT || nb != SZ_OUT + out->len_buf))
  {
    if (!exit_received)
      fprintf (stderr, "Error: %d bytes read from clusters instead of %d (%d + %d)\n",
        nb, SZ_OUT + out->len_buf, SZ_OUT, out->len_buf);
    return -1;
  }

  if (!is_gdbserver && out->type == GOT_ASYNC_EXIT)
    exit_received = 1;

  if (debug_mux)
  {
    if (!is_gdbserver)
      fprintf (stderr, "%d async type=0x%x virtid=%d vehicle=%d ans=0x%llx len_buf=%d\n",
        ++cnt, out->type, out->virt_id, out->vehicle_id, out->answer, out->len_buf);
    else
      fprintf (stderr, "message from gdbserver\n");
  }

  memset (mux_h, 'X', SZ_MUXH);
  mux_h->id = is_gdbserver ? CHAR_MUX_GDB : CHAR_MUX_CLUSTERS;
  mux_h->len = nb;
  if (fd_write (fd_mux, (unsigned char *) buf, SZ_MUXH + nb, src, "mux", "package") < 0)
    return -1;

  return 0;
}

/* returns: -1: error, CHAR_MUX_GDB: for gdbserver, CHAR_MUX_CLUSTERS: for clusters */
static int
fd_mux_read_dispatch (int fd_mux, unsigned char *buf, int buf_len, int fd_gdb, int fd_clusters)
{
  enum {MUX_ST_HEADER = 0, MUX_ST_PKG};
  int already_polled, nb, buf_pos, read_state, nb_to_read, remain_pkg, pkg_size = 0, ret = -1;
  struct mux_header *mux_h = (struct mux_header *) buf;
  const char *dest = "";

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
        fprintf (stderr, "Connection to host closed\n");
      return -1;
    }
    already_polled = 0;
    buf_pos += nb;

    if (read_state == MUX_ST_HEADER)
    {
      if (buf_pos != SZ_MUXH)
        continue;

      ret = mux_h->id;
      if ((ret != CHAR_MUX_GDB && ret != CHAR_MUX_CLUSTERS) || (mux_h->x != 'X'))
      {
        fprintf (stderr, "Error: invalid package header: id=%d (should be %c or %c), length=%d, x=%c (should be X)\n",
          ret, CHAR_MUX_GDB, CHAR_MUX_CLUSTERS, mux_h->len, mux_h->x);
        ret = -1;
        break;
      }
      dest = (ret == CHAR_MUX_GDB) ? "gdbserver" : "clusters";
      remain_pkg = pkg_size = mux_h->len;
      buf_pos = 0;
      read_state = MUX_ST_PKG;
      nb_to_read = (remain_pkg <= buf_len) ? remain_pkg : buf_len;
      if (ret == CHAR_MUX_CLUSTERS && nb_to_read < SZ_INC)
      {
        fprintf (stderr, "Error: clusters command size %d too small (should be >= %d)\n", pkg_size, SZ_INC);
        ret = -1;
        break;
      }
    }
    else if (read_state == MUX_ST_PKG)
    {
      remain_pkg -= nb;
      if (ret == CHAR_MUX_GDB)
      {
        if (fd_write (fd_gdb, buf, nb, "mux", dest, "package") < 0)
        {
          ret = -1;
          break;
        }
        buf_pos = 0;
        nb_to_read = (remain_pkg <= buf_len) ? remain_pkg : buf_len;
      }
      else
        nb_to_read = remain_pkg;

      if (remain_pkg == 0)
      {
        if (debug_mux)
        {
          if (ret == CHAR_MUX_CLUSTERS) // for clusters
          {
            struct inc_cmd *in = (struct inc_cmd *) buf;
            fprintf (stderr, "message for clusters: virt_id=%u vehicle=%u len_buf=%u cmd=0x%llx\n",
              in->virt_id, in->vehicle_id, in->len_buf, in->cmd);
          }
          else
            fprintf (stderr, "message for gdbserver\n");
        }

        if (ret == CHAR_MUX_CLUSTERS)
        {
          int len_buf = ((struct inc_cmd *) buf)->len_buf;
          if (pkg_size != SZ_INC + len_buf)
          {
            fprintf (stderr, "Error: message size %d for debug server driver instead of %d + %d\n",
              pkg_size, SZ_INC, len_buf);
            ret = -1;
            break;
          }

          if ((nb = write (fd_clusters, buf, pkg_size)) != pkg_size)
          {
            fprintf (stderr, "Error: %d bytes written instead of %d to debug server driver file\n", nb, pkg_size);
            ret = -1;
            break;
          }
        }
        break;
      }
    }
  } // while (1)

  return ret;
}

static int
mux_poll_loop (void)
{
  int active, nmuxed, ret = 1;
  enum {MUX_HOST_FD = 0, MUX_GDBSERVER_FD, MUX_CLUSTERS_FD};
  struct pollfd pollfds[3];

  pollfds[MUX_HOST_FD].fd = fd_host;
  pollfds[MUX_HOST_FD].events = POLLIN;
  pollfds[MUX_GDBSERVER_FD].fd = fd_gdbserver;
  pollfds[MUX_GDBSERVER_FD].events = POLLIN;
  pollfds[MUX_CLUSTERS_FD].fd = fd_clusters;
  pollfds[MUX_CLUSTERS_FD].events = POLLIN;
  nmuxed = 3;

  while (1)
  {
    pollfds[MUX_HOST_FD].revents = pollfds[MUX_GDBSERVER_FD].revents = pollfds[MUX_CLUSTERS_FD].revents = 0;
    active = poll (pollfds, nmuxed, -1);
    if (active < 0 && errno == EINTR)
      continue;
    if (active < 0 || active == 0)
    {
      if (!exit_received)
        fprintf (stderr, "Error %d (%s): Poll from the host, gdbserver and clusters fds returned %d\n",
          errno, strerror (errno), active);
      break;
    }

    if (check_pollfd_err (pollfds[MUX_HOST_FD].revents, 0, "host"))
      break;
    if (check_pollfd_err (pollfds[MUX_GDBSERVER_FD].revents, exit_received, "clusters"))
      break;
    if (check_pollfd_err (pollfds[MUX_CLUSTERS_FD].revents, exit_received, "clusters"))
      break;

    if (pollfds[MUX_HOST_FD].revents)
    {
      int from = fd_mux_read_dispatch (pollfds[MUX_HOST_FD].fd, buf, sizeof (buf),
        pollfds[MUX_GDBSERVER_FD].fd, pollfds[MUX_CLUSTERS_FD].fd);
      if (from < 0)
        break;
    }

    if (pollfds[MUX_GDBSERVER_FD].revents)
    {
      if (fd_read_write_fd_mux (pollfds[MUX_GDBSERVER_FD].fd, buf, sizeof (buf),
        pollfds[MUX_HOST_FD].fd, 1) < 0)
        break;
    }

    if (pollfds[MUX_CLUSTERS_FD].revents)
    {
      if (fd_read_write_fd_mux (pollfds[MUX_CLUSTERS_FD].fd, buf, sizeof (buf),
        pollfds[MUX_HOST_FD].fd, 0) < 0)
        break;
    }
  }

  if (fd_host >= 0)
    close (fd_host);

  return ret;
}

int
mux_set_gdb_port (int port, int fd_listen_gdbserver)
{
  if ((fd_clusters = open (FILE_CLUSTERS, O_RDWR)) < 0)
  {
    fprintf (stderr, "Error: cannot open the clusters debug file %s\n", FILE_CLUSTERS);
    return -1;
  }

  signal (SIGUSR1, sig_usr1_handler);
  atexit (atexit_inform_peer);

  pid_gdbserver = getpid ();
  if (!(pid_gdb_gdbserver = fork ()))
  {
    // the new process (mux))
    setpgrp ();
    close (fd_listen_gdbserver);

    if ((fd_gdbserver = connect_to_port (port)) >= 0)
      mux_poll_loop ();

    if (fd_gdbserver >= 0)
      close (fd_gdbserver);
    if (fd_clusters >= 0)
      close (fd_clusters);
    exit (0);
  }

  // the old process (gdbserver)
  if (fd_host)
    close (fd_host);
  fd_host = -1;

  if (pid_gdb_gdbserver == -1)
  {
    fprintf (stderr, "Error %d (%s) while forking in %s\n", errno, strerror (errno), __FUNCTION__);
    return -1;
  }

  return 0;
}

int
mux_open_host_connection (char **pname)
{
  char *t, *t1, *name = *pname;
  int fd;

  debug_mux = (getenv ("DEBUG_MUX") != NULL);
  exit_received = 0;

  if ((t = strchr (name, ':')) != NULL)
  {
    int port = strtol (t + 1, &t1, 10);
    if (t[1] == 0 || *t1 != 0)
    {
      fprintf (stderr, "Error: invalid port name %s\n", name);
      return -1;
    }
    if ((fd = start_listen (&port, 1, 1)) < 0)
      return -1;
  }
  else
  {
    struct stat statbuf;

    if (stat (name, &statbuf) == 0 && (S_ISCHR (statbuf.st_mode) || S_ISFIFO (statbuf.st_mode)))
      fd = open (name, O_RDWR);
    else
    {
      fprintf (stderr, "Error: cannot open remote device\n");
      return -1;
    }

    #ifdef HAVE_TERMIOS
    {
      struct termios termios;
      tcgetattr (fd, &termios);

      termios.c_iflag = 0;
      termios.c_oflag = 0;
      termios.c_lflag = 0;
      termios.c_cflag &= ~(CSIZE | PARENB);
      termios.c_cflag |= CLOCAL | CS8;
      termios.c_cc[VMIN] = 1;
      termios.c_cc[VTIME] = 0;

      tcsetattr (fd, TCSANOW, &termios);
    }
    #endif

    #ifdef HAVE_TERMIO
    {
      struct termio termio;
      ioctl (fd, TCGETA, &termio);

      termio.c_iflag = 0;
      termio.c_oflag = 0;
      termio.c_lflag = 0;
      termio.c_cflag &= ~(CSIZE | PARENB);
      termio.c_cflag |= CLOCAL | CS8;
      termio.c_cc[VMIN] = 1;
      termio.c_cc[VTIME] = 0;

      ioctl (fd, TCSETA, &termio);
    }
    #endif

    #ifdef HAVE_SGTTY
    {
      struct sgttyb sg;

      ioctl (fd, TIOCGETP, &sg);
      sg.sg_flags = RAW;
      ioctl (fd, TIOCSETP, &sg);
    }
    #endif

    fprintf (stderr, "Remote debugging using %s\n", name);
  }

  *pname = "127.0.0.1:1234";

  fd_host = fd;
  return 0;
}
