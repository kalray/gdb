/* Target-dependent code for the Kalray K1 for GDB, the GNU debugger.

   Copyright (C) 2010 Kalray

   This file is part of GDB.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include "defs.h"
#include "frame-unwind.h"
#include "gdbcore.h"
#include "target-descriptions.h"
#include "user-regs.h"
#include "elf-bfd.h"
#include "gdbthread.h"

#include "elf/k1b.h"
#include "opcode/k1b.h"
#include "k1-common-tdep.h"

struct k1_frame_cache
{
  CORE_ADDR base; // base address
  CORE_ADDR pc;
  LONGEST framesize;
  CORE_ADDR saved_sp;
};

enum K1_ARCH k1_current_arch = K1_NUM_ARCHES;
static struct op_list *sp_adjust_insns[K1_NUM_ARCHES];
static struct op_list *sp_store_insns[K1_NUM_ARCHES];
static struct op_list *prologue_helper_insns[K1_NUM_ARCHES];

struct op_list *branch_insns[K1_NUM_ARCHES];

enum K1_ARCH
k1_arch (void)
{
  const struct target_desc *desc = target_current_description ();

  if (k1_current_arch != K1_NUM_ARCHES)
    return k1_current_arch;

  if (exec_bfd)
  {
    switch (elf_elfheader (exec_bfd)->e_flags & ELF_K1_CORE_MASK)
    {
      case ELF_K1_CORE_B_DP:
        k1_current_arch = K1_K1BDP;
        break;
      case ELF_K1_CORE_B_IO:
        k1_current_arch = K1_K1BIO;
        break;
      default:
        error (_ ("The K1 binary is compiled for an unknown core."));
    }
  }
  else if (desc && tdesc_architecture (desc))
  {
    const char *name = tdesc_architecture (desc)->printable_name;
    if (!strcmp (name, "k1:k1bdp"))
      k1_current_arch = K1_K1BDP;
    else if (!strcmp (name, "k1:k1bio"))
      k1_current_arch = K1_K1BIO;
    else
      error ("unable to find the current k1 architecture.");
  }

  gdb_assert (k1_current_arch != K1_NUM_ARCHES);

  return k1_current_arch;
}

const char *
k1_dummy_register_name (struct gdbarch *gdbarch, int regno)
{
  return "";
}

struct type *
k1_dummy_register_type (struct gdbarch *gdbarch, int regno)
{
  return builtin_type (gdbarch)->builtin_int;
}

const gdb_byte *
k1_breakpoint_from_pc (struct gdbarch *gdbarch, CORE_ADDR *pc, int *len)
{
  static const gdb_byte BREAK[] = {0xFF, 0xFF, 0x1, 0};
  *len = 4;

  return BREAK;
}

CORE_ADDR
k1_adjust_breakpoint_address (struct gdbarch *gdbarch, CORE_ADDR bpaddr)
{
  gdb_byte syllab_buf[4];
  uint32_t syllab;
  enum bfd_endian order = gdbarch_byte_order_for_code (gdbarch);
  CORE_ADDR adjusted = bpaddr;
  int i = 0;

  adjusted &= ~3;
  if (adjusted == 0 || ptid_equal (inferior_ptid, null_ptid) || !is_stopped (inferior_ptid))
    return adjusted;

  /* Look for the end of the bundle preceeding the requested address. */
  do
  {
    if (target_read_memory (adjusted - 4, syllab_buf, 4) != 0)
      return adjusted;

    syllab = extract_unsigned_integer (syllab_buf, 4, order);
  } while ((i++ < 7) && (syllab >> 31) && (adjusted -= 4));

  if (i == 8) /* Impossible for correct instructions */
    return bpaddr & ~3;

  return adjusted;
}

