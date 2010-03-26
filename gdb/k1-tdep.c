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
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#include "defs.h"
#include "arch-utils.h"
#include "dis-asm.h"
#include "dwarf2-frame.h"
#include "frame-unwind.h"
#include "gdbtypes.h"
#include "regcache.h"
#include "symtab.h"
#include "target.h"
#include "target-descriptions.h"
#include "user-regs.h"

struct gdbarch_tdep {

};

struct k1_frame_cache
{
    /* Base address.  */
    CORE_ADDR base;
    CORE_ADDR pc;
    LONGEST framesize;
    //  CORE_ADDR saved_regs[];
    CORE_ADDR saved_sp;
};

extern const char *k1_pseudo_register_name (struct gdbarch *gdbarch,
					    int regnr);
extern struct type *k1_pseudo_register_type (struct gdbarch *gdbarch, 
					     int reg_nr);
extern int k1_pseudo_register_reggroup_p (struct gdbarch *gdbarch, 
					  int regnum, 
					  struct reggroup *reggroup);

extern void k1_pseudo_register_read (struct gdbarch *gdbarch, 
				     struct regcache *regcache,
				     int regnum, gdb_byte *buf);

extern void k1_pseudo_register_write (struct gdbarch *gdbarch, 
				      struct regcache *regcache,
				      int regnum, const gdb_byte *buf);

extern int k1_dwarf2_reg_to_regnum (struct gdbarch *gdbarch, int reg);

extern const int k1_num_pseudo_regs;
extern const char *k1_pc_name;
extern const char *k1_sp_name;

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

static const gdb_byte *
k1_breakpoint_from_pc (struct gdbarch *gdbarch, CORE_ADDR *pc, int *len)
{
    //error ("Not implemented yet.");
    return NULL;
}

static CORE_ADDR
k1_adjust_breakpoint_address (struct gdbarch *gdbarch, CORE_ADDR bpaddr)
{
  gdb_byte syllab_buf[4];
  uint32_t syllab;
  enum bfd_endian order = gdbarch_byte_order_for_code (gdbarch);
  CORE_ADDR adjusted = bpaddr;
  int i = 0;

  /* Look for the end of the bundle preceeding the requested address. */
  do {
    if (target_read_memory (adjusted - 4, syllab_buf, 4) != 0)
      return adjusted;
    
    syllab = extract_unsigned_integer (syllab_buf, 4, order);
  } while ((i++ < 7) && (syllab >> 31) && (adjusted -= 4));

  if (i == 8) /* Impossible for correct instructions */
      return bpaddr;

  return adjusted;
}

/* Return PC of first real instruction.  */
/* This function is nearly cmopletely copied from
   symtab.c:skip_prologue_using_lineinfo (excpet the test in the loop
   at the end. */
static CORE_ADDR
k1_skip_prologue (struct gdbarch *gdbarch, CORE_ADDR func_addr)
{
    CORE_ADDR func_start, func_end;
    struct linetable *l;
    int ind, i, len;
    int best_lineno = 0;
    CORE_ADDR best_pc = func_addr;
    struct symtab *symtab;

    symtab = find_pc_symtab (func_addr);
    if (symtab == NULL)
	return func_addr;

    /* Give up if this symbol has no lineinfo table.  */
    l = LINETABLE (symtab);
    if (l == NULL)
	return func_addr;
    
    /* Get the range for the function's PC values, or give up if we
       cannot, for some reason.  */
    if (!find_pc_partial_function (func_addr, NULL, &func_start, &func_end))
	return func_addr;
    
    /* Linetable entries are ordered by PC values, see the commentary in
       symtab.h where `struct linetable' is defined.  Thus, the first
       entry whose PC is in the range [FUNC_START..FUNC_END[ is the
       address we are looking for.  */
    for (i = 0; i < l->nitems; i++)
	{
	    struct linetable_entry *item = &(l->item[i]);
	    
	    /* Don't use line numbers of zero, they mark special entries in
	       the table.  See the commentary on symtab.h before the
	       definition of struct linetable.  */
	    if (item->line > 0 && func_start < item->pc && item->pc < func_end)
		return item->pc;
	}
    
    return func_addr;
}

