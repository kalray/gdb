/**
 *  @file stub.c
 *
 *  @section LICENSE
 *  Copyright (C) 2009 Kalray
 *  @author Frederic, RISS frederic.riss@kalray.eu
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <netinet/tcp.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <unistd.h>
#include <pthread.h>
#include <poll.h>
#include <ctype.h>
#include <string.h>
#include <readline/chardefs.h>

#include "gdbstub.h"
#include "debug_agent.h"

#if 1
#define DEBUG(...) do { } while(0)
#else
#define DEBUG(...) fprintf(stderr, __VA_ARGS__)
#endif

#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x)

#include STRINGIFY(ARCH-regs.h)

#define MAX_PACKET_SIZE 0x4000

#define D_AGENT stub->agents[stub->data_context.agent].agent
#define E_AGENT stub->agents[stub->exec_context.agent].agent
#define D_VEHICLE stub->data_context.vehicle
#define E_VEHICLE stub->exec_context.vehicle
#define D_CONTEXT D_AGENT, D_VEHICLE
#define E_CONTEXT E_AGENT, E_VEHICLE

#define NEED_REAL_D_CONTEXT do { if (D_AGENT < 0 || D_VEHICLE < 0) {	\
			stub->error = "Need a real data context.";					\
			return send_err (stub, 0) ;										\
		}} while (0)

	static char tohex[] = "0123456789abcdef";

typedef struct str {
    char *buf;
    int len, capacity;
} *str_t;

static str_t new_str ()
{
    str_t str = malloc (sizeof (struct str));
    str->buf = malloc (100);
    str->len = 0;
    str->capacity = 99;
    str->buf[0] = '\0';

    return str;
}

static void delete_str (str_t str)
{
    free (str->buf);
    free (str);
}

__attribute__((unused))
static void reset_str (str_t str)
{
    str->len = 0;
    str->buf[0] = '\0';
}

static int str_printf (str_t str, char *fmt, ...)
    __attribute__((format(printf, 2, 3)));  /* 2=format 3=params */

static int str_printf (str_t str, char *fmt, ...) 
{
    int len, max_len;
    va_list ap;

 again:
    max_len = str->capacity - str->len;
    va_start(ap, fmt);
    len = vsnprintf(str->buf + str->len, max_len, fmt, ap);
    va_end(ap);

    if (len >= max_len) {
	str->buf = realloc(str->buf, str->capacity * 2);
	str->capacity *= 2;
	goto again;
    }

    str->len += len;

    return len;
}

struct context {
    int agent;
    int vehicle;
};

struct stopped_context {
    struct context ctxt;
    exec_state_t   state;
    int            val;

    int            notified;
    int            used;

    struct stopped_context *next;
};

struct agent {
    int id;
    debug_agent_t *agent;

    char **vehicles;
    int    nbvehicles;
    int    attached;

    char  *reg_desc_xml;

    struct stopped_context *contexts;
    unsigned char vehicles_modes_seen[17];
    struct agent *ddr_peer_united_io;
    struct agent *eth_peer_united_io;
};

struct gdbstub {
    /* Debug agents */
    struct agent *agents;
    int nb_agents;

    /* Debug loop handling */
    pthread_t control_thread;
    int event_fd_read;
    int event_fd_write;

    /* GDB communications */
    int infd, outfd;
    union {
	char raw_data[MAX_PACKET_SIZE+1];
	struct {
	    char header;
	    char payload[MAX_PACKET_SIZE-1];
	};
    };
    unsigned int data_len;

    /* state */
    core_register_descr_t       *registers;
    int                     nbregs;
    int                    *all_reg_ids;
    const char             *error;
    int                     thread_iter;
    struct context          data_context;
    struct context          exec_context;
    bool                    attached;
    bool                    pending_stop_notification;
    struct stopped_context *stopped_contexts, *stopped_contexts_tail;
    struct stopped_context *stopped_contexts_iter;
    struct stopped_context *unattached_stopped_contexts;
    char *osdata;
    char *threads;

    bool exited;
    int exit_value;

    /* options */
    bool no_acks;
    bool non_stop;
    bool async;
    int  debug;
    bool skip_exit_when_stub_exited;
};

static void *control_thread (void *void_stub);

static bool printf_packet(struct gdbstub *stub, char *fmt, ...)
    __attribute__((format(printf, 2, 3)));  /* 2=format 3=params */


static struct stopped_context *insert_stopped_context (struct gdbstub *stub, 
						       int agent, int vehicle) {
    exec_state_t state;
    int val;
    struct stopped_context *ctxt;

    if (debug_agent_is_executing (stub->agents[agent].agent, vehicle, 
				  &state, &val) != RET_OK
	|| state == EXEC_RUNNING || state == EXEC_POWEROFF)
	return NULL;

    if (agent >= stub->nb_agents
        || vehicle >= stub->agents[agent].nbvehicles) {
        fprintf (stderr, "Inconsistent data passed to insert_stopped_context: %i:%i\n", 
                 agent, vehicle);
        return NULL;
    }
    
    ctxt = stub->agents[agent].contexts + vehicle;

    if (ctxt->notified || ctxt->used) {
        return ctxt;
    }
    if (ctxt->next) {
        return ctxt;
    }

    ctxt->state = state;
    ctxt->val = val;

    ctxt->used = 1;

    if (stub->agents[agent].attached == 0) {
        ctxt->next = stub->unattached_stopped_contexts;
        stub->unattached_stopped_contexts = ctxt;
    } else if (stub->stopped_contexts_tail) {
	stub->stopped_contexts_tail->next = ctxt;
        stub->stopped_contexts_tail = ctxt;
    } else {
	stub->stopped_contexts = ctxt;
	stub->stopped_contexts_iter = ctxt;
        stub->stopped_contexts_tail = ctxt;
    }

    return ctxt;
}

int gdbstub_listen(int port, int port_end, struct gdbstub *stub)
{
    struct   sockaddr_in sin;
    int sd;

    /* get an internet domain socket */
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
	perror("socket");
	return -1;
    }
    
    /* complete the socket structure */
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    
    /* bind the socket to the port number */
    do {
	sin.sin_port = htons(port);
	if (bind(sd, (struct sockaddr *) &sin, sizeof(sin)) == -1)
	    ++port;
	else
	    break;
    } while (port <= port_end);

    if (port > port_end)
	return -1;
    
    /* show that we are willing to listen */
    if (listen(sd, 1) == -1) {
	perror("listen");
	return -1;
    }
    
    stub->infd = stub->outfd = sd;
    return port;
}

void gdbstub_accept(struct gdbstub *stub)
{
    struct   sockaddr_in pin;
    socklen_t addrlen;
    int sd_current;
    socklen_t tmp;

    /* wait for a client to talk to us */
    addrlen = sizeof(pin); 
    if ((sd_current = accept(stub->infd, 
			     (struct sockaddr *)  &pin, &addrlen)) == -1) {
	perror("accept");
    }
    
    /* Enable TCP keep alive process. */
    tmp = 1;
    setsockopt (sd_current, SOL_SOCKET, SO_KEEPALIVE,
		(char *) &tmp, sizeof (tmp));
    
  /* Tell TCP not to delay small packets.  This greatly speeds up
     interactive response. */
    tmp = 1;
    setsockopt (sd_current, IPPROTO_TCP, TCP_NODELAY,
		(char *) &tmp, sizeof (tmp));
    
    stub->infd = stub->outfd = sd_current;
}

static void notify_callback(debug_agent_t *da, int vehicle, void *notify_data)
{
  struct gdbstub *stub = notify_data;
  int i;

  for (i = 0; i < stub->nb_agents; ++i)
  {
    if (stub->agents[i].agent == da)
      break;
  }

  if (i == stub->nb_agents)
  {
    fprintf (stderr, "Unknown agent notifying the gdbstub!\n");
    return;
  }

  gdbstub_notify_context_stop (stub, i, vehicle);
}

static void notify_cpu_level_seen_callback (debug_agent_t *da, int vehicle,
  void *notify_data, unsigned char mode)
{
  struct gdbstub *stub = notify_data;
  int i;

  for (i = 0; i < stub->nb_agents; ++i)
  {
    if (stub->agents[i].agent == da)
      break;
  }

  if (i == stub->nb_agents)
  {
    fprintf (stderr, "Unknown agent notifying the gdbstub about user mode!\n");
    return;
  }
  
  stub->agents[i].vehicles_modes_seen[vehicle] |= mode;
}