static int
k1_has_create_stack_frame (struct gdbarch *gdbarch, CORE_ADDR addr)
{
  gdb_byte syllab_buf[4];
  uint32_t syllab;
  enum bfd_endian order = gdbarch_byte_order_for_code (gdbarch);
  k1bfield *add_reg_desc, *sbf_reg_desc;
  int reg;
  int i = 0, ops_idx, has_make = 0;
  struct op_list *ops;
  k1_bitfield_t *bfield;

  #define NUM_INSN_LISTS 3

  struct op_list_desc
  {
    struct op_list *ops;
    int sp_idx;
  };

  typedef struct op_list_desc prologue_ops[NUM_INSN_LISTS];
  prologue_ops prologue_insns_full[] =
  {
    [K1_K1BIO] =
    {
      { sp_adjust_insns[K1_K1BIO], 0 /* Dest register */},
      { sp_store_insns[K1_K1BIO], 1 /* Base register */},
      { prologue_helper_insns[K1_K1BIO], -1 /* unused */},
    },
    [K1_K1BDP] =
    {
      { sp_adjust_insns[K1_K1BDP], 0 /* Dest register */},
      { sp_store_insns[K1_K1BDP], 1 /* Base register */},
      { prologue_helper_insns[K1_K1BDP], -1 /* unused */},
    },
  };

  prologue_ops *prologue_insns = &prologue_insns_full [k1_arch ()];

  do
  {
  next_addr:
    if (target_read_memory (addr, syllab_buf, 4) != 0)
      return 0;
    syllab = extract_unsigned_integer (syllab_buf, 4, order);

    for (ops_idx = 0; ops_idx < NUM_INSN_LISTS; ++ops_idx)
    {
      ops = (*prologue_insns)[ops_idx].ops;
      while (ops)
      {
        k1opc_t *op = ops->op;
        if ((syllab & op->codeword[0].mask) != op->codeword[0].opcode)
          goto next;

        if (strcmp (op->as_op, "make") == 0)
        {
          if (i == 0)
            has_make = 1;
          else
            return 0;
        }
        else if (has_make && i == 1)
        {
          if (strcmp (op->as_op, "sbf")) return 0;
        }
        else if (has_make)
        {
          return 0;
        }

        if ((*prologue_insns)[ops_idx].sp_idx < 0)
        {
          addr += op->coding_size / 8;
          ++i;
          goto next_addr;
        }

        bfield = &op->format[(*prologue_insns)[ops_idx].sp_idx]->bfield[0];
        reg = (syllab >> bfield->to_offset) & ((1 << bfield->size) - 1);
        if (reg == 12)
          return 1;
      next:
        ops = ops->next;
      }
    }
  } while (0);

  return 0;
}

/* Return PC of first real instruction.  */

/* This function is nearly cmopletely copied from
   symtab.c:skip_prologue_using_lineinfo (excpet the test in the loop
   at the end. */
CORE_ADDR
k1_skip_prologue (struct gdbarch *gdbarch, CORE_ADDR func_addr)
{
  CORE_ADDR func_start, func_end;
  struct linetable *l;
  int ind, i, len;
  int best_lineno = 0;
  CORE_ADDR best_pc = func_addr;
  struct symtab_and_line sal;
  struct compunit_symtab *cust;
  int gcc_compiled = 0;

  sal = find_pc_line (func_addr, 0);
  if (sal.symtab == NULL)
    return func_addr;

  cust = find_pc_compunit_symtab (func_addr);
  if (cust->producer && strncmp (cust->producer, "GNU C", 5) == 0)
    gcc_compiled = 1;

  /* Give up if this symbol has no lineinfo table.  */
  l = SYMTAB_LINETABLE (sal.symtab);
  if (l == NULL)
    return func_addr;

  /* Get the range for the function's PC values, or give up if we
     cannot, for some reason.  */
  if (!find_pc_partial_function (func_addr, NULL, &func_start, &func_end))
    return func_addr;

  if (!gcc_compiled && !k1_has_create_stack_frame (gdbarch, func_addr))
    return func_addr;

  /* Linetable entries are ordered by PC values, see the commentary in
     symtab.h where `struct linetable' is defined.  Thus, the first
     entry whose PC is in the range [FUNC_START..FUNC_END[ is the
     address we are looking for.  */
  if (gcc_compiled)
  {
    ind = 0;
    for (i = 0; i < l->nitems; i++)
    {
      struct linetable_entry *item = &(l->item[i]);

      if (item->line > 0 && func_start <= item->pc && item->pc < func_end)
      {
        if (ind == 0)
          ++ind;
        else
          return item->pc;
      }
    }
  }
  else
  {
    for (i = 0; i < l->nitems; i++)
    {
      struct linetable_entry *item = &(l->item[i]);

      /* Don't use line numbers of zero, they mark special entries in
        the table.  See the commentary on symtab.h before the
        definition of struct linetable.  */
      if (item->line > 0 && func_start < item->pc && item->pc < func_end)
        return item->pc;
    }
  }

  return func_addr;
}