static CORE_ADDR
k1_unwind_pc (struct gdbarch *gdbarch, struct frame_info *next_frame)
{
    return frame_unwind_register_unsigned (next_frame, gdbarch_pc_regnum(gdbarch));
}

/* Given a GDB frame, determine the address of the calling function's
   frame.  This will be used to create a new GDB frame struct.  */

static void
k1_frame_this_id (struct frame_info *this_frame,
		  void **this_prologue_cache, struct frame_id *this_id)
{
    if (get_frame_func (this_frame) == get_frame_pc (this_frame)) {
	*this_id = frame_id_build (get_frame_sp (this_frame), get_frame_func (this_frame));
    }
    /* struct k1_frame_cache *cache = k1_frame_cache (this_frame, */
    /* 						   this_prologue_cache); */
    
    /* /\* This marks the outermost frame.  *\/ */
    /* if (cache->base == 0) */
    /* 	return; */
    
    /* *this_id = frame_id_build (cache->saved_sp, cache->pc); */
}

/* Get the value of register regnum in the previous stack frame.  */

static struct value *
k1_frame_prev_register (struct frame_info *this_frame,
			void **this_prologue_cache, int regnum)
{
    if (get_frame_func (this_frame) == get_frame_pc (this_frame)) {
	if (regnum == gdbarch_pc_regnum (get_frame_arch (this_frame))) 
	    return frame_unwind_got_register (this_frame,
					      regnum,
					      user_reg_map_name_to_regnum (get_frame_arch (this_frame), "ra", -1));
	if (regnum == gdbarch_sp_regnum (get_frame_arch (this_frame))) 
	    return frame_unwind_got_register (this_frame, regnum, regnum);
    }
    /* struct k1_frame_cache *cache = k1_frame_cache (this_frame, */
    /* 						   this_prologue_cache); */
    
    /* gdb_assert (regnum >= 0); */
    
    /* if (regnum == MOXIE_SP_REGNUM && cache->saved_sp) */
    /* 	return frame_unwind_got_constant (this_frame, regnum, cache->saved_sp); */
    
    /* if (regnum < MOXIE_NUM_REGS && cache->saved_regs[regnum] != REG_UNAVAIL) */
    /* 	return frame_unwind_got_memory (this_frame, regnum, */
    /* 					cache->saved_regs[regnum]); */
    
    /* return frame_unwind_got_register (this_frame, regnum, regnum); */

    return frame_unwind_got_optimized (this_frame, regnum);
}

static const struct frame_unwind k1_frame_unwind = {
    NORMAL_FRAME,
    k1_frame_this_id,
    k1_frame_prev_register,
    NULL,
    default_frame_sniffer
};

static int k1_print_insn (bfd_vma pc, disassemble_info *di)
{
    return print_insn_k1 (pc, di);
}

static void k1_store_return_value (struct gdbarch *gdbarch,
				   struct type *type,
                                   struct regcache *regcache,
                                   const gdb_byte *buf)
{
    int len = TYPE_LENGTH (type);
    // FIXME: extract first arg id from MDS
    int i = 0;
    int sz = register_size (gdbarch, 0);
    
    while (len > 0) {
	regcache_raw_write (regcache, i, buf + i*sz);
	i++, len -= sz;
    }
}

static void k1_extract_return_value (struct gdbarch *gdbarch,
				     struct type *type,
                                     struct regcache *regcache,
                                     gdb_byte *buf)
{
    int len = TYPE_LENGTH (type);
    // FIXME: extract first arg id from MDS
    int i = 0;
    int sz = register_size (gdbarch, 0);
    
    while (len > 0) {
	regcache_raw_read (regcache, i, buf + i*sz);
	i++, len -= sz;
    }
}

static enum return_value_convention
k1_return_value (struct gdbarch *gdbarch, struct type *func_type,
                 struct type *type, struct regcache *regcache,
                 gdb_byte *readbuf, const gdb_byte *writebuf)
{
    if (TYPE_LENGTH (type) > 32)
        return RETURN_VALUE_STRUCT_CONVENTION;

    if (writebuf)
        k1_store_return_value (gdbarch, type, regcache, writebuf);
    else if (readbuf)
        k1_extract_return_value (gdbarch, type, regcache, readbuf);
    return RETURN_VALUE_REGISTER_CONVENTION;
}