static void gdbstub_build_thread_list (struct gdbstub *stub)
{
  str_t str;
  int i, j, rm_idx;

  str = new_str ();
  str_printf (str, "<?xml version=\"1.0\"?>\n<threads>\n");
  
  for (i = 0; i < stub->nb_agents; ++i)
  {
    if (stub->agents[i].attached == 0 || stub->agents[i].ddr_peer_united_io)
      continue;

    /* Make the resource manager the first in the list. */
    rm_idx = debug_agent_get_rm_idx (stub->agents[i].agent);

    str_printf (str, "<thread id =\"p%x.%x\">%s</thread>\n",
      i + 1, rm_idx + 1, stub->agents[i].vehicles[rm_idx]);
    
    for (j = 0; j < stub->agents[i].nbvehicles; j++)
      if (j != rm_idx)
        str_printf (str, "<thread id =\"p%x.%x\">%s</thread>\n",
          i + 1, j + 1, stub->agents[i].vehicles[j]);
    
    if (stub->agents[i].eth_peer_united_io)
    {
      struct agent *eth = stub->agents[i].eth_peer_united_io;
      for (j = 0; j < eth->nbvehicles; j++)
        str_printf (str, "<thread id =\"p%x.%x\">%s</thread>\n", 
          i + 1, j + stub->agents[i].nbvehicles + 1, eth->vehicles[j]);
    }
  }
  str_printf (str, "</threads>");
  stub->threads = strdup (str->buf);

  delete_str (str);
}

static void
gdbstub_build_osdata (struct gdbstub *stub)
{
    str_t str;
    int i;
    
    str = new_str ();
    str_printf (str, "<?xml version=\"1.0\"?>\n"
		"<!DOCTYPE target SYSTEM \"osdata.dtd\">\n"
		"<osdata type=\"processes\">\n");
  if (stub->nb_agents == 1 && stub->agents[0].nbvehicles == 1)
  {
    str_printf (str, "<item>\n"
      "<column name=\"pid\">1</column>\n"
      "<column name=\"command\">%s</column>\n"
      "</item>\n",
      stub->agents[0].agent->attributes.elf_file ?: "ISS");
  }
  else
  {
    exec_state_t state;
    int val;
        
    for (i = 0; i < stub->nb_agents; ++i) 
    {
      if (stub->agents[i].ddr_peer_united_io)
        continue;

      debug_agent_is_executing (stub->agents[i].agent, debug_agent_get_rm_idx (stub->agents[i].agent), &state, &val);
      if (state == EXEC_POWEROFF && stub->agents[i].eth_peer_united_io)
        debug_agent_is_executing (stub->agents[i].eth_peer_united_io->agent, 0, &state, &val);

      str_printf (str, "<item>\n"
        "<column name=\"pid\">%i</column>\n"
        "<column name=\"cluster\">%s %s</column>\n"
	"<column name=\"running\">%s</column>\n"
	"<column name=\"command\">%s</column>\n"
	"</item>\n", i + 1,
        debug_agent_get_cluster_name (stub->agents[i].agent),
	debug_agent_get_desc (stub->agents[i].agent) ?: "",
        state == EXEC_POWEROFF ? "no" : "yes",
        stub->agents[i].agent->attributes.elf_file ?: "");
    }
  }
    
  str_printf (str, "</osdata>");
  stub->osdata = strdup (str->buf);
  delete_str (str);
}

static int
has_stopped_contexts (const struct agent *agent)
{
    int i, val;
    exec_state_t state = EXEC_RUNNING;

    for (i = 0; i < agent->nbvehicles 
             &&  (state == EXEC_RUNNING 
                  || state == EXEC_POWEROFF); ++i)
        debug_agent_is_executing (agent->agent, i, &state, &val);
        
    return state != EXEC_RUNNING && state != EXEC_POWEROFF;
}

static int compare_da_ids (const void *e1, const void *e2)
{
    const struct agent *a1 = e1, *a2 = e2;
    int context_diff;

    context_diff = has_stopped_contexts (a2) - has_stopped_contexts (a1);
    if (context_diff)
        return context_diff;

    return debug_agent_get_id (a1->agent) - debug_agent_get_id (a2->agent);
}

struct gdbstub *gdbstub_init(debug_agent_t *agents)
{
    core_register_descr_t *reg;
    char **vehicle;
    int i, j;
    struct gdbstub *res;
    int nb_agents = 1;
    debug_agent_t *it = agents;

    res = calloc (sizeof(*res), 1);
    res->skip_exit_when_stub_exited = false;

    while (it->next) {
        nb_agents++;
        it = it->next;
    }

    res->agents = malloc (sizeof(*res->agents) * nb_agents);
    res->nb_agents = nb_agents;

    for (i = 0, it = agents; i < nb_agents; ++i, it = it->next) {
  memset (&res->agents[i].vehicles_modes_seen, 0, sizeof (res->agents[i].vehicles_modes_seen));
	res->agents[i].id = i;
	res->agents[i].agent = it;
  res->agents[i].reg_desc_xml = NULL;
	vehicle = debug_agent_get_vehicles (it);
	res->agents[i].attached = 0;
  res->agents[i].ddr_peer_united_io = NULL;
  res->agents[i].eth_peer_united_io = NULL;

	res->agents[i].vehicles = vehicle; 
	while (*vehicle) ++vehicle;
	res->agents[i].nbvehicles = vehicle - res->agents[i].vehicles;
	it->attributes.notify_stop = notify_callback;
	it->attributes.notify_cpu_level_seen = notify_cpu_level_seen_callback;
	it->attributes.notify_data = res;
    res->agents[i].attached = 0;
    res->agents[i].contexts = calloc (res->agents[i].nbvehicles, 
                                      sizeof(struct stopped_context));
    }
        
    qsort (res->agents, nb_agents, sizeof(*res->agents), compare_da_ids);

    for (i = 0; i < nb_agents; ++i)
    {
      int j;
      it = res->agents[i].agent;
      if (it->attributes.da_eth_peer_united_io)
      {
        for (j = 0; j < nb_agents; j++)
          if (res->agents[j].agent == it->attributes.da_eth_peer_united_io)
          {
            res->agents[i].eth_peer_united_io = &res->agents[j];
            break;
          }
      }
      
      if (it->attributes.da_ddr_peer_united_io)
      {
        for (j = 0; j < nb_agents; j++)
          if (res->agents[j].agent == it->attributes.da_ddr_peer_united_io)
          {
            res->agents[i].ddr_peer_united_io = &res->agents[j];
            break;
          }
      }
    }
    
    res->agents[0].attached = 1;
    if (res->agents[0].eth_peer_united_io)
      res->agents[0].eth_peer_united_io->attached = 1;
    res->async = debug_agent_is_async (agents);

    for (i = 0, it = agents; i < nb_agents; ++i, it = it->next) {
        for (j = 0; j  <res->agents[i].nbvehicles; ++j) {
            res->agents[i].contexts[j].ctxt.agent = i;
            res->agents[i].contexts[j].ctxt.vehicle = j;
        }
    }

    /* FIXME: support heterogeneous clients. */
    reg = res->registers = debug_agent_register_names (agents, 0);
    while (reg->name) ++reg;
    res->nbregs = reg - res->registers;

    reg = malloc (sizeof(*reg)*res->nbregs);
    memcpy (reg, res->registers, sizeof(*reg)*res->nbregs);
    res->registers = reg;

    res->all_reg_ids = malloc(sizeof(int)*(res->nbregs+1));
    for (i = 0; i < res->nbregs; ++i)
	res->all_reg_ids[i] = res->registers[i].index;
    
    res->all_reg_ids[res->nbregs] = -1;
    
    gdbstub_build_osdata (res);
    gdbstub_build_thread_list (res);

    res->data_context.agent = res->exec_context.agent = 0;
	res->data_context.vehicle = res->exec_context.vehicle = 0;

    if (pipe (&res->event_fd_read)) {
	perror ("pipe");
	exit (-1);
    }

    return res;
}

void gdbstub_start(struct gdbstub *stub, bool thread)
{
    int i, j;

    for (i = 0; i < stub->nb_agents; ++i)
	for (j = 0; j < stub->agents[i].nbvehicles; ++j)
	    insert_stopped_context (stub, i, j);

    if (thread) {
	if (pthread_create (&stub->control_thread, NULL, control_thread, stub)) {
	    perror ("pthread_create");
	    exit (-1);
	}
    } else {
	control_thread (stub);
    }

}

const char *get_error(struct gdbstub *stub)
{
    return stub->error;
}