CORE_ADDR
k1_unwind_pc (struct gdbarch *gdbarch, struct frame_info *next_frame)
{
  return frame_unwind_register_unsigned (next_frame, gdbarch_pc_regnum (gdbarch));
}

/* Given a GDB frame, determine the address of the calling function's
   frame.  This will be used to create a new GDB frame struct.  */
static void
k1_frame_this_id (struct frame_info *this_frame,
  void **this_prologue_cache, struct frame_id *this_id)
{
  if (get_frame_func (this_frame) == get_frame_pc (this_frame))
  {
    *this_id = frame_id_build (get_frame_sp (this_frame) - 16, get_frame_func (this_frame));
  }
}

/* Get the value of register regnum in the previous stack frame.  */
static struct value *
k1_frame_prev_register (struct frame_info *this_frame,
  void **this_prologue_cache, int regnum)
{
  if (get_frame_func (this_frame) == get_frame_pc (this_frame))
  {
    if (regnum == gdbarch_pc_regnum (get_frame_arch (this_frame)))
    {
      return frame_unwind_got_register (this_frame, regnum,
        user_reg_map_name_to_regnum (get_frame_arch (this_frame), "ra", -1));
    }
    return frame_unwind_got_register (this_frame, regnum, regnum);
  }

  return frame_unwind_got_optimized (this_frame, regnum);
}

const struct frame_unwind k1_frame_unwind =
{
  NORMAL_FRAME,
  default_frame_unwind_stop_reason,
  k1_frame_this_id,
  k1_frame_prev_register,
  NULL,
  default_frame_sniffer
};

void
k1_dwarf2_frame_init_reg (struct gdbarch *gdbarch, int regnum,
  struct dwarf2_frame_state_reg *reg,
  struct frame_info *this_frame)
{
  if (regnum == gdbarch_pc_regnum (gdbarch))
    reg->how = DWARF2_FRAME_REG_RA;
  else if (regnum == gdbarch_sp_regnum (gdbarch))
  {
    reg->how = DWARF2_FRAME_REG_CFA_OFFSET;
    reg->loc.offset = -16; /* Scratch area */
  }
}

int
k1_print_insn (bfd_vma pc, disassemble_info *di)
{
  int res;
  gdb_byte buf[4];

  res = print_insn_k1 (pc, di);

  target_read_memory (pc + res - 4, buf, 4);
  /* Check if the last syllab has the parallel bit set. If so mark
     the current instruction as having a delay slot.  This forces
     x/i to display the following instructions until the end of the
     bundle.  This is of course a lie, but it manages what ze zant
     to achieve: print the full bundle when invoking 'x/i $pc'.  */
  if (buf[3] & 0x80)
  {
    di->insn_info_valid = 1;
    di->branch_delay_insns = 1;
  }

  return res;
}

static CORE_ADDR
k1_frame_align (struct gdbarch *gdbarch, CORE_ADDR addr)
{
  return addr - addr % 8;
}

struct frame_id
k1_dummy_id (struct gdbarch *gdbarch, struct frame_info *this_frame)
{
  CORE_ADDR sp = get_frame_register_unsigned (this_frame, gdbarch_sp_regnum (gdbarch));
  return frame_id_build (sp + 16, get_frame_pc (this_frame));
}

