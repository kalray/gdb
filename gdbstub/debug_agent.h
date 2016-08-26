/**
 *  @file debug_agent.h
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

#ifndef _DEBUG_AGENT_H
#define	_DEBUG_AGENT_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include "debug_agent_interface.h"
#include "debug_common.h"
#include "mppa_multiloader.h"


#define DEBUG_AGENT_GENERIC_NAME "Debug Agent"

#define DEBUG_AGENT_INTERFACE_VERSION 1

#define DEBUG_AGENT_DESC_SIZE	128

struct configuration_s;
struct debug_agent_;
  
/**
 * @struct debug_agent_attributes_t
 * @brief Debug agent attributes
 *
 */
typedef struct {
  const char  *name;
  struct configuration_s   *configuration;

  int         da_argc;                                /**< The DA argument count */
  char        **da_argv;                              /**< The DA arguments*/

  int         no_load_elf;                            /**< Do not load the ELF file */
  const char  *elf_file;                              /**< The ELF file to load */
  int         argc;                                   /**< argc for the program to run */
  char        **argv;                                 /**< argv for the program to run */
  char        **env;                                  /**< env for the program to run*/
  void        (*notify_stop)(debug_agent_t *da, int vehicle, void *); /**< Callback to notify the debug agent instantiator of a vehicle stop */
  void        (*notify_cpu_level_seen)(debug_agent_t *da, int vehicle, void *, unsigned char mode);
  void        *notify_data;                         /**< Token to pass to the notify_stop callback */

  uint64_t    *global_cycle_ptr;

  mppa_ml_binary_t mppa; /**< Multibinary descriptor */
  mppa_ml_loader_t mppa_multiloader; /**< Callbacks to multiloader funcs */

  int no_segment_load_enable; /**< debug agent should load program by segment instead of by sections */

  /*
   * TLS auto duplication is mainly here for backward compatibility in bare mode.
   * Previously, tbss/tdata sections were simply duplicated 17 times (1/core).
   * Now, these sections are "master" copies and are copied at runtime (in the BSP).
   */
  int tls_auto_duplicate;

  /*
   * When set to something different than 0,
   * debug agent should set spawn type to PCI_SPAWN
   */
  int fake_pcie_spawn;
  
  int os_supported_debug_mode;
  int break_on_spawn;
  
  /* link between ddr and eth of bostan for a united presentation to gdb */
  struct debug_agent_         *da_ddr_peer_united_io;
  struct debug_agent_         *da_eth_peer_united_io;
} debug_agent_attributes_t;

/**
 * @struct debug_agent_t
 * @brief Debug agent main data structure
 *
 */
struct debug_agent_ {
  debug_agent_attributes_t    attributes;
  debug_agent_interface_t     interface;
  debug_attributes_t          debug_attr;
  struct debug_agent_        *next;
};

#define FOREACH_DEBUG_AGENT(agent, agents) for (agent = agents; agent; agent = agent->next)

#define FOREACH_VEHICLE(vehicle, agent) for (vehicle = debug_agent_get_vehicles (agent); *vehicle; ++vehicle)

/**
 * @fn debug_agent_ctor
 * @brief Create a new debug agent
 *
 * @param[in] attributes The debug agent attributes
 * @return On success, a newly allocated debug agent is returned otherwize NULL.
 */
debug_agent_t *debug_agent_ctor (debug_agent_attributes_t *attributes);
typedef debug_agent_t *(*debug_agent_ctor_fct)(debug_agent_attributes_t *attributes);

/**
 * @fn debug_agent_initializer
 * @brief Initialize a new debug agent (use for native simulation)
 *
 * @param[in] debug_agent A pointer to the debug agent to initialize
 * @param[in] attributes The debug agent attributes
 * @return On success, the debug agent initialized is returned otherwize NULL.
 */
debug_agent_t *debug_agent_initializer(debug_agent_t *debug_agent, debug_agent_attributes_t *attributes);
typedef debug_agent_t *(*debug_agent_initializer_fct)(debug_agent_t *debug_agent, debug_agent_attributes_t *attributes);