static bool get_message (struct gdbstub *stub)
{
    int len;

 again:
    stub->data_len = len = read (stub->infd, stub->raw_data, 1);

    if (len <= 0) {
	stub->error = "Connection closed";
	return false;
    } else if (len == 1 && stub->raw_data[0] == '+') {
	goto again;
    } else if (len == 1 && stub->raw_data[0] == '\003') {
	return true;
    }

    /* Each command should start with '$' and end with '#nn' */
    if (stub->header != '$') {
	stub->error = "Malformed packet (does not start with '$')";
	return false;
    }

    while (stub->data_len < 5 || stub->raw_data[stub->data_len - 3] != '#') {
	len = read (stub->infd, stub->raw_data + stub->data_len, 1);
	
	if (len <= 0) {
	    stub->error = "Connection closed";
	    return false;
	}
	
	stub->data_len += len;
    }
    
    stub->raw_data[stub->data_len] = 0;
    DEBUG("GOT: '%s'\n", stub->raw_data);

    /* Here we could verify the checksum. But as we use only reliable
       transports... */
    if (stub->no_acks)
	return true;
    
    DEBUG("SENDING ACK\n");
    if (write(stub->outfd, "+", 1) != 1) {
	stub->error = "Connection closed";
	return false;
    }
    DEBUG("DONE ACK\n");
    
    return true;
}

static bool send_answer (struct gdbstub *stub)
{
    unsigned char *data = (unsigned char*)stub->payload;
    int i;
    unsigned char csum = 0, c;

    for (i = 0; i < stub->data_len-1; ++i)
	csum += *data++;
    printf_packet(stub, "#%2x", csum);

    stub->raw_data[stub->data_len] = 0;
    DEBUG ("SENDING: '");
    for (i = 0; i < stub->data_len; ++i) {
	if (isprint(stub->raw_data[i]))
	    DEBUG("%c", stub->raw_data[i]);
	else
	    DEBUG("\\%i", stub->raw_data[i]);
    }
    DEBUG ("'\n");

    if (write(stub->outfd, stub->raw_data, stub->data_len) != stub->data_len) {
	stub->error = "Connection closed";
	return false;
    }

    if (stub->no_acks)
	return true;

    DEBUG ("GETTING ACK;\n");
    if (read(stub->infd, &c, 1) != 1 || c != '+') {
	stub->error = "Communication error";
	return false;
    }
    DEBUG ("ACK;\n");

    return true;
}

static int starts_with (char *pattern, char *string) {
    return ! strncmp (pattern, string, strlen (pattern));
}

static int fromhex_helper (char c) {
    if (c <= '9') return c - '0';
    if (c <= 'F') return c - 'A' + 10;
    return c - 'a' + 10;
}

static int fromhex(char *c) {
    int c1, c2;
    
    c1 = fromhex_helper (c[0]);
    c2 = fromhex_helper (c[1]);
    
    return (c1 << 4) | c2;
}

static void prepare_to_answer(struct gdbstub *stub) 
{
    stub->data_len = 1;
    stub->header = '$';
}

static bool printf_packet(struct gdbstub *stub, char *fmt, ...)
{
    int len;
    va_list ap;

    va_start(ap, fmt);
    len = vsprintf(stub->raw_data + stub->data_len, fmt, ap);
    va_end(ap);

    stub->data_len += len;
    if (stub->data_len >= MAX_PACKET_SIZE - 4) {
	stub->error = "Overflow in answer packet construction";
	return false;
    }

    return true;
}

static bool nprintf_packet(struct gdbstub *stub, int size, char *fmt,...)
    __attribute__((format(printf, 3, 4)));  /* 2=format 3=params */

static bool nprintf_packet(struct gdbstub *stub, int size, char *fmt, ...)
{
    int len;
    va_list ap;

    va_start(ap, fmt);
    len = vsnprintf(stub->raw_data + stub->data_len, size, fmt, ap);
    va_end(ap);

    if (len >= size) len = size-1;
    stub->data_len += len;
    if (stub->data_len >= MAX_PACKET_SIZE - 4) {
	stub->error = "Overflow in answer packet construction";
	return false;
    }

    return true;
}

static int send_ok(struct gdbstub *stub) 
{
    stub->data_len = 3;
    stub->header = '$';
    stub->payload[0] = 'O';
    stub->payload[1] = 'K';
    return send_answer (stub);
}

static int send_err(struct gdbstub *stub, int quiet) 
{
	if(!quiet)
		printf ("Sending error reply. Stub error is:\n\t%s\n", stub->error);

    stub->header = '$';
    stub->data_len = 4;
    stub->payload[0] = 'E';
    stub->payload[1] = '0';
    stub->payload[2] = '0';
    return send_answer (stub);
}

static int send_unsupported(struct gdbstub *stub) 
{
    stub->data_len = 1;
    stub->header = '$';
    return send_answer (stub);
}


static bool parse_thread_id (struct gdbstub *stub, struct context *ctxt, char *id, char **end)
{
  char *endptr;

  ctxt->agent = 0;
  ctxt->vehicle = 0;

  if (id[0] == 'p')
  {
    ctxt->agent = strtol (id+1, &endptr, 16) - 1;
    if (*endptr != '.')
      return false;

    ctxt->vehicle = strtol (endptr+1, &endptr, 16) - 1;
    if (ctxt->agent >= 0 && ctxt->agent < stub->nb_agents)
    {
      struct agent *ag = &stub->agents[ctxt->agent];
      if (ag->eth_peer_united_io && ctxt->vehicle >= ag->nbvehicles)
      {
        ctxt->vehicle -= ag->nbvehicles;
        ctxt->agent = ag->eth_peer_united_io->contexts[0].ctxt.agent;
      }
    }
  }
  else
  {
    ctxt->vehicle = strtol(id, &endptr, 16) - 1;
    if (endptr == id)
      return false;
  }

  if (end != NULL) *end = endptr;

  return true;
}

static void set_exec_context (struct gdbstub *stub, struct context ctxt) {

    if (ctxt.agent == -1)
	ctxt.agent = 0;
    if (ctxt.vehicle == -1)
	// ?????
	// ctxt.vehicle = stub->agents[ctxt.agent].nbvehicles-1;
	ctxt.vehicle = 0;

    stub->exec_context = ctxt;
}

static void set_data_context (struct gdbstub *stub, struct context ctxt) {

    if (ctxt.agent == -1)
	ctxt.agent = 0;
    if (ctxt.vehicle == -1)
	// ?????
	//ctxt.vehicle = stub->agents[ctxt.agent].nbvehicles-1;
	ctxt.vehicle = 0;

    stub->data_context = ctxt;
}

static bool handle_g (struct gdbstub *stub)
{
  reg_t buf[stub->nbregs];
    int i;

    prepare_to_answer(stub);
    NEED_REAL_D_CONTEXT;
    memset (buf, 0, stub->nbregs * sizeof (reg_t));

    /* We don't check return code of debug_agent_read_registers because
     * we do not return an error since GDB will sometimes ask for the
     * registers of stopped contexts (most importantly at attach time).
     */
    debug_agent_read_registers(D_CONTEXT,
			       stub->all_reg_ids, buf, stub->nbregs);

    for (i = 0 ; i < stub->nbregs; ++i){
      if (stub->registers[i].name[0] != '\0'){
	unsigned char *c = (unsigned char*)&(buf[i].u32);
	printf_packet(stub, "%c%c%c%c%c%c%c%c", 
		      tohex[c[0] >> 4], tohex[c[0] & 0xf], 
		      tohex[c[1] >> 4], tohex[c[1] & 0xf], 
		      tohex[c[2] >> 4], tohex[c[2] & 0xf], 
		      tohex[c[3] >> 4], tohex[c[3] & 0xf]); 
      }
    }
    return send_answer(stub);
}

static bool handle_p (struct gdbstub *stub)
{
    errcode_t err;
    char *endptr;
    reg_t reg_buf;
    char  *buf = (char*)&(reg_buf.u32);

    int i;

    i = strtol((char*)stub->payload + 1, &endptr, 16);
    if (*endptr != '#') {
		stub->error = "Malformed p packet";
		return send_err (stub, 0) ;
    }
    NEED_REAL_D_CONTEXT;
    err = debug_agent_read_register(D_CONTEXT, stub->all_reg_ids[i], &reg_buf);
    
    if (err != RET_OK) {
		stub->error = "Error during read_register";
		return send_err (stub, 0) ;
    }
    
    prepare_to_answer(stub);
    
    printf_packet(stub, "%c%c%c%c%c%c%c%c", 
		  tohex[buf[0] >> 4], tohex[buf[0] & 0xf], 
		  tohex[buf[1] >> 4], tohex[buf[1] & 0xf], 
		  tohex[buf[2] >> 4], tohex[buf[2] & 0xf], 
		  tohex[buf[3] >> 4], tohex[buf[3] & 0xf]); 
    
    return send_answer(stub);
}