CORE_ADDR
k1_push_dummy_call (struct gdbarch *gdbarch, struct value *function, struct regcache *regcache,
  CORE_ADDR bp_addr, int nargs, struct value **args, CORE_ADDR sp, int struct_return, CORE_ADDR struct_addr)
{
  enum bfd_endian byte_order = gdbarch_byte_order (gdbarch);
  CORE_ADDR orig_sp = sp;
  int i, j;
  const gdb_byte *val;
  struct gdbarch_tdep *tdep = gdbarch_tdep (gdbarch);
  gdb_byte *argslotsbuf = NULL;
  unsigned int argslotsnb = 0;
  int len;
  int r0_regnum = user_reg_map_name_to_regnum (get_regcache_arch (regcache), "r0", -1);

  /* Allocate arguments to the virtual argument slots  */
  for (i = 0; i < nargs; i++)
  {
    int typelen = TYPE_LENGTH (value_enclosing_type (args[i]));
    int newslots = (typelen + 3) / 4;
    if (typelen > 4 && argslotsnb & 1)
      ++argslotsnb;
    argslotsbuf = xrealloc (argslotsbuf, (argslotsnb + newslots)*4);
    memset (&argslotsbuf[argslotsnb * 4], 0, newslots * 4);
    memcpy (&argslotsbuf[argslotsnb * 4], value_contents (args[i]), typelen);
    argslotsnb += newslots;
  }

  for (i = 0; i < (argslotsnb > 8 ? 8 : argslotsnb); i++)
    regcache_cooked_write (regcache, i + r0_regnum, &argslotsbuf[i * 4]);

  sp = k1_frame_align (gdbarch, sp);
  len = argslotsnb - 8;
  if (len > 0)
  {
    /* Align stack correctly and copy args there */
    if (argslotsnb & 0x1)
      sp -= 4;

    sp -= len * 4;
    write_memory (sp, argslotsbuf + 8 * 4, len * 4);
  }

  /* Scratch area */
  sp -= 16;

  if (struct_return)
    regcache_cooked_write_unsigned (regcache, r0_regnum + 15, struct_addr);

  regcache_cooked_write_unsigned (regcache, gdbarch_sp_regnum (gdbarch), sp);
  regcache_cooked_write_unsigned (regcache, tdep->ra_regnum, bp_addr);

  return sp + 16;
}

static void
k1_store_return_value (struct gdbarch *gdbarch, struct type *type, struct regcache *regcache, const gdb_byte *buf)
{
  int len = TYPE_LENGTH (type);
  int i = 0;
  int r0_regnum = user_reg_map_name_to_regnum (get_regcache_arch (regcache), "r0", -1);
  int sz = register_size (gdbarch, 0);

  while (len > sz)
  {
    regcache_raw_write (regcache, i + r0_regnum, buf + i * sz);
    i++, len -= sz;
  }
  if (len > 0)
  {
    gdb_byte tmp[4] = {0};
    memcpy (tmp, buf + i*sz, len);
    regcache_raw_write (regcache, i + r0_regnum, tmp);
  }
}

static void
k1_extract_return_value (struct gdbarch *gdbarch, struct type *type, struct regcache *regcache, gdb_byte *buf)
{
  int len = TYPE_LENGTH (type);
  int i = 0;
  int r0_regnum = user_reg_map_name_to_regnum (get_regcache_arch (regcache), "r0", -1);
  int sz = register_size (gdbarch, 0);

  while (len > sz)
  {
    regcache_raw_read (regcache, i + r0_regnum, buf + i * sz);
    i++, len -= sz;
  }
  if (len > 0)
  {
    gdb_byte tmp[4];
    regcache_raw_read (regcache, i + r0_regnum, tmp);
    memcpy (buf + i*sz, tmp, len);
  }
}

enum return_value_convention
k1_return_value (struct gdbarch *gdbarch, struct value *func_type, struct type *type,
  struct regcache *regcache, gdb_byte *readbuf, const gdb_byte *writebuf)
{
  if (TYPE_LENGTH (type) > 32)
    return RETURN_VALUE_STRUCT_CONVENTION;

  if (writebuf)
    k1_store_return_value (gdbarch, type, regcache, writebuf);
  else if (readbuf)
    k1_extract_return_value (gdbarch, type, regcache, readbuf);
  return RETURN_VALUE_REGISTER_CONVENTION;
}

int
k1_get_longjmp_target (struct frame_info *frame, CORE_ADDR *pc)
{
  /* R0 point to the jmpbuf, and RA is at offset 0x34 in the buf */
  gdb_byte buf[4];
  CORE_ADDR r0;
  struct gdbarch *gdbarch = get_frame_arch (frame);
  enum bfd_endian byte_order = gdbarch_byte_order (gdbarch);

  get_frame_register (frame, user_reg_map_name_to_regnum (get_frame_arch (frame), "r0", -1), buf);
  r0 = extract_unsigned_integer (buf, 4, byte_order);
  if (target_read_memory (r0 + 0x34, buf, 4))
    return 0;

  *pc = extract_unsigned_integer (buf, 4, byte_order);
  return 1;
}