/**
 * @fn debug_agent_dtor
 * @brief Destroy a debug agent instance
 *
 * @param[in] driver The debug agent instance to destroy
 * @return On success, zero is returned otherwize -1 is returned.
 */
int debug_agent_dtor (debug_agent_t *driver);
typedef int (*debug_agent_dtor_fct)(debug_agent_t *driver);

/**
 * @fn debug_agent_usage
 * @brief Print the usage and options of a debug instance
 *
 * @return Nothing
 */
void debug_agent_usage();
typedef void (*debug_agent_usage_fct)();

/**
 * @fn debug_agent_version
 * @brief Print the version debug instance
 *
 * @return Nothing
 */
void debug_agent_version();
typedef void (*debug_agent_version_fct)();

/**
 * @fn debug_agent_cmd_to_config
 * @brief Build a configuration from a command line
 *
 * param[in] argc The number of arguments
 * param[in] argv The command line
 * @return On success, a newly allocated configuration is returned otherwize NULL
 */
int debug_agent_cmd_to_config(debug_agent_attributes_t *debug_agent_attr);
typedef int (*debug_agent_cmd_to_config_fct)(debug_agent_attributes_t *debug_agent_attr);

static inline errcode_t debug_agent_finish_initialization (debug_agent_t *da) {
    return da->interface.finish_initialization(da);
}

static inline errcode_t debug_agent_strerror (debug_agent_t *da, int vehicle, char **strerror) {
    return da->interface.strerror(da, vehicle, strerror);
}

static inline int debug_agent_get_id (debug_agent_t *da) {
    return da->interface.get_id ? da->interface.get_id(da) : 0;
}

static inline char * debug_agent_get_desc (debug_agent_t *da) {
    return da->interface.get_desc ? da->interface.get_desc(da) : NULL;
}

static inline const char *debug_agent_get_architecture (debug_agent_t *da) {
    return da->interface.get_architecture(da);
}

static inline const char *debug_agent_get_core (debug_agent_t *da) {
    return da->interface.get_core(da);
}

static inline char **debug_agent_get_vehicles (debug_agent_t *da) {
    return da->interface.get_vehicles(da);
}

static inline const char *debug_agent_get_cluster_name (debug_agent_t *da)
{
  return da->interface.get_cluster_name (da);
}

static inline bool debug_agent_is_async (debug_agent_t *da) {
    return da->interface.is_async(da);
}

static inline core_register_descr_t *debug_agent_register_names(debug_agent_t *da, int vehicle) {
    return da->interface.register_names(da, vehicle);
}
    
static inline int debug_agent_get_register_index(debug_agent_t *da, int vehicle, char *reg_name){
    return da->interface.get_register_index(da, vehicle, reg_name);
}

static inline errcode_t debug_agent_read_register(debug_agent_t *da, int vehicle, int regnum, reg_t *buf){
    return da->interface.read_register(da, vehicle, regnum, buf);
}

static inline errcode_t debug_agent_write_register(debug_agent_t *da, int vehicle, int regnum, reg_t buf) {
    return da->interface.write_register(da, vehicle, regnum, buf);
}

static inline errcode_t debug_agent_read_registers(debug_agent_t *da, int vehicle, int *regnums, reg_t *buf, int count) {
    return da->interface.read_registers(da, vehicle, regnums, buf, count);
}

static inline errcode_t debug_agent_write_registers(debug_agent_t *da, int vehicle, int *regnums, reg_t *buf, int count) {
    return da->interface.write_registers(da, vehicle, regnums, buf, count);
}

static inline errcode_t debug_agent_read_memory(debug_agent_t *da, int vehicle, uint64_t addr, void *buf, int buf_size) {
    return da->interface.read_memory(da, vehicle, addr, buf, buf_size);
}

static inline errcode_t debug_agent_write_memory(debug_agent_t *da, int vehicle, uint64_t addr, void *buf, int buf_size) {
    return da->interface.write_memory(da, vehicle, addr, buf, buf_size);
}

static inline errcode_t debug_agent_read_dcache(debug_agent_t *da, int vehicle, uint64_t addr, void *buf, int buf_size) {
    return da->interface.read_dcache(da, vehicle, addr, buf, buf_size);
}

