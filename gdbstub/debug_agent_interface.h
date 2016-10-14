/**
 *  @file debug_agent_interface.h
 *
 *  @section LICENSE
 *  Copyright (C) 2009 Kalray
 *  @author Patrice, GERIN patrice.gerin@kalray.eu
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

#ifndef _DEBUG_AGENT_INTERFACE_H
#define	_DEBUG_AGENT_INTERFACE_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "core_regs.h"
#include "mppa_multiloader.h"

#define DEBUG_AGENT_GENERIC_NAME "Debug Agent"

#define DEBUG_AGENT_INTERFACE_VERSION 1

typedef enum errcode_ {
    RET_FAIL = -1,
    RET_OK = 0,
    RET_STOP,
    RET_IDLE,
    RET_TRAPOPCODE,
    RET_SYSTRAP,
    RET_BREAK,
    RET_STALL,
	RET_ABORT,
} errcode_t;

typedef struct {
  uint32_t  valid;
  reg_t		value;
} registers_value_t;

struct debug_agent_;
struct trace_record;
typedef struct debug_agent_ debug_agent_t;

typedef enum {BP_EXEC, BP_WRITE, BP_READ, BP_ACCESS} bp_type_t;
typedef enum {EXEC_RUNNING, EXEC_SIGNALED, EXEC_EXITED, EXEC_WATCHPOINT, EXEC_TARGET_SPECIFIC, EXEC_POWEROFF} exec_state_t;

typedef errcode_t (*da_finish_init_t) (debug_agent_t *self);

typedef int       (*da_get_id_t) (debug_agent_t *self);
typedef char *    (*da_get_desc_t) (debug_agent_t *self);
typedef const char *(*da_get_architecure_t) (debug_agent_t *self);
typedef const char *(*da_get_core_t) (debug_agent_t *self);
typedef char **   (*da_get_vehicles_t) (debug_agent_t *self);
typedef const char *(*da_get_cluster_name_t) (debug_agent_t *da);
typedef bool      (*da_is_async_t) (debug_agent_t *self);
typedef bool      (*da_fake_pcie_spawn_t) (debug_agent_t *self);
typedef core_register_descr_t *(*da_register_names_t) (debug_agent_t *self, int vehicle);
typedef int       (*da_get_register_index_t) (debug_agent_t *self, int vehicle, char *reg_name);
typedef errcode_t (*da_read_register_t) (debug_agent_t *self, int vehicle, int regnum, reg_t *buf);
typedef errcode_t (*da_write_register_t) (debug_agent_t *self, int vehicle, int regnum, reg_t buf);
typedef errcode_t (*da_read_registers_t) (debug_agent_t *self, int vehicle, int *regnums, reg_t *buf, int count);
typedef errcode_t (*da_write_registers_t) (debug_agent_t *self, int vehicle, int *regnums, reg_t *buf, int count);
typedef errcode_t (*da_read_memory_t) (debug_agent_t *self, int vehicle, uint64_t addr, void *buf, uint64_t buf_size);
typedef errcode_t (*da_write_memory_t) (debug_agent_t *self, int vehicle, uint64_t addr, void *buf, uint64_t buf_size);
typedef errcode_t (*da_icache_invalidate_t) (debug_agent_t *self, int vehicle);
typedef errcode_t (*da_stepi_t) (debug_agent_t *self, int vehicle);
typedef errcode_t (*da_run_t) (debug_agent_t *self, int vehicle);
typedef errcode_t (*da_stop_t) (debug_agent_t *self, int vehicle, exec_state_t state, int val);
typedef errcode_t (*da_is_executing_t) (debug_agent_t *self, int vehicle, exec_state_t *state, int *val);
typedef errcode_t (*da_insert_breakpoint_t) (debug_agent_t *self, int vehicle, bp_type_t, uint64_t addr, uint64_t size);
typedef errcode_t (*da_remove_breakpoint_t) (debug_agent_t *self, int vehicle, bp_type_t, uint64_t addr, uint64_t size);
typedef errcode_t (*da_read_cycle_t) (debug_agent_t *self, int vehicle, uint64_t *val);
typedef errcode_t (*da_strerror_t) (debug_agent_t *self, int vehicle, char **str);
typedef errcode_t (*da_get_args_t) (debug_agent_t *self, int vehicle, int *argc, char ***argv);
typedef errcode_t (*da_get_elf_file_t) (debug_agent_t *self, int vehicle, const char **elf_file);
typedef errcode_t (*da_get_arg_t) (debug_agent_t *self, int vehicle, int num, uint32_t *arg);
typedef errcode_t (*da_set_return_t) (debug_agent_t *self, int vehicle, int ret);
typedef errcode_t (*da_load_elf_t) (debug_agent_t *self, char * elf, int argc, char * argv[]);
typedef errcode_t (*da_load_memory_t) (debug_agent_t *self, const char * elf);
typedef errcode_t (*da_execution_stall_t) (debug_agent_t *self, int vehicle, int cycles);
typedef errcode_t (*da_debug_enable_t) (debug_agent_t *self, int vehicle, int flag);
typedef errcode_t (*da_debug_mode_t) (debug_agent_t *self, int vehicle, int mode);
typedef errcode_t (*da_disassemble_bundle_t) (debug_agent_t *self, int vehicle, void *bundle, int size, struct trace_record *record, registers_value_t *r_regs, registers_value_t *w_regs, registers_value_t *r_sfr_regs, registers_value_t *w_sfr_regs);
typedef mppa_ml_loader_t *(*da_mppa_multiloader_t)(debug_agent_t *da);
typedef errcode_t (*da_stack_min_max_t)(debug_agent_t *self, int vehicle, uint64_t *min, uint64_t *max);
typedef errcode_t (*da_get_intsys_handlers_t)(debug_agent_t *self, int vehicle, void* buf, int buf_size);
typedef errcode_t (*da_set_debug_level_t)(debug_agent_t *self, int vehicle, int debug_level, int postpone);
typedef errcode_t (*da_inform_dsu_stepi_bkp_t)(debug_agent_t *self, int vehicle);
typedef errcode_t (*da_get_cpu_exec_level_t) (debug_agent_t *self, int vehicle, int *cpu_level);
typedef errcode_t (*da_set_stop_at_main_t) (debug_agent_t *self, int bstop);
typedef int (*da_get_rm_idx_t) (debug_agent_t *self);
typedef int (*is_hot_attached_t) (debug_agent_t *self);
typedef char *(*da_get_device_list_t) (debug_agent_t *self, const char *device_full_name);
typedef errcode_t (*da_set_kwatch_t) (debug_agent_t *da, const char *full_name, int watch_type,
  int bset, char **err_msg);

/**
 * @struct debug_agent_interface_t
 * @brief Debug agent API
 *
 */