static void
add_op (struct op_list **list, k1opc_t *op)
{
  struct op_list *op_l = malloc (sizeof (struct op_list));

  op_l->op = op;
  op_l->next = *list;

  *list = op_l;
}

void
k1_look_for_insns (void)
{
  int i;

  for (i = 0; i < K1_NUM_ARCHES; ++i)
  {
    k1opc_t *op;

    switch (i)
    {
      case K1_K1BDP: op = k1bdp_k1optab;
        break;
      case K1_K1BIO: op = k1bio_k1optab;
        break;
      default:
        internal_error (__FILE__, __LINE__, "Unknown arch id.");
    }

    while (op->as_op[0])
    {
      if (strcmp ("add", op->as_op) == 0)
      {
        add_op (&sp_adjust_insns[i], op);
        add_op (&prologue_helper_insns[i], op);
      }
      else if (strcmp ("sbf", op->as_op) == 0)
        add_op (&sp_adjust_insns[i], op);
      else if (strcmp ("swm", op->as_op) == 0)
        add_op (&sp_store_insns[i], op);
      else if (strcmp ("sdm", op->as_op) == 0)
        add_op (&sp_store_insns[i], op);
      else if (strcmp ("shm", op->as_op) == 0)
        add_op (&sp_store_insns[i], op);
      else if (strcmp ("sb", op->as_op) == 0)
        add_op (&sp_store_insns[i], op);
      else if (strcmp ("sh", op->as_op) == 0)
        add_op (&sp_store_insns[i], op);
      else if (strcmp ("sw", op->as_op) == 0)
        add_op (&sp_store_insns[i], op);
      else if (strcmp ("sd", op->as_op) == 0)
        add_op (&sp_store_insns[i], op);
      else if (strcmp ("make", op->as_op) == 0)
        add_op (&prologue_helper_insns[i], op);
      else if (strcmp ("sxb", op->as_op) == 0)
        add_op (&prologue_helper_insns[i], op);
      else if (strcmp ("sxh", op->as_op) == 0)
        add_op (&prologue_helper_insns[i], op);
      else if (strcmp ("extfz", op->as_op) == 0)
        add_op (&prologue_helper_insns[i], op);
      else if (strcmp ("call", op->as_op) == 0)
        add_op (&branch_insns[i], op);
      else if (strcmp ("icall", op->as_op) == 0)
        add_op (&branch_insns[i], op);
      else if (strcmp ("goto", op->as_op) == 0)
        add_op (&branch_insns[i], op);
      else if (strcmp ("igoto", op->as_op) == 0)
        add_op (&branch_insns[i], op);
      else if (strcmp ("ret", op->as_op) == 0)
        add_op (&branch_insns[i], op);
      else if (strcmp ("rfe", op->as_op) == 0)
        add_op (&branch_insns[i], op);
      else if (strcmp ("scall", op->as_op) == 0)
        add_op (&branch_insns[i], op);
      else if (strcmp ("trapa", op->as_op) == 0)
        add_op (&branch_insns[i], op);
      else if (strcmp ("trapo", op->as_op) == 0)
        add_op (&branch_insns[i], op);
      else if (strncmp ("cb.", op->as_op, 3) == 0)
        add_op (&branch_insns[i], op);
      else if (strncmp ("cjl.", op->as_op, 4) == 0)
        add_op (&branch_insns[i], op);
      else if (strcmp ("loopdo", op->as_op) == 0)
        add_op (&branch_insns[i], op);
      else if (strcmp ("loopgtz", op->as_op) == 0)
        add_op (&branch_insns[i], op);
      else if (strcmp ("loopnez", op->as_op) == 0)
        add_op (&branch_insns[i], op);
      else if (strcmp ("get", op->as_op) == 0)
        add_op (&branch_insns[i], op);
      ++op;
    }
  }
}
