/**
 *  @file gdbstub.h
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

#include "debug_agent.h"
#include "pthread.h"

#ifdef __cplusplus
extern "C" {
#endif

struct gdbstub;

struct gdbstub *gdbstub_init(debug_agent_t *agents);
void gdbstub_rt_add_agent (struct gdbstub *stub, debug_agent_t *da);
void gdbstub_start(struct gdbstub *, bool thread);

void gdbstub_set_skip_exit_when_stub_exited(struct gdbstub *stub, bool v);
int gdbstub_listen(int port_start, int port_end, struct gdbstub *stub); 
int gdbstub_accept(struct gdbstub *stub); 
void gdbstub_join (struct gdbstub *stub, pthread_t cthread);

void gdbstub_notify_context_stop(struct gdbstub *, int agent, int vehicle);

#ifdef __cplusplus
}
#endif