static bool handle_P (struct gdbstub *stub)
{
    errcode_t err;
    char *endptr, *val;
    unsigned int reg;
    reg_t reg_buf;
    char  *buf = (char*)&(reg_buf.u32);

    NEED_REAL_D_CONTEXT;
    reg = strtol((char*)stub->payload + 1, &val, 16);
    if (*val != '=') {
		stub->error = "Malformed P packet (reg)";
		return send_err (stub, 0) ;
    }

    strtol(val + 1, &endptr, 16);
    if (*endptr != '#') {
		stub->error = "Malformed P packet (val)";
		return send_err (stub, 0) ;
    }

    reg_buf.u64 = 0;
    buf[0] = fromhex(val+1);
    buf[1] = fromhex(val+3);
    buf[2] = fromhex(val+5);
    buf[3] = fromhex(val+7);

    err = debug_agent_write_register(D_CONTEXT, stub->all_reg_ids[reg], reg_buf);
    
    if (err != RET_OK) {
		stub->error = "Error during write_register";
		return send_err (stub, 0) ;
    }

    return send_ok (stub);
}

static bool handle_qSupported (struct gdbstub *stub)
{
    prepare_to_answer(stub);

    printf_packet(stub,
		  "qXfer:features:read+;"
		  "PacketSize=%x;"
		  "QStartNoAckMode+;"
		  "multiprocess+;"
		  "QNonStop+;"
		  "qXfer:threads:read+;"
		  "qXfer:osdata:read+",
		  MAX_PACKET_SIZE - 4);

    return send_answer(stub);
}

static void update_registers_xml (struct gdbstub *stub, struct agent *ag)
{
  struct reg_desc *desc;
  const char *core;
  int i, j;
  str_t str;
    
  if (ag->reg_desc_xml != NULL)
    return;

  core = debug_agent_get_core (ag->agent);
  desc = get_register_descriptions (core);
  
  if (desc == NULL)
  {
    fprintf (stderr, "Unable to find register description for core %s.", core);
    return;
  }
  
  str = new_str ();

  str_printf (str, 
    "<?xml version=\"1.0\"?>\n"
    "<!DOCTYPE target SYSTEM \"gdb-target.dtd\">\n"
    "<target version=\"1.0\">\n"
    "    <architecture>%s</architecture>\n"
    "    <feature name=\"eu.kalray.core.%s\">\n",
    core, core);

  for (i = 0; i < stub->nbregs; ++i)
  {
    if (stub->registers[i].name[0] == '\0')
      continue;

    for (j = 0; desc[j].name != NULL; ++j)
    {
      if (strcmp (desc[j].name, stub->registers[i].name))
    		continue;

	    str_printf (str, desc[j].desc, i);
	    break;
    }

  	if (desc[j].name == NULL)
      stub->registers[i].name = "";
  }

  str_printf (str, "</feature></target>");
  ag->reg_desc_xml = strdup (str->buf);
  delete_str (str);
}

static bool handle_qXfer_features_read (struct gdbstub *stub)
{
    char *endptr;
    int length, offset;
    struct agent *ag;

    if (strncmp ("target.xml:", (char*)stub->payload + 20, 11)) {
	prepare_to_answer (stub);
	return send_answer (stub);
    }

    offset = strtol((char*)stub->payload + 31, &endptr, 16);
    if (*endptr != ',') {
		stub->error = "Malformed qXfer:threads:osdata packet (offset)";
		return send_err (stub, 0) ;
    }

    length = strtol(endptr+1, &endptr, 16);
    if (*endptr != '#') {
		stub->error = "Malformed qXfer:threads:osdata packet (length)";
		return send_err (stub, 0) ;
    }

    prepare_to_answer (stub);

    ag = &stub->agents[stub->data_context.agent];
    update_registers_xml (stub, ag);

    if (offset >= strlen (ag->reg_desc_xml))
	nprintf_packet (stub, length, "l");
    else
	nprintf_packet (stub, length, "m%s", ag->reg_desc_xml + offset);

    return send_answer (stub);
}

static bool handle_qXfer_osdata_read (struct gdbstub *stub)
{
    char *endptr;
    int length, offset;

    if (strncmp (":", (char*)stub->payload + 18, 1)) {
	prepare_to_answer (stub);
	return send_answer (stub);
    }

    offset = strtol((char*)stub->payload + 19, &endptr, 16);
    if (*endptr != ',') {
		stub->error = "Malformed qXfer:threads:osdata packet (offset)";
		return send_err (stub, 0) ;
    }

    length = strtol(endptr+1, &endptr, 16);
    if (*endptr != '#') {
		stub->error = "Malformed qXfer:threads:osdata packet (length)";
		return send_err (stub, 0) ;
    }

    if (offset == 0)
        gdbstub_build_osdata (stub);

    prepare_to_answer (stub);
    if (offset >= strlen (stub->osdata))
	nprintf_packet (stub, length, "l");
    else
	nprintf_packet (stub, length, "m%s", stub->osdata + offset);

    return send_answer (stub);
}

static bool handle_qXfer_threads_read (struct gdbstub *stub)
{
    char *endptr;
    int length, offset;

    offset = strtol((char*)stub->payload + 20, &endptr, 16);
    if (*endptr != ',') {
		stub->error = "Malformed qXfer:threads:read packet (offset)";
		return send_err (stub, 0) ;
    }

    length = strtol(endptr+1, &endptr, 16);
    if (*endptr != '#') {
		stub->error = "Malformed qXfer:threads:read packet (length)";
		return send_err (stub, 0) ;
    }

    prepare_to_answer (stub);
    if (offset >= strlen (stub->threads))
	nprintf_packet (stub, length, "l");
    else
	nprintf_packet (stub, length, "m%s", stub->threads + offset);

    return send_answer (stub);
}

static bool handle_G (struct gdbstub *stub)
{
  reg_t reg_buf;
  char *buf = (char*)&(reg_buf.u32);

    int i, j;
    errcode_t err;

    NEED_REAL_D_CONTEXT;
    for (i = 0, j = 0; i < stub->nbregs; ++i) {
		if (stub->registers[i].name[0] == 0) continue;
		reg_buf.u64 = 0;
		buf[0] = fromhex (stub->payload + 1 + 0 + j*8);
		buf[1] = fromhex (stub->payload + 1 + 2 + j*8);
		buf[2] = fromhex (stub->payload + 1 + 4 + j*8);
		buf[3] = fromhex (stub->payload + 1 + 6 + j*8);

		err = debug_agent_write_register(D_CONTEXT, i, reg_buf);
		++j;
		if (err != RET_OK) {
			prepare_to_answer (stub);
			return send_err (stub, 0) ;
		}

    }

    prepare_to_answer (stub);
    return send_ok (stub);
}

static errcode_t read_memory(debug_agent_t *da,
			     int vehicle, unsigned int addr, void *buf, int buf_size) {
    return debug_agent_read_dcache (da, vehicle, addr, buf, buf_size);
}

static errcode_t write_memory(debug_agent_t *da,
			      int vehicle, unsigned int addr, void *buf, int buf_size) {
    return debug_agent_write_dcache (da, vehicle, addr, buf, buf_size);
}

static bool kalray_is_hot_attached (struct gdbstub *stub)
{
  int ha = debug_agent_is_hot_attached (stub->agents[0].agent);
  
  prepare_to_answer (stub);
  
  stub->payload[0] = ha ? '1' : '0';
  stub->data_len = 1 + 1;
  return send_answer (stub);
}

static bool kalray_set_debug_level (struct gdbstub *stub)
{
  int level = * (char *) (stub->payload + 2) - '0';
  
  if (* (char *) (stub->payload + 3) != '#')
  {
    stub->error = "Invalid kD (set_debug_level) packet!";
		return send_err (stub, 0);
  }

  //printf ("stub received debug level %d.\n", i);
  debug_agent_set_debug_level (D_CONTEXT, level, 0 /*postpone*/);
  
  return send_ok (stub);
}

static bool kalray_set_postponed_debug_level (struct gdbstub *stub)
{
  int level = * (char *) (stub->payload + 2) - '0';
  struct context ctxt;

  if (!parse_thread_id (stub, &ctxt, stub->payload + 3, NULL))
  {
    stub->error = "Malformed thread id in kP packet";
    return send_err (stub, 0) ;
  }
  
  //printf ("postpone debug level %d for p %d.%d\n", level, ctxt.agent, ctxt.vehicle);
  debug_agent_set_debug_level (stub->agents[ctxt.agent].agent,
    ctxt.vehicle, level, 1 /* postpone */);
  
  return send_ok (stub);
}

static bool kalray_get_os_supported_debug_level (struct gdbstub *stub)
{
  struct context ctxt;
  
  prepare_to_answer (stub);
  if (!parse_thread_id (stub, &ctxt, stub->payload + 2, NULL))
  {
    stub->error = "Malformed thread id in km packet";
    return send_err (stub, 0) ;
  }
  //printf ("Agent %d  os_supported_debug_mode %d\n", ctxt.agent,
  //  stub->agents[ctxt.agent].agent->attributes.os_supported_debug_mode);
  stub->payload[0] = '0' + stub->agents[ctxt.agent].agent->attributes.os_supported_debug_mode;
  stub->data_len = 1 + 1;
  return send_answer (stub);
}