static inline errcode_t debug_agent_write_dcache(debug_agent_t *da, int vehicle, uint64_t addr, void *buf, int buf_size) {
    return da->interface.write_dcache(da, vehicle, addr, buf, buf_size);
}

static inline errcode_t debug_agent_icache_invalidate(debug_agent_t *da, int vehicle) {
    return da->interface.icache_invalidate(da, vehicle);
}

static inline errcode_t debug_agent_stepi(debug_agent_t *da, int vehicle) {
    return da->interface.stepi(da, vehicle);
}

static inline errcode_t debug_agent_run(debug_agent_t *da, int vehicle) {
    return da->interface.run(da, vehicle);
}

static inline errcode_t debug_agent_stop(debug_agent_t *da, int vehicle, exec_state_t state, int val) {
    return da->interface.stop(da, vehicle, state, val);
}

static inline errcode_t debug_agent_is_executing(debug_agent_t *da, int vehicle, exec_state_t *state, int *val) {
    return da->interface.is_executing(da, vehicle, state, val);
}

static inline errcode_t debug_agent_insert_breakpoint(debug_agent_t *da, int vehicle, bp_type_t type, uint64_t addr, int size) {
    return da->interface.insert_breakpoint(da, vehicle, type, addr, size);	
}

static inline errcode_t debug_agent_remove_breakpoint(debug_agent_t *da, int vehicle, bp_type_t type, uint64_t addr, int size) {
    return da->interface.remove_breakpoint(da, vehicle, type, addr, size);	
}

static inline errcode_t debug_agent_read_cycle(debug_agent_t *da, int vehicle, uint64_t *val) {
    return da->interface.read_cycle(da, vehicle, val);
}

static inline errcode_t debug_agent_get_intsys_handlers (
  debug_agent_t *da, int vehicle, void *buf, int buf_size) 
{
  if (da->interface.get_intsys_handlers)
    return da->interface.get_intsys_handlers (da, vehicle, buf, buf_size);

  return RET_ABORT;
}

static inline errcode_t debug_agent_set_debug_level (debug_agent_t *da,
  int vehicle, int debug_level, int postpone)
{
  if (da->interface.set_debug_level)
    return da->interface.set_debug_level (da, vehicle, debug_level, postpone);

  return RET_ABORT;
}

static inline errcode_t debug_agent_inform_dsu_stepi_bkp (debug_agent_t *da, int vehicle)
{
  if (da->interface.inform_dsu_stepi_bkp)
    return da->interface.inform_dsu_stepi_bkp (da, vehicle);

  return RET_ABORT;
}

static inline errcode_t debug_agent_get_cpu_exec_level (debug_agent_t *da,
  int vehicle, int* cpu_level)
{
  if (da->interface.get_cpu_exec_level)
    return da->interface.get_cpu_exec_level (da, vehicle, cpu_level);

  return RET_ABORT;
}

static inline errcode_t debug_agent_set_stop_at_main (debug_agent_t *da, int bstop)
{
  if (da->interface.set_stop_at_main)
    return da->interface.set_stop_at_main (da, bstop);

  return RET_ABORT;
}

static inline int debug_agent_get_rm_idx (debug_agent_t *da)
{
  return da->interface.get_rm_idx (da);
}

static inline int debug_agent_is_hot_attached (debug_agent_t *da)
{
  if (da->interface.is_hot_attached)
    return da->interface.is_hot_attached (da);

  return 0;
}

static inline char *debug_agent_get_device_list (debug_agent_t *da, const char *device_full_name)
{
  if (da->interface.get_device_list)
    return da->interface.get_device_list (da, device_full_name);

  return NULL;
}

static inline errcode_t debug_agent_set_kwatch (debug_agent_t *da, const char *full_name,
  int watch_type, int bset, char **err_msg)
{
  if (da->interface.set_kwatch)
    return da->interface.set_kwatch (da, full_name, watch_type, bset, err_msg);

  if (err_msg)
    *err_msg = strdup ("not implemented");

  return RET_ABORT;
}

#ifdef	__cplusplus
}
#endif

#endif	/* _DEBUG_AGENT_H */
