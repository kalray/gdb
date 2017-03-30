#ifndef _K1_COMMON_TDEP_
#define _K1_COMMON_TDEP_

#include "dwarf2-frame.h"
#include "dis-asm.h"
#include "opcode/k1c.h"

enum K1_ARCH
{
  K1_K1PE,
  K1_K1RM,
  K1_NUM_ARCHES
};

struct gdbarch_tdep
{
  int ev_regnum;
  int ls_regnum;
  int le_regnum;
  int lc_regnum;
  int ps_regnum;
  int ra_regnum;
  int spc_regnum;

  int local_regnum;
};

struct op_list
{
  k1opc_t *op;
  struct op_list *next;
};

extern enum K1_ARCH k1_current_arch;
extern const struct frame_unwind k1_frame_unwind;
extern struct op_list *branch_insns[K1_NUM_ARCHES];

enum K1_ARCH k1_arch (void);
void k1_look_for_insns (void);
const char *k1c_pseudo_register_name (struct gdbarch *gdbarch, int regnr);
struct type *k1c_pseudo_register_type (struct gdbarch *gdbarch, int reg_nr);
int k1c_pseudo_register_reggroup_p (struct gdbarch *gdbarch, int regnum, struct reggroup *reggroup);
enum register_status k1c_pseudo_register_read (struct gdbarch *gdbarch,
  struct regcache *regcache, int regnum, gdb_byte *buf);
void k1c_pseudo_register_write (struct gdbarch *gdbarch, struct regcache *regcache,
    int regnum, const gdb_byte *buf);
int k1c_dwarf2_reg_to_regnum (struct gdbarch *gdbarch, int reg);
const int k1c_num_pseudos (struct gdbarch *);
const char *k1c_pc_name (struct gdbarch *);
const char *k1c_sp_name (struct gdbarch *);
const char *k1_dummy_register_name (struct gdbarch *gdbarch, int regno);
struct type *k1_dummy_register_type (struct gdbarch *gdbarch, int regno);
void k1_dwarf2_frame_init_reg (struct gdbarch *gdbarch, int regnum,
  struct dwarf2_frame_state_reg *reg, struct frame_info *this_frame);
enum return_value_convention k1_return_value (struct gdbarch *gdbarch,
  struct value *func_type, struct type *type, struct regcache *regcache,
  gdb_byte *readbuf, const gdb_byte *writebuf);
struct frame_id k1_dummy_id (struct gdbarch *gdbarch, struct frame_info *this_frame);
CORE_ADDR k1_push_dummy_call (struct gdbarch *gdbarch, struct value *function,
  struct regcache *regcache, CORE_ADDR bp_addr, int nargs, struct value **args,
  CORE_ADDR sp, int struct_return, CORE_ADDR struct_addr);
CORE_ADDR k1_skip_prologue (struct gdbarch *gdbarch, CORE_ADDR func_addr);
CORE_ADDR k1_unwind_pc (struct gdbarch *gdbarch, struct frame_info *next_frame);
const gdb_byte *k1_breakpoint_from_pc (struct gdbarch *gdbarch, CORE_ADDR *pc, int *len);
CORE_ADDR k1_adjust_breakpoint_address (struct gdbarch *gdbarch, CORE_ADDR bpaddr);
int k1_print_insn (bfd_vma pc, disassemble_info *di);
int k1_get_longjmp_target (struct frame_info *frame, CORE_ADDR *pc);

#endif