static bool kalray_get_intsys_handlers (struct gdbstub *stub)
{
  int i ,size = 4 * 3;
  char buf[size];
  int *pint_buf = (int *) buf;
  pint_buf[0] = 0xFFFFFFFF;
  pint_buf[1] = 0xFFFFFFFF;
  pint_buf[2] = 0xFFFFFFFF;
  
  /*errcode_t ret =*/ debug_agent_get_intsys_handlers (D_CONTEXT, buf, 12);
  
  prepare_to_answer (stub);
  
  for (i = 0; i < size; ++i)
  {
		stub->payload[i * 2] = tohex[(buf[i] >> 4) & 0xf];
		stub->payload[i * 2 + 1] = tohex[buf[i] & 0xf];
  }
  stub->data_len = 1 + size * 2;

  return send_answer (stub);
}

static bool kalray_get_device_list (struct gdbstub *stub)
{
  char *sep, *device_full_name, *list = NULL;
  long offset = 0, length = 0;

  offset = strtol (stub->payload + 3, &sep, 16); // format: kl:<ofs>,<size>:<device_full_name>
  if (*sep != ',')
    goto err;

  length = strtol (sep + 1, &sep, 16);
  if (*sep != ':')
    goto err;

  device_full_name = sep + 1;
  sep = strchr (device_full_name, '#');
  if (!sep)
    goto err;
  *sep = 0;

  prepare_to_answer (stub);
  list = debug_agent_get_device_list (stub->agents[0].agent, device_full_name);
  if (!list || offset >= strlen (list))
    nprintf_packet (stub, length, "l");
  else
    nprintf_packet (stub, length, "m%s", list + offset);

  if (list)
    free (list);

  return send_answer (stub);

err:
  stub->error = "Invalid kl (get_device_list) packet!";
  return send_err (stub, 0);
}

static bool kalray_set_kwatch (struct gdbstub *stub)
{
  int watch_type = stub->payload[2] - '0'; // format: kW<type><set>:<reg_full_name>
  int bset = stub->payload[3] - '0';
  char *full_name = stub->payload + 5, *err_msg = NULL;

  if (!debug_agent_set_kwatch (stub->agents[0].agent, full_name, watch_type, bset, &err_msg))
    return send_ok (stub);

  prepare_to_answer (stub);
  nprintf_packet (stub, 290, "E00%s", err_msg ? err_msg : "");
  if (err_msg)
    free (err_msg);
  return send_answer (stub);
}

static bool kalray_get_vehicle_modes_seen (struct gdbstub *stub)
{
  struct context ctxt;

  if (!parse_thread_id (stub, &ctxt, stub->payload + 2, NULL))
  {
    stub->error = "Malformed thread id in km packet";
    return send_err (stub, 0) ;
  }

  prepare_to_answer (stub);
  stub->payload[0] = '0' + stub->agents[ctxt.agent].vehicles_modes_seen[ctxt.vehicle];
  stub->data_len = 1 + 1;

  return send_answer (stub);
}

static bool kalray_set_stop_at_main (struct gdbstub *stub)
{
  int bstop = stub->payload[2] - '0';

  debug_agent_set_stop_at_main (D_AGENT, bstop);
  
  return send_ok (stub);
}

static bool kalray_inform_dsu_stepi_bkp (struct gdbstub *stub)
{
  debug_agent_inform_dsu_stepi_bkp (D_CONTEXT);
  
  return send_ok (stub);
}

static bool kalray_get_cpu_exec_level (struct gdbstub *stub)
{
  int cpu_level = 0;
  
  /*errcode_t ret = */debug_agent_get_cpu_exec_level (D_CONTEXT, &cpu_level);
  
  prepare_to_answer (stub);
  stub->payload[0] = '0' + cpu_level;
  stub->data_len = 1 + 1;

  return send_answer (stub);
}

static bool handle_m (struct gdbstub *stub)
{
    unsigned int addr, len;
    char *endptr;
    unsigned char *buf;
    int i;
    errcode_t err;

    NEED_REAL_D_CONTEXT;
    addr = strtoul((char*)stub->payload + 1, &endptr, 16);
    if (*endptr != ',') {
		stub->error = "Malformed m packet (addr)";
		return send_err (stub, 0) ;
    }

    len = strtoul(endptr + 1, &endptr, 16);
    if (*endptr != '#') {
		stub->error = "Malformed m packet (len)";
		return send_err (stub, 0) ;
    }

    buf = alloca(len); memset (buf, 0, len);
    prepare_to_answer (stub);
    /* Don't check return value. This is done on purpose for the testsuite.
     * The memset above takes care of not returning garbage to GDB. */
    err = read_memory (D_CONTEXT, addr, buf, len);

	if(err == RET_SYSTRAP){
		stub->error = "Bad address\n";
	    return send_err (stub, 1 /* Don't print this */) ;
	}

    for (i = 0; i < len; ++i) {
		stub->payload[i*2] = tohex[(buf[i] >> 4) & 0xf];
		stub->payload[i*2+1] = tohex[buf[i] & 0xf];
    }
    stub->data_len = len*2 + 1;

    return send_answer (stub);
}

static void icache_invalidate (struct gdbstub *stub)
{
  int k, i;
  struct agent *ag = &stub->agents[stub->data_context.agent];

  for (k = 0; k < 2; k++)
  {
    for (i = 0; i < ag->nbvehicles; ++i)
      debug_agent_icache_invalidate (ag->agent, i);
    if (!k)
    {
      if (ag->ddr_peer_united_io)
        ag = ag->ddr_peer_united_io;
      else if (ag->eth_peer_united_io)
        ag = ag->eth_peer_united_io;
      else
        break;
    }
  }

}

static bool handle_M (struct gdbstub *stub)
{
    unsigned int addr, len;
    char *endptr;
    unsigned char *buf;
    int i;
    errcode_t err = RET_OK;

    NEED_REAL_D_CONTEXT;
    addr = strtoul((char*)stub->payload + 1, &endptr, 16);
    if (*endptr != ',') {
		stub->error = "Malformed M packet (addr)";
		return send_err (stub, 0) ;
    }

    len = strtoul(endptr + 1, &endptr, 16);
    if (*endptr != ':') {
		stub->error = "Malformed M packet (len)";
		return send_err (stub, 0) ;
    }

    buf = malloc (len);
    for (i = 0; i < len; ++i) {
		if ((i+1)*2+1 > stub->data_len) {
			stub->error = "Malformed M packet (len)";
			return send_err (stub, 0) ;
		}

		buf[i] = fromhex(stub->payload + 1 + i*2);
    }

    if (len) {
        err = write_memory (D_CONTEXT, addr, buf, len);
        icache_invalidate (stub);
   }
     
    prepare_to_answer (stub);
    if (err != RET_OK) {
        stub->error = "Error writing memory";
        return send_err (stub, err == RET_SYSTRAP) ;
    }

    return send_ok (stub);
}

static unsigned int unescape (char* src, unsigned int src_len) {
    char *c;
    unsigned int len = src_len;

    while ((c = memchr (src, '}', src_len))) {
	*c = c[1] ^ 0x20;
	src_len -= (c - src) + 1;
	memmove (c+1, c+2, src_len);
	src = c+1;
	len -= 1;
    }

    return len;
}

static bool handle_X (struct gdbstub *stub)
{
    unsigned int addr, len, len2;
    char *endptr;
    errcode_t err = RET_OK;

    NEED_REAL_D_CONTEXT;
    addr = strtoul((char*)stub->payload + 1, &endptr, 16);
    if (*endptr != ',') {
		stub->error = "Malformed X packet (addr)";
		return send_err (stub, 0) ;
    }

    len = strtoul(endptr + 1, &endptr, 16);
    if (*endptr != ':') {
		stub->error = "Malformed X packet (len)";
		return send_err (stub, 0) ;
    }

    /* Remove the header and the checksum from the length */
    len2 = unescape (endptr + 1,
					 stub->data_len - (endptr + 1 - stub->raw_data) - 3);

    if (len != len2) {
		stub->error = "Malformed X packet (len2)";
		return send_err (stub, 0) ;
    }

    if (len) {
        err = write_memory (D_CONTEXT, addr, endptr + 1, len);
        icache_invalidate (stub);
    }

    prepare_to_answer (stub);
    if (err != RET_OK) {
        stub->error = "Error writing memory";
        return send_err (stub, err == RET_SYSTRAP) ;
    }
    
    return send_ok (stub);
}