static struct gdbarch *
k1_gdbarch_init (struct gdbarch_info info, struct gdbarch_list *arches)
{
  struct gdbarch *gdbarch;
  struct gdbarch_tdep *tdep;
  const struct target_desc *tdesc;
  struct tdesc_arch_data *tdesc_data;
  int i, has_pc = -1, has_sp = -1;

  /* If there is already a candidate, use it.  */
  arches = gdbarch_list_lookup_by_info (arches, &info);
  if (arches != NULL)
    return arches->gdbarch;

  tdep = xzalloc (sizeof (struct gdbarch_tdep));
  gdbarch = gdbarch_alloc (&info, tdep);

  /* This could (should?) be extracted from MDS */
  set_gdbarch_short_bit (gdbarch, 16);
  set_gdbarch_int_bit (gdbarch, 32);
  set_gdbarch_long_bit (gdbarch, 32);
  set_gdbarch_long_long_bit (gdbarch, 64);
  set_gdbarch_float_bit (gdbarch, 32);
  set_gdbarch_double_bit (gdbarch, 64);
  set_gdbarch_long_double_bit (gdbarch, 128);
  set_gdbarch_ptr_bit (gdbarch, 32);

  /* Get the k1 target description from INFO.  */
  tdesc = info.target_desc;
  if (tdesc_has_registers (tdesc)) {
      set_gdbarch_num_regs (gdbarch, 0);
      tdesc_data = tdesc_data_alloc ();
      tdesc_use_registers (gdbarch, tdesc, tdesc_data);

      for (i = 0; i < gdbarch_num_regs (gdbarch); ++i)
	  if (strcmp (tdesc_register_name(gdbarch, i), k1_pc_name) == 0)
	      has_pc = i;
	  else if (strcmp (tdesc_register_name(gdbarch, i), k1_sp_name) == 0)
	      has_sp = i;
      
      if (has_pc < 0)
	  error ("There's no '%s' register!", k1_pc_name);
      
      if (has_sp < 0)
	  error ("There's no '%s' register!", k1_sp_name);
      
      set_gdbarch_pc_regnum (gdbarch, has_pc);
      set_gdbarch_sp_regnum (gdbarch, has_sp);
  } else {
      set_gdbarch_num_regs (gdbarch, 1);
      set_gdbarch_register_name (gdbarch, k1_dummy_register_name);
      set_gdbarch_register_type (gdbarch, k1_dummy_register_type);
  }

  set_gdbarch_num_pseudo_regs (gdbarch, k1_num_pseudo_regs);

  set_tdesc_pseudo_register_name (gdbarch, k1_pseudo_register_name);
  set_tdesc_pseudo_register_type (gdbarch, k1_pseudo_register_type);
  set_tdesc_pseudo_register_reggroup_p (gdbarch,
					k1_pseudo_register_reggroup_p);

  set_gdbarch_pseudo_register_read (gdbarch, k1_pseudo_register_read);
  set_gdbarch_pseudo_register_write (gdbarch, k1_pseudo_register_write);
  set_gdbarch_dwarf2_reg_to_regnum (gdbarch, k1_dwarf2_reg_to_regnum);

  set_gdbarch_return_value (gdbarch, k1_return_value);

  set_gdbarch_skip_prologue (gdbarch, k1_skip_prologue);
  set_gdbarch_unwind_pc (gdbarch, k1_unwind_pc);
  dwarf2_append_unwinders (gdbarch);
  frame_unwind_append_unwinder (gdbarch, &k1_frame_unwind);

  set_gdbarch_breakpoint_from_pc (gdbarch, k1_breakpoint_from_pc);
  set_gdbarch_adjust_breakpoint_address (gdbarch, 
					 k1_adjust_breakpoint_address);
  /* Settings that should be unnecessary.  */
  set_gdbarch_inner_than (gdbarch, core_addr_lessthan);
  set_gdbarch_print_insn (gdbarch, k1_print_insn);

  return gdbarch;
}

extern initialize_file_ftype _initialize_k1_tdep; /* -Wmissing-prototypes */

void
_initialize_k1_tdep (void)
{
  gdbarch_register (bfd_arch_k1, k1_gdbarch_init, NULL);
}