typedef struct {
  debug_agent_t *self;

  da_mppa_multiloader_t multiloader;
  da_finish_init_t finish_initialization;
  da_get_id_t get_id;
  da_get_desc_t get_desc;
  da_get_architecure_t get_architecture;
  da_get_core_t get_core;
  da_get_vehicles_t get_vehicles;
  da_get_cluster_name_t get_cluster_name;
  da_register_names_t register_names;
  da_is_async_t is_async;
  da_fake_pcie_spawn_t fake_pcie_spawn;

  da_get_register_index_t get_register_index;
  da_read_register_t read_register;
  da_write_register_t write_register;
  da_read_registers_t read_registers;
  da_write_registers_t write_registers;
  da_read_memory_t read_memory;
  da_read_memory_t read_dcache;
  da_write_memory_t write_memory;
  da_write_memory_t write_dcache;
  da_icache_invalidate_t icache_invalidate;
  da_stepi_t stepi;
  da_run_t run;
  da_stop_t stop;
  da_is_executing_t is_executing;
  da_insert_breakpoint_t insert_breakpoint;
  da_remove_breakpoint_t remove_breakpoint;
  da_read_cycle_t read_cycle;
  da_strerror_t strerror;
  da_get_args_t get_args;
  da_get_arg_t get_arg;
  da_get_elf_file_t get_elf_file;
  da_set_return_t set_return;
  da_load_elf_t load_elf;
  da_load_memory_t load_memory;
  da_execution_stall_t execution_stall;
  da_disassemble_bundle_t disassemble_bundle;
  da_debug_enable_t   debug_enable;
  da_debug_mode_t   debug_mode;
  da_stack_min_max_t  stack_min_max;
  da_get_intsys_handlers_t get_intsys_handlers;
  da_set_debug_level_t set_debug_level;
  da_inform_dsu_stepi_bkp_t inform_dsu_stepi_bkp;
  da_get_cpu_exec_level_t  get_cpu_exec_level;
  da_set_stop_at_main_t set_stop_at_main;
  da_get_rm_idx_t get_rm_idx;
  is_hot_attached_t is_hot_attached;
  da_get_device_list_t get_device_list;
  da_set_kwatch_t set_kwatch;
} debug_agent_interface_t;

#ifdef	__cplusplus
}
#endif

#endif	/* _DEBUG_AGENT_INTERFACE_H */