static bool parse_breakpoint_packet (struct gdbstub *stub, bp_type_t *type,
				     unsigned int *addr, int *length) 
{
    unsigned int t;
    char *endptr;

    t = strtoul((char*)stub->payload + 1, &endptr, 16);
    if (*endptr != ',') {
	stub->error = "Malformed bp packet (type)";
	return false;
    }

    *addr = strtoul(endptr + 1, &endptr, 16);
    if (*endptr != ',') {
	stub->error = "Malformed bp packet (addr)";
	return false;
    }

    *length = strtol(endptr + 1, &endptr, 16);
    if (*endptr != '#') {
	stub->error = "Malformed bp packet (len)";
	return false;
    }

    switch (t) {
    case 2:
	*type = BP_WRITE;
    break;
    case 3:
	*type = BP_READ;
    break;
    case 4:
	*type = BP_ACCESS;
    break;
    default:
	stub->error = "Unkown bp type";
	return false;
    }
    
    return true;
}

static bool handle_z (struct gdbstub *stub)
{
    bp_type_t type;
    unsigned int addr;
    int length;

    NEED_REAL_D_CONTEXT;
    if (!parse_breakpoint_packet (stub, &type, &addr, &length)) {
		prepare_to_answer (stub);
		return send_answer (stub);
    }

    if (debug_agent_remove_breakpoint (D_CONTEXT, 
									   type, addr, length) != RET_OK) {
		stub->error = "Error removing breakpoint";
		return send_err (stub, 0) ;
    }

    return send_ok (stub);
}

static bool handle_Z (struct gdbstub *stub)
{
    bp_type_t type;
    unsigned int addr;
    int length;

    NEED_REAL_D_CONTEXT;
    if (!parse_breakpoint_packet (stub, &type, &addr, &length)) {
		prepare_to_answer (stub);
		return send_answer (stub);
    }

    if (debug_agent_insert_breakpoint (D_CONTEXT, 
									   type, addr, length) != RET_OK) {
		stub->error = "Error inserting breakpoint";
		return send_err (stub, 0) ;
    }
    
    return send_ok (stub);
}

static bool append_stop_packet (struct gdbstub *stub, struct stopped_context *c,
    bool bchange_context)
{
    struct context ctxt = {c->ctxt.agent, c->ctxt.vehicle};
    struct context gdb_ctx = ctxt;
    
    if (gdb_ctx.agent >= 0 && gdb_ctx.agent < stub->nb_agents)
    {
      struct agent *ag = &stub->agents[gdb_ctx.agent];
      if (ag->ddr_peer_united_io)
      {
        gdb_ctx.agent = ag->ddr_peer_united_io->contexts[0].ctxt.agent;
        gdb_ctx.vehicle += ag->ddr_peer_united_io->nbvehicles;
      }
    }
    
    switch (c->state) {
    case EXEC_SIGNALED:
	printf_packet(stub, "T%02xthread:p%x.%x;", 
		      c->val, gdb_ctx.agent + 1, gdb_ctx.vehicle + 1);
	break;
    case EXEC_EXITED:
	stub->exited = true;
	stub->exit_value = c->val;
	printf_packet(stub, "W%02x;process:%x;", c->val, gdb_ctx.agent + 1);
	break;
    case EXEC_WATCHPOINT:
	printf_packet(stub, "T05thread:p%x.%x;watch:%x;",
		      gdb_ctx.agent + 1, gdb_ctx.vehicle + 1, c->val);
	break;
    default:
	return false;
    }

    if (bchange_context)
    {
        set_exec_context (stub, ctxt);
        set_data_context (stub, ctxt);
    }

    return true;
}

static bool handle_H (struct gdbstub *stub)
{
    struct context ctxt;

    if (!parse_thread_id (stub, &ctxt, stub->payload + 2, NULL)) {
		stub->error = "Malformed thread id in H packet";
		return send_err (stub, 0) ;
    }


    switch (stub->payload[1]) {
    case 'g':
		if (ctxt.vehicle == -2)
			ctxt.vehicle = -1;
		set_data_context (stub, ctxt);
		break;
    case 'c':
		set_exec_context (stub, ctxt);
		break;
    default:
		return send_unsupported (stub);
    }
    
    return send_ok (stub);
}

static bool handle_qC (struct gdbstub *stub)
{
    struct context ctxt;
    struct agent *ag;

    prepare_to_answer (stub);

    if (!stub->stopped_contexts_tail || stub->agents[stub->stopped_contexts_tail->ctxt.agent].attached == 0)
    {
    	ctxt.agent = 0;
    	ctxt.vehicle = 0;
    }
    else
    {
      ctxt = stub->stopped_contexts_tail->ctxt;
      ag = &stub->agents[ctxt.agent];
      if (ag->ddr_peer_united_io)
      {
        ctxt.agent = ag->ddr_peer_united_io->contexts[0].ctxt.agent;
        if (stub->agents[ctxt.agent].attached)
        {
          //printf ("GDBSTUB handle_qC ioeth context -> ioddr\n");
          ctxt.vehicle += ag->ddr_peer_united_io->nbvehicles;
        }
        else
        {
          ctxt.agent = 0;
          ctxt.vehicle = 0;
        }
      }
    }
    
    printf_packet (stub, "QCp%x.%x", ctxt.agent+1, ctxt.vehicle+1);
    return send_answer (stub);
}

static bool handle_threadinfo (struct gdbstub *stub)
{
    int i, nbvehicles;

    prepare_to_answer (stub);

   if (stub->thread_iter < stub->nb_agents &&  stub->agents[stub->thread_iter].ddr_peer_united_io)
     stub->thread_iter++;

    if (stub->thread_iter >= stub->nb_agents) {
	printf_packet (stub, "l");
	return send_answer (stub);
    }

    printf_packet (stub, "mp%x.%x", stub->thread_iter+1, 1);
    nbvehicles = stub->agents[stub->thread_iter].nbvehicles;
    if (stub->agents[stub->thread_iter].eth_peer_united_io)
      nbvehicles += stub->agents[stub->thread_iter].eth_peer_united_io->nbvehicles;
    for (i = 2; i <= nbvehicles; ++i) {
	printf_packet (stub, ",p%x.%x", stub->thread_iter+1, i);
    }
    stub->thread_iter++;
    return send_answer (stub);
}

static bool handle_qGetTLSAddr (struct gdbstub *stub)
{
    unsigned int v1, v2;
    char *endptr;
    struct context ctxt;

    if (!parse_thread_id (stub, &ctxt, stub->payload + 12, &endptr)) {
		stub->error = "Malformed thread id in qGetTLSAddr packet";
		return send_err (stub, 0) ;
    }

    if (*endptr != ',') {
		stub->error = "Malformed qGetTLSAddr packet (thread id)";
		return send_err (stub, 0) ;
    }

    v1 = strtoul(endptr + 1, &endptr, 16);
    if (*endptr != ',') {
		stub->error = "Malformed bp packet (v1)";
		return send_err (stub, 0) ;
    }

    v2 = strtoul(endptr + 1, &endptr, 16);
    if (*endptr != '#') {
		stub->error = "Malformed bp packet (v2)";
		return send_err (stub, 0) ;
    }
    prepare_to_answer (stub);

    printf_packet (stub, "%x", v1 + v2);
    return send_answer (stub);
}

static bool handle_question (struct gdbstub *stub)
{
    prepare_to_answer (stub);
    
    if (stub->non_stop) {
		stub->stopped_contexts_iter = stub->stopped_contexts;
    }
    
    if (stub->stopped_contexts_iter == NULL) {
		stub->error = "No stopped threads for ? packet";
		return send_ok (stub);
    }

    prepare_to_answer (stub);

	if (!append_stop_packet (stub, stub->stopped_contexts_iter, true)) {
		stub->error = "Invalid context state in ? packet";
		return send_err (stub, 0) ;
    }

    stub->stopped_contexts_iter = stub->stopped_contexts_iter->next;
    return send_answer (stub);
}

struct exec_request {
    struct gdbstub *stub;
    struct context ctxt;
    errcode_t (*run_cmd) (debug_agent_t *, int);
};

static void *exec_request_helper (void *req)
{
    struct exec_request *request = (struct exec_request *)req;
    errcode_t err;

    pthread_detach(pthread_self());
    DEBUG("RUNNING COMMAND IN THREAD %i %i\n", 
	  request->ctxt.agent, request->ctxt.vehicle);
    err = request->run_cmd (request->stub->agents[request->ctxt.agent].agent,
			    request->ctxt.vehicle);
    if (err != RET_OK && err != RET_STOP) {
	request->stub->error = "Error in debug_agent_run";
    }

    DEBUG("EXECUTION STOPPED\n");    
    gdbstub_notify_context_stop (request->stub, 
				 request->ctxt.agent, request->ctxt.vehicle);
    DEBUG("NOTIFIED\n");    
    free (request);
    return NULL;
}

static errcode_t run_in_thread (struct gdbstub *stub, int agent, int vehicle,
				errcode_t (*run_cmd) (debug_agent_t *, int))
{
    struct exec_request *req = malloc (sizeof(*req));
    pthread_t thread;

    req->stub = stub;
    req->ctxt.agent = agent;
    req->ctxt.vehicle = vehicle;
    req->run_cmd = run_cmd;

    DEBUG("CREATING THREAD\n");
    if (pthread_create (&thread, NULL, exec_request_helper, req) != 0) {
	perror("pthread_create");
	DEBUG("ERROR CREATING THREAD");
    }
    return RET_OK;
}

static errcode_t debug_agent_interrupt (debug_agent_t *da, int vehicle) 
{
    return debug_agent_stop (da, vehicle, EXEC_SIGNALED, 5);
}

static bool context_matches (struct context *c1, struct context *c2)
{
    if (c1->agent != -1 && c1->agent != -2 
		&& c2->agent != -1 && c2->agent != -2
		&& c1->agent != c2->agent) 
		return false;

    if (c1->vehicle != -1 && c1->vehicle != -2 
		&& c2->vehicle != -1 && c2->vehicle != -2
		&& c1->vehicle != c2->vehicle) 
		return false;

    return true;
}

static bool handle_vCont_run (struct gdbstub *stub, 
							  errcode_t (*run_cmd) (debug_agent_t *, int))
{
	struct context ctxt = { -1, -1};
    char *endptr;
    int i, j;
    errcode_t err;
    struct stopped_context *ctxt_iter, **prev_next_field;

    if (stub->payload[7] == ':') {
		if (!parse_thread_id (stub, &ctxt, stub->payload + 8, &endptr)) {
			stub->error = "Malformed thread id in vCont";
			return send_err (stub, 0) ;
		}
		/* if (*endptr != '#') { */
		/*     stub->error = "Multiple vCont actions not supported"; */
		/*     return send_err (stub, 0) ; */
		/* } */
    } else if (stub->payload[9] == ':') {
		if (!parse_thread_id (stub, &ctxt, stub->payload + 10, &endptr)) {
			stub->error = "Malformed thread id in vCont";
			return send_err (stub, 0) ;
		}
		/* if (*endptr != '#') { */
		/*     stub->error = "Multiple vCont actions not supported"; */
		/*     return send_err (stub, 0) ; */
		/* } */
    } else if (stub->payload[7] == '#') {
		ctxt.agent = -2;
		ctxt.vehicle = -2;
    } else if (stub->payload[9] == '#') {
		ctxt.agent = -2;
		ctxt.vehicle = -2;
    }

    if (ctxt.agent == -1 || ctxt.vehicle == -1) {
		stub->error = "Process/thread id 0 makes no sense for vCont";
		return send_err (stub, 0) ;
    }
    
    if (ctxt.agent == -2) {
		for (i = 0; i < stub->nb_agents; ++i) 
			for (j = 0; j < stub->agents[i].nbvehicles; ++j) {
				if (stub->async)
					err = run_cmd (stub->agents[i].agent, j);
				else
					err = run_in_thread (stub, i, j, run_cmd);
				if (err != RET_OK && err != RET_STOP) {
					stub->error = "Error in debug_agent_run";
					return send_err (stub, 0) ;
				}
			}
    } else if (ctxt.vehicle == -2) {
		DEBUG("vCont ALL THREADS %i\n", ctxt.agent);
		for (j = 0; j < stub->agents[ctxt.agent].nbvehicles; ++j) {
			if (stub->async)
				err = run_cmd (stub->agents[ctxt.agent].agent, j);
			else
				err = run_in_thread (stub, ctxt.agent, j, run_cmd);
			if (err != RET_OK && err != RET_STOP) {
				stub->error = "Error in debug_agent_run";
				return send_err (stub, 0) ;
			}
		}
    } else {
		if (stub->async)
			err = run_cmd (stub->agents[ctxt.agent].agent, ctxt.vehicle);
		else
			err = run_in_thread (stub, ctxt.agent, ctxt.vehicle, run_cmd);
		if (err != RET_OK && err != RET_STOP) {
			stub->error = "Error in debug_agent_run";
			return send_err (stub, 0) ;
		}
    }
    
    if (run_cmd != debug_agent_interrupt) {
		prev_next_field = &stub->stopped_contexts;
		ctxt_iter = stub->stopped_contexts;
		while (ctxt_iter) {
			if (context_matches(&ctxt_iter->ctxt, &ctxt)) {
                struct stopped_context *old_ctxt = ctxt_iter;
				*prev_next_field = ctxt_iter->next;
                ctxt_iter = ctxt_iter->next;
				/* Shouldn't happen, but... */
				if (stub->stopped_contexts_iter == old_ctxt)
					stub->stopped_contexts_iter = old_ctxt->next;
				if (stub->stopped_contexts_tail == old_ctxt) {
                    struct stopped_context *tail = stub->stopped_contexts;

                    while (tail && tail->next)
                        tail = tail->next;
					stub->stopped_contexts_tail = tail;
                }
                old_ctxt->notified = old_ctxt->used = 0;
                old_ctxt->next = NULL;
                old_ctxt->state = 0;
			} else {
                prev_next_field = &ctxt_iter->next;
                ctxt_iter = ctxt_iter->next;
            }
		}
    }

    if (stub->non_stop) {
		return send_ok (stub);
    } else {
		stub->pending_stop_notification = true;
		return true;
    }
}

static bool handle_vCont_c (struct gdbstub *stub)
{
    return handle_vCont_run (stub, debug_agent_run);
}

static bool handle_vCont_s (struct gdbstub *stub)
{
    return handle_vCont_run (stub, debug_agent_stepi);
}

static bool handle_vCont_t (struct gdbstub *stub)
{
    return handle_vCont_run (stub, debug_agent_interrupt);
}

static bool handle_vCont (struct gdbstub *stub)
{
    if (stub->payload[5] == '?') {
	prepare_to_answer (stub);
	printf_packet (stub, "vCont;c;C;s;S;t");
	return send_answer (stub);
    }
    
    if (stub->payload[5] != ';') {
		stub->error = "Malformed vCont packet";
		return send_err (stub, 0) ;
    }

    switch (stub->payload[6]) {
    case 'c':
    case 'C':
	return handle_vCont_c (stub);
    case 's':
    case 'S':
	return handle_vCont_s (stub);
    case 't':
	return handle_vCont_t (stub);
    }

    stub->error = "Unsupported vCont command";
    return send_err (stub, 0) ;
}

static bool handle_vKill (struct gdbstub *stub)
{
    /* FIXME */
    stub->exited = 0;
    stub->exit_value = 0;
    return send_ok (stub);
}

static bool handle_vStopped (struct gdbstub *stub)
{
    while (stub->stopped_contexts_iter && stub->stopped_contexts_iter->notified)
            stub->stopped_contexts_iter = stub->stopped_contexts_iter->next;

    if (stub->stopped_contexts_iter == NULL) {
	stub->pending_stop_notification = false;
	return send_ok (stub);
    }

    prepare_to_answer (stub);
    stub->stopped_contexts_iter->notified = 1;
    if (!append_stop_packet (stub, stub->stopped_contexts_iter, false)) {
        stub->error = "Invalid context state in vStopped packet";
        return send_err (stub, 0) ;
    } 
    stub->stopped_contexts_iter = stub->stopped_contexts_iter->next;

    return send_answer (stub);
}

static bool add_stopped_context (struct gdbstub *stub, int agent, int vehicle)
{
    struct stopped_context *ctxt;

    ctxt = insert_stopped_context (stub, agent, vehicle);
    
    if (ctxt == NULL)
    {
	    stub->error = "Context wasn't stopped";
	    return false;
    }

    if (ctxt->notified || !stub->agents[agent].attached)
        return true;

    if (stub->non_stop && stub->pending_stop_notification)
    {
	    /* If it's the first delayed notification, setup the thread
	       iterator */
	    if (stub->stopped_contexts_iter == NULL)
	        stub->stopped_contexts_iter = stub->stopped_contexts_tail;
    }
    else if (stub->non_stop)
    {
	    stub->pending_stop_notification = true;
        ctxt->notified = 1;

    	prepare_to_answer (stub);
    	stub->header = '%';
    	printf_packet (stub, "Stop:");
    	append_stop_packet (stub, ctxt, false);
    	if (stub->stopped_contexts_iter == ctxt)
    	    stub->stopped_contexts_iter = stub->stopped_contexts_iter->next;

    	return send_answer (stub);
    }
    else if (!stub->non_stop && stub->pending_stop_notification)
    {
	    stub->pending_stop_notification = false;
	    prepare_to_answer (stub);
	    append_stop_packet (stub, ctxt, true);

	    return send_answer (stub);
    }

    return true;
}

static bool handle_vAttach (struct gdbstub *stub)
{
    char *endptr;
    unsigned long i, ctx_eth = -1;
    struct stopped_context *ctxt = stub->unattached_stopped_contexts;
    struct stopped_context **prev_ctxt = &stub->unattached_stopped_contexts;

    if (stub->payload[7] != ';') {
		stub->error = "Malformed vAttach packet";
		return send_err (stub, 0) ;
    }

    i = strtoul((char*)stub->payload + 8, &endptr, 16);
    if (*endptr != '#') {
		stub->error = "Malformed vAttach packet";
		return send_err (stub, 0) ;
    }
    
    if (i > stub->nb_agents || i == 0) {
        stub->error = "No such agent id.";
        return send_err (stub, 0) ;
    }

    stub->agents[i-1].attached = 1;
    if (stub->agents[i - 1].eth_peer_united_io)
    {
      stub->agents[i - 1].eth_peer_united_io->attached = 1;
      ctx_eth = stub->agents[i - 1].eth_peer_united_io->contexts[0].ctxt.agent; 
    }

    gdbstub_build_thread_list (stub);

    prepare_to_answer(stub);
    send_ok (stub);

    while (ctxt) {
        if (ctxt->ctxt.agent == i - 1 || ctxt->ctxt.agent == ctx_eth) {
            *prev_ctxt = ctxt->next;
            ctxt->next = NULL;
            ctxt->notified = ctxt->used = ctxt->state = 0;
            add_stopped_context (stub, ctxt->ctxt.agent, ctxt->ctxt.vehicle);
            ctxt = *prev_ctxt;
        }
        else
        {
          prev_ctxt = &ctxt->next;
          ctxt = ctxt->next;
        }
    }

    return true;
}

/* Precondition: data is available on the connection filedescriptor */
bool handle_command (struct gdbstub *stub)
{
    if (!get_message(stub))
	return false;

    if( stub->raw_data[0] == '\003' ) {
      return debug_agent_interrupt(E_CONTEXT);
    }

    switch (stub->payload[0]) {
    case '?':
	return handle_question (stub);
    case 'g':
	return handle_g (stub);
    case 'G':
	return handle_G (stub);
    case 'H':
	return handle_H (stub);
    case 'k': //Kalray commands
	    switch (stub->payload[1])
      {
        case 'a':
          return kalray_is_hot_attached (stub);
        case 'B':
          return kalray_inform_dsu_stepi_bkp (stub);
        case 'c':
          return kalray_get_cpu_exec_level (stub);
        case 'd':
          return kalray_get_os_supported_debug_level (stub);
        case 'D':
          return kalray_set_debug_level (stub);
        case 'h':
          return kalray_get_intsys_handlers (stub);
        case 'l':
          return kalray_get_device_list (stub);
        case 'm':
          return kalray_get_vehicle_modes_seen (stub);
        case 'P':
          return kalray_set_postponed_debug_level (stub);
        case 's':
          return kalray_set_stop_at_main (stub);
        case 'W':
          return kalray_set_kwatch (stub);
        default:
          goto unknown;
      }
    case 'm':
	return handle_m (stub);
    case 'M':
	return handle_M (stub);
    case 'p':
	return handle_p (stub);
    case 'P':
	return handle_P (stub);
    case 'q':
	switch (stub->payload[1]) {
	case 'A':
	    if (!starts_with ("qAttached:", stub->payload)) goto unknown;
	    prepare_to_answer (stub); 
	    printf_packet (stub, "%s", stub->attached ? "1" : "0");
	    return send_answer (stub);
	case 'C':
            if (stub->payload[2] == '#')
                return handle_qC (stub);
            else
                break;
	case 'G':
	    if (!starts_with ("qGetTLSAddr:", stub->payload)) goto unknown;
	    return handle_qGetTLSAddr (stub);
	case 'S':
	    if (!starts_with ("qSupported", stub->payload)) goto unknown;
	    return handle_qSupported (stub);
	case 'X':
	    if (!starts_with ("qXfer:", stub->payload)) goto unknown;
	    switch(stub->payload[6]) {
	    case 'f':
		if (!starts_with ("features:read:", stub->payload + 6)) 
		    goto unknown;
		return handle_qXfer_features_read (stub);
	    case 'o':
		if (!starts_with ("osdata:read:", stub->payload + 6))
		    goto unknown;
		return handle_qXfer_osdata_read (stub);
	    case 't':
		if (!starts_with ("threads:read:", stub->payload + 6)) 
		    goto unknown;
		return handle_qXfer_threads_read (stub);
	    default:
		goto unknown;
	    }
	case 'f':
	    if (!starts_with ("qfThreadInfo", stub->payload)) goto unknown;
	    stub->thread_iter = 0;
	    return handle_threadinfo (stub);
	case 's':
	    if (!starts_with ("qsThreadInfo", stub->payload)) goto unknown;
	    return handle_threadinfo (stub);
	default:
	    goto unknown;
	}
    case 'Q':
	switch (stub->payload[1]) {
	case 'N':
	    if (!starts_with ("QNonStop:", stub->payload)) goto unknown;
	    stub->non_stop = stub->payload[9] == '1';
	    return send_ok (stub);
	case 'S':
	    if (!starts_with ("QStartNoAckMode", stub->payload)) goto unknown;
	    stub->no_acks = true;
	    return send_ok (stub);
	default:
	    goto unknown;
	}
    case 'T':
	return send_ok (stub);
    case 'v':
	switch (stub->payload[1]) {
	case 'A':
	    if (!starts_with ("vAttach;", stub->payload)) goto unknown;
	    return handle_vAttach (stub);
	case 'C':
	    if (!starts_with ("vCont", stub->payload)) goto unknown;
	    return handle_vCont (stub);
	case 'K':
	    if (!starts_with ("vKill;", stub->payload)) goto unknown;
	    return handle_vKill (stub);
	case 'S':
	    if (!starts_with ("vStopped", stub->payload)) goto unknown;
	    return handle_vStopped (stub);
	default:
	    goto unknown;
	}
    case 'X':
	return handle_X (stub);
    case 'z':
	return handle_z (stub);
    case 'Z':
	return handle_Z (stub);
    default:
	goto unknown;
    }

 unknown:
    return send_unsupported (stub);
}

void gdbstub_notify_context_stop(struct gdbstub *stub, int agent, int vehicle)
{
    struct context ctxt = {agent, vehicle};

    if (write (stub->event_fd_write, &ctxt, sizeof(ctxt)) != sizeof(ctxt)) {
	perror ("write");
	exit (4);
    }
}

void gdbstub_set_skip_exit_when_stub_exited(struct gdbstub *stub, bool v){
  stub->skip_exit_when_stub_exited = v;
}

#define EXIT_THREAD(stub, val) do{ my_exit(stub, val); return NULL; }while(0)

/*
 * If skip_exit_when_stub_exited is set and stub has exited,
 * then simply stop the thread.
 *
 * - k1-mppa/k1-cluster should handle the process exit on their
 *   own. Forcing the exit may raise error
 *
 * - k1-jtag-runner (ie. runner) does not know how to handle exit
 *   after gdb has been attached on its own and currently relies on
 *   the stub to force termination with correct return status.
 *
 */
static void my_exit (struct gdbstub *stub, int val)
{
  if (stub->exited) {
    if (!stub->skip_exit_when_stub_exited){
    	exit (stub->exit_value);
    } else {
      return;
    }
  } else {
    exit (val);
  }
}

void gdbstub_join (struct gdbstub *stub)
{
  if (stub->control_thread)
    pthread_join (stub->control_thread, NULL);
  stub->control_thread = 0;
}

static void *control_thread (void *void_stub)
{
    struct gdbstub *stub = void_stub;
    struct pollfd pollfds[] = {
	{ stub->event_fd_read, POLLIN, 0 },
	{ stub->infd, POLLIN, 0 },
    };
	
    while (true) {
	int active = poll (pollfds, 2, -1);
	
	if (active < 0 || active == 0) {
	    perror ("poll");
	    EXIT_THREAD(stub, -1);
	}

	if (pollfds[0].revents & (POLLERR|POLLHUP|POLLNVAL)
	    || pollfds[1].revents & (POLLERR|POLLHUP|POLLNVAL)) {
	    perror ("poll2");
	    EXIT_THREAD(stub, -1);
	}
	
	if (pollfds[0].revents) {
	    /* Execution event */
	    struct context data; /* agent and vehicle in that order */
	    
	    if (read (stub->event_fd_read, &data, sizeof(data)) != sizeof(data)) {
		perror ("read");
		EXIT_THREAD (stub, -1);
	    }

    if (!add_stopped_context (stub, data.agent, data.vehicle))
    {
      printf ("GDBSTUB add_stopped_context error %s\n", stub->error);
      //EXIT_THREAD (stub, 2);
    }
	}

	if (pollfds[1].revents) {
	    /* GDB request */
	  if (!handle_command (stub)){
		EXIT_THREAD (stub, 3);
	  }
	}
	
	pollfds[0].revents = pollfds[1].revents = 0;
    }
    return NULL;
}
