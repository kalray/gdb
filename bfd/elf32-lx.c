/*
  THIS FILE HAS BEEN MODIFIED OR ADDED BY STMicroelectronics, Inc. 1999-2003
*/

/**
*** (c) Copyright Hewlett-Packard Company 1999-2003
**/


/**
*** static char sccs_id[] = "@(#)elf32-lx.c	1.7 02/11/00 21:26:39";
**/

/* Generic support for 32-bit ELF
   Copyright 1993 Free Software Foundation, Inc.

This file is part of BFD, the Binary File Descriptor library.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#include "bfd.h"
#include "sysdep.h"
#include "libbfd.h"
#include "elf-bfd.h"
#include "elf32-lx.h"

#define MAX_FWD_BRANCH_OFFSET (((1 << 22) - 1) << 2)
#define MAX_BWD_BRANCH_OFFSET (-((1 << 22) << 2))
#define NOP_BUNDLE ((bfd_vma)0x80000000)
#define NOP_NOP_BUNDLE ((bfd_vma)0xb1000001)

/* This does not include any relocations, but should be good enough
   for GDB to read the file.  */

typedef struct bfd_hash_entry *(*new_hash_entry_func)
  PARAMS ((struct bfd_hash_entry *, struct bfd_hash_table *, const char *));

/* In dynamically (linker-) created sections, we generally need to keep track
   of the place a symbol or expression got allocated to. This is done via hash
   tables that store entries of the following type.  */

struct elf_lx_dyn_sym_info
{
  /* The addend for which this entry is relevant.  */
  bfd_vma addend;

  /* Next addend in the list.  */
  struct elf_lx_dyn_sym_info *next;

  bfd_vma got_offset;
  bfd_vma fptr_offset;
  bfd_vma pltoff_offset;
  bfd_vma plt_offset;
  bfd_vma plt2_offset;
  bfd_vma tprel_offset;
  bfd_vma dtpndx_offset;
  bfd_vma dtpldm_offset;

  /* The symbol table entry, if any, that this was derived from.  */
  struct elf_link_hash_entry *h;

  /* Used to count non-got, non-plt relocations for delayed sizing
     of relocation sections.  */
  struct elf_lx_dyn_reloc_entry
  {
    struct elf_lx_dyn_reloc_entry *next;

    /* The section that is the target of the relocation. */
    asection *sec;

    /* The section that should contain the dynamic relocation. */
    asection *srel;

    /* The type of the dynamic relocation. */ 
    int type;

    /* Repeat count. */
    int count;
  } *reloc_entries;

  /* True when the section contents have been updated.  */
  unsigned got_done : 1;
  unsigned fptr_done : 1;
  unsigned pltoff_done : 1;
  unsigned tprel_done : 1;
  unsigned dtpldm_done : 1;
  unsigned dtpndx_done : 1;

  /* True for the different kinds of linker data we want created.  */
  unsigned want_fptr : 1;
  unsigned want_ltoff_fptr : 1;
  unsigned want_plt : 1;
  unsigned want_plt2 : 1;
  unsigned want_pltoff : 1;

  unsigned want_tprel;
  unsigned want_dtpndx;
  unsigned want_dtpldm;

  /* Count of the number of references to the got entry for this.
     Zero if there are none. */
  long int want_got;
};

struct elf_lx_local_hash_entry
{
  struct bfd_hash_entry root;
  struct elf_lx_dyn_sym_info *info;

  /* True if this hash entry's addends was translated for
     SHF_MERGE optimization.  */
  unsigned sec_merge_done : 1;
};

struct elf_lx_local_hash_table
{
  struct bfd_hash_table root;
  /* No additional fields for now.  */
};

struct elf_lx_link_hash_entry
{
  struct elf_link_hash_entry root;
  /* A pointer to the most recently used stub hash entry against this
     symbol. */
  struct elf_lx_stub_hash_entry *stub_cache;
  struct elf_lx_dyn_sym_info *info;
  /* Non-zero means allocate that many bytes of .dynbss. */
  bfd_vma dynbss_size;
  /* True when space has been allocated in dynbss section. */
  bfd_boolean dynbss_allocated;
  /* True when space has been allocated for relocation in .rel.dynbss section. */
  bfd_boolean rel_dynbss_allocated;
};

struct elf_lx_link_hash_table
{
  /* The main hash table */
  struct elf_link_hash_table root;

  /* The stub hash table.  */
  struct bfd_hash_table stub_hash_table;

  /* Linker stub bfd.  */
  bfd *stub_bfd;

  /* Linker call-backs.  */
  asection * (*add_stub_section) PARAMS ((const char *, asection *));
  void (*layout_sections_again) PARAMS ((void));

  /* Array to keep track of which stub sections have been created, and
     information on stub grouping.  */
  struct map_stub {
    /* This is the section to which stubs in the group will be
       attached.  */
    asection *link_sec;
    /* The stub section.  */
    asection *stub_sec;
  } *stub_group;

  /* Number of stubs against global syms.  */
  unsigned long stub_globals;

  /* Count of stubs for non-global symbols created so far. */
  unsigned long local_stub_count;

  /* Assorted information used by elf_lx_size_stubs.  */
  unsigned int bfd_count;
  int top_index;
  asection **input_list;

  asection *got_sec;		/* the linkage table section (or NULL) */
  asection *rel_got_sec;	/* dynamic relocation section for same */
  asection *fptr_sec;		/* function descriptor table (or NULL) */
  asection *plt_sec;		/* the primary plt section (or NULL) */
  asection *pltoff_sec;		/* PIC ABI: private descriptors for plt (or NULL) */
                                /* Embedded ABI: gotplt function pointers */
  asection *rel_pltoff_sec;	/* dynamic relocation section for same */
  asection *dynbss_sec;         /* dynbss section (or NULL) */
  asection *rel_dynbss_sec;     /* dynamic relocation section for same */
  bfd_size_type minplt_entries;	/* number of minplt entries */
  unsigned reltext : 1;		/* are there relocs against readonly sections? */
  unsigned self_dtpldm_done : 1;/* has self DTPLDM entry been finished? */
  unsigned transform_to_absolute : 1; /* transform GP-relative insts to absolute? */
  bfd_vma self_dtpldm_offset;   /* got offset to self DTPLDM entry */

  bfd *got_bfd;                 /* The bfd that owns the .got section */

  /* rel_hash information for the GOT static relocations. */
  struct elf_link_hash_entry **got_rel_hash;

  struct elf_lx_local_hash_table loc_hash_table;
};

#define elf_lx_hash_table(p) \
  ((struct elf_lx_link_hash_table *) ((p)->hash))

#define lx_stub_hash_lookup(table, string, create, copy) \
  ((struct elf_lx_stub_hash_entry *) \
   bfd_hash_lookup ((table), (string), (create), (copy)))

static bfd_boolean elf_lx_relocate_section (
                 bfd *output_bfd, struct bfd_link_info *info, bfd *input_bfd, 
                 asection *input_section, bfd_byte *contents, 
                 Elf_Internal_Rela *relocs, Elf_Internal_Sym *local_syms, 
                 asection **local_sections);
static bfd_boolean lx_elf_set_mach_from_flags PARAMS ((bfd *));
static bfd_boolean lx_mach_has_interlocks PARAMS ((bfd *));
static bfd_boolean lx_mach_has_st240_encodings PARAMS ((bfd *));
static bfd_boolean lx_elf_set_flags_from_mach PARAMS ((bfd *));
static bfd_boolean lx_elf_merge_private_bfd_data PARAMS ((bfd *, bfd *));
static bfd_boolean lx_elf_print_private_bfd_data PARAMS ((bfd *, PTR));
static bfd_boolean lx_elf_set_private_flags PARAMS ((bfd *, flagword));
static bfd_boolean lx_elf_copy_private_flags PARAMS ((bfd *, bfd *));
static bfd_boolean lx_elf_is_local_label_name PARAMS((bfd *, const char *));

static reloc_howto_type *bfd_elf32_bfd_reloc_name_lookup
  PARAMS ((bfd *abfd ATTRIBUTE_UNUSED, const char *r_name));
static reloc_howto_type *bfd_elf32_bfd_reloc_type_lookup
  PARAMS ((bfd *abfd, bfd_reloc_code_real_type code));
static bfd_boolean abi_uses_reloc_p PARAMS ((bfd_boolean, unsigned int));
static void lx_info_to_howto_rel PARAMS ((bfd *, arelent *, Elf_Internal_Rela *));
static void lx_info_to_howto  PARAMS((bfd *, 
				      arelent *, Elf_Internal_Rela *));
static bfd_boolean pic_abi_p PARAMS((bfd *));
static bfd_boolean reloc_matching_pair PARAMS((bfd *, Elf_Internal_Rela *,
					   Elf_Internal_Rela *, bfd_byte *));
static bfd_boolean elf32_lx_relax_got_load PARAMS((bfd *, struct bfd_link_info *,
					       bfd_byte *, Elf_Internal_Rela *,
					       unsigned int,
					       struct elf_link_hash_entry *,
					       Elf_Internal_Rela *,
					       Elf_Internal_Rela *,
					       bfd_boolean *, bfd_boolean *));
static bfd_boolean elf_lx_relax_section PARAMS((bfd *, asection *,
					    struct bfd_link_info *, bfd_boolean *));
static bfd_boolean elf_lx_dynamic_symbol_p PARAMS((struct elf_link_hash_entry *,
					       struct bfd_link_info *));
static bfd_boolean main_program_weak_def_p PARAMS((struct elf_link_hash_entry *,
					       struct bfd_link_info *));
static bfd_boolean elf_lx_local_hash_table_init
  PARAMS((struct elf_lx_local_hash_table *, bfd *, new_hash_entry_func));
static struct bfd_hash_entry *elf_lx_new_loc_hash_entry
  PARAMS((struct bfd_hash_entry *, struct bfd_hash_table *, const char *));
static struct bfd_hash_entry *elf_lx_new_elf_hash_entry
  PARAMS((struct bfd_hash_entry *, struct bfd_hash_table *, const char *));
static struct bfd_hash_entry *stub_hash_newfunc
  PARAMS ((struct bfd_hash_entry *, struct bfd_hash_table *, const char *));
static void transfer_relocs PARAMS((struct elf_lx_dyn_sym_info *,
				    struct elf_lx_dyn_sym_info *));
static void elf_lx_hash_copy_indirect (struct bfd_link_info *info,
				       struct elf_link_hash_entry *,
				       struct elf_link_hash_entry *);
static void elf_lx_hash_hide_symbol PARAMS((struct bfd_link_info *,
					    struct elf_link_hash_entry *,
					    bfd_boolean));
static struct bfd_link_hash_table *elf_lx_hash_table_create PARAMS((bfd *));
static void elf_lx_hash_table_free
  PARAMS ((struct bfd_link_hash_table *));
static struct elf_lx_local_hash_entry *elf_lx_local_hash_lookup
  PARAMS((struct elf_lx_local_hash_table *, const char *, bfd_boolean, bfd_boolean));
static bfd_boolean elf_lx_global_dyn_sym_thunk
  PARAMS((struct bfd_hash_entry *, PTR));
static bfd_boolean elf_lx_local_dyn_sym_thunk PARAMS((struct bfd_hash_entry *,
						  PTR));
static void elf_lx_dyn_sym_traverse
  PARAMS((struct elf_lx_link_hash_table *,
	  bfd_boolean (*func) (struct elf_lx_dyn_sym_info *, PTR),
	  PTR));
static bfd_boolean elf_lx_create_dynamic_sections
  PARAMS((bfd *, struct bfd_link_info *));
static struct elf_lx_local_hash_entry *get_local_sym_hash
  PARAMS((struct elf_lx_link_hash_table *, bfd *, const Elf_Internal_Rela *,
	  bfd_boolean));
static struct elf_lx_dyn_sym_info *get_dyn_sym_info
  PARAMS((struct elf_lx_link_hash_table *, struct elf_link_hash_entry *,
	  bfd *, const Elf_Internal_Rela *, bfd_boolean));
static asection *get_got PARAMS((bfd *, struct bfd_link_info *,
				 struct elf_lx_link_hash_table *));
static asection *get_fptr PARAMS((bfd *, struct bfd_link_info *,
				  struct elf_lx_link_hash_table *));
static asection *get_pltoff PARAMS((bfd *, struct bfd_link_info *,
				    struct elf_lx_link_hash_table *));
static asection *get_reloc_section
  PARAMS((bfd *, struct elf_lx_link_hash_table *, asection *, bfd_boolean));
static bfd_boolean count_dyn_reloc
  PARAMS((bfd *, struct elf_lx_dyn_sym_info *, asection *, asection *, int));
static bfd_boolean elf_lx_check_relocs
  PARAMS((bfd *, struct bfd_link_info *, asection *,
	  const Elf_Internal_Rela *));
static bfd_boolean allocate_global_data_got
  PARAMS((struct elf_lx_dyn_sym_info *, PTR));
static bfd_boolean allocate_global_fptr_got
  PARAMS((struct elf_lx_dyn_sym_info *, PTR));
static bfd_boolean allocate_local_got PARAMS((struct elf_lx_dyn_sym_info *, PTR));
static long global_sym_index PARAMS((struct elf_link_hash_entry *));
static bfd_boolean allocate_fptr PARAMS((struct elf_lx_dyn_sym_info *, PTR));
static bfd_boolean allocate_plt_entries
  PARAMS((struct elf_lx_dyn_sym_info *, PTR));
static bfd_boolean allocate_plt2_entries
  PARAMS((struct elf_lx_dyn_sym_info *, PTR));
static bfd_boolean allocate_pltoff_entries
  PARAMS((struct elf_lx_dyn_sym_info *, PTR));
static bfd_boolean allocate_dynrel_entries
  PARAMS((struct elf_lx_dyn_sym_info *, PTR));
static bfd_boolean elf_lx_adjust_dynamic_symbol
  PARAMS((struct bfd_link_info *, struct elf_link_hash_entry *));
static void group_sections
  PARAMS ((struct elf_lx_link_hash_table *, bfd_size_type, bfd_boolean));
static enum elf_lx_stub_type lx_type_of_stub
  PARAMS ((bfd *, struct bfd_link_info *, asection *, const Elf_Internal_Rela *,
	   struct elf_lx_link_hash_entry *, bfd_vma));
static char *lx_stub_name
  PARAMS ((const asection *, const asection *,
	   const struct elf_lx_link_hash_entry *,
	   const Elf_Internal_Rela *));
static struct elf_lx_stub_hash_entry *lx_get_stub_entry
  PARAMS ((const asection *, const asection *,
	   struct elf_link_hash_entry *,
	   const Elf_Internal_Rela *,
	   struct elf_lx_link_hash_table *));
static struct elf_lx_stub_hash_entry *lx_add_stub
  PARAMS ((const char *, asection *, struct elf_lx_link_hash_table *));
static bfd_boolean lx_size_one_stub
  PARAMS ((struct bfd_hash_entry *, PTR));
static bfd_boolean lx_build_one_stub
  PARAMS ((struct bfd_hash_entry *, PTR));
static bfd_boolean elf_lx_size_dynamic_sections
  PARAMS((bfd *, struct bfd_link_info *));
static void elf_lx_install_dyn_reloc
  PARAMS((bfd *, struct bfd_link_info *, asection *, asection *, bfd_vma,
	  unsigned int, long, bfd_vma, bfd_boolean *));
static bfd_vma set_got_entry
  PARAMS((bfd *, struct bfd_link_info *, struct elf_lx_dyn_sym_info *,
	  long, bfd_vma, bfd_vma, unsigned int));
static bfd_vma set_fptr_entry
  PARAMS((bfd *, struct bfd_link_info *, struct elf_lx_dyn_sym_info *,
	  bfd_vma));
static bfd_vma set_pltoff_entry
  PARAMS((bfd *, struct bfd_link_info *, struct elf_lx_dyn_sym_info *,
	  bfd_vma, bfd_boolean));
static bfd_vma elf_lx_tprel_base
  PARAMS ((struct bfd_link_info *info));
static bfd_vma elf_lx_dtprel_base
  PARAMS ((struct bfd_link_info *info));
static bfd_boolean elf_lx_relocate_section
  PARAMS((bfd *, struct bfd_link_info *, bfd *, asection *, bfd_byte *,
	  Elf_Internal_Rela *, Elf_Internal_Sym *, asection **));
static bfd_boolean elf_lx_output_arch_local_syms(
  bfd *output_bfd,
  struct bfd_link_info *info,
  void *finfo, bfd_boolean (*func) (void *, const char *,
				    Elf_Internal_Sym *,
				    asection *,
				    struct elf_link_hash_entry *));
static bfd_boolean elf_lx_finish_dynamic_symbol
  PARAMS((bfd *, struct bfd_link_info *, struct elf_link_hash_entry *,
	  Elf_Internal_Sym *));
static bfd_boolean elf_lx_finish_dynamic_sections
  PARAMS((bfd *, struct bfd_link_info *info));
static bfd_boolean lx_elf_set_private_flags PARAMS((bfd *, flagword));
static enum elf_reloc_type_class elf_lx_reloc_type_class
  PARAMS((const Elf_Internal_Rela *rela));
static void elf_lx_merge_symbol_attribute (struct elf_link_hash_entry *h,
					   const Elf_Internal_Sym *isym,
					   bfd_boolean definition,
					   bfd_boolean dynamic);

int elf_lx_get_compact_info (bfd *abfd, sec_ptr sec,
			     bfd_vma *initial_lma, bfd_vma *compress_lma,
			     bfd_size_type *initial_size, 
			     bfd_size_type *compress_size);
int elf_lx_sort_sections_by_lma (const void *arg1, const void *arg2);
static unsigned int elf_lx_action_discarded (asection *sec);

static const char *lx_elf_print_symbol_all
  PARAMS ((bfd *, PTR, asymbol *));
/* Defined in bfd/binary.c.  Used to set architecture of input binary files.  */
extern enum bfd_architecture bfd_external_binary_architecture;
extern unsigned long         bfd_external_machine;

bfd_reloc_status_type _bfd_final_link_relocate (reloc_howto_type *howto,
						bfd *input_bfd,
						asection *input_section,
						bfd_byte *contents,
						bfd_vma address,
						bfd_vma value,
						bfd_vma addend);

static reloc_howto_type elf32_lx_howto_table[] =
{
  HOWTO (R_LX_NONE,		/* type */
	 0,			/* rightshift */
	 0,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_LX_NONE",		/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0,			/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Simple 16 bit relocation -- for data */

  HOWTO (R_LX_16,		/* type */
	 0,			/* rightshift */
	 1,			/* size (0 = byte, 1 = short, 2 = long) */
	 16,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_LX_16",		/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0x0000ffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* Simple 32 bit relocation -- for data */

  HOWTO (R_LX_32,		/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_LX_32",		/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /*  32 bit pcrel relocation -- for data */

  HOWTO (R_LX_32_PCREL,		/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,			/* bitsize */
	 TRUE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_LX_32_PCREL",	/* name */
	 FALSE,			/* partial_inplace */
	 0,			/* src_mask */
	 0xffffffff,		/* dst_mask */
	 TRUE), 		/* pcrel_offset */

  /* A PC Relative 24-bit relocation, shifted by 2  */
  /* right container                                */

  HOWTO (R_LX_23_PCREL, 	/* type */
	 2,	                /* rightshift */
	 2,	                /* size (0 = byte, 1 = short, 2 = long) */
	 23,	                /* bitsize */
	 TRUE,	                /* pc_relative */
	 0,	                /* bitpos */
	 complain_overflow_signed, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_LX_23_PCREL",	/* name */
	 FALSE,	                /* partial_inplace */
	 0x0,		        /* src_mask */
	 0x7fffff,   		/* dst_mask */
	 TRUE), 		/* pcrel_offset */

    /* A HI part of a 32 bit absolute relocation */

  HOWTO (R_LX_HI23,		/* type */
	 9,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 23,			/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_LX_HI23",		/* name */
	 FALSE,			/* partial_inplace */
	 0x0, 		        /* src_mask */
	 0x7fffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* low part of a 32 immediate */

  HOWTO (R_LX_LO9,		/* type */
	 0,			/* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 9,			/* bitsize */
	 FALSE,			/* pc_relative */
	 12,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc,	/* special_function */
	 "R_LX_LO9",		/* name */
	 FALSE,			/* partial_inplace */
	 0x0, 		        /* src_mask */
	 0x1ff000,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* high part of a GP-relative reference */

  HOWTO (R_LX_GPREL_HI23,       /* type */
	 9,                     /* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 23,    		/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_LX_GPREL_HI23",	/* name */
	 FALSE,			/* partial_inplace */
	 0x0, 		        /* src_mask */
	 0x7fffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* low part of a GP-relative reference */

  HOWTO (R_LX_GPREL_LO9,        /* type */
	 0,                     /* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 9,    	        	/* bitsize */
	 FALSE,			/* pc_relative */
	 12,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_LX_GPREL_LO9",	/* name */
	 FALSE,			/* partial_inplace */
	 0x0, 		        /* src_mask */
	 0x1ff000,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* 32-bit relative to load address */

  HOWTO (R_LX_REL32,            /* type */
	 0,                     /* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,    	       	/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_LX_REL32",	        /* name */
	 FALSE,			/* partial_inplace */
	 0x0, 		        /* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* high part of a reference to a global offset table entry */

  HOWTO (R_LX_GOTOFF_HI23,      /* type */
	 9,                     /* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 23,    		/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_LX_GOTOFF_HI23",	/* name */
	 FALSE,			/* partial_inplace */
	 0x0, 		        /* src_mask */
	 0x7fffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* low part of a reference to a global offset table entry */

  HOWTO (R_LX_GOTOFF_LO9,       /* type */
	 0,                     /* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 9,    	        	/* bitsize */
	 FALSE,			/* pc_relative */
	 12,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_LX_GOTOFF_LO9",	/* name */
	 FALSE,			/* partial_inplace */
	 0x0, 		        /* src_mask */
	 0x1ff000,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* high part of an optimizable reference to a global offset table entry */

  HOWTO (R_LX_GOTOFFX_HI23,     /* type */
	 9,                     /* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 23,    		/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_LX_GOTOFFX_HI23",	/* name */
	 FALSE,			/* partial_inplace */
	 0x0, 		        /* src_mask */
	 0x7fffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* low part of an optimizable reference to a global offset table entry */

  HOWTO (R_LX_GOTOFFX_LO9,      /* type */
	 0,                     /* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 9,    	        	/* bitsize */
	 FALSE,			/* pc_relative */
	 12,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_LX_GOTOFFX_LO9",	/* name */
	 FALSE,			/* partial_inplace */
	 0x0, 		        /* src_mask */
	 0x1ff000,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* 32-bit relocation for data, resolves to link-time value */

  HOWTO (R_LX_LTV32,            /* type */
	 0,                     /* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,    		/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_LX_LTV32",	        /* name */
	 FALSE,			/* partial_inplace */
	 0x0, 		        /* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* 32-bit segment-relative relocation for data */

  HOWTO (R_LX_SEGREL32,         /* type */
	 0,                     /* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,    		/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_LX_SEGREL32",	/* name */
	 FALSE,			/* partial_inplace */
	 0x0, 		        /* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* 32-bit relocation to obtain official function pointer */

  HOWTO (R_LX_FPTR32,           /* type */
	 0,                     /* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,    		/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_LX_FPTR32",	        /* name */
	 FALSE,			/* partial_inplace */
	 0x0, 		        /* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* high part of a reference to a local function descriptor in the GOT */

  HOWTO (R_LX_PLTOFF_HI23,      /* type */
	 9,                     /* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 23,    		/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_LX_PLTOFF_HI23",	/* name */
	 FALSE,			/* partial_inplace */
	 0x0, 		        /* src_mask */
	 0x7fffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* low part of a reference to a local function descriptor in the GOT */

  HOWTO (R_LX_PLTOFF_LO9,       /* type */
	 0,                     /* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 9,    	        	/* bitsize */
	 FALSE,			/* pc_relative */
	 12,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_LX_PLTOFF_LO9",	/* name */
	 FALSE,			/* partial_inplace */
	 0x0, 		        /* src_mask */
	 0x1ff000,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* high part of a reference to a GOT entry that points to an official
     function descriptor */

  HOWTO (R_LX_GOTOFF_FPTR_HI23, /* type */
	 9,                     /* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 23,    		/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_LX_GOTOFF_FPTR_HI23", /* name */
	 FALSE,			/* partial_inplace */
	 0x0, 		        /* src_mask */
	 0x7fffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* low part of a reference to a GOT entry that points to an official
     function descriptor */

  HOWTO (R_LX_GOTOFF_FPTR_LO9,  /* type */
	 0,                     /* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 9,    	        	/* bitsize */
	 FALSE,			/* pc_relative */
	 12,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_LX_GOTOFF_FPTR_LO9", /* name */
	 FALSE,			/* partial_inplace */
	 0x0, 		        /* src_mask */
	 0x1ff000,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* 32-bit relocation to obtain official function pointer */

  HOWTO (R_LX_IPLT,             /* type */
	 0,                     /* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,    		/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_LX_IPLT",	        /* name */
	 FALSE,			/* partial_inplace */
	 0x0, 		        /* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* high part of a GP-relative reference */

  HOWTO (R_LX_NEG_GPREL_HI23,   /* type */
	 9,                     /* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 23,    		/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_LX_NEG_GPREL_HI23",	/* name */
	 FALSE,			/* partial_inplace */
	 0x0, 		        /* src_mask */
	 0x7fffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* low part of a GP-relative reference */

  HOWTO (R_LX_NEG_GPREL_LO9,    /* type */
	 0,                     /* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 9,    	        	/* bitsize */
	 FALSE,			/* pc_relative */
	 12,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_LX_NEG_GPREL_LO9",	/* name */
	 FALSE,			/* partial_inplace */
	 0x0, 		        /* src_mask */
	 0x1ff000,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* copy initialized data to bss */

  HOWTO (R_LX_COPY,             /* type */
	 0,                     /* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,    	        /* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_LX_COPY",	        /* name */
	 FALSE,			/* partial_inplace */
	 0x0, 		        /* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* 32-bit relocation to obtain function pointer */

  HOWTO (R_LX_JMP_SLOT,         /* type */
	 0,                     /* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,    		/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_LX_JMP_SLOT",	/* name */
	 FALSE,			/* partial_inplace */
	 0x0, 		        /* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* high part of a TP-relative reference */

  HOWTO (R_LX_TPREL_HI23,       /* type */
	 9,                     /* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 23,    		/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_LX_TPREL_HI23",	/* name */
	 FALSE,			/* partial_inplace */
	 0x0, 		        /* src_mask */
	 0x7fffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* low part of a TP-relative reference */

  HOWTO (R_LX_TPREL_LO9,        /* type */
	 0,                     /* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 9,    	        	/* bitsize */
	 FALSE,			/* pc_relative */
	 12,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_LX_TPREL_LO9",	/* name */
	 FALSE,			/* partial_inplace */
	 0x0, 		        /* src_mask */
	 0x1ff000,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* 32-bit TP-relative reference */

  HOWTO (R_LX_TPREL32,          /* type */
	 0,                     /* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,    	       	/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_LX_TPREL32",	/* name */
	 FALSE,			/* partial_inplace */
	 0x0, 		        /* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* high part of a reference to a global offset table entry containing a TP-relative offset */

  HOWTO (R_LX_GOTOFF_TPREL_HI23,/* type */
	 9,                     /* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 23,    		/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_LX_GOTOFF_TPREL_HI23",	/* name */
	 FALSE,			/* partial_inplace */
	 0x0, 		        /* src_mask */
	 0x7fffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* low part of a reference to a global offset table entry containing a TP-relative offset */

  HOWTO (R_LX_GOTOFF_TPREL_LO9, /* type */
	 0,                     /* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 9,    	        	/* bitsize */
	 FALSE,			/* pc_relative */
	 12,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_LX_GOTOFF_TPREL_LO9",	/* name */
	 FALSE,			/* partial_inplace */
	 0x0, 		        /* src_mask */
	 0x1ff000,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* high part of a reference to a global offset table entry containing a ti_index */

  HOWTO (R_LX_GOTOFF_DTPLDM_HI23,      /* type */
	 9,                     /* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 23,    		/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_LX_GOTOFF_DTPLDM_HI23",	/* name */
	 FALSE,			/* partial_inplace */
	 0x0, 		        /* src_mask */
	 0x7fffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* low part of a reference to a global offset table entry containing a ti_index */

  HOWTO (R_LX_GOTOFF_DTPLDM_LO9,/* type */
	 0,                     /* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 9,    	        	/* bitsize */
	 FALSE,			/* pc_relative */
	 12,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_LX_GOTOFF_DTPLDM_LO9",	/* name */
	 FALSE,			/* partial_inplace */
	 0x0, 		        /* src_mask */
	 0x1ff000,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* high part of offset from start of TLS block */

  HOWTO (R_LX_DTPREL_HI23,      /* type */
	 9,                     /* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 23,    		/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_LX_DTPREL_HI23",	/* name */
	 FALSE,			/* partial_inplace */
	 0x0, 		        /* src_mask */
	 0x7fffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* low part of offset from start of TLS block */

  HOWTO (R_LX_DTPREL_LO9,       /* type */
	 0,                     /* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 9,    	        	/* bitsize */
	 FALSE,			/* pc_relative */
	 12,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_LX_DTPREL_LO9",	/* name */
	 FALSE,			/* partial_inplace */
	 0x0, 		        /* src_mask */
	 0x1ff000,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* 32-bit TLS dynamic module ID */

  HOWTO (R_LX_DTPMOD32,         /* type */
	 0,                     /* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,    	       	/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_LX_DTPMOD32",	/* name */
	 FALSE,			/* partial_inplace */
	 0x0, 		        /* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* 32-bit offset from start of TLS block */

  HOWTO (R_LX_DTPREL32,         /* type */
	 0,                     /* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 32,    	       	/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_LX_DTPREL32",	/* name */
	 FALSE,			/* partial_inplace */
	 0x0, 		        /* src_mask */
	 0xffffffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* high part of a reference to a global offset table entry containing a ti_index */

  HOWTO (R_LX_GOTOFF_DTPNDX_HI23,      /* type */
	 9,                     /* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 23,    		/* bitsize */
	 FALSE,			/* pc_relative */
	 0,			/* bitpos */
	 complain_overflow_bitfield, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_LX_GOTOFF_DTPNDX_HI23",	/* name */
	 FALSE,			/* partial_inplace */
	 0x0, 		        /* src_mask */
	 0x7fffff,		/* dst_mask */
	 FALSE),		/* pcrel_offset */

  /* low part of a reference to a global offset table entry containing a ti_index */

  HOWTO (R_LX_GOTOFF_DTPNDX_LO9,       /* type */
	 0,                     /* rightshift */
	 2,			/* size (0 = byte, 1 = short, 2 = long) */
	 9,    	        	/* bitsize */
	 FALSE,			/* pc_relative */
	 12,			/* bitpos */
	 complain_overflow_dont, /* complain_on_overflow */
	 bfd_elf_generic_reloc, /* special_function */
	 "R_LX_GOTOFF_DTPNDX_LO9",	/* name */
	 FALSE,			/* partial_inplace */
	 0x0, 		        /* src_mask */
	 0x1ff000,		/* dst_mask */
	 FALSE)		        /* pcrel_offset */

};

struct lx_reloc_map
{
   bfd_reloc_code_real_type bfd_reloc_val;
  unsigned char elf_reloc_val;
  bfd_boolean used_in_embedded_abi;
  bfd_boolean used_in_pic_abi;
};

static const struct lx_reloc_map lx_reloc_map[] =
{
  { BFD_RELOC_NONE,                  R_LX_NONE,               TRUE,  TRUE  },
  { BFD_RELOC_16,                    R_LX_16,                 TRUE,  TRUE  },
  { BFD_RELOC_32,                    R_LX_32,                 TRUE,  TRUE  },
  { BFD_RELOC_32_PCREL,              R_LX_32_PCREL,           TRUE,  TRUE  },
  { BFD_RELOC_LX_23_PCREL,           R_LX_23_PCREL,           TRUE,  TRUE  },
  { BFD_RELOC_LX_HI23,               R_LX_HI23,               TRUE,  FALSE },
  { BFD_RELOC_LX_LO9,                R_LX_LO9,                TRUE,  FALSE },
  { BFD_RELOC_LX_GPREL_HI23,         R_LX_GPREL_HI23,         TRUE,  TRUE  },
  { BFD_RELOC_LX_GPREL_LO9,          R_LX_GPREL_LO9,          TRUE,  TRUE  },
  { BFD_RELOC_LX_REL32,              R_LX_REL32,              TRUE,  TRUE  },
  { BFD_RELOC_LX_GOTOFF_HI23,        R_LX_GOTOFF_HI23,        TRUE,  TRUE  },
  { BFD_RELOC_LX_GOTOFF_LO9,         R_LX_GOTOFF_LO9,         TRUE,  TRUE  },
  { BFD_RELOC_LX_GOTOFFX_HI23,       R_LX_GOTOFFX_HI23,       TRUE,  TRUE  },
  { BFD_RELOC_LX_GOTOFFX_LO9,        R_LX_GOTOFFX_LO9,        TRUE,  TRUE  },
  { BFD_RELOC_LX_LTV32,              R_LX_LTV32,              TRUE,  TRUE  },
  { BFD_RELOC_LX_SEGREL32,           R_LX_SEGREL32,           TRUE,  TRUE  },
  { BFD_RELOC_LX_FPTR32,             R_LX_FPTR32,             FALSE, TRUE  },
  { BFD_RELOC_LX_PLTOFF_HI23,        R_LX_PLTOFF_HI23,        FALSE, TRUE  },
  { BFD_RELOC_LX_PLTOFF_LO9,         R_LX_PLTOFF_LO9,         FALSE, TRUE  },
  { BFD_RELOC_LX_GOTOFF_FPTR_HI23,   R_LX_GOTOFF_FPTR_HI23,   FALSE, TRUE  },
  { BFD_RELOC_LX_GOTOFF_FPTR_LO9,    R_LX_GOTOFF_FPTR_LO9,    FALSE, TRUE  },
  { BFD_RELOC_LX_IPLT,               R_LX_IPLT,               FALSE, TRUE  },
  { BFD_RELOC_LX_NEG_GPREL_HI23,     R_LX_NEG_GPREL_HI23,     TRUE,  TRUE  },
  { BFD_RELOC_LX_NEG_GPREL_LO9,      R_LX_NEG_GPREL_LO9,      TRUE,  TRUE  },
  { BFD_RELOC_LX_COPY,               R_LX_COPY,               TRUE,  FALSE },
  { BFD_RELOC_LX_JMP_SLOT,           R_LX_JMP_SLOT,           TRUE,  FALSE },
  { BFD_RELOC_LX_TPREL_HI23,         R_LX_TPREL_HI23,         TRUE,  TRUE  },
  { BFD_RELOC_LX_TPREL_LO9,          R_LX_TPREL_LO9,          TRUE,  TRUE  },
  { BFD_RELOC_LX_TPREL32,            R_LX_TPREL32,            TRUE,  TRUE  },
  { BFD_RELOC_LX_GOTOFF_TPREL_HI23,  R_LX_GOTOFF_TPREL_HI23,  TRUE,  TRUE  },
  { BFD_RELOC_LX_GOTOFF_TPREL_LO9,   R_LX_GOTOFF_TPREL_LO9,   TRUE,  TRUE  },
  { BFD_RELOC_LX_GOTOFF_DTPLDM_HI23, R_LX_GOTOFF_DTPLDM_HI23, TRUE,  TRUE  },
  { BFD_RELOC_LX_GOTOFF_DTPLDM_LO9,  R_LX_GOTOFF_DTPLDM_LO9,  TRUE,  TRUE  },
  { BFD_RELOC_LX_DTPREL_HI23,        R_LX_DTPREL_HI23,        TRUE,  TRUE  },
  { BFD_RELOC_LX_DTPREL_LO9,         R_LX_DTPREL_LO9,         TRUE,  TRUE  },
  { BFD_RELOC_LX_DTPMOD32,           R_LX_DTPMOD32,           TRUE,  TRUE  },
  { BFD_RELOC_LX_DTPREL32,           R_LX_DTPREL32,           TRUE,  TRUE  },
  { BFD_RELOC_LX_GOTOFF_DTPNDX_HI23, R_LX_GOTOFF_DTPNDX_HI23, TRUE,  TRUE  },
  { BFD_RELOC_LX_GOTOFF_DTPNDX_LO9,  R_LX_GOTOFF_DTPNDX_LO9,  TRUE,  TRUE  }
};

static reloc_howto_type *
bfd_elf32_bfd_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED,
				 const char *r_name)
{
  unsigned int i;

  for (i = 0;
       i < (sizeof (elf32_lx_howto_table)
	    / sizeof (elf32_lx_howto_table[0]));
       i++)
    if (elf32_lx_howto_table[i].name != NULL
	&& strcasecmp (elf32_lx_howto_table[i].name, r_name) == 0)
      return &elf32_lx_howto_table[i];

  return NULL;
}

static reloc_howto_type *
bfd_elf32_bfd_reloc_type_lookup(abfd, code)
     bfd *abfd ATTRIBUTE_UNUSED;
     bfd_reloc_code_real_type code;
{
  unsigned int i;

  for (i = 0; i < sizeof(lx_reloc_map) / sizeof (struct lx_reloc_map); i++)
    {
      if (lx_reloc_map[i].bfd_reloc_val == code)
	return &elf32_lx_howto_table[lx_reloc_map[i].elf_reloc_val];
    }
  return NULL;
}

static bfd_boolean
abi_uses_reloc_p (pic_abi_p, r_type)
     bfd_boolean pic_abi_p;
     unsigned int r_type;
{
  unsigned int i;
  for (i = 0; i < sizeof(lx_reloc_map) / sizeof (struct lx_reloc_map); i++)
    {
      if (lx_reloc_map[i].elf_reloc_val == r_type)
	return pic_abi_p ? lx_reloc_map[i].used_in_pic_abi
	  : lx_reloc_map[i].used_in_embedded_abi;
    }
  return FALSE;
}

/* Set the howto pointer for an LX ELF reloc. */

static void lx_info_to_howto_rel (bfd *abfd ATTRIBUTE_UNUSED, 
				  arelent *cache_ptr, Elf_Internal_Rela *dst)
{
  unsigned r_type;
  
  r_type = ELF32_R_TYPE(dst->r_info);
  BFD_ASSERT (r_type < (unsigned int) R_LX_max);
  cache_ptr->howto = &elf32_lx_howto_table[r_type];
}

static void lx_info_to_howto (bfd *abfd ATTRIBUTE_UNUSED, 
			      arelent *cache_ptr, Elf_Internal_Rela *dst)
{
  lx_info_to_howto_rel(abfd, cache_ptr, dst);

  /* If we ever need to do any extra processing with dst->r_addend
     (the field omitted in an Elf_Internal_Rel) we can do it here.  */
}

/* The name of the dynamic interpreter.  This is put in the .interp
   section.  */

#define ELF_DYNAMIC_INTERPRETER "/lib/ld-linux.so.2"

#define L_RELOCATABLE 0

#define INSN_IS_IMML_P(I)          (((I) & 0x7f800000) == 0x15000000)
#define INSN_IS_IMMR_P(I)          (((I) & 0x7f800000) == 0x15800000)
#define INSN_IS_ADDI_R16_P(I)      (((I) & 0x7fe00fc0) == 0x08000400)
#define INSN_IS_CALL_P(I)          (((I) & 0x7f800000) == 0x30000000)
#define INSN_IS_LDW_OR_LDWD_P(B,I) (lx_mach_has_st240_encodings (B) \
                                    ? (((I) & 0x7fe00000) == 0x21000000) \
                                    : (((I) & 0x7f600000) == 0x20000000))
#define INSN_IS_BUNDLE_END_P(I)    (((I) & 0x80000000) == 0x80000000)
#define LDW_TO_ADD(I)              (((I) & ~0x7fe00000) | 0x08000000)

static struct elf_link_hash_entry *
symbol_for_relax (bfd *abfd,
		  unsigned long r_symndx)
{
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry *h;

  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;

  if (r_symndx < symtab_hdr->sh_info)
    h = NULL;
  else
    {
      h = elf_sym_hashes (abfd)[r_symndx - symtab_hdr->sh_info];
      BFD_ASSERT (h != NULL);
      while (h->root.type == bfd_link_hash_indirect
	     || h->root.type == bfd_link_hash_warning)
	h = (struct elf_link_hash_entry *)h->root.u.i.link;
    }
  return h;
}

static bfd_boolean
reloc_matching_pair (abfd, rel1, rel2, contents)
     bfd *abfd;
     Elf_Internal_Rela *rel1;
     Elf_Internal_Rela *rel2;
     bfd_byte *contents;
{
  unsigned int rel1_r_type = ELF32_R_TYPE (rel1->r_info);
  unsigned int rel2_r_type = ELF32_R_TYPE (rel2->r_info);

  /* Ensure high reloc. is in rel1 and low reloc is in rel2. */
  if (rel1_r_type == R_LX_GPREL_LO9
      || rel1_r_type == R_LX_GOTOFF_LO9
      || rel1_r_type == R_LX_GOTOFFX_LO9
      || rel1_r_type == R_LX_GOTOFF_TPREL_LO9
      || rel1_r_type == R_LX_GOTOFF_DTPLDM_LO9
      || rel1_r_type == R_LX_GOTOFF_DTPNDX_LO9)
    {
      unsigned int temp_r_type;
      Elf_Internal_Rela *temp_reloc;

      temp_r_type = rel1_r_type; rel1_r_type = rel2_r_type; rel2_r_type = temp_r_type;
      temp_reloc = rel1; rel1 = rel2; rel2 = temp_reloc;
    }
  if (ELF32_R_SYM (rel1->r_info) == ELF32_R_SYM (rel2->r_info)
      && rel1->r_addend == rel2->r_addend
      && ((rel1_r_type == R_LX_GPREL_HI23 && rel2_r_type == R_LX_GPREL_LO9)
	  || (rel1_r_type == R_LX_GOTOFF_HI23 && rel2_r_type == R_LX_GOTOFF_LO9)
	  || (rel1_r_type == R_LX_GOTOFFX_HI23 && rel2_r_type == R_LX_GOTOFFX_LO9)
	  || (rel1_r_type == R_LX_GOTOFF_TPREL_HI23
	      && rel2_r_type == R_LX_GOTOFF_TPREL_LO9)
	  || (rel1_r_type == R_LX_GOTOFF_DTPLDM_HI23
	      && rel2_r_type == R_LX_GOTOFF_DTPLDM_LO9)
	  || (rel1_r_type == R_LX_GOTOFF_DTPNDX_HI23
	      && rel2_r_type == R_LX_GOTOFF_DTPNDX_LO9)))
    {
      bfd_vma insn = bfd_get_32 (abfd, contents + rel1->r_offset);

      if (INSN_IS_IMML_P(insn)
	  && rel1->r_offset == rel2->r_offset + 4)
	return TRUE;
      else if (INSN_IS_IMMR_P(insn)
	       && rel1->r_offset + 4 == rel2->r_offset)
	return TRUE;
    }
  return FALSE;
}

static bfd_boolean elf32_lx_relax_got_load (abfd, link_info, contents, irel, r_type, h,
					irelocs, irelend,
					changed_contents, changed_relocs)
     bfd *abfd;
     struct bfd_link_info *link_info;
     bfd_byte *contents;
     Elf_Internal_Rela *irel;
     unsigned int r_type;
     struct elf_link_hash_entry *h;
     Elf_Internal_Rela *irelocs ATTRIBUTE_UNUSED;
     Elf_Internal_Rela *irelend;
     bfd_boolean *changed_contents;
     bfd_boolean *changed_relocs;
{
  struct elf_lx_dyn_sym_info *dyn_i;
  struct elf_lx_link_hash_table *lx_info;

  if (! elf_lx_dynamic_symbol_p (h, link_info)
      && irel < (irelend - 1)
      && reloc_matching_pair (abfd, irel, irel + 1, contents))
    {
      Elf_Internal_Rela *low_rel;
      Elf_Internal_Rela *high_rel;
      bfd_vma insn;

      if (r_type == R_LX_GOTOFF_LO9)
	{
	  low_rel = irel;
	  high_rel = irel + 1;
	}
      else
	{
	  low_rel = irel + 1;
	  high_rel = irel;
	  r_type = ELF32_R_TYPE (low_rel->r_info);
	}

      insn = bfd_get_32 (abfd, contents + low_rel->r_offset);

      if (r_type == R_LX_GOTOFF_LO9 && 
	  INSN_IS_LDW_OR_LDWD_P(abfd, insn))
	{ /* R_LX_GOTOFF_LO9 with an ldw or ldw.d,
	     R_LX_GOTOFF_HI23 with imml or immr.
	     Make the transformation. */
	  /* Instruction is ldw or ldw.d, convert to add. */
	  insn = LDW_TO_ADD (insn);
	  bfd_put_32 (abfd, insn, contents + low_rel->r_offset);
	  *changed_contents = TRUE;
#if 0
	  (*_bfd_error_handler)
	    (_("%s: information: converting @gotoff to @gprel for %s"),
	     bfd_get_filename (abfd),
	     (h != NULL) ? h->root.root.string : "local symbol");
#endif
	  /* Change relocation to GPREL. */
	  low_rel->r_info = ELF32_R_INFO (ELF32_R_SYM (low_rel->r_info), R_LX_GPREL_LO9);
	  high_rel->r_info = ELF32_R_INFO (ELF32_R_SYM (high_rel->r_info),
							R_LX_GPREL_HI23);
	  *changed_relocs = TRUE;
	  lx_info = elf_lx_hash_table (link_info);
	  dyn_i = get_dyn_sym_info (lx_info, h, abfd, irel, FALSE);
	  BFD_ASSERT (dyn_i != NULL);
	  BFD_ASSERT (dyn_i->want_got > 1);

	  dyn_i->want_got -= 2;
	}
    }
  return TRUE;
}

static bfd_boolean
elf_lx_relax_section (abfd, sec, link_info, again)
     bfd *abfd;
     asection *sec;
     struct bfd_link_info *link_info;
     bfd_boolean *again;
{
  Elf_Internal_Shdr *symtab_hdr;
  Elf_Internal_Rela *internal_relocs;
  Elf_Internal_Rela *irel, *irelend;
  bfd_byte *contents;
  struct elf_link_hash_entry *h;
  bfd_boolean changed_contents = FALSE;
  bfd_boolean changed_relocs = FALSE;

  /* We are not currently changing any sizes, so only one pass.  */
  *again = FALSE;

  if (link_info->relocatable
      || (sec->flags & SEC_RELOC) == 0
      || sec->reloc_count == 0)
    return TRUE;

  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;

  internal_relocs = (_bfd_elf_link_read_relocs
		     (abfd, sec, (PTR) NULL, (Elf_Internal_Rela *) NULL,
		      link_info->keep_memory));
  if (internal_relocs == NULL)
    return FALSE;
  if (elf_section_data (sec)->this_hdr.contents != NULL)
    contents = elf_section_data (sec)->this_hdr.contents;
  else
    {
      if (!bfd_malloc_and_get_section (abfd, sec, &contents))
	goto error_return;
    }

  irelend = internal_relocs + sec->reloc_count;
  for (irel = internal_relocs; irel < irelend; irel++)
    {
      unsigned int  r_type = ELF32_R_TYPE (irel->r_info);
      unsigned long r_symndx = ELF32_R_SYM (irel->r_info);

      h = symbol_for_relax (abfd, r_symndx);

      switch (r_type)
	{
	default:
	  break;
	case R_LX_GOTOFF_HI23:
	case R_LX_GOTOFF_LO9:
	  if (!elf32_lx_relax_got_load (abfd, link_info, contents, irel,
					r_type, h, internal_relocs, irelend,
					&changed_contents,
					&changed_relocs))
	    goto error_return;
	  break;
	}
    }

  if (contents != NULL
      && elf_section_data (sec)->this_hdr.contents != contents)
    {
      if (!changed_contents && !link_info->keep_memory)
	free (contents);
      else
	{
	  /* Cache the section contents for elf_link_input_bfd. */
	  elf_section_data (sec)->this_hdr.contents = contents;
	}
    }

  if (elf_section_data (sec)->relocs != internal_relocs)
    {
      if (!changed_relocs)
	free (internal_relocs);
      else
	elf_section_data (sec)->relocs = internal_relocs;
    }

  return TRUE;

 error_return:
  if (contents != NULL
      && elf_section_data (sec)->this_hdr.contents != contents)
    free (contents);
  if (internal_relocs != NULL
      && elf_section_data (sec)->relocs != internal_relocs)
    free (internal_relocs);
  return FALSE;
}

/* The size in bytes of an entry in the procedure linkage table.  */

#define PLT_HEADER_SIZE     32
#define PLT_MIN_ENTRY_SIZE  16
#define PLT_FULL_ENTRY_SIZE 32
#define PLT_FULL_ENTRY_SIZE_PIC_ABI 40
#define PLT_RESERVED_WORDS   3

/* PLT templates for PIC ABI */

static const bfd_vma pic_abi_plt_header_pre_st240[PLT_HEADER_SIZE / 4] =
  {
    /* ldw $r0.63=4[$r0.8]    */ 0x20004FC8,
    /* mov $r0.11=$r0.63   ;; */ 0x8000BFC0,
    /* ldw $r0.10=0[$r0.8] ;; */ 0xA0000288,
    /* ldw $r0.14=8[$r0.8] ;; */ 0xA0008388,
    /* nop                 ;; */ NOP_BUNDLE,
    /* goto $r0.63            */ 0x31800000,
    /* mov $r0.63=$r0.11   ;; */ 0x8003F2C0,
    /* nop                 ;; */ NOP_BUNDLE,
  };

static const bfd_vma pic_abi_plt_header_st240[PLT_HEADER_SIZE / 4] =
  {
    /* ldw $r0.63=4[$r0.8]    */ 0x21004FC8,
    /* mov $r0.11=$r0.63   ;; */ 0x8000BFC0,
    /* ldw $r0.10=0[$r0.8] ;; */ 0xA1000288,
    /* ldw $r0.14=8[$r0.8] ;; */ 0xA1008388,
    /* goto $r0.63            */ 0x31800000,
    /* mov $r0.63=$r0.11   ;; */ 0x8003F2C0,
    /* nop                 ;; */ NOP_BUNDLE,
    /* nop                 ;; */ NOP_BUNDLE,
  };

static const bfd_vma plt_min_entry[PLT_MIN_ENTRY_SIZE / 4] =
  {
    /* goto .PLT0             */ 0x31000000,
    /* mov $r0.9=0 	      */ 0x08000240,
    /* imml 0              ;; */ 0x95000000,
    /* nop                 ;; */ NOP_BUNDLE
  };

static const bfd_vma pic_abi_plt_full_entry_pre_st240[PLT_FULL_ENTRY_SIZE_PIC_ABI / 4] =
  {
    /* immr 0                 */ 0x15800000,
    /* ldw $r0.63=0[$r0.14]   */ 0x20000FCE,
    /* mov $r0.9=$r0.63    ;; */ 0x80009FC0,
    /* mov $r0.8=$r0.14    ;; */ 0x80008380,
    /* immr 0                 */ 0x15800000,
    /* ldw $r0.14=0[$r0.14];; */ 0xa000038E,
    /* nop                 ;; */ NOP_BUNDLE,
    /* goto $r0.63            */ 0x31800000,
    /* mov $r0.63=$r0.9    ;; */ 0x8003F240,
    /* nop                 ;; */ NOP_BUNDLE
  };

static const bfd_vma pic_abi_plt_full_entry_st240[PLT_FULL_ENTRY_SIZE_PIC_ABI / 4] =
  {
    /* immr 0                 */ 0x15800000,
    /* ldw $r0.63=0[$r0.14]   */ 0x21000FCE,
    /* mov $r0.9=$r0.63    ;; */ 0x80009FC0,
    /* mov $r0.8=$r0.14    ;; */ 0x80008380,
    /* immr 0                 */ 0x15800000,
    /* ldw $r0.14=0[$r0.14];; */ 0xa100038E,
    /* goto $r0.63            */ 0x31800000,
    /* mov $r0.63=$r0.9    ;; */ 0x8003F240,
    /* nop                 ;; */ NOP_BUNDLE,
    /* nop                 ;; */ NOP_BUNDLE
  };

/* PLT templates for embedded ABI */

static const bfd_vma embedded_abi_plt_header_pre_st240[PLT_HEADER_SIZE / 4] =
  {
    /* ldw $r0.63=4[$r0.14]   */ 0x20004FCE,
    /* mov $r0.11=$r0.63   ;; */ 0x8000BFC0,
    /* ldw $r0.10=0[$r0.14];; */ 0xA000028E,
    /* goto 1              ;; */ NOP_NOP_BUNDLE,
    /* goto $r0.63            */ 0x31800000,
    /* mov $r0.63=$r0.11   ;; */ 0x8003F2C0,
    /* nop                 ;; */ NOP_BUNDLE,
    /* nop                 ;; */ NOP_BUNDLE
  };

static const bfd_vma embedded_abi_plt_header_st240[PLT_HEADER_SIZE / 4] =
  {
    /* ldw $r0.63=4[$r0.14]   */ 0x21004FCE,
    /* mov $r0.11=$r0.63   ;; */ 0x8000BFC0,
    /* ldw $r0.10=0[$r0.14];; */ 0xA100028E,
    /* goto $r0.63            */ 0x31800000,
    /* mov $r0.63=$r0.11   ;; */ 0x8003F2C0,
    /* nop                 ;; */ NOP_BUNDLE,
    /* nop                 ;; */ NOP_BUNDLE,
    /* nop                 ;; */ NOP_BUNDLE
  };

static const bfd_vma embedded_abi_absolute_plt_header_pre_st240[PLT_HEADER_SIZE / 4] =
  {
    /* immr 0                 */ 0x15800000,
    /* ldw $r0.63=0[$r0.0]    */ 0x20000FC0,
    /* mov $r0.11=$r0.63   ;; */ 0x8000BFC0,
    /* ldw $r0.10=0[$r0.0]    */ 0x20000280,
    /* imml 0              ;; */ 0x95000000,
    /* goto 1              ;; */ NOP_NOP_BUNDLE,
    /* goto $r0.63            */ 0x31800000,
    /* mov $r0.63=$r0.11   ;; */ 0x8003F2C0,
  };

static const bfd_vma embedded_abi_absolute_plt_header_st240[PLT_HEADER_SIZE / 4] =
  {
    /* immr 0                 */ 0x15800000,
    /* ldw $r0.63=0[$r0.0]    */ 0x21000FC0,
    /* mov $r0.11=$r0.63   ;; */ 0x8000BFC0,
    /* ldw $r0.10=0[$r0.0]    */ 0x21000280,
    /* imml 0              ;; */ 0x95000000,
    /* goto $r0.63            */ 0x31800000,
    /* mov $r0.63=$r0.11   ;; */ 0x8003F2C0,
    /* nop                 ;; */ NOP_BUNDLE
  };

/* We use the same plt_min_entry for PIC ABI and embedded ABI,
   PIC and absolute. */

/* We use the same plt_full_entry template for embedded ABI PIC and
   absolute, but adjust the base register in one instruction. */

static const bfd_vma embedded_abi_plt_full_entry_pre_st240[PLT_FULL_ENTRY_SIZE / 4] =
  {
    /* immr 0                 */ 0x15800000,
    /* Adjust the base register on the next instruction:
       For PIC, use $r0.14, for absolute use $r0.0. */
    /* ldw $r0.63=0[$r0.0]    */ 0x20000FC0,
    /* mov $r0.9=$r0.63    ;; */ 0x80009FC0,
    /* goto 1              ;; */ NOP_NOP_BUNDLE,
    /* nop                 ;; */ NOP_BUNDLE,
    /* goto $r0.63            */ 0x31800000,
    /* mov $r0.63=$r0.9    ;; */ 0x8003F240,
    /* nop                 ;; */ NOP_BUNDLE
  };

static const bfd_vma embedded_abi_plt_full_entry_st240[PLT_FULL_ENTRY_SIZE / 4] =
  {
    /* immr 0                 */ 0x15800000,
    /* Adjust the base register on the next instruction:
       For PIC, use $r0.14, for absolute use $r0.0. */
    /* ldw $r0.63=0[$r0.0]    */ 0x21000FC0,
    /* mov $r0.9=$r0.63    ;; */ 0x80009FC0,
    /* goto $r0.63            */ 0x31800000,
    /* mov $r0.63=$r0.9    ;; */ 0x8003F240,
    /* nop                 ;; */ NOP_BUNDLE,
    /* nop                 ;; */ NOP_BUNDLE,
    /* nop                 ;; */ NOP_BUNDLE
  };

static const bfd_vma abs_long_branch_stub[] =
  {
    /* immr 0                 */ 0x15800000,
    /* movi $r63=0            */ 0x08000fc0,
    /* mov $r9=$r63        ;; */ 0x80009fc0,
    /* goto 1              ;; */ NOP_NOP_BUNDLE,
    /* goto $r63              */ 0x31800000,
    /* mov $r63=$r9        ;; */ 0x8003f240
  };

static const bfd_vma pic_long_branch_stub_pre_st240[] =
  {
    /* call $r63=8            */ 0x30000002,
    /* mov $r9=$r63        ;; */ 0x80009fc0,
    /* immr 0                 */ 0x15800000,
    /* add $r63=$r63,0     ;; */ 0x88000fff,
    /* goto 1              ;; */ NOP_NOP_BUNDLE,
    /* goto $r63              */ 0x31800000,
    /* mov $r63=$r9        ;; */ 0x8003f240
  };

static const bfd_vma pic_long_branch_stub_st240[] =
  {
    /* immr 0                 */ 0x15800000,
    /* addpc $r9 = 0       ;; */ 0x89000240,
    /* mov $r63 = $r9         */ 0x0003f240,
    /* mov $r9 = $r63      ;; */ 0x80009fc0,
    /* goto $r63              */ 0x31800000,
    /* mov $r63 = $r9      ;; */ 0x8003f240
  };

/* Section name for stubs is the associated section name plus this
   string.  */
#define STUB_SUFFIX ".stub"

enum elf_lx_stub_type {
  lx_stub_none,
  lx_stub_abs_long_branch,
  lx_stub_pic_long_branch_pre_st240,
  lx_stub_pic_long_branch_st240
};

struct elf_lx_stub_hash_entry {

  /* Base hash table entry structure.  */
  struct bfd_hash_entry root;

  /* The stub section.  */
  asection *stub_sec;

  /* Offset within stub_sec of the beginning of this stub.  */
  bfd_vma stub_offset;

  /* Given the symbol's value and its section we can determine its final
     value when building the stubs (so the stub knows where to jump).  */
  bfd_vma target_value;
  asection *target_section;

  enum elf_lx_stub_type stub_type;

  /* The symbol table entry, if any, that this was derived from.  */
  struct elf_lx_link_hash_entry *h;

  /* A symbol labelling the stub.  Only created if needed. */
  struct elf_lx_link_hash_entry *sh;

  /* Where this stub is being called from, or, in the case of combined
     stub sections, the first input section in the group.  */
  asection *id_sec;
};

/* Are we using the PIC ABI or the embedded ABI? */

static bfd_boolean
pic_abi_p (abfd)
     bfd *abfd;
{
  flagword flags;

  flags = lx_elf_get_private_flags (abfd);
  return (flags & ELF_LX_ABI_MASK) == ELF_LX_ABI_PIC;
}

/* Should we do dynamic things to this symbol?  */

static bfd_boolean
elf_lx_dynamic_symbol_p (h, info)
     struct elf_link_hash_entry *h;
     struct bfd_link_info *info;
{
  if (h == NULL)
    return FALSE;

  while (h->root.type == bfd_link_hash_indirect
	 || h->root.type == bfd_link_hash_warning)
    h = (struct elf_link_hash_entry *) h->root.u.i.link;

  if (h->dynindx == -1)
    return FALSE;
  /* Visibility attributes not supported in old binutils version */
  switch (ELF_ST_VISIBILITY (h->other))
    {
    case STV_INTERNAL:
    case STV_HIDDEN:
      return FALSE;
    }
  if (h->root.type == bfd_link_hash_undefweak
      || h->root.type == bfd_link_hash_defweak)
    return TRUE;

  if ((info->unresolved_syms_in_objects == RM_IGNORE
       && ELF_ST_VISIBILITY (h->other) == STV_DEFAULT)
      || (h->def_dynamic && h->ref_regular))
    return TRUE;
  return FALSE;
}

static bfd_boolean
main_program_weak_def_p (h, info)
     struct elf_link_hash_entry *h;
     struct bfd_link_info *info;
{
  if (h != NULL
      && ! info->shared)
    {

      while (h->root.type == bfd_link_hash_indirect
	     || h->root.type == bfd_link_hash_warning)
	h = (struct elf_link_hash_entry *) h->root.u.i.link;

      if (h->root.type == bfd_link_hash_defweak
	  && h->def_regular)
	return TRUE;
    }

  return FALSE;
}

static bfd_boolean
non_preemptible_def_p (struct elf_link_hash_entry *h,
		       struct bfd_link_info *info)
{
  if (!h)
    /* Local symbols are non-preemptible. */
    return TRUE;
  
  while (h->root.type == bfd_link_hash_indirect
	 || h->root.type == bfd_link_hash_warning)
    h = (struct elf_link_hash_entry *) h->root.u.i.link;

  /* Symbols with restricted visibility are non-preemptible. */
  switch (ELF_ST_VISIBILITY (h->other))
    {
    case STV_INTERNAL:
    case STV_HIDDEN:
      return TRUE;
    case STV_PROTECTED:
     /* Unfortunately, STV_PROTECTED symbols defined in a shared
	library can be prempted, but STV_PROTECTED symbols defined
	in a main program cannot be preempted. */
      return (h->def_regular
	      ? TRUE
	      : FALSE);
    }

  /* Symbol with a regular, non-weak definition in a main
     program is non-preemptible. */
  /* Care: common symbols can have neither ELF_LINK_HASH_DEF_REGULAR
     or ELF_LINK_HASH_DEF_DYNAMIC.  Also, symbols can have both
     ELF_LINK_HASH_DEF_REGULAR and ELF_LINK_HASH_DEF_DYNAMIC. */
  if ((h->def_regular
       || ! h->def_dynamic)
      && ! main_program_weak_def_p (h, info))
    return TRUE;

  return FALSE;
}


static bfd_boolean
elf_lx_local_hash_table_init (ht, abfd, new)
     struct elf_lx_local_hash_table *ht;
     bfd *abfd ATTRIBUTE_UNUSED;
     new_hash_entry_func new;
{
  memset (ht, 0, sizeof (*ht));
  return bfd_hash_table_init (&ht->root, new, sizeof(struct elf_lx_local_hash_entry));
}

static struct bfd_hash_entry*
elf_lx_new_loc_hash_entry (entry, table, string)
     struct bfd_hash_entry *entry;
     struct bfd_hash_table *table;
     const char *string;
{
  struct elf_lx_local_hash_entry *ret;
  ret = (struct elf_lx_local_hash_entry *) entry;

  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (!ret)
    ret = bfd_hash_allocate (table, sizeof (*ret));

  if (!ret)
    return 0;

  /* Initialize our local data.  All zeros, and definitely easier
     than setting a handful of bit fields.  */
  memset (ret, 0, sizeof (*ret));

  /* Call the allocation method of the superclass.  */
  ret = ((struct elf_lx_local_hash_entry *)
	 bfd_hash_newfunc ((struct bfd_hash_entry *) ret, table, string));

  return (struct bfd_hash_entry *) ret;
}

static struct bfd_hash_entry*
elf_lx_new_elf_hash_entry (entry, table, string)
     struct bfd_hash_entry *entry;
     struct bfd_hash_table *table;
     const char *string;
{
  struct elf_lx_link_hash_entry *ret;
  ret = (struct elf_lx_link_hash_entry *) entry;

  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (!ret)
    ret = bfd_hash_allocate (table, sizeof (*ret));

  if (!ret)
    return 0;

  /* Initialize our local data.  All zeros, and definitely easier
     than setting a handful of bit fields.  */
  memset (ret, 0, sizeof (*ret));

  /* Call the allocation method of the superclass.  */
  ret = ((struct elf_lx_link_hash_entry *)
	 _bfd_elf_link_hash_newfunc ((struct bfd_hash_entry *) ret,
				     table, string));
  if (ret != NULL)
    ret->stub_cache = NULL;

  return (struct bfd_hash_entry *) ret;
}

/* Initialize an entry in the stub hash table.  */

static struct bfd_hash_entry *
stub_hash_newfunc (entry, table, string)
     struct bfd_hash_entry *entry;
     struct bfd_hash_table *table;
     const char *string;
{
  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (entry == NULL)
    {
      entry = bfd_hash_allocate (table,
				 sizeof (struct elf_lx_stub_hash_entry));
      if (entry == NULL)
	return entry;
    }

  /* Call the allocation method of the superclass.  */
  entry = bfd_hash_newfunc (entry, table, string);
  if (entry != NULL)
    {
      struct elf_lx_stub_hash_entry *eh;

      /* Initialize the local fields.  */
      eh = (struct elf_lx_stub_hash_entry *) entry;
      eh->stub_sec = NULL;
      eh->stub_offset = 0;
      eh->target_value = 0;
      eh->target_section = NULL;
      eh->stub_type = lx_stub_none;
      eh->h = NULL;
      eh->id_sec = NULL;
    }

  return entry;
}

static void
transfer_relocs (dst, src)
     struct elf_lx_dyn_sym_info *dst;
     struct elf_lx_dyn_sym_info *src;
{
  struct elf_lx_dyn_reloc_entry **pp;
  struct elf_lx_dyn_reloc_entry *p;

  /* Note the strange way the relocs are transferred:
     for items that are on both src and dst lists,
     we update the count on the dst list and remove the item
     from the src list.
     for items that are only on the src list, leave them there.
     After walking the whole src list, append the whole dst
     list to the src list, then move the src list to the dst.
  */
  for (pp = &src->reloc_entries; (p = *pp) != NULL; )
    {
      struct elf_lx_dyn_reloc_entry *q;

      for (q = dst->reloc_entries; q != NULL; q = q->next)
	{
	  if (p->sec == q->sec
	      && p->srel == q->srel
	      && p->type == q->type)
	    {
	      q->count += p->count;
	      /* Remove this item from src list. */
	      *pp = p->next;
	      break;
	    }
	}
      if (q == NULL)
	pp = &p->next;
    }
  *pp = dst->reloc_entries;
  dst->reloc_entries = src->reloc_entries;
  src->reloc_entries = NULL;
}

static void
elf_lx_hash_copy_indirect (struct bfd_link_info *info,
			   struct elf_link_hash_entry *xdir,
			   struct elf_link_hash_entry *xind)
{
  struct elf_lx_link_hash_entry *dir, *ind;

  dir = (struct elf_lx_link_hash_entry *) xdir;
  ind = (struct elf_lx_link_hash_entry *) xind;

#if 0
  printf ("copy_indirect (%s) %s (%p) => %s (%p)\n",
	  (xind->root.type == bfd_link_hash_indirect
	   ? "indirect"
	   : "weak to strong"),
	  xind->root.root.string, xind,
	  xdir->root.root.string, xdir);
#endif

  if (ind->root.root.type == bfd_link_hash_indirect)
    {
      struct elf_lx_dyn_sym_info *dyn_i;

      if (dir->info == NULL)
	{
	  dir->info = ind->info;
	  ind->info = NULL;
	  /* Fix up the dyn_sym_info pointers to the global symbol.  */
	  for (dyn_i = dir->info; dyn_i; dyn_i = dyn_i->next)
	    dyn_i->h = &dir->root;
	}
      BFD_ASSERT (ind->info == NULL);
      dir->dynbss_size = ind->dynbss_size;
      ind->dynbss_size = 0;
      dir->dynbss_allocated = ind->dynbss_allocated;
      ind->dynbss_allocated = FALSE;
      dir->rel_dynbss_allocated = ind->rel_dynbss_allocated;
      ind->rel_dynbss_allocated = FALSE;
    }
  else
    {
      /* We are copying a weak definition to a strong
	 definition.  Transfer the relocs over, but otherwise
	 leave the info lists intact.
      */
      if (ind->info != NULL)
	{
	  struct elf_lx_dyn_sym_info *p;
	  struct elf_lx_dyn_sym_info *q;
	  struct elf_lx_dyn_sym_info *prevq;

	  /* Transfer the ind relocs to the dir list. */
	  if (dir->info != NULL
	      && xind->root.type == bfd_link_hash_indirect)
	    abort ();

	  for (p = ind->info; p != NULL; p = p->next)
	    {
	      for (prevq = NULL, q = dir->info;
		   q != NULL;
		   prevq = q, q = q->next)
		if (q->addend == p->addend)
		  break;
	      if (q == NULL)
		{
		  /* This addend not found on the direct list.
		     Create it, and append it to the direct list.
		  */

		  /* Memory leak: I would like to use bfd_zalloc here,
		     but I do not have a bfd available. */
		  q = (struct elf_lx_dyn_sym_info *)
		    bfd_malloc ((bfd_size_type) sizeof *q);
		  memset (q, 0, (bfd_size_type) sizeof *q);
		  q->h = &dir->root;
		  q->reloc_entries = NULL;
		  q->next = NULL;
		  if (prevq == NULL)
		    dir->info = q;
		  else
		    prevq->next = q;
		}
	      transfer_relocs (q, p);
	    }
	}
    }

#if 0
  {
    struct elf_lx_dyn_sym_info *p;
    printf ("  info list: ");
    for (p = dir->info; p; p = p->next)
      printf ("{ %p, %ld } ", p->h, (long)p->addend);
    printf ("\n");
  }
#endif
  _bfd_elf_link_hash_copy_indirect (info, xdir, xind);

}

static void
elf_lx_hash_hide_symbol (info, xh, force_local)
     struct bfd_link_info *info;
     struct elf_link_hash_entry *xh;
     bfd_boolean force_local;
{
  struct elf_lx_link_hash_entry *h;
  struct elf_lx_dyn_sym_info *dyn_i;

  h = (struct elf_lx_link_hash_entry *)xh;

  _bfd_elf_link_hash_hide_symbol (info, &h->root, force_local);

  for (dyn_i = h->info; dyn_i; dyn_i = dyn_i->next)
    dyn_i->want_plt2 = 0;
}

/* Create the derived linker hash table.  The lx ELF port uses this
   derived hash table to keep information specific to the lx ElF
   linker (without using static variables).  */

static struct bfd_link_hash_table*
elf_lx_hash_table_create (abfd)
     bfd *abfd;
{
  struct elf_lx_link_hash_table *ret;

  ret = bfd_zalloc (abfd, (bfd_size_type) sizeof (*ret));
  if (!ret)
    return 0;
  if (!_bfd_elf_link_hash_table_init (&ret->root, abfd,
				      elf_lx_new_elf_hash_entry,
				      sizeof (struct elf_lx_link_hash_entry),
				      LX_ELF_DATA))
    {
      bfd_release (abfd, ret);
      return 0;
    }

  if (!elf_lx_local_hash_table_init (&ret->loc_hash_table, abfd,
				     elf_lx_new_loc_hash_entry))
    return 0;

  if (!bfd_hash_table_init (&ret->stub_hash_table, stub_hash_newfunc,
			    sizeof (struct elf_lx_stub_hash_entry)))
    return 0;

  return &ret->root.root;
}

/* Free the derived linker hash table.  */

static void
elf_lx_hash_table_free (hash)
     struct bfd_link_hash_table *hash;
{
  struct elf_lx_link_hash_table *ret
    = (struct elf_lx_link_hash_table *) hash;

  bfd_hash_table_free (&ret->stub_hash_table);
  _bfd_generic_link_hash_table_free (hash);
}

/* Look up an entry in an lx ELF linker hash table.  */

static INLINE struct elf_lx_local_hash_entry *
elf_lx_local_hash_lookup(table, string, create, copy)
     struct elf_lx_local_hash_table *table;
     const char *string;
     bfd_boolean create, copy;
{
  return ((struct elf_lx_local_hash_entry *)
	  bfd_hash_lookup (&table->root, string, create, copy));
}

/* Traverse both local and global hash tables.  */

struct elf_lx_dyn_sym_traverse_data
{
  bfd_boolean (*func) PARAMS ((struct elf_lx_dyn_sym_info *, PTR));
  PTR data;
};

static bfd_boolean
elf_lx_global_dyn_sym_thunk (xentry, xdata)
     struct bfd_hash_entry *xentry;
     PTR xdata;
{
  struct elf_lx_link_hash_entry *entry
    = (struct elf_lx_link_hash_entry *) xentry;
  struct elf_lx_dyn_sym_traverse_data *data
    = (struct elf_lx_dyn_sym_traverse_data *) xdata;
  struct elf_lx_dyn_sym_info *dyn_i;

  if (entry->root.root.type == bfd_link_hash_warning)
    entry = (struct elf_lx_link_hash_entry *) entry->root.root.u.i.link;

  for (dyn_i = entry->info; dyn_i; dyn_i = dyn_i->next)
    if (! (*data->func) (dyn_i, data->data))
      return FALSE;
  return TRUE;
}

static bfd_boolean
elf_lx_local_dyn_sym_thunk (xentry, xdata)
     struct bfd_hash_entry *xentry;
     PTR xdata;
{
  struct elf_lx_local_hash_entry *entry
    = (struct elf_lx_local_hash_entry *) xentry;
  struct elf_lx_dyn_sym_traverse_data *data
    = (struct elf_lx_dyn_sym_traverse_data *) xdata;
  struct elf_lx_dyn_sym_info *dyn_i;

  for (dyn_i = entry->info; dyn_i; dyn_i = dyn_i->next)
    if (! (*data->func) (dyn_i, data->data))
      return FALSE;
  return TRUE;
}

static void
elf_lx_dyn_sym_traverse (lx_info, func, data)
     struct elf_lx_link_hash_table *lx_info;
     bfd_boolean (*func) PARAMS ((struct elf_lx_dyn_sym_info *, PTR));
     PTR data;
{
  struct elf_lx_dyn_sym_traverse_data xdata;

  xdata.func = func;
  xdata.data = data;

  elf_link_hash_traverse (&lx_info->root,
			  elf_lx_global_dyn_sym_thunk, &xdata);
  bfd_hash_traverse (&lx_info->loc_hash_table.root,
		     elf_lx_local_dyn_sym_thunk, &xdata);
}


static bfd_boolean
elf_lx_create_dynamic_sections (abfd, info)
     bfd *abfd;
     struct bfd_link_info *info;
{
  struct elf_lx_link_hash_table *lx_info;
  asection *s;

  if (! _bfd_elf_create_dynamic_sections (abfd, info))
    return FALSE;

  lx_info = elf_lx_hash_table (info);

  lx_info->plt_sec = bfd_get_section_by_name (abfd, ".plt");
  if (lx_info->got_sec == NULL)
    {
      lx_info->got_sec = bfd_get_section_by_name (abfd, ".got");
      lx_info->got_bfd = abfd;
    }

  if (!get_pltoff (abfd, info, lx_info))
    return FALSE;

  if (pic_abi_p (abfd))
    {
      s = bfd_make_section_with_flags (abfd, ".rela.lx.pltoff",
				       (SEC_ALLOC | SEC_LOAD
					| SEC_HAS_CONTENTS
					| SEC_IN_MEMORY
					| SEC_LINKER_CREATED
					| SEC_READONLY));
      if (s == NULL
	  || !bfd_set_section_alignment (abfd, s, 2))
	return FALSE;
      lx_info->rel_pltoff_sec = s;
    }
  else
    lx_info->rel_pltoff_sec = bfd_get_section_by_name (abfd, ".rela.plt");

  s = bfd_make_section_with_flags (abfd, ".rela.got",
				   (SEC_ALLOC | SEC_LOAD
				    | SEC_HAS_CONTENTS
				    | SEC_IN_MEMORY
				    | SEC_LINKER_CREATED
				    | SEC_READONLY));
  if (s == NULL
      || !bfd_set_section_alignment (abfd, s, 2))
    return FALSE;
  lx_info->rel_got_sec = s;
  lx_info->dynbss_sec = bfd_get_section_by_name (abfd, ".dynbss");
  lx_info->rel_dynbss_sec = bfd_get_section_by_name (abfd, ".rela.bss");
  return TRUE;
}

/* Find and/or create a hash entry for local symbol.  */
static struct elf_lx_local_hash_entry *
get_local_sym_hash (lx_info, abfd, rel, create)
     struct elf_lx_link_hash_table *lx_info;
     bfd *abfd;
     const Elf_Internal_Rela *rel;
     bfd_boolean create;
{
  char *addr_name;
  size_t len;
  struct elf_lx_local_hash_entry *ret;
  asection *sec = abfd->sections;
  unsigned int h = (((sec->id & 0xff) << 24) | ((sec->id & 0xff00) << 8))
                   ^ ELF32_R_SYM (rel->r_info) ^ (sec->id >> 16);

  /* Construct a string for use in the elf_lx_local_hash_table.
     name describes what was once anonymous memory.  */

  len = sizeof (unsigned int)*2 + 1 + sizeof (bfd_vma)*4 + 1 + 1;
  len += 10;	/* %u slop */

  addr_name = bfd_malloc (len);
  if (addr_name == NULL)
    return 0;
  sprintf (addr_name, "%u:%lx",
	   h, (unsigned long) ELF32_R_SYM (rel->r_info));

  /* Collect the canonical entry data for this address.  */
  ret = elf_lx_local_hash_lookup (&lx_info->loc_hash_table,
				  addr_name, create, create);
  free (addr_name);
  return ret;
}

/* Find and/or create a descriptor for dynamic symbol info.  This will
   vary based on global or local symbol, and the addend to the reloc.  */

static struct elf_lx_dyn_sym_info *
get_dyn_sym_info (lx_info, h, abfd, rel, create)
     struct elf_lx_link_hash_table *lx_info;
     struct elf_link_hash_entry *h;
     bfd *abfd;
     const Elf_Internal_Rela *rel;
     bfd_boolean create;
{
  struct elf_lx_dyn_sym_info **pp;
  struct elf_lx_dyn_sym_info *dyn_i;
  bfd_vma addend = rel ? rel->r_addend : 0;

  if (h)
    pp = &((struct elf_lx_link_hash_entry *)h)->info;
  else
    {
      struct elf_lx_local_hash_entry *loc_h;

      loc_h = get_local_sym_hash (lx_info, abfd, rel, create);
      if (!loc_h)
	{
	  BFD_ASSERT (!create);
	  return NULL;
	}

      pp = &loc_h->info;
    }

#if 0
  printf ("get_dyn_sym_info: h = %p addend = %ld\n",
	  h,
	  (long)addend);
#endif

  for (dyn_i = *pp; dyn_i && dyn_i->addend != addend; dyn_i = *pp)
    pp = &dyn_i->next;

  if (dyn_i == NULL && create)
    {
      dyn_i = ((struct elf_lx_dyn_sym_info *)
	       bfd_zalloc (abfd, (bfd_size_type) sizeof *dyn_i));
      *pp = dyn_i;
      dyn_i->addend = addend;
    }

  return dyn_i;
}

static asection *
get_got (abfd, info, lx_info)
     bfd *abfd;
     struct bfd_link_info *info;
     struct elf_lx_link_hash_table *lx_info;
{
  asection *got;
  bfd *dynobj;

  got = lx_info->got_sec;
  if (!got)
    {
      dynobj = lx_info->root.dynobj;
      if (!dynobj)
	lx_info->root.dynobj = dynobj = abfd;
      if (!_bfd_elf_create_got_section (dynobj, info))
	return 0;

      got = bfd_get_section_by_name (dynobj, ".got");
      BFD_ASSERT (got);
      lx_info->got_sec = got;
      lx_info->got_bfd = abfd;
    }

  return got;
}

/* Create function descriptor section (.opd).  This section is called .opd
   because it contains "official procedure descriptors".  The "official"
   refers to the fact that these descriptors are used when taking the address
   of a procedure, thus ensuring a unique address for each procedure.  */

static asection *
get_fptr (abfd, info, lx_info)
     bfd *abfd;
     struct bfd_link_info *info ATTRIBUTE_UNUSED;
     struct elf_lx_link_hash_table *lx_info;
{
  asection *fptr;
  bfd *dynobj;

  fptr = lx_info->fptr_sec;
  if (!fptr)
    {
      dynobj = lx_info->root.dynobj;
      if (!dynobj)
	lx_info->root.dynobj = dynobj = abfd;

      fptr = bfd_make_section_with_flags (dynobj, ".opd",
					  (SEC_ALLOC
					   | SEC_LOAD
					   | SEC_HAS_CONTENTS
					   | SEC_IN_MEMORY
					   | SEC_READONLY
					   | SEC_LINKER_CREATED));
      if (fptr == NULL
	  || !bfd_set_section_alignment (abfd, fptr, 3))
	{
	  BFD_ASSERT (0);
	  return NULL;
	}

      lx_info->fptr_sec = fptr;
    }

  return fptr;
}

static asection *
get_pltoff (abfd, info, lx_info)
     bfd *abfd;
     struct bfd_link_info *info ATTRIBUTE_UNUSED;
     struct elf_lx_link_hash_table *lx_info;
{
  asection *pltoff;
  bfd *dynobj;

  pltoff = lx_info->pltoff_sec;
  if (!pltoff)	
    {
      if (pic_abi_p (abfd))
	{
	  dynobj = lx_info->root.dynobj;
	  if (!dynobj)
	    lx_info->root.dynobj = dynobj = abfd;
	  
	  pltoff = bfd_make_section_with_flags (dynobj, ELF_STRING_lx_pltoff,
						(SEC_ALLOC
						 | SEC_LOAD
						 | SEC_HAS_CONTENTS
						 | SEC_IN_MEMORY
						 | SEC_LINKER_CREATED));
	  if (pltoff == NULL
	      || !bfd_set_section_alignment (abfd, pltoff, 3))
	    {
	      BFD_ASSERT (0);
	      return NULL;
	    }

	}
      else
	pltoff = bfd_get_section_by_name (abfd, ".got.plt");
      lx_info->pltoff_sec = pltoff;
    }

  return pltoff;
}

static asection *
get_reloc_section (abfd, lx_info, sec, create)
     bfd *abfd;
     struct elf_lx_link_hash_table *lx_info;
     asection *sec;
     bfd_boolean create;
{
  const char *srel_name;
  asection *srel;
  bfd *dynobj;

  srel_name = (bfd_elf_string_from_elf_section
	       (abfd, elf_elfheader(abfd)->e_shstrndx,
		_bfd_elf_single_rel_hdr (sec)->sh_name));
  if (srel_name == NULL)
    return NULL;

  BFD_ASSERT ((strncmp (srel_name, ".rela", 5) == 0
	       && strcmp (bfd_get_section_name (abfd, sec),
			  srel_name+5) == 0)
	      || (strncmp (srel_name, ".rel", 4) == 0
		  && strcmp (bfd_get_section_name (abfd, sec),
			     srel_name+4) == 0));

  dynobj = lx_info->root.dynobj;
  if (!dynobj)
    lx_info->root.dynobj = dynobj = abfd;

  srel = bfd_get_section_by_name (dynobj, srel_name);
  if (srel == NULL && create)
    {
      srel = bfd_make_section_with_flags (dynobj, srel_name,
					  (SEC_ALLOC
					   | SEC_LOAD
					   | SEC_HAS_CONTENTS
					   | SEC_IN_MEMORY
					   | SEC_LINKER_CREATED
					   | SEC_READONLY));
      if (srel == NULL
	  || !bfd_set_section_alignment (dynobj, srel, 2))
	return NULL;
    }

  return srel;
}

static bfd_boolean
count_dyn_reloc (abfd, dyn_i, sec, srel, type)
     bfd *abfd;
     struct elf_lx_dyn_sym_info *dyn_i;
     asection *sec;
     asection *srel;
     int type;
{
  struct elf_lx_dyn_reloc_entry *rent;

  for (rent = dyn_i->reloc_entries; rent; rent = rent->next)
    if (rent->srel == srel && rent->type == type && rent->sec == sec)
      break;

  if (!rent)
    {
      rent = ((struct elf_lx_dyn_reloc_entry *)
	      bfd_alloc (abfd, (bfd_size_type) sizeof (*rent)));
      if (!rent)
	return FALSE;

      rent->next = dyn_i->reloc_entries;
      rent->sec = sec;
      rent->srel = srel;
      rent->type = type;
      rent->count = 0;
      dyn_i->reloc_entries = rent;
    }
  rent->count++;

  return TRUE;
}

/* Determine the type of stub needed, if any, for a call.  */

static enum elf_lx_stub_type
lx_type_of_stub (abfd, info, input_sec, rel, hash, destination)
     bfd *abfd;
     struct bfd_link_info *info;
     asection *input_sec;
     const Elf_Internal_Rela *rel;
     struct elf_lx_link_hash_entry *hash ATTRIBUTE_UNUSED;
     bfd_vma destination;
{
  bfd_vma location;
  bfd_signed_vma branch_offset;

  /* Determine where the call point is.  */
  location = (input_sec->output_offset
	      + input_sec->output_section->vma
	      + rel->r_offset);

  branch_offset = (bfd_signed_vma)(destination - location);
  if (branch_offset > MAX_FWD_BRANCH_OFFSET
      || (branch_offset < MAX_BWD_BRANCH_OFFSET))
    return (info->shared)
      ? (lx_mach_has_st240_encodings (abfd)
	 ? lx_stub_pic_long_branch_st240
	 : lx_stub_pic_long_branch_pre_st240)
      : lx_stub_abs_long_branch;

  return lx_stub_none;
}

/* Build a name for an entry in the stub hash table.  */

static char *
lx_stub_name (input_section, sym_sec, hash, rel)
     const asection *input_section;
     const asection *sym_sec;
     const struct elf_lx_link_hash_entry *hash;
     const Elf_Internal_Rela *rel;
{
  char *stub_name;
  bfd_size_type len;

  if (hash)
    {
      len = 8 + 1 + strlen (hash->root.root.root.string) + 1 + 8 + 1;
      stub_name = bfd_malloc (len);
      if (stub_name != NULL)
	{
	  sprintf (stub_name, "%08x_%s+%x",
		   input_section->id & 0xffffffff,
		   hash->root.root.root.string,
		   (int) rel->r_addend & 0xffffffff);
	}
    }
  else
    {
      len = 8 + 1 + 8 + 1 + 8 + 1 + 8 + 1;
      stub_name = bfd_malloc (len);
      if (stub_name != NULL)
	{
	  sprintf (stub_name, "%08x_%x:%x+%x",
		   input_section->id & 0xffffffff,
		   sym_sec->id & 0xffffffff,
		   (int) ELF32_R_SYM (rel->r_info) & 0xffffffff,
		   (int) rel->r_addend & 0xffffffff);
	}
    }
  return stub_name;
}

void
lx_elf_init_stub_bfd (abfd, info)
     bfd *abfd;
     struct bfd_link_info *info;
{
  struct elf_lx_link_hash_table *htab;

  elf_elfheader (abfd)->e_ident[EI_CLASS] = ELFCLASS32;

  htab = elf_lx_hash_table (info);
  htab->stub_bfd = abfd;
}

/* Look up an entry in the stub hash.  Stub entries are cached because
   creating the stub name takes a bit of time.  */

static struct elf_lx_stub_hash_entry *
lx_get_stub_entry (input_section, sym_sec, hash, rel, htab)
     const asection *input_section;
     const asection *sym_sec;
     struct elf_link_hash_entry *hash;
     const Elf_Internal_Rela *rel;
     struct elf_lx_link_hash_table *htab;
{
  struct elf_lx_stub_hash_entry *stub_entry;
  struct elf_lx_link_hash_entry *h = (struct elf_lx_link_hash_entry *) hash;
  const asection *id_sec;

  if ((input_section->flags & SEC_CODE) == 0)
    return NULL;

  /* If this input section is part of a group of sections sharing one
     stub section, then use the id of the first section in the group.
     Stub names need to include a section id, as there may well be
     more than one stub used to reach say, printf, and we need to
     distinguish between them.  */
  id_sec = htab->stub_group[input_section->id].link_sec;

  if (h != NULL && h->stub_cache != NULL
      && h->stub_cache->h == h
      && h->stub_cache->id_sec == id_sec)
    {
      stub_entry = h->stub_cache;
    }
  else
    {
      char *stub_name;

      stub_name = lx_stub_name (id_sec, sym_sec, h, rel);
      if (stub_name == NULL)
	return NULL;

      stub_entry = lx_stub_hash_lookup (&htab->stub_hash_table,
					stub_name, FALSE, FALSE);
      if (h != NULL)
	h->stub_cache = stub_entry;

      free (stub_name);
    }

  return stub_entry;
}

/* Add a new stub entry to the stub hash.  Not all fields of the new
   stub entry are initialised.  */

static struct elf_lx_stub_hash_entry *
lx_add_stub (stub_name, section, htab)
     const char *stub_name;
     asection *section;
     struct elf_lx_link_hash_table *htab;
{
  asection *link_sec;
  asection *stub_sec;
  struct elf_lx_stub_hash_entry *stub_entry;

  link_sec = htab->stub_group[section->id].link_sec;
  stub_sec = htab->stub_group[section->id].stub_sec;
  if (stub_sec == NULL)
    {
      stub_sec = htab->stub_group[link_sec->id].stub_sec;
      if (stub_sec == NULL)
	{
	  size_t namelen;
	  bfd_size_type len;
	  char *s_name;

	  namelen = strlen (link_sec->name);
	  len = namelen + sizeof (STUB_SUFFIX);
	  s_name = bfd_alloc (htab->stub_bfd, len);
	  if (s_name == NULL)
	    return NULL;

	  memcpy (s_name, link_sec->name, namelen);
	  memcpy (s_name + namelen, STUB_SUFFIX, sizeof (STUB_SUFFIX));
	  stub_sec = (*htab->add_stub_section) (s_name, link_sec);
	  if (stub_sec == NULL)
	    return NULL;
	  htab->stub_group[link_sec->id].stub_sec = stub_sec;
	}
      htab->stub_group[section->id].stub_sec = stub_sec;
    }

  /* Enter this entry into the linker stub hash table.  */
  stub_entry = lx_stub_hash_lookup (&htab->stub_hash_table, stub_name,
				    TRUE, FALSE);
  if (stub_entry == NULL)
    {
      (*_bfd_error_handler) (_("%B: cannot create stub entry %s"),
			     section->owner,
			     stub_name);
      return NULL;
    }

  stub_entry->stub_sec = stub_sec;
  stub_entry->stub_offset = 0;
  stub_entry->id_sec = link_sec;
  return stub_entry;
}

static bfd_boolean
lx_build_one_stub (gen_entry, in_arg)
     struct bfd_hash_entry *gen_entry;
     PTR in_arg;
{
  struct elf_lx_stub_hash_entry *stub_entry;
  struct bfd_link_info *info;
  struct elf_lx_link_hash_table *htab;
  asection *stub_sec;
  bfd *stub_bfd;
  bfd_vma stub_addr;
  bfd_byte *loc;
  bfd_vma sym_value;
  unsigned long symndx;
  Elf_Internal_Rela *relocs;
  int template_size;
  int size;
  const bfd_vma *template;
  int i;
  bfd_boolean has_interlocks;
  bfd_vma addend;

  /* Massage our args to the form they really have.  */
  stub_entry = (struct elf_lx_stub_hash_entry *) gen_entry;
  info = (struct bfd_link_info *) in_arg;

  htab = elf_lx_hash_table (info);
  stub_sec = stub_entry->stub_sec;

  /* Make a note of the offset within the stubs for this entry.  */
  stub_entry->stub_offset = stub_sec->size;
  loc = stub_sec->contents + stub_entry->stub_offset;

  stub_bfd = htab->stub_bfd;

  /* This is the address of the start of the stub */
  stub_addr = stub_sec->output_section->vma + stub_sec->output_offset
    + stub_entry->stub_offset;

  /* This is the address of the stub destination */
  sym_value = (stub_entry->target_value
	       + stub_entry->target_section->output_offset
	       + stub_entry->target_section->output_section->vma);

  symndx = 0;
  addend = sym_value;
  relocs = NULL;

  if (info->emitrelocations)
    {
      struct bfd_elf_section_data *elfsec_data;
      struct elf_lx_link_hash_entry *h;
      struct bfd_link_hash_entry *bh = NULL;
      struct elf_link_hash_entry *eh;
      bfd_vma stub_offset;
      const char *stub_name = stub_entry->root.string;

      h = stub_entry->h;

      /* Create a symbol to label the stub.
	 For the relocations that are redirected to this stub, we will rewrite
	 them to refer to this symbol.
      */
      stub_offset = stub_sec->output_offset + stub_entry->stub_offset;
      /* Make a symbol to mark the start of the stub. */
      if (! _bfd_generic_link_add_one_symbol (info, stub_bfd, stub_name, BSF_GLOBAL,
					      stub_sec, stub_offset,
					      NULL, TRUE, FALSE, &bh))
	return FALSE;
      stub_entry->sh = (struct elf_lx_link_hash_entry *)bh;
      eh = (struct elf_link_hash_entry *)bh;
      eh->type = STT_FUNC;
      eh->other = (eh->other & ~ELF_ST_VISIBILITY (-1)) | STV_HIDDEN;
      elf_lx_hash_hide_symbol (info, eh, TRUE);

      if (stub_entry->stub_type != lx_stub_abs_long_branch)
	/* We are only able to emit relocations for an absolute long
	   branch stub.  The PC-relative long branch stubs require
	   relocation types that do not exist (we would need something
	   like R_LX_LO9_PCREL/R_LX_HI23_PCREL).
	*/
	{
	  (*_bfd_error_handler)
	    (_("%s: cannot emit relocations for PIC far call to %s"),
	       bfd_get_filename (stub_entry->id_sec->owner),
	       (h != NULL) ? h->root.root.root.string: "local symbol");
	  return FALSE;
	}
      if (h != NULL)
	{
	  struct elf_link_hash_entry **hashes;
	  
	  hashes = elf_sym_hashes (stub_bfd);
	  if (hashes == NULL)
	    {
	      bfd_size_type hsize;
	      
	      hsize = (htab->stub_globals + 1) * sizeof (*hashes);
	      hashes = bfd_zalloc (stub_bfd, hsize);
	      if (hashes == NULL)
		return FALSE;
	      elf_sym_hashes (stub_bfd) = hashes;
	      htab->stub_globals = 1;
	    }
	  symndx = htab->stub_globals++;
	  hashes[symndx] = &h->root;
	  addend = 0;
	}

      elfsec_data = elf_section_data (stub_sec);
      relocs = elfsec_data->relocs;
      if (relocs == NULL)
	{
	  bfd_size_type relsize;
	  relsize = stub_sec->reloc_count * sizeof (*relocs);
	  relocs = bfd_alloc (stub_bfd, relsize);
	  if (relocs == NULL)
	    return FALSE;
	  elfsec_data->relocs = relocs;
	  _bfd_elf_single_rel_hdr (stub_sec)->sh_size = relsize;
	  _bfd_elf_single_rel_hdr (stub_sec)->sh_entsize = sizeof (Elf32_External_Rela);
	  stub_sec->reloc_count = 0;
	}
    }

  switch (stub_entry->stub_type)
    {
    case lx_stub_abs_long_branch:
      template = abs_long_branch_stub;
      template_size = (sizeof(abs_long_branch_stub) / sizeof (bfd_vma)) * 4;
      break;
    case lx_stub_pic_long_branch_pre_st240:
      template = pic_long_branch_stub_pre_st240;
      template_size = (sizeof(pic_long_branch_stub_pre_st240) / sizeof (bfd_vma)) * 4;
      break;
    case lx_stub_pic_long_branch_st240:
      template = pic_long_branch_stub_st240;
      template_size = (sizeof(pic_long_branch_stub_st240) / sizeof (bfd_vma)) * 4;
      break;
    default:
      BFD_FAIL ();
      return FALSE;
    }

  has_interlocks = lx_mach_has_interlocks (stub_bfd);
  size = 0;
  for (i = 0; i < (template_size / 4); i++)
    if (!has_interlocks
	|| (template[i] != NOP_BUNDLE && template[i] != NOP_NOP_BUNDLE))
      {
	bfd_put_32 (stub_bfd, template[i], loc + size);
	size += 4;
      }
  if (size & 7)
    { /* To ensure all stubs are even-aligned (convenient because
	 the templates do not work on odd-alignment) we tail pad. */
      bfd_put_32 (stub_bfd, NOP_BUNDLE, loc + size);
      size += 4;
    }
  stub_sec->size += size;
  if (stub_entry->sh)
    stub_entry->sh->root.size = size;

  switch (stub_entry->stub_type)
    {
    case lx_stub_abs_long_branch:
      _bfd_final_link_relocate (elf32_lx_howto_table + R_LX_HI23, stub_bfd,
				stub_sec, stub_sec->contents,
				stub_entry->stub_offset, sym_value, 0);
      if (info->emitrelocations)
	{
	  Elf_Internal_Rela *r = relocs + stub_sec->reloc_count++;
	  r->r_offset = stub_entry->stub_offset;
	  r->r_info = ELF32_R_INFO (symndx, R_LX_HI23);
	  r->r_addend = addend;
	}
      _bfd_final_link_relocate (elf32_lx_howto_table + R_LX_LO9, stub_bfd,
				stub_sec, stub_sec->contents,
				stub_entry->stub_offset + 4, sym_value,  0);
      if (info->emitrelocations)
	{
	  Elf_Internal_Rela *r = relocs + stub_sec->reloc_count++;
	  r->r_offset = stub_entry->stub_offset + 4;
	  r->r_info = ELF32_R_INFO (symndx, R_LX_LO9);
	  r->r_addend = addend;
	}
      break;
    case lx_stub_pic_long_branch_pre_st240:
      /* We want the value relative to the address
	 8 bytes from the start of the stub */
      sym_value -= stub_addr + 8;
      _bfd_final_link_relocate (elf32_lx_howto_table + R_LX_HI23, stub_bfd,
				stub_sec, stub_sec->contents,
				stub_entry->stub_offset + 8, sym_value, 0);
      _bfd_final_link_relocate (elf32_lx_howto_table + R_LX_LO9, stub_bfd,
				stub_sec, stub_sec->contents,
				stub_entry->stub_offset + 12, sym_value,  0);
      break;
    case lx_stub_pic_long_branch_st240:
      /* We want the value relative to the address
	 of the first bundle of the stub */
      sym_value -= stub_addr;
      _bfd_final_link_relocate (elf32_lx_howto_table + R_LX_HI23, stub_bfd,
				stub_sec, stub_sec->contents,
				stub_entry->stub_offset + 0, sym_value, 0);
      _bfd_final_link_relocate (elf32_lx_howto_table + R_LX_LO9, stub_bfd,
				stub_sec, stub_sec->contents,
				stub_entry->stub_offset + 4, sym_value,  0);
    default:
      break;
    }

  return TRUE;
}

/* As above, but don't actually build the stub.  Just bump offset so
   we know stub section sizes.  */

static bfd_boolean
lx_size_one_stub (gen_entry, in_arg)
     struct bfd_hash_entry *gen_entry;
     PTR in_arg;
{
  struct elf_lx_stub_hash_entry *stub_entry;
  struct bfd_link_info *info;
  struct elf_lx_link_hash_table *htab;
  const bfd_vma *template;
  int template_size;
  int size;
  int i;
  bfd_boolean has_interlocks;
  int relocs;
  asection *stub_sec;

  /* Massage our args to the form they really have.  */
  stub_entry = (struct elf_lx_stub_hash_entry *) gen_entry;
  info = in_arg;
  htab = elf_lx_hash_table (info);

  relocs = 0;
  stub_sec = stub_entry->stub_sec;

  switch (stub_entry->stub_type)
    {
    case lx_stub_abs_long_branch:
      template =  abs_long_branch_stub;
      template_size = (sizeof(abs_long_branch_stub) / sizeof (bfd_vma)) * 4;
      relocs = 2;
      break;
    case lx_stub_pic_long_branch_pre_st240:
      template = pic_long_branch_stub_pre_st240;
      template_size = (sizeof(pic_long_branch_stub_pre_st240) / sizeof (bfd_vma)) * 4;
      break;
    case lx_stub_pic_long_branch_st240:
      template = pic_long_branch_stub_st240;
      template_size = (sizeof(pic_long_branch_stub_st240) / sizeof (bfd_vma)) * 4;
      break;
    default:
      BFD_FAIL ();
      return FALSE;
      break;
    }

  has_interlocks = lx_mach_has_interlocks (stub_sec->owner);
  size = 0;
  for (i = 0; i < (template_size/4); i++)
    if (!has_interlocks	||
	(template[i] != NOP_BUNDLE && template[i] != NOP_NOP_BUNDLE))
      size += 4;
  size = (size + 7) & ~7;
  stub_sec->size += size;

  if (info->emitrelocations)
    {
      stub_sec->reloc_count += relocs;
      stub_sec->flags |= SEC_RELOC;
    }
  
  return TRUE;
}

/* External entry points for sizing and building linker stubs.  */

/* Set up various things so that we can make a list of input sections
   for each output section included in the link.  Returns -1 on error,
   0 when no stubs will be needed, and 1 on success.  */

int
elf_lx_setup_section_lists (output_bfd, info)
     bfd *output_bfd;
     struct bfd_link_info *info;
{
  bfd *input_bfd;
  unsigned int bfd_count;
  int top_id, top_index;
  asection *section;
  asection **input_list, **list;
  bfd_size_type amt;
  struct elf_lx_link_hash_table *htab = elf_lx_hash_table (info);

  if (elf_hash_table_id ((struct elf_link_hash_table *)htab) != LX_ELF_DATA)
    return 0;

  /* Count the number of input BFDs and find the top input section id.  */
  for (input_bfd = info->input_bfds, bfd_count = 0, top_id = 0;
       input_bfd != NULL;
       input_bfd = input_bfd->link_next)
    {
      bfd_count += 1;
      for (section = input_bfd->sections;
	   section != NULL;
	   section = section->next)
	{
	  if (top_id < section->id)
	    top_id = section->id;
	}
    }
  htab->bfd_count = bfd_count;

  amt = sizeof (struct map_stub) * (top_id + 1);
  htab->stub_group = (struct map_stub *) bfd_zmalloc (amt);
  if (htab->stub_group == NULL)
    return -1;

  /* We can't use output_bfd->section_count here to find the top output
     section index as some sections may have been removed, and
     strip_excluded_output_sections doesn't renumber the indices. */
  for (section = output_bfd->sections, top_index = 0;
       section != NULL;
       section = section->next)
    {
      if (top_index < section->index)
	top_index = section->index;
    }

  htab->top_index = top_index;
  amt = sizeof (asection *) * (top_index + 1);
  input_list = (asection **) bfd_malloc (amt);
  htab->input_list = input_list;
  if (input_list == NULL)
    return -1;

  /* For sections we aren't interested in, mark their entries with a
     value we can check later.  */
  list = input_list + top_index;
  do
    *list = bfd_abs_section_ptr;
  while (list-- != input_list);

  for (section = output_bfd->sections;
       section != NULL;
       section = section->next)
    {
      if ((section->flags & SEC_CODE) != 0)
	input_list[section->index] = NULL;
    }

  return 1;
}

/* The linker repeatedly calls this function for each input section,
   in the order that input sections are linked into output sections.
   Build lists of input sections to determine groupings between which
   we may insert linker stubs.  */

void
elf_lx_next_input_section (info, isec)
     struct bfd_link_info *info;
     asection *isec;
{
  struct elf_lx_link_hash_table *htab = elf_lx_hash_table (info);

  if (isec->output_section->index <= htab->top_index)
    {
      asection **list = htab->input_list + isec->output_section->index;
      if (*list != bfd_abs_section_ptr)
	{
	  /* Steal the link_sec pointer for our list.  */
#define PREV_SEC(sec) (htab->stub_group[(sec)->id].link_sec)
	  /* This happens to make the list in reverse order,
	     which is what we want.  */
	  PREV_SEC (isec) = *list;
	  *list = isec;
	}
    }
}

/* See whether we can group stub sections together.  Grouping stub
   sections may result in fewer stubs.  More importantly, we need to
   put all .init* and .fini* stubs at the beginning of the .init or
   .fini output sections respectively, because glibc splits the
   _init and _fini functions into multiple parts.  Putting a stub in
   the middle of a function is not a good idea.  */

static void
group_sections (htab, stub_group_size, stubs_always_before_branch)
     struct elf_lx_link_hash_table *htab;
     bfd_size_type stub_group_size;
     bfd_boolean stubs_always_before_branch;
{
  asection **list = htab->input_list + htab->top_index;
  do
    {
      asection *tail = *list;
      if (tail == bfd_abs_section_ptr)
	continue;
      while (tail != NULL)
	{
	  asection *curr;
	  asection *prev;
	  bfd_size_type total;

	  curr = tail;
	  total = tail->size;
	  while ((prev = PREV_SEC (curr)) != NULL
		 && ((total += curr->output_offset - prev->output_offset)
		     < stub_group_size))
	    curr = prev;

	  /* OK, the size from the start of CURR to the end is less
	     than stub_group_size and thus can be handled by one stub
	     section.  (or the tail section is itself larger than
	     stub_group_size, in which case we may be toast.)
	     We should really be keeping track of the total size of
	     stubs added here, as stubs contribute to the final output
	     section size. */
	  do
	    {
	      prev = PREV_SEC (tail);
	      /* Set up this stub group.  */
	      htab->stub_group[tail->id].link_sec = curr;
	    }
	  while (tail != curr && (tail = prev) != NULL);

	  /* But wait, there's more!  Input sections up to stub_group_size
	     bytes before the stub section can be handled by it too.  */
	  if (!stubs_always_before_branch)
	    {
	      total = 0;
	      while (prev != NULL
		     && ((total += tail->output_offset - prev->output_offset)
			 < stub_group_size))
		{
		  tail = prev;
		  prev = PREV_SEC (tail);
		  htab->stub_group[tail->id].link_sec = curr;
		}
	    }
	  tail = prev;
	}
    }
  while (list-- != htab->input_list);
  free (htab->input_list);
#undef PREV_SEC
}

/* Determine and set the size of the stub section for a final link.

   The basic idea here is to examine all the relocations looking for
   PC-relative calls to a target that is unreachable with a "bl"
   instruction.  */

bfd_boolean
elf_lx_size_stubs (output_bfd, stub_bfd, info, group_size,
		   add_stub_section, layout_sections_again)
     bfd *output_bfd;
     bfd *stub_bfd;
     struct bfd_link_info *info;
     bfd_signed_vma group_size;
     asection * (*add_stub_section) PARAMS ((const char *, asection *));
     void (*layout_sections_again) PARAMS ((void));
{
  bfd_size_type stub_group_size;
  bfd_boolean stubs_always_before_branch;
  bfd_boolean stub_changed = 0;
  struct elf_lx_link_hash_table *htab = elf_lx_hash_table (info);

  /* Propagate mach to stub bfd, because it may not have been
     finalized when we created stub_bfd. */
  bfd_set_arch_mach (stub_bfd, bfd_get_arch (output_bfd),
		     bfd_get_mach (output_bfd));

  /* Stash our params away.  */
  htab->add_stub_section = add_stub_section;
  htab->layout_sections_again = layout_sections_again;
  stubs_always_before_branch = group_size < 0;
  if (group_size < 0)
    stub_group_size = -group_size;
  else
    stub_group_size = group_size;
  if (stub_group_size == 1)
    {
      /* Default values.  */
      /* Normal branch range is +-16MB.  This value is
	 77K less than that, which allows for 2757 28-byte stubs.
	 If we exceed that, then we will fail to link.  The user
	 will have to relink with an explicit group size option.
      */
      stub_group_size = 16700000;
    }

  group_sections (htab, stub_group_size, stubs_always_before_branch);

  while (1)
    {
      bfd *input_bfd;
      unsigned int bfd_indx;
      asection *stub_sec;

      for (input_bfd = info->input_bfds, bfd_indx = 0;
	   input_bfd != NULL;
	   input_bfd = input_bfd->link_next, bfd_indx++)
	{
	  Elf_Internal_Shdr *symtab_hdr;
	  asection *section;
	  Elf_Internal_Sym *local_syms = NULL;

	  /* We'll need the symbol table in a second.  */
	  symtab_hdr = &elf_tdata (input_bfd)->symtab_hdr;
	  if (symtab_hdr->sh_info == 0)
	    continue;

	  /* Walk over each section attached to the input bfd.  */
	  for (section = input_bfd->sections;
	       section != NULL;
	       section = section->next)
	    {
	      Elf_Internal_Rela *internal_relocs, *irelaend, *irela;

	      /* If there aren't any relocs, then there's nothing more
		 to do.  */
	      if ((section->flags & SEC_RELOC) == 0
		  || section->reloc_count == 0
		  || (section->flags & SEC_CODE) == 0)
		continue;

	      /* If this section is a link-once section that will be
		 discarded, then don't create any stubs.  */
	      if (section->output_section == NULL
		  || section->output_section->owner != output_bfd)
		continue;

	      /* Get the relocs.  */
	      internal_relocs
		= _bfd_elf_link_read_relocs (input_bfd, section, NULL,
					       (Elf_Internal_Rela *) NULL,
					       info->keep_memory);
	      if (internal_relocs == NULL)
		goto error_ret_free_local;

	      /* Now examine each relocation.  */
	      irela = internal_relocs;
	      irelaend = irela + section->reloc_count;
	      for (; irela < irelaend; irela++)
		{
		  unsigned int r_type, r_indx;
		  enum elf_lx_stub_type stub_type;
		  struct elf_lx_stub_hash_entry *stub_entry;
		  asection *sym_sec;
		  bfd_vma sym_value;
		  bfd_vma destination;
		  struct elf_lx_link_hash_entry *hash;
		  char *stub_name;
		  const asection *id_sec;

		  r_type = ELF32_R_TYPE (irela->r_info);
		  r_indx = ELF32_R_SYM (irela->r_info);

		  if (r_type >= (unsigned int) R_LX_max)
		    {
		      bfd_set_error (bfd_error_bad_value);
		    error_ret_free_internal:
		      if (elf_section_data (section)->relocs == NULL)
			free (internal_relocs);
		      goto error_ret_free_local;
		    }

		  /* Only look for stubs on call instructions.  */
		  if (r_type != (unsigned int) R_LX_23_PCREL)
		    continue;

		  /* Now determine the call target, its name, value,
		     section.  */
		  sym_sec = NULL;
		  sym_value = 0;
		  destination = 0;
		  hash = NULL;
		  if (r_indx < symtab_hdr->sh_info)
		    {
		      /* It's a local symbol.  */
		      Elf_Internal_Sym *sym;
		      Elf_Internal_Shdr *hdr;

		      if (local_syms == NULL)
			{
			  local_syms
			    = (Elf_Internal_Sym *) symtab_hdr->contents;
			  if (local_syms == NULL)
			    local_syms
			      = bfd_elf_get_elf_syms (input_bfd, symtab_hdr,
						      symtab_hdr->sh_info, 0,
						      NULL, NULL, NULL);
			  if (local_syms == NULL)
			    goto error_ret_free_internal;
			}

		      sym = local_syms + r_indx;
		      hdr = elf_elfsections (input_bfd)[sym->st_shndx];
		      sym_sec = hdr->bfd_section;
		      if (ELF_ST_TYPE (sym->st_info) != STT_SECTION)
			sym_value = sym->st_value;
		      destination = (sym_value + irela->r_addend
				     + sym_sec->output_offset
				     + sym_sec->output_section->vma);
		    }
		  else
		    {
		      /* It's an external symbol.  */
		      int e_indx;

		      e_indx = r_indx - symtab_hdr->sh_info;
		      hash = ((struct elf_lx_link_hash_entry *)
			      elf_sym_hashes (input_bfd)[e_indx]);

		      while (hash->root.root.type == bfd_link_hash_indirect
			     || hash->root.root.type == bfd_link_hash_warning)
			hash = ((struct elf_lx_link_hash_entry *)
				hash->root.root.u.i.link);

		      if (hash->root.root.type == bfd_link_hash_defined
			  || hash->root.root.type == bfd_link_hash_defweak)
			{
			  sym_sec = hash->root.root.u.def.section;
			  sym_value = hash->root.root.u.def.value;
			  if (sym_sec->output_section != NULL)
			    destination = (sym_value + irela->r_addend
					   + sym_sec->output_offset
					   + sym_sec->output_section->vma);
			}
		      else if (hash->root.root.type == bfd_link_hash_undefweak
			       || hash->root.root.type == bfd_link_hash_undefined)
			/* For a shared library, these will need a PLT stub,
			   which is treated separately.
			   For absolute code, they cannot be handled.
			*/
			continue;
		      else
			{
			  bfd_set_error (bfd_error_bad_value);
			  goto error_ret_free_internal;
			}
		    }

		  /* Determine what (if any) linker stub is needed.  */
		  stub_type = lx_type_of_stub (output_bfd, info,
					       section, irela, hash,
					       destination);
		  if (stub_type == lx_stub_none)
		    continue;

		  /* Support for grouping stub sections.  */
		  id_sec = htab->stub_group[section->id].link_sec;

		  /* Get the name of this stub.  */
		  stub_name = lx_stub_name (id_sec, sym_sec, hash, irela);
		  if (!stub_name)
		    goto error_ret_free_internal;

		  stub_entry = lx_stub_hash_lookup (&htab->stub_hash_table,
						    stub_name,
						    FALSE, FALSE);
		  if (stub_entry != NULL)
		    {
		      /* The proper stub has already been created.  */
		      free (stub_name);
		      continue;
		    }

		  stub_entry = lx_add_stub (stub_name, section, htab);
		  if (stub_entry == NULL)
		    {
		      free (stub_name);
		      goto error_ret_free_internal;
		    }

		  stub_entry->target_value = sym_value;
		  stub_entry->target_section = sym_sec;
		  stub_entry->stub_type = stub_type;
		  stub_entry->h = hash;
		  
		  if (hash != NULL)
		    htab->stub_globals += 1;

		  stub_changed = TRUE;
		}

	      /* We're done with the internal relocs, free them.  */
	      if (elf_section_data (section)->relocs == NULL)
		free (internal_relocs);
	    }
	}

      if (!stub_changed)
	break;

      /* OK, we've added some stubs.  Find out the new size of the
	 stub sections.  */
      for (stub_sec = stub_bfd->sections;
	   stub_sec != NULL;
	   stub_sec = stub_sec->next)
	{
	  stub_sec->size = 0;
	}

      bfd_hash_traverse (&htab->stub_hash_table, lx_size_one_stub, info);

      /* Ask the linker to do its stuff.  */
      (*htab->layout_sections_again) ();
      stub_changed = FALSE;
    }

  return TRUE;

 error_ret_free_local:
  return FALSE;
}

/* Build all the stubs associated with the current output file.  The
   stubs are kept in a hash table attached to the main linker hash
   table.  We also set up the .plt entries for statically linked PIC
   functions here.  This function is called via lx_elf_finish in the
   linker.  */

bfd_boolean
elf_lx_build_stubs (info)
     struct bfd_link_info *info;
{
  asection *stub_sec;
  struct bfd_hash_table *table;
  struct elf_lx_link_hash_table *htab;

  htab = elf_lx_hash_table (info);

  for (stub_sec = htab->stub_bfd->sections;
       stub_sec != NULL;
       stub_sec = stub_sec->next)
    if (stub_sec->size != 0)
      {
	bfd_size_type size;

	/* Allocate memory to hold the linker stubs.  */
	size = stub_sec->size;
	stub_sec->contents = (unsigned char *) bfd_zalloc (htab->stub_bfd, size);
	if (stub_sec->contents == NULL)
	  return FALSE;
	stub_sec->size = 0;
      }

  /* Build the stubs as directed by the stub hash table.  */
  table = &htab->stub_hash_table;
  bfd_hash_traverse (table, lx_build_one_stub, info);

  return TRUE;
}

/* Look through the relocs for a section during the first phase, and
   calculate needed space in the global offset table, procedure linkage
   table, and dynamic reloc sections.  */

static bfd_boolean
elf_lx_check_relocs (abfd, info, sec, relocs)
     bfd *abfd;
     struct bfd_link_info *info;
     asection *sec;
     const Elf_Internal_Rela *relocs;
{
  struct elf_lx_link_hash_table *lx_info;
  const Elf_Internal_Rela *rel;
  const Elf_Internal_Rela *rel_end;
  Elf_Internal_Shdr *symtab_hdr;
  asection *got, *fptr, *srel;

  if (info->relocatable)
    return TRUE;

  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  lx_info = elf_lx_hash_table (info);

  got = fptr = srel = NULL;
  rel_end = relocs + sec->reloc_count;
  for (rel = relocs; rel < rel_end; rel++)
    {
      enum {
	NEED_GOT = 1,
	NEED_FPTR = 2,
	NEED_PLTOFF = 4,
	NEED_MIN_PLT = 8,
	NEED_FULL_PLT = 16,
	NEED_DYNREL = 32,
	NEED_LTOFF_FPTR = 64,
	NEED_TPREL = 128,
	NEED_DTPLDM = 256,
	NEED_DTPNDX = 512,
	NEED_DYNBSS = 1024
      };

      struct elf_link_hash_entry *h = NULL;
      unsigned long r_symndx = ELF32_R_SYM (rel->r_info);
      struct elf_lx_dyn_sym_info *dyn_i;
      int need_entry;
      bfd_boolean maybe_dynamic;
      int dynrel_type = R_LX_NONE;
      unsigned int r_type;

      if (r_symndx >= NUM_SHDR_ENTRIES (symtab_hdr))
	{
	  (*_bfd_error_handler) (_("%B: bad symbol index: %d"),
				 abfd, r_symndx);
	  return FALSE;
	}

      if (r_symndx >= symtab_hdr->sh_info)
	{
	  /* We're dealing with a global symbol -- find its hash entry
	     and mark it as being referenced.  */
	  long indx = r_symndx - symtab_hdr->sh_info;
	  h = elf_sym_hashes (abfd)[indx];
	  while (h->root.type == bfd_link_hash_indirect
		 || h->root.type == bfd_link_hash_warning)
	    h = (struct elf_link_hash_entry *) h->root.u.i.link;

	  h->ref_regular = 1;
	}

      /* We can only get preliminary data on whether a symbol is
	 locally or externally defined, as not all of the input files
	 have yet been processed.  Do something with what we know, as
	 this may help reduce memory usage and processing time later.  */
      maybe_dynamic = FALSE;
      if (h && ((info->shared
		      && (!info->symbolic || info->unresolved_syms_in_shared_libs == RM_IGNORE))
		|| ! h->def_regular
		|| h->root.type == bfd_link_hash_defweak))
	maybe_dynamic = TRUE;

      need_entry = 0;

      r_type = ELF32_R_TYPE (rel->r_info); 
      switch (r_type)
	{
	case R_LX_GOTOFF_FPTR_HI23:
	case R_LX_GOTOFF_FPTR_LO9:
	  need_entry = NEED_FPTR | NEED_GOT | NEED_LTOFF_FPTR;
	  break;

	case R_LX_FPTR32:
	  if (info->shared || h)
	    need_entry = NEED_FPTR | NEED_DYNREL;
	  else
	    need_entry = NEED_FPTR;
	  dynrel_type = R_LX_FPTR32;
	  break;

	case R_LX_GOTOFF_HI23:
	case R_LX_GOTOFF_LO9:
	case R_LX_GOTOFFX_HI23:
	case R_LX_GOTOFFX_LO9:
	  need_entry = NEED_GOT;
	  break;

	case R_LX_PLTOFF_HI23:
	case R_LX_PLTOFF_LO9:
	  need_entry = NEED_PLTOFF;
	  if (h)
	    {
	      if (maybe_dynamic)
		need_entry |= NEED_MIN_PLT;
	    }
	  else
	    {
	      (*info->callbacks->warning)
		(info, _("@pltoff reloc against local symbol"), 0,
		 abfd, 0, (bfd_vma) 0);
	    }
	  break;

	case R_LX_23_PCREL:
	  /* This is a call or jump (tailcall) to a symbol. */
	  /* Depending on where the symbol is defined, we may or may not
	     need a full plt entry.  Only skip if we know we'll not need
	     the entry -- static or symbolic, and the symbol definition
	     has already been seen.  */
	  if (maybe_dynamic && rel->r_addend == 0)
	    need_entry = NEED_FULL_PLT;
	  break;

	case R_LX_HI23:
	case R_LX_LO9:
	case R_LX_16:
	  /* R_LX_HI23 and R_LX_LO9 use absolute addresses.  They should
	     never appear in PIC code.  Currently, we only allow PIC
	     code in shared libraries, so if we are linking a shared
	     library, there is a mistake. */
	  /* R_LX_16 should not appear in a dynamic object.
	     ??? Should this be faulted? */
	  /* ??? Would also like to check if these relocations occur
	     in a PIC main program. */
	  if (info->shared || L_RELOCATABLE)
	    {
	      (*_bfd_error_handler) (_("%B: contains absolute relocation: %d"),
				     abfd, r_type);
	      return FALSE;
	    }
	  if (h && ! (info->shared || L_RELOCATABLE))
	    {
	      h->non_got_ref = 1;
	    }
	  if (maybe_dynamic)
	    need_entry = NEED_DYNBSS | NEED_DYNREL;
	  dynrel_type = r_type;
	  break;

	case R_LX_32:
	  if (h && !(info->shared || L_RELOCATABLE))
	    {
	      h->non_got_ref = 1;
	    }
	  /* Shared objects will always need at least a REL relocation.  */
	  if (info->shared || L_RELOCATABLE || maybe_dynamic)
	    need_entry = NEED_DYNREL;
	  dynrel_type = R_LX_32;
	  break;

	case R_LX_IPLT:
	  if (info->shared || maybe_dynamic)
	    need_entry = NEED_DYNREL;
	  dynrel_type = R_LX_IPLT;
	  break;

	case R_LX_32_PCREL:
	  /* A PC-relative relocation to a symbol in another load module
	     requires a dynamic relocation. */
	  if (maybe_dynamic)
	    need_entry = NEED_DYNREL;
	  dynrel_type = R_LX_32_PCREL;
	  break;

	case R_LX_TPREL32:
	  if (info->shared || maybe_dynamic)
	    need_entry = NEED_DYNREL;
	  dynrel_type = r_type;
	  if (info->shared)
	    info->flags |= DF_STATIC_TLS;
	  break;

	case R_LX_GOTOFF_TPREL_HI23:
	case R_LX_GOTOFF_TPREL_LO9:
	  need_entry = NEED_TPREL;
	  if (info->shared)
	    info->flags |= DF_STATIC_TLS;
	  break;

	case R_LX_GOTOFF_DTPLDM_HI23:
	case R_LX_GOTOFF_DTPLDM_LO9:
	  need_entry = NEED_DTPLDM;
	  break;

	case R_LX_GOTOFF_DTPNDX_HI23:
	case R_LX_GOTOFF_DTPNDX_LO9:
	  need_entry = NEED_DTPNDX;
	  break;

	case R_LX_DTPMOD32:
	case R_LX_DTPREL32:
	  if (info->shared || maybe_dynamic)
	    need_entry = NEED_DYNREL;
	  dynrel_type = r_type;
	  break;
	}

      if (!need_entry)
	continue;

      if ((need_entry & NEED_FPTR) != 0
	  && rel->r_addend)
	{
	  (*info->callbacks->warning)
	    (info, _("non-zero addend in @fptr reloc"), 0,
	     abfd, 0, (bfd_vma) 0);
	}

      dyn_i = get_dyn_sym_info (lx_info, h, abfd, rel, TRUE);

      /* Record whether or not this is a local symbol. */
      dyn_i->h = h;

      if (need_entry & (NEED_GOT | NEED_TPREL | NEED_DTPLDM | NEED_DTPNDX))
	{
	  if (!got)
	    {
	      got = get_got (abfd, info, lx_info);
	      if (!got)
		return FALSE;
	    }
	  if (need_entry & NEED_GOT)
	    dyn_i->want_got++;
	  if (need_entry & NEED_TPREL)
	    dyn_i->want_tprel++;
	  if (need_entry & NEED_DTPLDM)
	    dyn_i->want_dtpldm++;
	  if (need_entry & NEED_DTPNDX)
	    dyn_i->want_dtpndx++;
	}
      if (need_entry & NEED_FPTR)
	{
	  if (!fptr)
	    {
	      fptr = get_fptr (abfd, info, lx_info);
	      if (!fptr)
		return FALSE;
	    }

	  /* FPTRs for shared libraries are allocated by the dynamic
	     linker.  Make sure this local symbol will appear in the
	     dynamic symbol table. */
	  if (!h && info->shared)
	    {
	      /* Old binutils does not support this, so give up. */
	      if (! (bfd_elf_link_record_local_dynamic_symbol
		     (info, abfd, (long) r_symndx)))
		return FALSE;
	    }
	  dyn_i->want_fptr = 1;
	}
      if (need_entry & NEED_LTOFF_FPTR)
	dyn_i->want_ltoff_fptr = 1;
      if (need_entry & (NEED_MIN_PLT | NEED_FULL_PLT))
	{
	  if (!lx_info->root.dynobj)
	    lx_info->root.dynobj = abfd;
	  h->needs_plt = 1;
	  dyn_i->want_plt = 1;
	}
      if (need_entry & NEED_FULL_PLT)
	dyn_i->want_plt2 = 1;
      if (need_entry & NEED_PLTOFF)
	dyn_i->want_pltoff = 1;
      if ((need_entry & NEED_DYNREL) && (sec->flags & SEC_ALLOC))
	{
	  if (!srel)
	    {
	      srel = get_reloc_section (abfd, lx_info, sec, TRUE);
	      if (!srel)
		return FALSE;
	    }
	  if (!count_dyn_reloc (abfd, dyn_i, sec, srel, dynrel_type))
	    return FALSE;
	}
    }
  return TRUE;
}

struct elf_lx_allocate_data
{
  struct bfd_link_info *info;
  bfd *output_bfd;
  bfd_size_type ofs;
  bfd_size_type align;
};

/* For cleanliness, and potentially faster dynamic loading, allocate
   external GOT entries first.  */

static bfd_boolean
allocate_global_data_got (dyn_i, data)
     struct elf_lx_dyn_sym_info *dyn_i;
     PTR data;
{
  struct elf_lx_allocate_data *x = (struct elf_lx_allocate_data *)data;
  struct elf_lx_link_hash_table *lx_info = elf_lx_hash_table (x->info);
  asection *got_sec = lx_info->got_sec;
  bfd_boolean emitrelocations = x->info->emitrelocations;

  if (dyn_i->want_got
      && ! dyn_i->want_fptr
      && elf_lx_dynamic_symbol_p (dyn_i->h, x->info))
     {
       dyn_i->got_offset = x->ofs;
       x->ofs += 4;
       if (emitrelocations)
	 got_sec->reloc_count++;
     }
  if (dyn_i->want_tprel)
    {
      dyn_i->tprel_offset = x->ofs;
      x->ofs += 4;
      if (emitrelocations)
	got_sec->reloc_count++;
    }
  if (dyn_i->want_dtpldm)
    {
      if (elf_lx_dynamic_symbol_p (dyn_i->h, x->info))
	{
	  dyn_i->dtpldm_offset = x->ofs;
	  x->ofs += 8;
	  if (emitrelocations)
	    got_sec->reloc_count++;
	}
      else
	{
	  if (lx_info->self_dtpldm_offset == (bfd_vma) -1)
	    {
	      lx_info->self_dtpldm_offset = x->ofs;
	      x->ofs += 8;
	      if (emitrelocations)
		got_sec->reloc_count++;
	    }
	  dyn_i->dtpldm_offset = lx_info->self_dtpldm_offset;
	}
    }
  if (dyn_i->want_dtpndx)
    {
      dyn_i->dtpndx_offset = x->ofs;
      x->ofs += 8;
      if (emitrelocations)
	got_sec->reloc_count += 2;
    }
  return TRUE;
}

/* Next, allocate all the GOT entries used by GOTOFF_FPTR relocs.  */

static bfd_boolean
allocate_global_fptr_got (dyn_i, data)
     struct elf_lx_dyn_sym_info *dyn_i;
     PTR data;
{
  struct elf_lx_allocate_data *x = (struct elf_lx_allocate_data *)data;
  struct elf_lx_link_hash_table *lx_info = elf_lx_hash_table (x->info);
  asection *got_sec = lx_info->got_sec;

  if (dyn_i->want_got
      && dyn_i->want_fptr
      && elf_lx_dynamic_symbol_p (dyn_i->h, x->info))
    {
      dyn_i->got_offset = x->ofs;
      x->ofs += 4;
      if (x->info->emitrelocations)
	got_sec->reloc_count += 1;
    }
  return TRUE;
}

/* Lastly, allocate all the GOT entries for local data.  */

static bfd_boolean
allocate_local_got (dyn_i, data)
     struct elf_lx_dyn_sym_info *dyn_i;
     PTR data;
{
  struct elf_lx_allocate_data *x = (struct elf_lx_allocate_data *)data;
  struct elf_lx_link_hash_table *lx_info = elf_lx_hash_table (x->info);
  asection *got_sec = lx_info->got_sec;

  if (dyn_i->want_got
      && ! elf_lx_dynamic_symbol_p (dyn_i->h, x->info))
    {
      dyn_i->got_offset = x->ofs;
      x->ofs += 4;
      if (x->info->emitrelocations)
	got_sec->reloc_count += 1;
    }
  return TRUE;
}

/* Search for the index of a global symbol in it's defining object file.  */

static long
global_sym_index (h)
     struct elf_link_hash_entry *h;
{
  struct elf_link_hash_entry **p;
  bfd *obj;

  BFD_ASSERT (h->root.type == bfd_link_hash_defined
	      || h->root.type == bfd_link_hash_defweak);

  obj = h->root.u.def.section->owner;
  for (p = elf_sym_hashes (obj); *p != h; ++p)
    continue;

  return p - elf_sym_hashes (obj) + elf_tdata (obj)->symtab_hdr.sh_info;
}

/* Allocate function descriptors.  We can do these for every function
   in a main executable that is not exported.  */

static bfd_boolean
allocate_fptr (dyn_i, data)
     struct elf_lx_dyn_sym_info *dyn_i;
     PTR data;
{
  struct elf_lx_allocate_data *x = (struct elf_lx_allocate_data *)data;

  if (dyn_i->want_fptr)
    {
      struct elf_link_hash_entry *h = dyn_i->h;

      if (h)
	while (h->root.type == bfd_link_hash_indirect
	       || h->root.type == bfd_link_hash_warning)
	  h = (struct elf_link_hash_entry *) h->root.u.i.link;

      if (x->info->shared)
	{
	  if (h && h->dynindx == -1)
	    {
	      BFD_ASSERT ((h->root.type == bfd_link_hash_defined)
			  || (h->root.type == bfd_link_hash_defweak));

	      if (!bfd_elf_link_record_local_dynamic_symbol
		    (x->info, h->root.u.def.section->owner,
		     global_sym_index (h)))
		return FALSE;
	    }

	  dyn_i->want_fptr = 0;
	}
      else if (h == NULL || h->dynindx == -1)
	{
	  dyn_i->fptr_offset = x->ofs;
	  x->ofs += 8;
	}
      else
	dyn_i->want_fptr = 0;
    }
  return TRUE;
}

/* Allocate all the minimal PLT entries.  */

static bfd_boolean
allocate_plt_entries (dyn_i, data)
     struct elf_lx_dyn_sym_info *dyn_i;
     PTR data;
{
  struct elf_lx_allocate_data *x = (struct elf_lx_allocate_data *)data;

  if (dyn_i->want_plt)
    {
      struct elf_link_hash_entry *h = dyn_i->h;

      if (h)
	while (h->root.type == bfd_link_hash_indirect
	       || h->root.type == bfd_link_hash_warning)
	  h = (struct elf_link_hash_entry *) h->root.u.i.link;

      /* ??? Versioned symbols seem to lose ELF_LINK_HASH_NEEDS_PLT.  */
      if (elf_lx_dynamic_symbol_p (h, x->info)
	  && !main_program_weak_def_p (h, x->info))
	{
	  bfd_size_type offset = x->ofs;
	  if (offset == 0)
	    offset = PLT_HEADER_SIZE;
	  dyn_i->plt_offset = offset;
	  
	  x->ofs = offset + PLT_MIN_ENTRY_SIZE;

	  dyn_i->want_pltoff = 1;
	}
      else
	{
	  dyn_i->want_plt = 0;
	  dyn_i->want_plt2 = 0;
	}
    }
  return TRUE;
}

/* Allocate all the full PLT entries.  */

static bfd_boolean
allocate_plt2_entries (dyn_i, data)
     struct elf_lx_dyn_sym_info *dyn_i;
     PTR data;
{
  struct elf_lx_allocate_data *x = (struct elf_lx_allocate_data *)data;

  if (dyn_i->want_plt2)
    {
      struct elf_link_hash_entry *h = dyn_i->h;
      bfd_size_type ofs = x->ofs;

      dyn_i->plt2_offset = ofs;

      x->ofs = ofs + (pic_abi_p (x->output_bfd)
		      ? PLT_FULL_ENTRY_SIZE_PIC_ABI
		      : PLT_FULL_ENTRY_SIZE);

      while (h->root.type == bfd_link_hash_indirect
	     || h->root.type == bfd_link_hash_warning)
	h = (struct elf_link_hash_entry *) h->root.u.i.link;
      dyn_i->h->plt.offset = ofs;

      /* If this symbol is not defined in a regular file, and we are
	 not generating a shared library, then set the symbol to this
	 location in the .plt.  This is required to make function
	 pointers compare as equal between the normal executable and
	 the shared library.  */
      if (! pic_abi_p (x->output_bfd)
	  && ! x->info->shared
	  && h
	  && ! h->def_regular)
	{
	  struct elf_lx_link_hash_table *lx_info;
	  
	  lx_info = elf_lx_hash_table (x->info);
	  h->root.u.def.section = lx_info->plt_sec;
	  h->root.u.def.value = ofs;
	}
    }
  return TRUE;
}

/* Allocate all the PLTOFF entries requested by relocations and
   plt entries.  We can't share space with allocated FPTR entries,
   because the latter are not necessarily addressable by the GP.
   ??? Relaxation might be able to determine that they are.
   PIC ABI: Allocate local function descriptors in .pltoff.
   Embedded ABI: Allocate function pointers in .got.plt. */

static bfd_boolean
allocate_pltoff_entries (dyn_i, data)
     struct elf_lx_dyn_sym_info *dyn_i;
     PTR data;
{
  struct elf_lx_allocate_data *x = (struct elf_lx_allocate_data *)data;

  if (dyn_i->want_pltoff)
    {
      dyn_i->pltoff_offset = x->ofs;
      if (pic_abi_p (x->output_bfd))
	/* pltoff contains function descriptors */
	x->ofs += 8;
      else
	/* pltoff (.got.plt) contains function pointers */
	x->ofs += 4;
    }
  return TRUE;
}

/* Allocate dynamic relocations for those symbols that turned out
   to be dynamic.  */

static bfd_boolean
allocate_dynrel_entries (dyn_i, data)
     struct elf_lx_dyn_sym_info *dyn_i;
     PTR data;
{
  struct elf_lx_allocate_data *x = (struct elf_lx_allocate_data *)data;
  struct elf_lx_link_hash_table *lx_info;
  struct elf_lx_dyn_reloc_entry *rent;
  bfd_boolean dynamic_symbol, shared;

  lx_info = elf_lx_hash_table (x->info);
  dynamic_symbol = elf_lx_dynamic_symbol_p (dyn_i->h, x->info);
  shared = x->info->shared;

 /* Take care of the normal data relocations.  */

  for (rent = dyn_i->reloc_entries; rent; rent = rent->next)
    {
      int count = rent->count;

      switch (rent->type)
	{
	case R_LX_FPTR32:
	  /* Allocate one iff !want_fptr, which by this point will
	     be true only if we're actually allocating one statically
	     in the main executable.  */
	  if (dyn_i->want_fptr)
	    continue;
	  break;
	case R_LX_IPLT:
	case R_LX_JMP_SLOT:
	  if (!dynamic_symbol && !shared)
	    continue;
	  /* Use two REL relocations for IPLT relocations
	     against local symbols, one REL relocation for
	     JMP_SLOT relocations against local symbols.  */
	  if (!dynamic_symbol && rent->type == R_LX_IPLT)
	    count *= 2;
	  break;
	case R_LX_32_PCREL:
	  if (!dynamic_symbol)
	    continue;
	  break;
	case R_LX_32:
	  /* Dynamic symbols need a relocation.
	     All symbols in shared/L_relocatable links need a relocation,
	     unless they are absolute.
	  */
	  if (! dynamic_symbol && ! (shared || L_RELOCATABLE))
	    continue;
	  break;
	case R_LX_LO9:
	case R_LX_HI23:
	  continue;
	case R_LX_TPREL32:
	case R_LX_DTPREL32:
	case R_LX_DTPMOD32:
	  break;
	default:
	  abort ();
	}
      rent->srel->size += sizeof (Elf32_External_Rela) * count;
      if (count && (rent->sec->flags & SEC_READONLY))
	lx_info->reltext = 1;
    }

  /* Take care of the GOT and PLT relocations.  */

  if (((dynamic_symbol || shared) && dyn_i->want_got)
      || (dyn_i->want_ltoff_fptr && dyn_i->h && dyn_i->h->dynindx != -1))
    lx_info->rel_got_sec->size += sizeof (Elf32_External_Rela);
  if ((dynamic_symbol || shared) && dyn_i->want_tprel)
    lx_info->rel_got_sec->size += sizeof (Elf32_External_Rela);
  if (dynamic_symbol && dyn_i->want_dtpldm)
    lx_info->rel_got_sec->size += sizeof (Elf32_External_Rela);
  if ((dynamic_symbol || shared) && dyn_i->want_dtpndx)
    {
      lx_info->rel_got_sec->size += sizeof (Elf32_External_Rela);
      if (dynamic_symbol)
	lx_info->rel_got_sec->size += sizeof (Elf32_External_Rela);
    }

  if (dyn_i->want_pltoff)
    {
      bfd_size_type t = 0;

      /* Dynamic symbols get one IPLT relocation.
         Local symbols in shared libraries get two REL
         relocations (in the PIC ABI which uses fn descriptors)
	 or one REL relocation (in the embedded ABI).
	 Local symbols in main applications get nothing.  */
      if (dynamic_symbol)
	t = sizeof (Elf32_External_Rela);
      else if (shared)
	{
	  if (pic_abi_p (x->output_bfd))
	    t = 2 * sizeof (Elf32_External_Rela);
	  else
	    t = sizeof (Elf32_External_Rela);
	}
      lx_info->rel_pltoff_sec->size += t;
    }

  /* Take care of the COPY relocation. */
  {
    struct elf_lx_link_hash_entry *lx_h = (struct elf_lx_link_hash_entry *)dyn_i->h;

    if (lx_h && lx_h->dynbss_size && ! lx_h->rel_dynbss_allocated)
      {
	lx_info->rel_dynbss_sec->size += sizeof (Elf32_External_Rela);
	lx_h->rel_dynbss_allocated = TRUE;
      }
  }

  return TRUE;
}

/* Adjust a symbol defined by a dynamic object and referenced by a
   regular object.  The current definition is in some section of the
   dynamic object, but we're not including those sections.  We have to
   change the definition to something the rest of the link can
   understand.  */

static bfd_boolean
elf_lx_adjust_dynamic_symbol (info, h)
     struct bfd_link_info *info;
     struct elf_link_hash_entry *h;
{
  bfd *dynobj;
  struct elf_lx_link_hash_table *lx_info;
  struct elf_lx_dyn_sym_info *dyn_i;
  struct elf_lx_dyn_sym_info *d;
  struct elf_lx_dyn_reloc_entry *rent;

#if 0
  printf ("elf_lx_adjust_dynamic_symbol: %s (%p)\n", h->root.root.string, h);
#endif

  lx_info = elf_lx_hash_table (info);

  dynobj = elf_hash_table (info)->dynobj;

  /* Make sure we know what is going on here.  */
  BFD_ASSERT (dynobj != NULL
	      && (h->needs_plt
		  || h->u.weakdef != NULL
		  || (h->def_dynamic
		      && h->ref_regular
		      && ! h->def_regular)));

  dyn_i = get_dyn_sym_info (lx_info, h, NULL, NULL, FALSE);

  /* Undefined symbols with PLT entries should be re-defined
     to be the PLT entry.  */
  if (h->type == STT_FUNC
      || h->needs_plt)
    {
      if (! dyn_i
	  || (! info->shared
	      && ! h->def_dynamic
	      && ! h->ref_dynamic
	      && h->root.type != bfd_link_hash_undefined))
	{
	  h->needs_plt = 0;
	  return TRUE;
	}

      dyn_i->want_plt = 1;
      dyn_i->want_plt2 = 1;
      /* We want to set the symbol to the location in the .plt
	 section, but cannot do that until it is allocated in
	 size_dynamic_sections. */
      return TRUE;
    }

  /* If this is a weak symbol, and there is a real definition, the
     processor independent code will have arranged for us to see the
     real definition first, and we can just use the same value.  */
  if (h->u.weakdef != NULL)
    {
      BFD_ASSERT (h->u.weakdef->root.type == bfd_link_hash_defined
                  || h->u.weakdef->root.type == bfd_link_hash_defweak);
      h->root.u.def.section = h->u.weakdef->root.u.def.section;
      h->root.u.def.value = h->u.weakdef->root.u.def.value;
      return TRUE;
    }

  /* If this is a reference to a symbol defined by a dynamic object which
     is not a function, we might allocate the symbol in our .dynbss section
     and allocate a COPY dynamic relocation.
  */

  /* If we are creating a shared library, we must presume that the
     only references to the symbol are via the global offset table.
     For such cases we need not do anything here; the relocations will
     be handled correctly by relocate_section.  */
  if (info->shared)
    return TRUE;

  /* If there are no references to this symbol that do not use the
     GOT, we don't need to generate a copy reloc.  */
  if (! h->non_got_ref)
    return TRUE;

  /* If -z nocopyreloc was given, we won't generate them either.  */
  if (info->nocopyreloc)
    {
      h->non_got_ref = 0;
      return TRUE;
    }

  for (d = ((struct elf_lx_link_hash_entry *)h)->info; d; d = d->next)
    {
      for (rent = d->reloc_entries; rent; rent = rent->next)
	{
	  asection *s = rent->sec->output_section;
	  if (s != NULL && (s->flags & SEC_READONLY) != 0)
	    break;
	}
      if (rent)
	break;
    }

  /* If we didn't find any dynamic relocs in read-only sections, then
     we'll be keeping the dynamic relocs and avoiding the copy reloc.  */
  if (! d)
    {
      h->non_got_ref = 0;
      return TRUE;
    }

  /* We must allocate the symbol in our .dynbss section, which will
     become part of the .bss section of the executable.  There will be
     an entry for this symbol in the .dynsym section.  The dynamic
     object will contain position independent code, so all references
     from the dynamic object to this symbol will go through the global
     offset table.  The dynamic linker will use the .dynsym entry to
     determine the address it must put in the global offset table, so
     both the dynamic object and the regular object will refer to the
     same memory location for the variable.  */

#if 0
  printf ("Allocating %s (%p) in bss\n", h->root.root.string, h);
#endif

  if ((h->root.u.def.section->flags & SEC_ALLOC) != 0)
    {
      h->needs_copy = 1;
    }

  {
    struct elf_lx_link_hash_entry *lx_h = (struct elf_lx_link_hash_entry *)h;
    asection *s = lx_info->dynbss_sec;
    unsigned int power_of_two = bfd_log2 (h->size);

    if (power_of_two > 3)
      power_of_two = 3;

    s->size = BFD_ALIGN (s->size, (bfd_size_type) (1 << power_of_two));
    if (power_of_two > bfd_get_section_alignment (lx_info->root.dynobj, s))
      {
	if (! bfd_set_section_alignment (lx_info->root.dynobj, s, power_of_two))
	  return FALSE;
      }

    /* Define the symbol as being at this point in the section.  */
    h->root.u.def.section = s;
    h->root.u.def.value = s->size;

    /* Increment the section size to make room for the symbol.  */
    s->size += h->size;

    lx_h->dynbss_size = h->size;
  }

  return TRUE;
}

static bfd_boolean
elf_lx_size_dynamic_sections (output_bfd, info)
     bfd *output_bfd;
     struct bfd_link_info *info;
{
  struct elf_lx_allocate_data data;
  struct elf_lx_link_hash_table *lx_info;
  asection *sec;
  bfd *dynobj;
  bfd_boolean relplt = FALSE;

  dynobj = elf_hash_table(info)->dynobj;
  lx_info = elf_lx_hash_table (info);
  lx_info->self_dtpldm_offset = (bfd_vma) -1;
  BFD_ASSERT(dynobj != NULL);
  data.info = info;
  data.output_bfd = output_bfd;

  /* Set the contents of the .interp section to the interpreter.  */
  if (lx_info->root.dynamic_sections_created
      && !info->shared)
    {
      sec = bfd_get_section_by_name (dynobj, ".interp");
      BFD_ASSERT (sec != NULL);
      sec->contents = (bfd_byte *) ELF_DYNAMIC_INTERPRETER;
      sec->size = strlen (ELF_DYNAMIC_INTERPRETER) + 1;
    }

  /* Allocate the GOT entries.  */

  if (lx_info->got_sec)
    {
      asection *got_sec = lx_info->got_sec;

      data.ofs = 0;
      elf_lx_dyn_sym_traverse (lx_info, allocate_global_data_got, &data);
      elf_lx_dyn_sym_traverse (lx_info, allocate_global_fptr_got, &data);
      elf_lx_dyn_sym_traverse (lx_info, allocate_local_got, &data);
      got_sec->size = data.ofs;
      if (got_sec->reloc_count > 0)
	{
	  /* Allocate relocs now, but do not fill it until we enter
	     elf_lx_relocate_section for the got section. */
	  struct bfd_elf_section_data *elfsec_data = elf_section_data (got_sec);
	  bfd_size_type relsize = got_sec->reloc_count * sizeof (Elf_Internal_Rela);
	  Elf_Internal_Rela *relocs = bfd_zalloc(lx_info->got_bfd, relsize);
	  bfd_size_type relhash_size = (got_sec->reloc_count
					* sizeof (struct elf_link_hash_entry *));
	  BFD_ASSERT (elfsec_data->relocs == 0);
	  elfsec_data->relocs = relocs;
	  _bfd_elf_single_rel_hdr (got_sec)->sh_size = relsize;
	  _bfd_elf_single_rel_hdr (got_sec)->sh_entsize = sizeof (Elf32_External_Rela);
	  lx_info->got_rel_hash = bfd_zmalloc (relhash_size);
	}
    }

  /* Allocate the FPTR entries.  */

  if (lx_info->fptr_sec)
    {
      data.ofs = 0;
      elf_lx_dyn_sym_traverse (lx_info, allocate_fptr, &data);
      lx_info->fptr_sec->size = data.ofs;
    }

  /* Now that we've seen all of the input files, we can decide which
     symbols need plt entries.  Allocate the minimal PLT entries first.
     We do this even though dynamic_sections_created may be FALSE, because
     this has the side-effect of clearing want_plt and want_plt2.  */

  data.ofs = 0;
  elf_lx_dyn_sym_traverse (lx_info, allocate_plt_entries, &data);

  lx_info->minplt_entries = 0;
  if (data.ofs)
    {
      lx_info->minplt_entries
	= (data.ofs - PLT_HEADER_SIZE) / PLT_MIN_ENTRY_SIZE;
    }

  /* Align the pointer for the plt2 entries.  */
  data.ofs = (data.ofs + 31) & (bfd_vma) -32;

  elf_lx_dyn_sym_traverse (lx_info, allocate_plt2_entries, &data);
  if (data.ofs != 0)
    {
      BFD_ASSERT (lx_info->root.dynamic_sections_created);

      lx_info->plt_sec->size = data.ofs;

      /* If we've got a .plt, we need some extra memory for the dynamic
	 linker.  We stuff these in .got.plt.  */
      sec = bfd_get_section_by_name (dynobj, ".got.plt");
      sec->size = 4 * PLT_RESERVED_WORDS;
    }

  /* Allocate the PLTOFF entries.  */

  if (lx_info->pltoff_sec)
    {
      if (pic_abi_p (output_bfd))
	data.ofs = 0;
      else
	/* In the embedded ABI, the PLTOFF entries go in .got.plt.
           Remember the start of .got.plt is reserved.
	*/
	data.ofs = lx_info->pltoff_sec->size;
      elf_lx_dyn_sym_traverse (lx_info, allocate_pltoff_entries, &data);
      lx_info->pltoff_sec->size = data.ofs;
    }

  if (lx_info->root.dynamic_sections_created)
    {
      /* Allocate space for the dynamic relocations that turned out to be
	 required.  */

      if (info->shared && lx_info->self_dtpldm_offset != (bfd_vma) -1)
	lx_info->rel_got_sec->size += 2 * sizeof (Elf32_External_Rela);
      elf_lx_dyn_sym_traverse (lx_info, allocate_dynrel_entries, &data);
    }

  /* We have now determined the sizes of the various dynamic sections.
     Allocate memory for them.  */
  for (sec = dynobj->sections; sec != NULL; sec = sec->next)
    {
      bfd_boolean strip;

      if (!(sec->flags & SEC_LINKER_CREATED))
	continue;

      /* If we don't need this section, strip it from the output file.
	 There were several sections primarily related to dynamic
	 linking that must be create before the linker maps input
	 sections to output sections.  The linker does that before
	 bfd_elf_size_dynamic_sections is called, and it is that
	 function which decides whether anything needs to go into
	 these sections.  */

      strip = (sec->size == 0);

      if (sec == lx_info->got_sec)
	strip = FALSE;
      else if (sec == lx_info->rel_got_sec)
	{
	  if (strip)
	    lx_info->rel_got_sec = NULL;
	  else
	    /* We use the reloc_count field as a counter if we need to
	       copy relocs into the output file.  */
	    sec->reloc_count = 0;
	}
      else if (sec == lx_info->fptr_sec)
	{
	  if (strip)
	    lx_info->fptr_sec = NULL;
	}
      else if (sec == lx_info->plt_sec)
	{
	  if (strip)
	    lx_info->plt_sec = NULL;
	}
      else if (sec == lx_info->pltoff_sec)
	{
	  if (strip)
	    lx_info->pltoff_sec = NULL;
	}
      else if (sec == lx_info->rel_pltoff_sec)
	{
	  if (strip)
	    lx_info->rel_pltoff_sec = NULL;
	  else
	    {
	      relplt = TRUE;
	      /* We use the reloc_count field as a counter if we need to
		 copy relocs into the output file.  */
	      sec->reloc_count = 0;
	    }
	}
      else
	{
	  const char *name;

	  /* It's OK to base decisions on the section name, because none
	     of the dynobj section names depend upon the input files.  */
	  name = bfd_get_section_name (dynobj, sec);

	  if (strcmp (name, ".got.plt") == 0)
	    strip = FALSE;
	  else if (strncmp (name, ".rel", 4) == 0)
	    {
	      if (!strip)
		{
		  /* We use the reloc_count field as a counter if we need to
		     copy relocs into the output file.  */
		  sec->reloc_count = 0;
		}
	    }
	  else
	    continue;
	}

      if (strip)
	sec->flags |= SEC_EXCLUDE;
      else
	{
	  /* Allocate memory for the section contents.  */
	  sec->contents = (bfd_byte *) bfd_zalloc (dynobj, sec->size);
	  if (sec->contents == NULL && sec->size != 0)
	    return FALSE;
	}
    }

  if (elf_hash_table (info)->dynamic_sections_created)
    {
      /* Add some entries to the .dynamic section.  We fill in the values
	 later (in finish_dynamic_sections) but we must add the entries now
	 so that we get the correct size for the .dynamic section.  */

      if (!info->shared)
	{
	  /* The DT_DEBUG entry is filled in by the dynamic linker and used
	     by the debugger.  */
#define add_dynamic_entry(TAG, VAL) \
  _bfd_elf_add_dynamic_entry (info, (bfd_vma) (TAG), (bfd_vma) (VAL))

	  if (!add_dynamic_entry (DT_DEBUG, 0))
	    return FALSE;
	}

      if (!add_dynamic_entry (DT_PLTGOT, 0))
	return FALSE;

      if (relplt)
	{
	  if (!add_dynamic_entry (DT_PLTRELSZ, 0)
	      || !add_dynamic_entry (DT_PLTREL, DT_RELA)
	      || !add_dynamic_entry (DT_JMPREL, 0))
	    return FALSE;
	}

      if (!add_dynamic_entry (DT_RELA, 0)
	  || !add_dynamic_entry (DT_RELASZ, 0)
	  || !add_dynamic_entry (DT_RELAENT, sizeof (Elf32_External_Rela)))
	return FALSE;

      if (lx_info->reltext)
	{
	  if (!add_dynamic_entry (DT_TEXTREL, 0))
	    return FALSE;
	  info->flags |= DF_TEXTREL;
	}
    }

  return TRUE;
}

static void
elf_lx_install_dyn_reloc (abfd, info, sec, srel, offset, type,
			  dynindx, addend, relocate)
     bfd *abfd;
     struct bfd_link_info *info;
     asection *sec;
     asection *srel;
     bfd_vma offset;
     unsigned int type;
     long dynindx;
     bfd_vma addend;
     bfd_boolean *relocate;
{
  Elf_Internal_Rela outrel;

  BFD_ASSERT (dynindx != -1);
  outrel.r_info = ELF32_R_INFO (dynindx, type);
  outrel.r_addend = addend;
  outrel.r_offset = _bfd_elf_section_offset (abfd, info, sec, offset);
  if (outrel.r_offset >= (bfd_vma) -2)
    {
      /* When we have a dynamic relocation against .eh_frame, but we are
	 going to make addresses in the .eh_frame to be pc-relative, then
	 bfd_elf_section_offset returns -2.  In this case,
	 bfd_elf_write_section_eh_frame requires us to put the absolute
	 address in the output section, and nullify the dynamic relocation.
	 It will then rewrite this absolute address to be pc-relative.
      */
      *relocate = (outrel.r_offset == (bfd_vma) -2);
      outrel.r_info = ELF32_R_INFO (0, R_LX_NONE);
      outrel.r_addend = 0;
      outrel.r_offset = 0;
    }
  else    
    outrel.r_offset += sec->output_section->vma + sec->output_offset;

  bfd_elf32_swap_reloca_out (abfd, &outrel,
			     (bfd_byte *) (((Elf32_External_Rela *) 
					    srel->contents)
					   + srel->reloc_count++));
  BFD_ASSERT ((srel->size > 0 
	       && (sizeof (Elf32_External_Rela) * srel->reloc_count
		   <= srel->size)));
}

static void
elf_lx_install_got_static_reloc (struct bfd_link_info *info,
				 bfd_vma got_offset, struct elf_link_hash_entry *h,
				 unsigned int r_type,
				 bfd_vma addend, bfd_vma value)
{
  struct elf_lx_link_hash_table *lx_info = elf_lx_hash_table (info);
  asection *got_sec = lx_info->got_sec;
  struct bfd_elf_section_data *elfsec_data = elf_section_data (got_sec);
  unsigned int reloc_number = elfsec_data->rel.count++;
  unsigned long symndx = STN_UNDEF;

  Elf_Internal_Rela *rel = elfsec_data->relocs + reloc_number;
  bfd_vma offset = got_offset + got_sec->output_section->vma + got_sec->output_offset;

  if (h)
    lx_info->got_rel_hash[reloc_number] = h;
  else
    {
      /* For a local symbol, we do not know the symbol table index (it may not even
	 have one), so we put the full value in the addend, and set the
	 symbol index to undef.
	 We could probably do better, by setting it relative to the section symbol of
	 the output section that contains it. */
      addend += value;
    }
  rel->r_info = ELF32_R_INFO (symndx, r_type);
  rel->r_addend = addend;
  rel->r_offset = offset;
}

/* Store an entry for target address DYN_I + ADDEND in the global offset
   table and return the address of the linkage table entry.  */

static bfd_vma
set_got_entry (abfd, info, dyn_i, dynindx, addend, value, dyn_r_type)
     bfd *abfd;
     struct bfd_link_info *info;
     struct elf_lx_dyn_sym_info *dyn_i;
     long dynindx;
     bfd_vma addend;
     bfd_vma value;
     unsigned int dyn_r_type;
{
  struct elf_lx_link_hash_table *lx_info;
  asection *got_sec;
  bfd_boolean done;
  bfd_vma got_offset;
  bfd_vma value2 = 0;
  bfd_boolean put_value2 = FALSE;

  lx_info = elf_lx_hash_table (info);
  got_sec = lx_info->got_sec;

  switch (dyn_r_type)
    {
    case R_LX_TPREL32:
      done = dyn_i->tprel_done;
      dyn_i->tprel_done = TRUE;
      got_offset = dyn_i->tprel_offset;
      break;
    case R_LX_GOTOFF_DTPLDM_LO9:
      /* We want a ldm ti_index structure in the GOT */
      if (dyn_i->dtpldm_offset != lx_info->self_dtpldm_offset)
	{
	  done = dyn_i->dtpldm_done;
	  dyn_i->dtpldm_done = TRUE;
	}
      else
	{
	  done = lx_info->self_dtpldm_done;
	  lx_info->self_dtpldm_done = TRUE;
	  dynindx = 0;
	}
      got_offset = dyn_i->dtpldm_offset;
      dyn_r_type = R_LX_DTPMOD32;
      break;
    case R_LX_GOTOFF_DTPNDX_LO9:
      /* We want an ndx ti_index structure in the GOT */
      done = dyn_i->dtpndx_done;
      dyn_i->dtpndx_done = TRUE;
      got_offset = dyn_i->dtpndx_offset;
      value2 = value;
      if (! info->shared && ! elf_lx_dynamic_symbol_p (dyn_i->h, info))
	value = 1;
      else
	value = 0;
      put_value2 = TRUE;
      break;
    default:
      done = dyn_i->got_done;
      dyn_i->got_done = TRUE;
      got_offset = dyn_i->got_offset;
      break;
    }

  BFD_ASSERT ((got_offset & 3) == 0);

  if (! done)
    {
      unsigned int static_r_type = dyn_r_type;
      bfd_vma static_addend = addend;
      bfd_boolean dyn_needed;
      bfd_boolean dummy;

      /* Store the target address in the global offset table entry.  */
      bfd_put_32 (abfd, value, got_sec->contents + got_offset);
      if (put_value2)
	bfd_put_32 (abfd, value2, got_sec->contents + got_offset + 4);

      dyn_needed = (info->shared
		    || elf_lx_dynamic_symbol_p (dyn_i->h, info)
		    || (dynindx != -1 && dyn_r_type == R_LX_FPTR32));

      /* Install a dynamic relocation if needed.  */
      if (dynindx == -1
	  && dyn_r_type != R_LX_TPREL32
	  && dyn_r_type != R_LX_DTPMOD32
	  && dyn_r_type != R_LX_GOTOFF_DTPNDX_LO9)
	{
	  dyn_r_type = R_LX_REL32;
	  dynindx = 0;
	  addend = value;
	}

      if (dyn_r_type == R_LX_GOTOFF_DTPNDX_LO9)
	{
	  /* Create an ndx ti_index structure in the GOT */
	  if (dyn_needed)
	    elf_lx_install_dyn_reloc (abfd, NULL, got_sec,
				      lx_info->rel_got_sec,
				      got_offset, R_LX_DTPMOD32,
				      dynindx, addend, &dummy);
	  if (info->emitrelocations)
	    elf_lx_install_got_static_reloc (info, got_offset,
					     dyn_i->h, R_LX_DTPMOD32,
					     static_addend, value);
	  
	  if (dynindx != 0)
	    {
	      if (dyn_needed)
		elf_lx_install_dyn_reloc (abfd, NULL, got_sec,
					  lx_info->rel_got_sec,
					  got_offset + 4, R_LX_DTPREL32,
					  dynindx, addend, &dummy);
	      if (info->emitrelocations)
		elf_lx_install_got_static_reloc (info, got_offset + 4,
						 dyn_i->h, R_LX_DTPREL32,
						 static_addend, value2);
	    }
	}
      else
	{
	  if (dyn_needed)
	    elf_lx_install_dyn_reloc (abfd, NULL, got_sec,
				      lx_info->rel_got_sec,
				      got_offset, dyn_r_type,
				      dynindx, addend, &dummy);
	  if (info->emitrelocations)
	    elf_lx_install_got_static_reloc (info, got_offset, dyn_i->h,
					     static_r_type, static_addend,
					     value);
	}
    }
  
  /* Return the address of the global offset table entry.  */
  value = (got_sec->output_section->vma
	   + got_sec->output_offset
	   + got_offset);

  return value;
}

/* Fill in a function descriptor consisting of the function's code
   address and its global pointer.  Return the descriptor's address.  */

static bfd_vma
set_fptr_entry (abfd, info, dyn_i, value)
     bfd *abfd;
     struct bfd_link_info *info;
     struct elf_lx_dyn_sym_info *dyn_i;
     bfd_vma value;
{
  struct elf_lx_link_hash_table *lx_info;
  asection *fptr_sec;

  lx_info = elf_lx_hash_table (info);
  fptr_sec = lx_info->fptr_sec;

  if (!dyn_i->fptr_done)
    {
      dyn_i->fptr_done = 1;

      /* Fill in the function descriptor.  */
      bfd_put_32 (abfd, value, fptr_sec->contents + dyn_i->fptr_offset);
      bfd_put_32 (abfd, _bfd_get_gp_value (abfd),
		  fptr_sec->contents + dyn_i->fptr_offset + 4);
    }

  /* Return the descriptor's address.  */
  value = (fptr_sec->output_section->vma
	   + fptr_sec->output_offset
	   + dyn_i->fptr_offset);

  return value;
}

/* Fill in a PLTOFF entry consisting of the function's code address
   and its global pointer.  Return the descriptor's address.  */

static bfd_vma
set_pltoff_entry (abfd, info, dyn_i, value, is_plt)
     bfd *abfd;
     struct bfd_link_info *info;
     struct elf_lx_dyn_sym_info *dyn_i;
     bfd_vma value;
     bfd_boolean is_plt;
{
  struct elf_lx_link_hash_table *lx_info;
  asection *pltoff_sec;

  lx_info = elf_lx_hash_table (info);
  pltoff_sec = lx_info->pltoff_sec;

  /* Don't do anything if this symbol uses a real PLT entry.  In
     that case, we'll fill this in during finish_dynamic_symbol.  */
  if ((! dyn_i->want_plt || is_plt)
      && !dyn_i->pltoff_done)
    {
      bfd_vma gp = _bfd_get_gp_value (abfd);

      /* Fill in the function descriptor.  */
      bfd_put_32 (abfd, value, pltoff_sec->contents + dyn_i->pltoff_offset);
      if (pic_abi_p (abfd))
	bfd_put_32 (abfd, gp, pltoff_sec->contents + dyn_i->pltoff_offset + 4);

      /* Install dynamic relocations if needed.  */
      if (!is_plt && info->shared)
	{
	  bfd_boolean dummy;

	  elf_lx_install_dyn_reloc (abfd, NULL, pltoff_sec,
				    lx_info->rel_pltoff_sec,
				    dyn_i->pltoff_offset,
				    R_LX_REL32, 0, value, &dummy);
	  if (pic_abi_p (abfd))
	    elf_lx_install_dyn_reloc (abfd, NULL, pltoff_sec,
				      lx_info->rel_pltoff_sec,
				      dyn_i->pltoff_offset + 4,
				      R_LX_REL32, 0, gp, &dummy);
	}

      dyn_i->pltoff_done = 1;
    }

  /* Return the descriptor's address.  */
  value = (pltoff_sec->output_section->vma
	   + pltoff_sec->output_offset
	   + dyn_i->pltoff_offset);

  return value;
}

/* Return the base VMA address which should be subtracted from real addresses
   when resolving @tprel() relocation.
   Main program TLS (whose template starts at PT_TLS p_vaddr)
   is assigned offset round(16, PT_TLS p_align).  */

static bfd_vma
elf_lx_tprel_base (info)
     struct bfd_link_info *info;
{
  asection *tls_sec = elf_hash_table (info)->tls_sec;

  BFD_ASSERT (tls_sec != NULL);
  return (tls_sec->vma - align_power ((bfd_vma) 16, tls_sec->alignment_power));
}

/* Return the base VMA address which should be subtracted from real addresses
   when resolving @dtprel() relocation.
   This is PT_TLS segment p_vaddr.  */

static bfd_vma
elf_lx_dtprel_base (info)
     struct bfd_link_info *info;
{
  asection *tls_sec = elf_hash_table (info)->tls_sec;
  
  BFD_ASSERT (tls_sec != NULL);
  return tls_sec->vma;
}

/* Called after we have determined section placement.  If sections
   move, we'll be called again.  Provide a value for GP.  */

bfd_boolean
elf_lx_set_gp (output_bfd, info)
     bfd *output_bfd;
     struct bfd_link_info *info;
{
  /* Decide where gp should point.
     We point it at the .got section if that exists,
     otherwise at the lowest addressed data section. */
  bfd_vma gp_val;
  struct elf_lx_link_hash_table *lx_info = elf_lx_hash_table (info);

  asection *got_sec = lx_info->got_sec;
  if (got_sec)
    gp_val = got_sec->output_section->vma;
  else
    {
      bfd_vma min_vma = (bfd_vma) -1;
      asection *os;
      
      for (os = output_bfd->sections; os ; os = os->next)
	{
	  if ((os->flags & (SEC_ALLOC | SEC_DATA))
	      == (SEC_ALLOC | SEC_DATA)
	      && os->size > 0)
	    {
	      if (os->vma < min_vma)
		min_vma = os->vma;
	    }
	}
      gp_val = min_vma;
    }
  _bfd_set_gp_value (output_bfd, gp_val);
  return TRUE;
}

static void
transform_tls_relocs (bfd *abfd,
		      struct bfd_link_info *info, 
		      Elf_Internal_Rela *relstart,
		      Elf_Internal_Rela *relend,
		      bfd_byte *contents)
{
  Elf_Internal_Rela *rel;
  struct elf_lx_link_hash_table *lx_info;
  struct elf_link_hash_entry *tls_get_addr;

  if (info->shared
      || info->relocatable)
    /* No transformations are performed for shared libs or relocatable links */
    return;

  lx_info = elf_lx_hash_table (info);
  tls_get_addr = elf_link_hash_lookup (&lx_info->root, "__tls_get_addr",
				       FALSE, FALSE, TRUE);
  for (rel = relstart; rel < relend; rel++)
    {
      unsigned int r_type;
      unsigned long r_symndx;

      r_type = ELF32_R_TYPE (rel->r_info);
      r_symndx = ELF32_R_SYM (rel->r_info);

      switch (r_type)
	{
	case R_LX_23_PCREL:
	case R_LX_GOTOFF_DTPLDM_HI23:
	case R_LX_GOTOFF_DTPLDM_LO9:
	case R_LX_GOTOFF_DTPNDX_HI23:
	case R_LX_GOTOFF_DTPNDX_LO9:
	  {
	    /* Any one of these could begin a TLS GD/LD sequence. */

	    Elf_Internal_Rela *low_rel = NULL;
	    Elf_Internal_Rela *high_rel = NULL;
	    Elf_Internal_Rela *call_rel = NULL;
	    bfd_vma low_offset = rel->r_offset;
	    bfd_vma high_offset = rel->r_offset;
	    bfd_vma offset;
	    bfd_vma insn;
	    Elf_Internal_Rela *rel2;
	    struct elf_link_hash_entry *h = NULL;
	    struct elf_link_hash_entry *call_h = NULL;

	    for (rel2 = rel; rel2 < (rel + 3) && rel2 < relend; rel2++)
	      {
		unsigned int rel2_type = ELF32_R_TYPE (rel2->r_info);
		if (rel2_type == R_LX_23_PCREL)
		  {
		    call_rel = rel2;
		    call_h = symbol_for_relax (abfd, ELF32_R_SYM (rel2->r_info));
		  }
		else if (rel2_type == R_LX_GOTOFF_DTPLDM_LO9
			 || rel2_type == R_LX_GOTOFF_DTPNDX_LO9)
		  {
		    low_rel = rel2;
		    r_symndx = ELF32_R_SYM (rel2->r_info);
		    h = symbol_for_relax (abfd, r_symndx);
		  }
		else if (rel2_type == R_LX_GOTOFF_DTPLDM_HI23
			 || rel2_type == R_LX_GOTOFF_DTPNDX_HI23)
		  high_rel = rel2;
		if (rel2->r_offset < low_offset)
		  low_offset = rel2->r_offset;
		else if (rel2->r_offset > high_offset)
		  high_offset = rel2->r_offset;
	      }
	    if (! (low_rel && high_rel && call_rel)
		|| ! non_preemptible_def_p (h, info)
		|| ! reloc_matching_pair (abfd, high_rel, low_rel, contents)
		|| ! INSN_IS_ADDI_R16_P(bfd_get_32 (abfd, contents + low_rel->r_offset))
		|| ! INSN_IS_CALL_P (bfd_get_32 (abfd, contents + call_rel->r_offset))
		|| call_h != tls_get_addr)
	      continue;

	    /* As a final check, all these operations must be in the same
	       bundle. */
	    for (offset = low_offset; offset < high_offset; offset += 4)
	      {
		insn = bfd_get_32 (abfd, contents + offset);
		if (INSN_IS_BUNDLE_END_P (insn))
		  break;
	      }
	    if (offset < high_offset)
	      /* There is a bundle terminator separating the relocations */
	      continue;

	    /* Transformation GD/LD => LE */
	    if (ELF32_R_TYPE (low_rel->r_info) == R_LX_GOTOFF_DTPLDM_LO9)
	      {
		/* Transforming LD to LE, so we want TPREL of module
		   containing sym (i.e. main program), not TPREL of sym. */
		r_symndx = 0;
		low_rel->r_addend += elf_lx_dtprel_base (info);
		high_rel->r_addend += elf_lx_dtprel_base (info);
	      }

	    low_rel->r_info = ELF32_R_INFO (r_symndx, R_LX_TPREL_LO9);
	    insn = bfd_get_32 (abfd, contents + low_rel->r_offset);
	    insn = (insn & ~0x3f) | 0x0d;
	    bfd_put_32 (abfd, insn, contents + low_rel->r_offset);

	    high_rel->r_info = ELF32_R_INFO (r_symndx, R_LX_TPREL_HI23);

	    call_rel->r_info = ELF32_R_INFO (0, R_LX_NONE);
	    insn = bfd_get_32 (abfd, contents + call_rel->r_offset);
	    insn = (insn & 0x80000000) | 0;
	    bfd_put_32 (abfd, insn, contents + call_rel->r_offset);
	  }
	  break;
	case R_LX_GOTOFF_TPREL_LO9:
	case R_LX_GOTOFF_TPREL_HI23:
	  {
	    /* These could begin a TLS IE sequence. */

	    Elf_Internal_Rela *low_rel;
	    Elf_Internal_Rela *high_rel;
	    struct elf_link_hash_entry *h;
	    bfd_vma insn;

	    h = symbol_for_relax (abfd, r_symndx);

	    if (rel == relend - 1
		|| ! reloc_matching_pair (abfd, rel, rel + 1, contents)
		|| ! non_preemptible_def_p (h, info))
	      continue;

	    if (r_type == R_LX_GOTOFF_TPREL_LO9)
	      {
		low_rel = rel;
		high_rel = rel + 1;
	      }
	    else
	      {
		high_rel = rel;
		low_rel = rel + 1;
	      }
	    if (! INSN_IS_LDW_OR_LDWD_P (abfd,
					 bfd_get_32 (abfd, contents + low_rel->r_offset)))
	      continue;

	    /* Transformation IE => LE */
	    low_rel->r_info = ELF32_R_INFO (r_symndx, R_LX_TPREL_LO9);
	    insn = bfd_get_32 (abfd, contents + low_rel->r_offset);
	    /* Rewrite ldw[.d] to be add */
	    insn = LDW_TO_ADD (insn);
	    /* Rewrite base register as r0 */
	    insn &= 0xffffffc0;

	    bfd_put_32 (abfd, insn, contents + low_rel->r_offset);

	    high_rel->r_info = ELF32_R_INFO (r_symndx, R_LX_TPREL_HI23);
	  }
	  break;
	}
    }
}

static bfd_boolean 
elf_lx_relocate_section (
                 bfd *output_bfd, struct bfd_link_info *info, bfd *input_bfd, 
                 asection *input_section, bfd_byte *contents, 
                 Elf_Internal_Rela *relocs, Elf_Internal_Sym *local_syms, 
                 asection **local_sections)
{
  struct elf_lx_link_hash_table *lx_info;
  Elf_Internal_Shdr *symtab_hdr;
  /*   struct elf_link_hash_entry **sym_hashes; ??? worry about this */
  Elf_Internal_Rela *rel, *relend;
  asection *srel;
  bfd_boolean ret_val = TRUE; /* for non-fatal errors */
  bfd_vma gp_val;
  struct elf_link_hash_entry **rel_hash;
  asection *output_section;

  /* sym_hashes = elf_sym_hashes (input_bfd); */
  lx_info = elf_lx_hash_table (info);

  /* Don't relocate stub sections.  */
  if (input_section->owner == lx_info->stub_bfd)
    return TRUE;

  symtab_hdr = &elf_tdata (input_bfd)->symtab_hdr;

  gp_val = _bfd_get_gp_value (output_bfd);
  srel = get_reloc_section (input_bfd, lx_info, input_section, FALSE);
  rel = relocs;
  relend = relocs + input_section->reloc_count;

  output_section = input_section->output_section;
  rel_hash = (elf_section_data (output_section)->rel.hashes
	      + elf_section_data (output_section)->rel.count);

  transform_tls_relocs (input_bfd, info, rel, relend, contents);

  for (; rel < relend; rel++)
    {
      unsigned int r_type;
      reloc_howto_type *howto;
      unsigned long r_symndx;
      Elf_Internal_Sym *sym;
      struct elf_link_hash_entry *h;
      struct elf_lx_dyn_sym_info *dyn_i;
      bfd_vma value;
      bfd_reloc_status_type r;
      asection *sym_sec;
      bfd_boolean dynamic_symbol_p;
      bfd_boolean undef_weak_ref;

      r_type = ELF32_R_TYPE (rel->r_info);
      if (r_type >= R_LX_max)
	{
	  (*_bfd_error_handler)
	    (_("%B: unknown relocation type %d"),
	     input_bfd, (int)r_type);
	  bfd_set_error (bfd_error_bad_value);
	  ret_val = FALSE;
	  continue;
	}
      howto = elf32_lx_howto_table + r_type;
      r_symndx = ELF32_R_SYM (rel->r_info);

      /* A little checking, to catch relocation types
	 that are not appropriate for the selected ABI. */
      if (!abi_uses_reloc_p (pic_abi_p (output_bfd), r_type))
	{
	  (*_bfd_error_handler)
	    (_("%B: contains relocation (%d) not appropriate for output ABI"),
	     input_bfd, (int) r_type);
	  ret_val = FALSE;
	  continue;
	}
	     
      /* This is a final link.  */
      h = NULL;
      sym = NULL;
      sym_sec = NULL;
      undef_weak_ref = FALSE;

      if (r_symndx < symtab_hdr->sh_info)
	{
	  /* Reloc against local symbol.  */
	  sym = local_syms + r_symndx;
	  sym_sec = local_sections[r_symndx];
	  value = _bfd_elf_rela_local_sym (output_bfd, sym, &sym_sec, rel);
	  if (! info->relocatable
	      && (sym_sec->flags & SEC_MERGE) != 0
	      && ELF_ST_TYPE (sym->st_info) == STT_SECTION
	      && (sym_sec->sec_info_type == ELF_INFO_TYPE_MERGE))
 	    {
	      struct elf_lx_local_hash_entry *loc_h;
      
	      loc_h = get_local_sym_hash (lx_info, input_bfd, rel, FALSE);
	      if (loc_h && ! loc_h->sec_merge_done)
		{
		  struct elf_lx_dyn_sym_info *dynent;
		  asection *msec;

		  for (dynent = loc_h->info; dynent; dynent = dynent->next)
		    {
		      msec = sym_sec;
		      dynent->addend =
			_bfd_merged_section_offset (output_bfd, &msec,
						    elf_section_data (msec)->sec_info,
						    sym->st_value + dynent->addend);
		      dynent->addend -= sym->st_value;
		      dynent->addend += msec->output_section->vma
					+ msec->output_offset
					- sym_sec->output_section->vma
					- sym_sec->output_offset;
		    }
		  loc_h->sec_merge_done = 1;
		}
	    }
	}
      else
	{
	  bfd_boolean unresolved_reloc;
	  bfd_boolean warned;

	  RELOC_FOR_GLOBAL_SYMBOL (info, input_bfd, input_section, rel,
				   r_symndx, symtab_hdr, elf_sym_hashes (input_bfd),
				   h, sym_sec, value,
				   unresolved_reloc, warned);

	  if (h->root.type == bfd_link_hash_undefweak)
	    {
	      undef_weak_ref = TRUE;
	      /* (pp) modify default in order to avoid linker overflow */
	      if (howto->pc_relative)
		value = (input_section->output_section->vma
			 + input_section->output_offset);
	      
	      else
		value = 0;
	    }
	  else if (warned)
	    continue;
	}
      
      /* For relocs against symbols from removed linkonce sections,
	 or sections discarded by a linker script, we just want the
	 section contents zeroed.  Avoid any special processing.  */
      if (sym_sec != NULL && elf_discarded_section (sym_sec))
	{
            RELOC_AGAINST_DISCARDED_SECTION (info, input_bfd, input_section, rel,
                                             relend, howto, contents);
	    continue;
	}

      /* TB: when building a relocatable with pcrel relocs against local
	 symbol resolved, ignore all reloc except R_LX_23_PCREL against
	 local symbol*/
	if (info->relocatable
	    && (! howto->pc_relative
		|| r_symndx >= symtab_hdr->sh_info))
	continue;

      value += rel->r_addend;
      dynamic_symbol_p = elf_lx_dynamic_symbol_p (h, info);

      switch (r_type)
	{
	case R_LX_NONE:
	  continue;

	case R_LX_HI23:
	case R_LX_LO9:
	case R_LX_16:
	case R_LX_32:
	case R_LX_DTPMOD32:
	  /* These require a dynamic relocation if the symbol is
	     dynamic or we are making a shared library. */
	  /* clarkes: do not emit a dynamic relocation for an absolute
	     symbol that is owned by a shared library or relocatable module - if
	     it is absolute then it should not need relocating.  Note this differs
	     from standard Linux behaviour for a shared library.  */
	  if ((dynamic_symbol_p
	       || ((info->shared || L_RELOCATABLE) && ! bfd_is_abs_section (sym_sec)))
	      && r_symndx != 0
	      && (input_section->flags & SEC_ALLOC) != 0
	      && (h == NULL || ! h->non_got_ref))
	    {
	      /* Dynamic relocation required. */
	      if ((input_section->flags & (SEC_CODE | SEC_READONLY)) != 0)
		{
		  (*_bfd_error_handler)
		    (_("%B: dynamic relocation of non-writeable section required (reference to %s)"),
		     input_bfd, (h != NULL) ? h->root.root.string : "local symbol");
		  ret_val = FALSE;
		  continue;
		}
	      else
		{
		  unsigned int dyn_r_type = r_type;
		  long dynindx = 0;
		  bfd_vma addend = 0;
		  bfd_boolean relocate = FALSE;

		  if (dynamic_symbol_p)
		    {
		      dynindx = h->dynindx;
		      addend = rel->r_addend;
		      value = 0;
		    }
		  else
		    {
		      switch (r_type)
			{
			case R_LX_32:
			  dyn_r_type = R_LX_REL32;
			  break;
			case R_LX_DTPMOD32:
			  value = 0;
			  break;
			default:
			  /* We can't represent this without a dynamic symbol. */
			  (*_bfd_error_handler)
			    (_("%B: linking non-pic code in a shared library"),
			     input_bfd);
			  ret_val = FALSE;
			  continue;
			}
		      addend = value;
		    }

		  elf_lx_install_dyn_reloc (output_bfd, info, input_section,
					    srel, rel->r_offset, dyn_r_type,
					    dynindx, addend, &relocate);
		  if (!relocate)
		    value = 0;
		}
	    }
	  else if (r_type == R_LX_DTPMOD32)
	    value = 1; /* Main program module number */
	  goto default_reloc;

	case R_LX_GPREL_HI23:
	case R_LX_GPREL_LO9:
	case R_LX_NEG_GPREL_HI23:
	case R_LX_NEG_GPREL_LO9:
	  if ((lx_info->transform_to_absolute
               || bfd_is_abs_section (sym_sec))
	      && ! info->shared
	      && (r_type == R_LX_GPREL_HI23
		  || r_type == R_LX_GPREL_LO9))
	    {
	      /* Convert the GP-relative instruction to absolute by
		 rewriting the base register.
	      */
	      if (rel < (relend - 1)
		  && reloc_matching_pair (output_bfd, rel, rel + 1, contents))
		{
		  /* Handle this relocation and the next one together. */

		  Elf_Internal_Rela *low_rel;
		  Elf_Internal_Rela *high_rel;
		  bfd_vma insn;

		  if (r_type == R_LX_GPREL_LO9)
		    {
		      low_rel = rel;
		      high_rel = rel + 1;
		    }
		  else
		    {
		      low_rel = rel + 1;
		      high_rel = rel;
		    }
		  insn = bfd_get_32 (input_bfd, contents + low_rel->r_offset);
		  if ((insn & 0x3f) != 0)
		    { /* Do the transformation. */
		      insn &= 0xffffffc0; /* Rewrite base register to be r0. */
		      bfd_put_32 (input_bfd, insn, contents + low_rel->r_offset);
		      low_rel->r_info = ELF32_R_INFO (r_symndx, R_LX_LO9);
		      high_rel->r_info = ELF32_R_INFO (r_symndx, R_LX_HI23);
#if 0
		      (*_bfd_error_handler)
			(_("%s: information: converting @gprel to absolute for %s"),
			 bfd_get_filename (output_bfd),
			 (h != NULL) ? h->root.root.string : "local symbol");
#endif
		    }
		  else
		    value -= gp_val;
		  r = _bfd_final_link_relocate (elf32_lx_howto_table
						+ ELF32_R_TYPE (low_rel->r_info),
						input_bfd, input_section, contents,
						low_rel->r_offset, value, 0);
		  if (r == bfd_reloc_ok)
		    {
		      r = _bfd_final_link_relocate (elf32_lx_howto_table
						    + ELF32_R_TYPE (high_rel->r_info),
						    input_bfd, input_section, contents,
						    high_rel->r_offset, value, 0);
		    }
		  rel++; /* We have processed the next rel */
		  break;
		}
	    }
	  if (dynamic_symbol_p)
	    {
	      (*_bfd_error_handler)
		(_("%B: @gprel relocation against dynamic symbol %s"),
		 input_bfd, h->root.root.string);
	      ret_val = FALSE;
	      continue;
	    }
	  value -= gp_val;
	  if (r_type == R_LX_NEG_GPREL_HI23 || r_type == R_LX_NEG_GPREL_LO9)
	    value = -value;
	  goto default_reloc;

	case R_LX_GOTOFF_HI23:
	case R_LX_GOTOFF_LO9:
	case R_LX_GOTOFFX_HI23:
	case R_LX_GOTOFFX_LO9:
	  dyn_i = get_dyn_sym_info (lx_info, h, input_bfd, rel, FALSE);
	  value = set_got_entry (input_bfd, info, dyn_i, (h ? h->dynindx : -1),
				 rel->r_addend, value, R_LX_32);
	  if (lx_info->transform_to_absolute
	      && ! info->shared)
	    {
	      /* Convert the GP-relative instruction to absolute by
		 rewriting the base register.
	      */
	      if (rel < (relend - 1)
		  && reloc_matching_pair (output_bfd, rel, rel + 1, contents))
		{
		  /* Handle this relocation and the next one together. */
		  Elf_Internal_Rela *low_rel;
		  Elf_Internal_Rela *high_rel;
		  bfd_vma insn;

		  if (r_type == R_LX_GOTOFF_LO9
		      || r_type == R_LX_GOTOFFX_LO9)
		    {
		      low_rel = rel;
		      high_rel = rel + 1;
		    }
		  else
		    {
		      low_rel = rel + 1;
		      high_rel = rel;
		    }
		  insn = bfd_get_32 (input_bfd, contents + low_rel->r_offset);
		  if ((insn & 0x3f) != 0)
		    { /* Do the transformation. */

		      insn &= 0xffffffc0; /* Rewrite base register to be r0 */
		      bfd_put_32 (input_bfd, insn, contents + low_rel->r_offset);
		      if (info->emitrelocations)
			{
			  struct elf_link_hash_entry *hgot = elf_hash_table(info)->hgot;
			  bfd_vma hgot_value = hgot->root.u.def.value
			    + hgot->root.u.def.section->output_section->vma
			    + hgot->root.u.def.section->output_offset;

			  low_rel->r_info = ELF32_R_INFO(STN_UNDEF, R_LX_LO9);
			  low_rel->r_addend = value - hgot_value;
			  rel_hash[low_rel - relocs] = hgot;
			  high_rel->r_info = ELF32_R_INFO(STN_UNDEF, R_LX_HI23);
			  high_rel->r_addend = value - hgot_value;
			  rel_hash[high_rel - relocs] = hgot;
			}
		    }
		  else
		    value -= gp_val;
		  r = _bfd_final_link_relocate (elf32_lx_howto_table
						+ ELF32_R_TYPE (low_rel->r_info),
						input_bfd, input_section, contents,
						low_rel->r_offset, value, 0);
		  if (r == bfd_reloc_ok)
		    {
		      r = _bfd_final_link_relocate (elf32_lx_howto_table
						    + ELF32_R_TYPE (high_rel->r_info),
						    input_bfd, input_section, contents,
						    high_rel->r_offset, value, 0);
		    }
		  rel++; /* We have processed the next rel */
		  break;
		}
	    }
	  value -= gp_val;
	  goto default_reloc;

	case R_LX_PLTOFF_HI23:
	case R_LX_PLTOFF_LO9:
          dyn_i = get_dyn_sym_info (lx_info, h, input_bfd, rel, FALSE);
	  value = set_pltoff_entry (output_bfd, info, dyn_i, value, FALSE);
	  value -= gp_val;
	  goto default_reloc;

	case R_LX_FPTR32:
          dyn_i = get_dyn_sym_info (lx_info, h, input_bfd, rel, FALSE);
	  if (dyn_i->want_fptr)
	    {
	      if (!undef_weak_ref)
		value = set_fptr_entry (output_bfd, info, dyn_i, value);
	    }
	  else
	    {
	      long dynindx;
	      bfd_boolean relocate = FALSE;

	      /* Otherwise, we expect the dynamic linker to create
		 the entry.  */

	      if (h)
		{
		  if (h->dynindx != -1)
		    dynindx = h->dynindx;
		  else
		    dynindx = (_bfd_elf_link_lookup_local_dynindx
			       (info, h->root.u.def.section->owner,
				global_sym_index (h)));
		}
	      else
		{
		  dynindx = (_bfd_elf_link_lookup_local_dynindx
			     (info, input_bfd, (long) r_symndx));
		}

	      elf_lx_install_dyn_reloc (output_bfd, info, input_section,
					srel, rel->r_offset, r_type,
					dynindx, rel->r_addend, &relocate);
	      if (!relocate) value = 0;
	    }

	  goto default_reloc;

	case R_LX_GOTOFF_FPTR_HI23:
	case R_LX_GOTOFF_FPTR_LO9:
	  {
	    long dynindx;

	    dyn_i = get_dyn_sym_info (lx_info, h, input_bfd, rel, FALSE);
	    if (dyn_i->want_fptr)
	      {
		BFD_ASSERT (h == NULL || h->dynindx == -1);
	        if (!undef_weak_ref)
	          value = set_fptr_entry (output_bfd, info, dyn_i, value);
		dynindx = -1;
	      }
	    else
	      {
	        /* Otherwise, we expect the dynamic linker to create
		   the entry.  */
	        if (h)
		  {
		    if (h->dynindx != -1)
		      dynindx = h->dynindx;
		    else
		      dynindx = (_bfd_elf_link_lookup_local_dynindx
				 (info, h->root.u.def.section->owner,
				  global_sym_index (h)));
		  }
		else
		  dynindx = (_bfd_elf_link_lookup_local_dynindx
			     (info, input_bfd, (long) r_symndx));
		value = 0;
	      }

	    value = set_got_entry (output_bfd, info, dyn_i, dynindx,
				   rel->r_addend, value, R_LX_FPTR32);
	    value -= gp_val;
	  }
	  goto default_reloc;

	case R_LX_32_PCREL:
	  /* Install a dynamic relocation for this reloc.  */
	  if (dynamic_symbol_p && r_symndx != 0)
	    {
	      bfd_boolean relocate = FALSE;
	      BFD_ASSERT (srel != NULL);

	      elf_lx_install_dyn_reloc (output_bfd, info, input_section,
					srel, rel->r_offset, r_type,
					h->dynindx, rel->r_addend, &relocate);
	    }
	  goto default_reloc;

	case R_LX_23_PCREL:
	  /* We should have created a PLT entry for any dynamic symbol.  */
	  dyn_i = NULL;
	  if (h)
	    dyn_i = get_dyn_sym_info (lx_info, h, NULL, NULL, FALSE);

	  if (dyn_i && dyn_i->want_plt2)
	    {
	      /* Should have caught this earlier.  */
	      BFD_ASSERT (rel->r_addend == 0);

	      value = (lx_info->plt_sec->output_section->vma
		       + lx_info->plt_sec->output_offset
		       + dyn_i->plt2_offset);
	    }
	  else
	    {
	      bfd_vma from;
	      bfd_signed_vma branch_offset;

	      /* Since there's no PLT entry, Validate that this is
		 locally defined.  */
	      BFD_ASSERT (undef_weak_ref || sym_sec->output_section != NULL);

	      /* If the symbol is undef_weak, we shouldn't be trying
		 to call it.  There's every chance that we'd wind up
		 with an out-of-range fixup here.  Don't bother setting
		 any value at all.  */
	      if (undef_weak_ref)
		continue;

	      from = (input_section->output_section->vma
		      + input_section->output_offset
		      + rel->r_offset);
	      branch_offset = (bfd_signed_vma)(value - from);
	      if (branch_offset > MAX_FWD_BRANCH_OFFSET
		  || branch_offset < MAX_BWD_BRANCH_OFFSET)
		{
		  /* The target is out of reach, so redirect the
		     branch to the local stub for this function.
		  */
		  struct elf_lx_stub_hash_entry *stub_entry;

		  stub_entry = lx_get_stub_entry (input_section, sym_sec, h,
						  rel, lx_info);
		  if (stub_entry != NULL)
		    {
		      value = (stub_entry->stub_offset
			       + stub_entry->stub_sec->output_offset
			       + stub_entry->stub_sec->output_section->vma);
		      if (info->emitrelocations)
			{
			  /* Simpler version where we put the stub symbol in
			     rel_hashes, and set the index to STN_UNDEF to prevent
			     . */
			  rel_hash[rel - relocs] =
			    (struct elf_link_hash_entry *)stub_entry->sh;

			  stub_entry->sh->root.indx = -2;
			  rel->r_info = ELF32_R_INFO(STN_UNDEF, r_type);
			}
		    }
		}
	    }
	  goto default_reloc;

	case R_LX_SEGREL32:
	  {
	    struct elf_segment_map *m;
	    Elf_Internal_Phdr *p;
	    
	    /* Find the segment that contains the output_section.  */
	    for (m = elf_tdata (output_bfd)->segment_map,
		   p = elf_tdata (output_bfd)->phdr;
		 m != NULL;
		 m = m->next, p++)
	      {
		int i;
		for (i = m->count - 1; i >= 0; i--)
		  if (m->sections[i] == sym_sec->output_section)
		    break;
		if (i >= 0)
		  break;
	      }
	    
	    if (m == NULL)
	      {
		r = bfd_reloc_notsupported;
	      }
	    else
	      {
		/* The VMA of the segment is the vaddr of the associated
		   program header.  */
		if (value > p->p_vaddr)
		  value -= p->p_vaddr;
		else
		  value = 0;
		goto default_reloc;
	      }
	  }
	  break;

	case R_LX_IPLT:
	case R_LX_JMP_SLOT:
	  /* Install a dynamic relocation for this reloc.  */
	  if ((dynamic_symbol_p || info->shared)
	      && (input_section->flags & SEC_ALLOC) != 0)
	    {
	      BFD_ASSERT (srel != NULL);
	      bfd_boolean relocate = FALSE;

	      /* If we don't need dynamic symbol lookup, install
		 RELATIVE relocations.  */
	      if (! dynamic_symbol_p)
		{
		  unsigned int dyn_r_type;

		  dyn_r_type = R_LX_REL32;

		  elf_lx_install_dyn_reloc (output_bfd, info,
					    input_section,
					    srel, rel->r_offset,
					    dyn_r_type, 0, value, &relocate);
		  if (r_type == R_LX_IPLT)
		    elf_lx_install_dyn_reloc (output_bfd, info,
					      input_section,
					      srel, rel->r_offset + 4,
					      dyn_r_type, 0, gp_val, &relocate);
		}
	      else
		elf_lx_install_dyn_reloc (output_bfd, info, input_section,
					  srel, rel->r_offset, r_type,
					  h->dynindx, rel->r_addend, &relocate);
	    }

	  howto = elf32_lx_howto_table + R_LX_32;
	  r = _bfd_final_link_relocate (howto, input_bfd, input_section,
					contents, rel->r_offset, value, 0);
	  if (r == bfd_reloc_ok && r_type == R_LX_IPLT)
	    r = _bfd_final_link_relocate (howto, input_bfd, input_section,
					  contents, rel->r_offset + 4, gp_val,
					  0);
	  break;

	case R_LX_LTV32:
	default_reloc:
	  r = _bfd_final_link_relocate (howto, input_bfd, input_section,
					contents, rel->r_offset, value, 0);
	  break;

	case R_LX_TPREL_HI23:
	case R_LX_TPREL_LO9:
	case R_LX_TPREL32:
	  value -= elf_lx_tprel_base (info);
	  goto default_reloc;

	case R_LX_DTPREL_HI23:
	case R_LX_DTPREL_LO9:
	case R_LX_DTPREL32:
	  value -= elf_lx_dtprel_base (info);
	  goto default_reloc;

	case R_LX_GOTOFF_TPREL_HI23:
	case R_LX_GOTOFF_TPREL_LO9:
	case R_LX_GOTOFF_DTPLDM_HI23:
	case R_LX_GOTOFF_DTPLDM_LO9:
	case R_LX_GOTOFF_DTPNDX_HI23:
	case R_LX_GOTOFF_DTPNDX_LO9:
	  {
	    int got_r_type;
	    long dynindx = h ? h->dynindx : -1;
	    bfd_vma r_addend = rel->r_addend;

	    switch (r_type)
	      {
	      default:
	      case R_LX_GOTOFF_TPREL_HI23:
	      case R_LX_GOTOFF_TPREL_LO9:
		if (!dynamic_symbol_p)
		  {
		    if (!info->shared)
		      value -= elf_lx_tprel_base (info);
		    else
		      {
			r_addend += value - elf_lx_dtprel_base (info);
			dynindx = 0;
		      }
		  }
		got_r_type = R_LX_TPREL32;
		break;
	      case R_LX_GOTOFF_DTPLDM_HI23:
	      case R_LX_GOTOFF_DTPLDM_LO9:
		if (!dynamic_symbol_p && !info->shared)
		  value = 1;  /* Main program module number */
		got_r_type = R_LX_GOTOFF_DTPLDM_LO9;
		break;
	      case R_LX_GOTOFF_DTPNDX_HI23:
	      case R_LX_GOTOFF_DTPNDX_LO9:
		if (!dynamic_symbol_p)
		  {
		    value -= elf_lx_dtprel_base (info);
		    dynindx = 0;
		  }
		got_r_type = R_LX_GOTOFF_DTPNDX_LO9;
		break;
	      }
	    dyn_i = get_dyn_sym_info (lx_info, h, input_bfd, rel, FALSE);
	    value = set_got_entry (input_bfd, info, dyn_i, dynindx, r_addend,
				   value, got_r_type);

	    if (lx_info->transform_to_absolute
		&& ! info->shared)
	      {
		/* Convert the GP-relative instruction to absolute by
		   rewriting the base register.
		*/
		if (rel < (relend - 1)
			 && reloc_matching_pair (output_bfd, rel, rel + 1,
						 contents))
		  {
		    /* Handle this relocation and the next one together. */
		    Elf_Internal_Rela *low_rel;
		    Elf_Internal_Rela *high_rel;
		    bfd_vma insn;
		    

		    if (r_type == R_LX_GOTOFF_TPREL_LO9
			|| r_type == R_LX_GOTOFF_DTPLDM_LO9
			|| r_type == R_LX_GOTOFF_DTPNDX_LO9)
		      {
			low_rel = rel;
			high_rel = rel + 1;
		      }
		    else
		      {
			low_rel = rel + 1;
			high_rel = rel;
		      }
		    insn = bfd_get_32 (input_bfd, contents + low_rel->r_offset);
		    if ((insn & 0x3f) != 0)
		      { /* Do the transformation. */
			insn &= 0xffffffc0; /* Rewrite base register to be r0 */
			bfd_put_32 (input_bfd, insn, contents + low_rel->r_offset);
			if (info->emitrelocations)
			  {
			    struct elf_link_hash_entry *hgot = elf_hash_table(info)->hgot;
			    bfd_vma hgot_value = hgot->root.u.def.value
			      + hgot->root.u.def.section->output_section->vma
			      + hgot->root.u.def.section->output_offset;

			    low_rel->r_info = ELF32_R_INFO(STN_UNDEF, R_LX_LO9);
			    low_rel->r_addend = value - hgot_value;
			    rel_hash[low_rel - relocs] = hgot;
			    high_rel->r_info = ELF32_R_INFO(STN_UNDEF, R_LX_HI23);
			    high_rel->r_addend = value - hgot_value;
			    rel_hash[high_rel - relocs] = hgot;
			  }
		      }
		    else
		      value -= gp_val;
		    r = _bfd_final_link_relocate (elf32_lx_howto_table
						  + ELF32_R_TYPE (low_rel->r_info),
						  input_bfd, input_section, contents,
						  low_rel->r_offset, value, 0);
		    if (r == bfd_reloc_ok)
		      {
			r = _bfd_final_link_relocate (elf32_lx_howto_table
						      + ELF32_R_TYPE (high_rel->r_info),
						      input_bfd, input_section, contents,
						      high_rel->r_offset, value, 0);
		      }
		    rel++; /* We have processed the next rel */
		    break;
		  }
	      }
	    else
	      value -= gp_val;
	    goto default_reloc;
	  }

	default:
	  r = bfd_reloc_notsupported;
	  break;
	}

      switch (r)
	{
	case bfd_reloc_ok:
	  break;
	  
	case bfd_reloc_undefined:
	  /* This can happen for global table relative relocs if
	     __gp is undefined.  This is a panic situation so we
	     don't try to continue.  */
	  (*info->callbacks->undefined_symbol)
	    (info, "__gp", input_bfd, input_section, rel->r_offset, 1);
	  return FALSE;

	case bfd_reloc_notsupported:
	  {
	    const char *name;

	    if (h)
	      name = h->root.root.string;
	    else
	      {
		name = bfd_elf_string_from_elf_section (input_bfd,
							symtab_hdr->sh_link,
							sym->st_name);
		if (name == NULL)
		  return FALSE;
		if (*name == '\0')
		  name = bfd_section_name (input_bfd, input_section);
	      }
	    if (!(*info->callbacks->warning) (info, _("unsupported reloc"),
					      name, input_bfd,
					      input_section, rel->r_offset))
	      return FALSE;
	    ret_val = FALSE;
	  }
	  break;

	case bfd_reloc_dangerous:
	case bfd_reloc_outofrange:
	case bfd_reloc_overflow:
	default:
	  {
	    const char *name;

	    if (h)
	      name = h->root.root.string;
	    else
	      {
		name = bfd_elf_string_from_elf_section (input_bfd,
							symtab_hdr->sh_link,
							sym->st_name);
		if (name == NULL)
		  return FALSE;
		if (*name == '\0')
		  name = bfd_section_name (input_bfd, input_section);
	      }
	    if (!(*info->callbacks->reloc_overflow) (info,
						     (h ? &h->root : NULL),
						     name,					     
						     howto->name,
						     (bfd_vma) 0,
						     input_bfd,
						     input_section,
						     rel->r_offset))
	      return FALSE;
	    ret_val = FALSE;
	  }
	  break;
	}
    }

  return ret_val;
}

static bfd_boolean elf_lx_output_arch_local_syms(
  bfd *output_bfd,
  struct bfd_link_info *info,
  void *finfo ATTRIBUTE_UNUSED,
  bfd_boolean (*func) (void *, const char *,
		       Elf_Internal_Sym *,
		       asection *,
		       struct elf_link_hash_entry *) ATTRIBUTE_UNUSED)
{
  /* Ugly: hijack this hook to do some late GOT relocation processing.
     It is not at all the purpose of this hook, but it happens to occur
     at the correct stage of processing. */
  struct elf_lx_link_hash_table *lx_info = elf_lx_hash_table (info);
  asection *got_sec = lx_info->got_sec;
  if (got_sec != NULL)
    {
      unsigned int reloc_count = got_sec->reloc_count;
      if (reloc_count > 0)
	{
	  asection *output_section = got_sec->output_section;
	  struct bfd_elf_section_data *esdo = elf_section_data (output_section);
	  struct bfd_elf_section_data *esdg = elf_section_data (got_sec);
	  Elf_Internal_Rela *relocs = esdg->relocs;
	  unsigned int rel_count = esdg->rel.count;
	  struct elf_link_hash_entry **rel_hash;
	  
	  rel_hash = esdo->rel.hashes + esdo->rel.count;
	  memcpy (rel_hash, lx_info->got_rel_hash, reloc_count * sizeof (*rel_hash));
	  free (lx_info->got_rel_hash);
	  lx_info->got_rel_hash = NULL;
	  if (rel_count < reloc_count)
	    {
	      /* We have reserved space for more relocs than we actually used.
		 So, tidy the trailing ones so that they have a sensible offset.
		 They will be of type R_LX_NONE.
	      */
	      bfd_vma last_offset;
	      Elf_Internal_Rela *rel;
	      
	      if (rel_count > 0)      
		last_offset = (relocs + rel_count - 1)->r_offset;
	      else
		last_offset = got_sec->output_section->vma + got_sec->output_offset;
	      for (rel = relocs + rel_count; rel < relocs + reloc_count; rel++)
		rel->r_offset = last_offset;
	      elf_section_data (got_sec)->rel.count = reloc_count;
	    }
	  return _bfd_elf_link_output_relocs (output_bfd, output_section,
					      _bfd_elf_single_rel_hdr (got_sec),
					      relocs, rel_hash);
	}
    }
  return TRUE;
}

static bfd_boolean
elf_lx_gc_sweep_hook (bfd *abfd, struct bfd_link_info *link_info,
		      asection *sec, const Elf_Internal_Rela *relocs)
{
  const Elf_Internal_Rela *rel, *relend;
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_lx_link_hash_table *lx_info;

  lx_info = elf_lx_hash_table (link_info);
  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;

  relend = relocs + sec->reloc_count;
  for (rel = relocs; rel < relend; rel++)
    {
      unsigned long r_symndx;
      unsigned int r_type;
      struct elf_link_hash_entry *h = NULL;
      struct elf_lx_dyn_sym_info *dyn_i;
      struct elf_lx_dyn_reloc_entry **pp;
      struct elf_lx_dyn_reloc_entry *p;

      r_symndx = ELF32_R_SYM (rel->r_info);
      if (r_symndx >= symtab_hdr->sh_info)
	{
	  long indx = r_symndx - symtab_hdr->sh_info;
	  h = elf_sym_hashes (abfd)[indx];
	  while (h->root.type == bfd_link_hash_indirect
		 || h->root.type == bfd_link_hash_warning)
	    h = (struct elf_link_hash_entry *) h->root.u.i.link;
	}
      dyn_i = get_dyn_sym_info (lx_info, h, abfd, rel, FALSE);
      if (dyn_i)
	{
	  for (pp = &dyn_i->reloc_entries; (p = *pp) != NULL; pp = &p->next)
	    if (p->sec == sec)
	      /* Everything must go for SEC. */
	      *pp = p->next;
	  r_type = ELF32_R_TYPE (rel->r_info);
	  /* Update want_got, want_tprel, want_dtpndx, want_dtpldm counts for
	     DYN_I. */
	  /* Note that we do not have refcounts available for PLT requirements, so
	     here we cannot eliminate unused PLT entries. */
	  switch (r_type)
	    {
	    case R_LX_GOTOFF_FPTR_HI23:
	    case R_LX_GOTOFF_FPTR_LO9:
	    case R_LX_GOTOFF_HI23:
	    case R_LX_GOTOFF_LO9:
	    case R_LX_GOTOFFX_HI23:
	    case R_LX_GOTOFFX_LO9:
	      if (dyn_i->want_got > 0)
		dyn_i->want_got--;
	      break;
	    case R_LX_GOTOFF_TPREL_HI23:
	    case R_LX_GOTOFF_TPREL_LO9:
	      if (dyn_i->want_tprel > 0)
		dyn_i->want_tprel--;
	      break;
	    case R_LX_GOTOFF_DTPLDM_HI23:
	    case R_LX_GOTOFF_DTPLDM_LO9:
	      if (dyn_i->want_dtpldm > 0)
		dyn_i->want_dtpldm--;
	      break;
	    case R_LX_GOTOFF_DTPNDX_HI23:
	    case R_LX_GOTOFF_DTPNDX_LO9:
	      if (dyn_i->want_dtpndx > 0)
		dyn_i->want_dtpndx--;
	      break;
	    }
	}
    }
  return TRUE;
}

static bfd_boolean
elf_lx_finish_dynamic_symbol (
			      bfd *output_bfd,
			      struct bfd_link_info *info,
			      struct elf_link_hash_entry *h,
			      Elf_Internal_Sym *sym)
{

  struct elf_lx_link_hash_table *lx_info;
  struct elf_lx_dyn_sym_info *dyn_i;

  lx_info = elf_lx_hash_table (info);
  dyn_i = get_dyn_sym_info (lx_info, h, NULL, NULL, FALSE);

  /* Fill in the PLT data, if required.  */
  if (dyn_i && dyn_i->want_plt)
    {
      Elf_Internal_Rela outrel;
      bfd_byte *loc;
      asection *plt_sec;
      bfd_vma plt_addr, pltoff_addr, gp_val, index, reloc_offset;
      Elf32_External_Rela *rel;
      int i;

      gp_val = _bfd_get_gp_value (output_bfd);

      /* Initialize the minimal PLT entry.  */

      index = (dyn_i->plt_offset - PLT_HEADER_SIZE) / PLT_MIN_ENTRY_SIZE;
      plt_sec = lx_info->plt_sec;
      loc = plt_sec->contents + dyn_i->plt_offset;
      reloc_offset = index * sizeof (Elf32_External_Rela);

      for (i = 0; i < (PLT_MIN_ENTRY_SIZE / 4); i++)
	bfd_put_32 (output_bfd, plt_min_entry[i],
		    loc + (4 * i));
      
      _bfd_final_link_relocate (elf32_lx_howto_table + R_LX_23_PCREL,
				output_bfd, plt_sec, plt_sec->contents,
				dyn_i->plt_offset + 0,
				plt_sec->output_section->vma, 0);
      _bfd_final_link_relocate (elf32_lx_howto_table + R_LX_LO9,
				output_bfd, plt_sec, plt_sec->contents,
				dyn_i->plt_offset + 4, reloc_offset, 0);
      _bfd_final_link_relocate (elf32_lx_howto_table + R_LX_HI23,
				output_bfd, plt_sec, plt_sec->contents,
				dyn_i->plt_offset + 8, reloc_offset, 0);

      plt_addr = (plt_sec->output_section->vma
		  + plt_sec->output_offset
		  + dyn_i->plt_offset);
      pltoff_addr = set_pltoff_entry (output_bfd, info, dyn_i, plt_addr,
				      TRUE);

      /* Initialize the FULL PLT entry, if needed.  */
      if (dyn_i->want_plt2)
	{
	  loc = plt_sec->contents + dyn_i->plt2_offset;

	  if (pic_abi_p (output_bfd))
	    {
	      const bfd_vma *template = (lx_mach_has_st240_encodings (output_bfd)
					 ? pic_abi_plt_full_entry_st240
					 : pic_abi_plt_full_entry_pre_st240);

	      for (i = 0; i < (PLT_FULL_ENTRY_SIZE_PIC_ABI / 4); i++)
		bfd_put_32 (output_bfd, template[i], loc + (4 * i));
	      _bfd_final_link_relocate (elf32_lx_howto_table + R_LX_HI23,
					output_bfd, plt_sec, plt_sec->contents,
					dyn_i->plt2_offset + 0,
					pltoff_addr - gp_val, 0);
	      _bfd_final_link_relocate (elf32_lx_howto_table + R_LX_LO9,
					output_bfd, plt_sec, plt_sec->contents,
					dyn_i->plt2_offset + 4,
					pltoff_addr - gp_val, 0);
	      _bfd_final_link_relocate (elf32_lx_howto_table + R_LX_HI23,
					output_bfd, plt_sec, plt_sec->contents,
					dyn_i->plt2_offset + 16,
					(pltoff_addr + 4)- gp_val, 0);
	      _bfd_final_link_relocate (elf32_lx_howto_table + R_LX_LO9,
					output_bfd, plt_sec, plt_sec->contents,
					dyn_i->plt2_offset + 20,
					(pltoff_addr + 4)- gp_val, 0);
	    }
	  else
	    { /* embedded ABI */
	      bfd_vma got_entry_addr = pltoff_addr;
	      bfd_vma base_reg = 0;
	      const bfd_vma *template = (lx_mach_has_st240_encodings (output_bfd)
					 ? embedded_abi_plt_full_entry_st240
					 : embedded_abi_plt_full_entry_pre_st240);

	      if (info->shared)
		{
		  got_entry_addr = pltoff_addr - gp_val;
		  base_reg = 14;
		}

	      for (i = 0; i < (PLT_FULL_ENTRY_SIZE / 4); i++)
		{
		  bfd_vma val = template[i];
		  if (i == 1)
		    val |= base_reg;
		  bfd_put_32 (output_bfd, val, loc + (4 * i));
		}
	      _bfd_final_link_relocate (elf32_lx_howto_table + R_LX_HI23,
					output_bfd, plt_sec,
					plt_sec->contents,
					dyn_i->plt2_offset + 0,
					got_entry_addr, 0);
	      _bfd_final_link_relocate (elf32_lx_howto_table + R_LX_LO9,
					output_bfd, plt_sec,
					plt_sec->contents,
					dyn_i->plt2_offset + 4,
					got_entry_addr, 0);
	    }
	      
	  /* Mark the symbol as undefined, rather than as defined in the
	     plt section.  Leave the value alone.  */
	  if (! h->def_regular)
	    sym->st_shndx = SHN_UNDEF;
	}

      /* Create the dynamic relocation.  */
      outrel.r_offset = pltoff_addr;
      if (pic_abi_p (output_bfd))
	outrel.r_info = ELF32_R_INFO (h->dynindx, R_LX_IPLT);
      else
	outrel.r_info = ELF32_R_INFO (h->dynindx, R_LX_JMP_SLOT);
      outrel.r_addend = 0;

      /* This is fun.  In the .lx.pltoff section, we've got entries
	 that correspond both to real PLT entries, and those that
	 happened to resolve to local symbols but need to be created
	 to satisfy @pltoff relocations.  The .rela.lx.pltoff
	 relocations for the real PLT should come at the end of the
	 section, so that they can be indexed by plt entry at runtime.

	 We emitted all of the relocations for the non-PLT @pltoff
	 entries during relocate_section.  So we can consider the
	 existing sec->reloc_count to be the base of the array of
	 PLT relocations.  */

      rel = (Elf32_External_Rela *)lx_info->rel_pltoff_sec->contents;
      rel += lx_info->rel_pltoff_sec->reloc_count;

      bfd_elf32_swap_reloca_out (output_bfd, &outrel, (bfd_byte *) (rel + index));
    }

  {
    /* Create a COPY relocation, if required. */
    struct elf_lx_link_hash_entry *lx_h = (struct elf_lx_link_hash_entry *)h;
    bfd_boolean dummy;

    if (lx_h->rel_dynbss_allocated)
      elf_lx_install_dyn_reloc (output_bfd, info,
				h->root.u.def.section,
				lx_info->rel_dynbss_sec,
				h->root.u.def.value,
				R_LX_COPY,
				h->dynindx,
				0,
				&dummy);
  }

  /* Mark some specially defined symbols as absolute.  */
  if (strcmp (h->root.root.string, "_DYNAMIC") == 0
      || strcmp (h->root.root.string, "_GLOBAL_OFFSET_TABLE_") == 0
      || strcmp (h->root.root.string, "_PROCEDURE_LINKAGE_TABLE_") == 0)
    sym->st_shndx = SHN_ABS;

  return TRUE;
}

static bfd_boolean
elf_lx_finish_dynamic_sections (
				bfd *abfd,
				struct bfd_link_info *info)
{
  struct elf_lx_link_hash_table *lx_info;
  bfd *dynobj;

  lx_info = elf_lx_hash_table (info);
  dynobj = lx_info->root.dynobj;

  if (elf_hash_table (info)->dynamic_sections_created)
    {
      Elf32_External_Dyn *dyncon, *dynconend;
      asection *sdyn;
      bfd_vma gp_val;

      sdyn = bfd_get_section_by_name (dynobj, ".dynamic");
      BFD_ASSERT (sdyn != NULL);
      dyncon = (Elf32_External_Dyn *) sdyn->contents;
      dynconend = (Elf32_External_Dyn *) (sdyn->contents + sdyn->size);

      gp_val = _bfd_get_gp_value (abfd);

      for (; dyncon < dynconend; dyncon++)
	{
	  Elf_Internal_Dyn dyn;

	  bfd_elf32_swap_dyn_in (dynobj, dyncon, &dyn);

	  switch (dyn.d_tag)
	    {
	    case DT_PLTGOT:
	      dyn.d_un.d_ptr = gp_val;
	      break;

	    case DT_PLTRELSZ:
	      dyn.d_un.d_val = (lx_info->minplt_entries
				* sizeof (Elf32_External_Rela));
	      break;

	    case DT_JMPREL:
	      /* See the comment above in finish_dynamic_symbol.  */
	      dyn.d_un.d_ptr = (lx_info->rel_pltoff_sec->output_section->vma
				+ lx_info->rel_pltoff_sec->output_offset
				+ (lx_info->rel_pltoff_sec->reloc_count
				   * sizeof (Elf32_External_Rela)));
	      break;

	    case DT_RELASZ:
	      /* Do not have RELASZ include JMPREL.  This makes things
		 easier on ld.so.  This is not what the rest of BFD set up.  */
	      dyn.d_un.d_val -= (lx_info->minplt_entries
				 * sizeof (Elf32_External_Rela));
	      break;
	    }

	  bfd_elf32_swap_dyn_out (abfd, &dyn, dyncon);
	}

      /* Initialize the PLT0 entry */
      if (lx_info->plt_sec)
	{
	  bfd_byte *loc = lx_info->plt_sec->contents;
	  int i;

	  if (pic_abi_p (abfd))
	    {
	      const bfd_vma *template = (lx_mach_has_st240_encodings (abfd)
					 ? pic_abi_plt_header_st240
					 : pic_abi_plt_header_pre_st240);

	      for (i = 0; i < (PLT_HEADER_SIZE / 4); i++)
		bfd_put_32 (abfd, template[i], loc + (4 * i));
	    }
	  else
	    { /* embedded ABI */
	      if (info->shared)
		{
		  const bfd_vma *template = (lx_mach_has_st240_encodings (abfd)
					     ? embedded_abi_plt_header_st240
					     : embedded_abi_plt_header_pre_st240);

		  for (i = 0; i < (PLT_HEADER_SIZE / 4); i++)
		    bfd_put_32 (abfd, template[i], loc + (4 * i));
		}
	      else
		{
		  const bfd_vma *template =
		    (lx_mach_has_st240_encodings (abfd)
		     ? embedded_abi_absolute_plt_header_st240
		     : embedded_abi_absolute_plt_header_pre_st240);

		  for (i = 0; i < (PLT_HEADER_SIZE / 4); i++)
		    {
		      bfd_vma val = template[i];

		      if (i == 0)
			val |= ((gp_val + 4) >> 9) & 0x7fffff;
		      else if (i == 1)
			val |= ((gp_val + 4) & 0x1ff) << 12;
		      else if (i == 3)
			val |= (gp_val & 0x1ff) << 12;
		      else if (i == 4)
			val |= (gp_val >> 9) & 0x7fffff;
		      bfd_put_32 (abfd, val, loc + (4 * i));
		    }
		}
	    }
	}
    }

  return TRUE;
}

void
bfd_elf32_lx_set_transform_to_absolute (struct bfd_link_info *info,
					bfd_boolean value)
{
  elf_lx_hash_table(info)->transform_to_absolute = value;
}

/* Return true if SYM represents a local label symbol. */

static bfd_boolean
lx_elf_is_local_label_name(
			   bfd *abfd ATTRIBUTE_UNUSED,
			   const char *name)
{
  return ((name[0] == '.' && name[1] == 'L') || 
	  (name[0] == 'L' && name[1] == '?') ||
	  (name[0] == '_' && name[1] == '?') ||
	  (name[0] == '?') ||
	  (name[0] == '$'));
}

/* 
   Merge backend specific data from an object file to the output
   object file when linking  -- we just use it to check object file
 consistency -- we could also check lxbe vs. multiflow here 
 see elf32-ppc.c for an example
*/
int first_core = 1;
int first_cut = 1;
int first_abi = 1;
int first_osabi = 1;
int first_code_generation_mode = 1;


static bfd_boolean
lx_elf_merge_private_bfd_data (
			       bfd *ibfd,
			       bfd *obfd)
{
  flagword input_flags;
  flagword output_flags;
  Elf_Internal_Ehdr * input_ehdrp;
  Elf_Internal_Ehdr * output_ehdrp;

  /* Check if we have the same endianess */

  if (ibfd->xvec->byteorder != obfd->xvec->byteorder
      && obfd->xvec->byteorder != BFD_ENDIAN_UNKNOWN
/* (pp) allow linking with other formats */
      && ibfd->xvec->byteorder != BFD_ENDIAN_UNKNOWN
     )
    {
      (*_bfd_error_handler)
	(_("%s: compiled for a %s endian system and target is %s endian"),
	 bfd_get_filename (ibfd),
	 bfd_big_endian (ibfd) ? "big" : "little",
	 bfd_big_endian (obfd) ? "big" : "little");

      bfd_set_error (bfd_error_wrong_format);
      return FALSE;
    }

#if 0
  lx_bfd_flags old_flags;
  lx_bfd_flags new_flags;
  new_flags.f = elf_elfheader (ibfd)->e_flags;
  old_flags.f = elf_elfheader (obfd)->e_flags;
  if (!elf_flags_init (obfd))	/* First call, no flags set */
    {
      elf_flags_init (obfd) = TRUE;
      elf_elfheader (obfd)->e_flags = new_flags.f;
    }

  else if (new_flags.f == old_flags.f)	
  	/* Same flags: always ok */
    ;
  else 
    {
      unsigned int old_rta = old_flags.obj_compat.rta;
      unsigned int new_rta = new_flags.obj_compat.rta;
      if (old_rta && new_rta && old_rta != new_rta)
        {
          (*_bfd_error_handler)(
	      _("%s: uses different e_flags [%x] (rta:%d) fields "
	      "than previous modules [%x] (rta:%d)"),
	      bfd_get_filename (ibfd), 
	      new_flags.f, new_flags.obj_compat.rta, 
	      old_flags.f, old_flags.obj_compat.rta);
          bfd_set_error(bfd_error_bad_value);
          return FALSE;
        }
    }
#endif

  input_flags = lx_elf_get_private_flags(ibfd);
  output_flags = lx_elf_get_private_flags(obfd);
  input_ehdrp = elf_elfheader (ibfd);
  output_ehdrp = elf_elfheader (obfd);

  if (!elf_flags_init (obfd)) {  /* First call, no flags set */
    lx_elf_set_private_flags (obfd, input_flags);
    output_ehdrp->e_ident[EI_ABIVERSION] = input_ehdrp->e_ident[EI_ABIVERSION] ;
    output_ehdrp->e_ident[EI_OSABI] = input_ehdrp->e_ident[EI_OSABI] ;
    return TRUE;
  }
 
  if (strcmp(core_printable_name(input_flags),core_printable_name(output_flags)))
  {
    flagword input_core;
    flagword output_core;

    (*_bfd_error_handler)("warning : mixing cores : %s (%s) and %s (%s)", bfd_get_filename (ibfd), core_printable_name(input_flags), bfd_get_filename (obfd), core_printable_name(output_flags) );  
    /* take the highest machine number */
    input_core = (input_flags & ELF_LX_CORE_MASK);
    output_core = (output_flags & ELF_LX_CORE_MASK);
    if (input_core > output_core)
	output_core = input_core;
      output_flags = ((output_flags | ELF_LX_CORE_MASK) & output_core);
      lx_elf_set_private_flags (obfd, output_flags);
  }

  if (strcmp(cut_printable_name(input_flags),cut_printable_name(output_flags)))
  {
    (*_bfd_error_handler)("warning : mixing cuts : %s (%s) and %s (%s)", bfd_get_filename (ibfd), cut_printable_name(input_flags), bfd_get_filename (obfd), cut_printable_name(output_flags) );  
  }

  if (strcmp(abi_printable_name(input_flags, input_ehdrp),abi_printable_name(output_flags, output_ehdrp)))
  {
    (*_bfd_error_handler)("warning : mixing abis : %s (%s) and %s (%s)", bfd_get_filename (ibfd), abi_printable_name(input_flags, input_ehdrp), bfd_get_filename (obfd), abi_printable_name(output_flags, output_ehdrp) );  
  }

  if (strcmp(osabi_printable_name(input_ehdrp),osabi_printable_name(output_ehdrp)))
  {
    (*_bfd_error_handler)("warning : mixing osabis : %s (%s) and %s (%s)", bfd_get_filename (ibfd), osabi_printable_name(input_ehdrp), bfd_get_filename (obfd), osabi_printable_name(output_ehdrp) );  
  }

    if (strcmp(code_generation_mode_printable_name(input_ehdrp),code_generation_mode_printable_name(output_ehdrp)))
  {
    (*_bfd_error_handler)("warning : mixing code generation modes : %s (%s) and %s (%s)", bfd_get_filename (ibfd), osabi_printable_name(input_ehdrp), bfd_get_filename (obfd), osabi_printable_name(output_ehdrp) );  
  }

  return TRUE;
}

static bfd_boolean
lx_elf_print_private_bfd_data (bfd *abfd, PTR farg)
{
  FILE *f = (FILE *) farg;

  lx_elf_dump_target_info(abfd, f);
  return _bfd_elf_print_private_bfd_data (abfd, farg);
}

static bfd_boolean
lx_elf_set_mach_from_flags (
			    bfd *abfd)
{
  flagword flags = elf_elfheader (abfd)->e_flags;

  switch (flags & ELF_LX_CORE_MASK)
    {
    case ELF_LX_CORE_ST210:
      bfd_default_set_arch_mach (abfd, bfd_arch_lx, bfd_mach_st210);
      break;
    case ELF_LX_CORE_ST220:
      bfd_default_set_arch_mach (abfd, bfd_arch_lx, bfd_mach_st220);
      break;
    case ELF_LX_CORE_ST230:
      bfd_default_set_arch_mach (abfd, bfd_arch_lx, bfd_mach_st230);
      break;
    case ELF_LX_CORE_ST231:
      bfd_default_set_arch_mach (abfd, bfd_arch_lx, bfd_mach_st231);
      break;
    case ELF_LX_CORE_ST240:
      bfd_default_set_arch_mach (abfd, bfd_arch_lx, bfd_mach_st240);
      break;
    default:
      return FALSE;
    }
  return TRUE;
}

/* We set the machine architecture from flags
   here.  */

static bfd_boolean
lx_elf_object_p (bfd *abfd)
{
  return lx_elf_set_mach_from_flags (abfd);
}


static bfd_boolean
lx_mach_has_interlocks (abfd)
     bfd *abfd;
{
  unsigned long mach = bfd_get_mach (abfd);
  return mach != bfd_mach_st210 && mach != bfd_mach_st220;
}

static bfd_boolean
lx_mach_has_st240_encodings (bfd *abfd)
{
  unsigned long mach = bfd_get_mach (abfd);
  return mach >= bfd_mach_st240;
}

static bfd_boolean
lx_elf_set_flags_from_mach (
			    bfd *abfd)
{
  flagword flags = elf_elfheader (abfd)->e_flags;
  unsigned long mach = bfd_get_mach(abfd);

  switch (mach)
    {
    case bfd_mach_st210:
      flags = ((flags | ELF_LX_CORE_MASK) & ELF_LX_CORE_ST210);
      lx_elf_set_private_flags (abfd, flags);
      break;
    case bfd_mach_st220:
      flags = ((flags | ELF_LX_CORE_MASK) & ELF_LX_CORE_ST220);
      lx_elf_set_private_flags (abfd, flags);
      break;
    case bfd_mach_st230:
      flags = ((flags | ELF_LX_CORE_MASK) & ELF_LX_CORE_ST230);
      lx_elf_set_private_flags (abfd, flags);
      break;
    case bfd_mach_st231:
      flags = ((flags | ELF_LX_CORE_MASK) & ELF_LX_CORE_ST231);
      lx_elf_set_private_flags (abfd, flags);
      break;
    case bfd_mach_st240:
      flags = ((flags | ELF_LX_CORE_MASK) & ELF_LX_CORE_ST240);
      lx_elf_set_private_flags (abfd, flags);
      break;
    default:
      return FALSE;
    }
  return TRUE;
}

/*
 * Function to set private flags in
 * BFD file
 */
static bfd_boolean
lx_elf_set_private_flags (
			  bfd *abfd,
			  flagword flags)
{
/*  BFD_ASSERT (!elf_flags_init (abfd)
	      || elf_elfheader (abfd)->e_flags == flags); */

  elf_elfheader (abfd)->e_flags = flags;
  elf_flags_init (abfd) = TRUE;
  return lx_elf_set_mach_from_flags (abfd);
}

flagword 
lx_elf_get_private_flags (bfd* abfd)
{
  flagword flags;
  flags=elf_elfheader (abfd)->e_flags;
  return flags;
}

bfd_boolean
lx_elf_copy_private_flags (bfd* ibfd, bfd* obfd)
{
    if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour) {
	/* Hope that the user passed -B <arch> */
	return lx_elf_set_flags_from_mach(obfd);
    }
	
    return lx_elf_set_private_flags(obfd, lx_elf_get_private_flags(ibfd));
}

const char *
core_printable_name(flagword flags)
{
      switch ( flags&ELF_LX_CORE_MASK )
        {
        case ELF_LX_CORE_ST210: return "st210";
        case ELF_LX_CORE_ST220: return "st220";
        case ELF_LX_CORE_ST230: return "st230";
        case ELF_LX_CORE_ST231: return "st231";
        case ELF_LX_CORE_ST240: return "st240";
        default: return "undefined core";
        }
}

const char *
cut_printable_name(flagword flags)
{
      switch ( flags&ELF_LX_CUT_MASK )
        {
        case ELF_LX_CUT_0: return "cut0";
        case ELF_LX_CUT_1: return "cut1";
        case ELF_LX_CUT_2: return "cut2";
        case ELF_LX_CUT_3: return "cut3";
        case ELF_LX_CUT_4: return "cut4";
        case ELF_LX_CUT_5: return "cut5";
        default:  return "undefined cut";
        }
}

const char *
abi_printable_name(flagword flags, Elf_Internal_Ehdr * i_ehdrp)
{
  int temp;
/* this is a bit tricky because on solaris, the previous implementation
   of the assembler was storing in the high end bits instead of the low
   end bits as was done on Win32 */

  temp = (flags&ELF_LX_ABI_MASK)|(flags >> _ELF_LX_RTA_BIT);

/* insure compatibility between .rta and assume directive */
  temp = temp | ((i_ehdrp->e_ident[EI_ABIVERSION])&ELF_LX_ABI_MASK);

      switch ( temp )
        {
        case ELF_LX_ABI_NO:  return "no-abi";
        case ELF_LX_ABI_MULTI: return "old-multiflow-abi";
        case ELF_LX_ABI_EMBED: return "lx-embedded-abi";
        case ELF_LX_ABI_PIC: return "pic-abi";
        case ELF_LX_ABI_GCC: return "gcc-abi";
	case ELF_LX_ABI_RELOC_EMBED: return "relocatable-embedded-abi";
        default: return "undefined abi";
        }
}

const char *
osabi_printable_name(Elf_Internal_Ehdr * i_ehdrp)
{
      switch ( i_ehdrp->e_ident[EI_OSABI] )
        {
        case ELFOSABI_NONE: return "bare-machine";
        case ELFOSABI_LINUX: return "linux";
        case ELFOSABI_OS21: return "os21";
        default: return "undefined osabi";
        }
}

const char *
code_generation_mode_printable_name(Elf_Internal_Ehdr * i_ehdrp)
{
      switch ( (i_ehdrp->e_ident[EI_ABIVERSION])&ELF_LX_MODE_MASK )
        {
        case ELF_LX_MODE_USER: return " user";
        case ELF_LX_MODE_KERNEL: return "kernel";
        default: return "undefined code generation mode";
        }
}

void
lx_elf_dump_target_info(bfd *abfd, FILE *writer)
{
  Elf_Internal_Ehdr * i_ehdrp = elf_elfheader (abfd);
 
  /* (pp) dump addtional flags for elf files only */
  if ( strstr(abfd->xvec->name,"elf")!=NULL )  /* the test is a bit dirty... */
    {                                          /* is it worth to clean it...? */
      flagword flags = lx_elf_get_private_flags(abfd);
      fprintf(writer,"eflags : %x -",flags);

      /* (pp) consider core info */
      fprintf(writer," %s", core_printable_name(flags));

      /* (pp) consider cut info */
      fprintf(writer," %s", cut_printable_name(flags));

      /* (pp) consider abi info */
      fprintf(writer," %s", abi_printable_name(flags, i_ehdrp));

      /* (pp) consider osabi info */
      fprintf(writer," %s", osabi_printable_name(i_ehdrp));

      /* (pp) consider code generation mode info */
      fprintf(writer," %s", code_generation_mode_printable_name(i_ehdrp));
    }
}

static enum elf_reloc_type_class
elf_lx_reloc_type_class (
			 const Elf_Internal_Rela *rela)
{
  switch ((int) ELF32_R_TYPE (rela->r_info))
    {
    case R_LX_REL32:
      return reloc_class_relative;
    case R_LX_IPLT:
    case R_LX_JMP_SLOT:
      return reloc_class_plt;
    case R_LX_COPY:
      return reloc_class_copy;
    default:
      return reloc_class_normal;
    }
}

/* Merge non visibility st_other attributes. Ensure that the
   STO_MOVEABLE and STO_USED flags for Binopt are copied into h->other, 
   even if this is not a definiton of the symbol. */
static void
elf_lx_merge_symbol_attribute (struct elf_link_hash_entry *h,
			       const Elf_Internal_Sym *isym,
			       bfd_boolean definition,
			       bfd_boolean dynamic ATTRIBUTE_UNUSED)
{
  if ((isym->st_other & ~ELF_ST_VISIBILITY (-1)) != 0)
    {
      unsigned char other;

      other = (definition ? isym->st_other : h->other);
      other &= ~ELF_ST_VISIBILITY (-1);
      h->other = other | ELF_ST_VISIBILITY (h->other);

      if (!definition
	  && is_STO_MOVEABLE (isym->st_other))
	  h->other |= STO_MOVEABLE;

      if (!definition
	  && is_STO_USED (isym->st_other))
	  h->other |= STO_USED;
    }
}

/*specific print_symbol  */
static const char *
lx_elf_print_symbol_all (
			 bfd *abfd ATTRIBUTE_UNUSED,
			 PTR filep,
			 asymbol *symbol)
{
  if (symbol->name == NULL || symbol->name [0] == '\0')
    /* Will be treated as if lx_elf_print_symbol_all were not defined */
    return NULL;
  else {
    char *name;
    size_t len;
    unsigned char st_other;
    bfd_print_symbol_vandf (abfd, (PTR) filep, symbol);
    st_other = ((elf_symbol_type *) symbol)->internal_elf_sym.st_other;
    len = strlen (symbol->name) + strlen(" (moveable)") + strlen (" (used)") + 1;
    name = bfd_alloc (abfd, (bfd_size_type) len);
    memcpy (name, symbol->name, strlen (symbol->name)+1);
    if (is_STO_MOVEABLE(st_other)) {
      sprintf (name, "%s (moveable)", name);
    }
    if (is_STO_USED(st_other)) {
      sprintf (name, "%s (used)", name);
    }
    return name;
  }
}

/* What to do when ld finds relocations against symbols defined in
   discarded sections.  */

static unsigned int
elf_lx_action_discarded (asection *sec)
{
  if (strcmp (".profile_info", sec->name) == 0)
    return 0;

#if 1 /* DFE */
  /* Open64 gnu.linkonce section generation requires to not complain
   * when a discarded section is referenced by a reloc.
   * This is a copy of _bfd_elf_default_action_discarded() that returns
   * PRETEND instead of COMPLAIN to avoid the warning.
   */
  if (sec->flags & SEC_DEBUGGING)
    return PRETEND;

  if (strcmp (".eh_frame", sec->name) == 0)
    return 0;

  if (strcmp (".gcc_except_table", sec->name) == 0)
    return 0;

  return PRETEND;
#else
  /* The default behavior for recent GCC version using COMDAT instead of
   * gnu.linkonce. Not yet supported by Open64.
   */
  return _bfd_elf_default_action_discarded (sec);
#endif
}

#ifndef ELF_ARCH
#define TARGET_BIG_SYM		        bfd_elf32_lx_vec
#define TARGET_BIG_NAME		        "elf32-lx"
#define TARGET_LITTLE_SYM               bfd_elf32_littlelx_vec
#define TARGET_LITTLE_NAME              "elf32-littlelx"
#define ELF_ARCH			bfd_arch_lx
#define ELF_TARGET_ID                   LX_ELF_DATA
#define ELF_MACHINE_CODE		EM_LX
#define ELF_MACHINE_ALT1		EM_LX_OLD
#define ELF_MAXPAGESIZE         	64
#endif /* ELF_ARCH */

#define elf_backend_object_p \
        lx_elf_object_p
#define bfd_elf32_bfd_is_local_label_name \
        lx_elf_is_local_label_name
#define elf_info_to_howto \
	lx_info_to_howto
#define elf_info_to_howto_rel \
        lx_info_to_howto_rel	  
#define elf_backend_relocate_section \
        elf_lx_relocate_section
#define elf_backend_output_arch_local_syms \
        elf_lx_output_arch_local_syms
#define bfd_elf32_bfd_set_private_flags	\
        lx_elf_set_private_flags
#define bfd_elf32_bfd_get_private_flags \
        lx_elf_get_private_flags
#define bfd_elf32_bfd_merge_private_bfd_data \
        lx_elf_merge_private_bfd_data
#define bfd_elf32_bfd_print_private_bfd_data \
        lx_elf_print_private_bfd_data
#define bfd_elf32_bfd_link_hash_table_create \
        elf_lx_hash_table_create
#define bfd_elf32_bfd_link_hash_table_free \
        elf_lx_hash_table_free
#define bfd_elf32_bfd_relax_section \
        elf_lx_relax_section
#define elf_backend_create_dynamic_sections \
        elf_lx_create_dynamic_sections
#define elf_backend_check_relocs \
        elf_lx_check_relocs
#define elf_backend_gc_sweep_hook \
        elf_lx_gc_sweep_hook
#define elf_backend_adjust_dynamic_symbol \
        elf_lx_adjust_dynamic_symbol
#define elf_backend_size_dynamic_sections \
        elf_lx_size_dynamic_sections
#define elf_backend_relocate_section \
        elf_lx_relocate_section
#define elf_backend_finish_dynamic_symbol \
        elf_lx_finish_dynamic_symbol
#define elf_backend_finish_dynamic_sections \
        elf_lx_finish_dynamic_sections
#define elf_backend_merge_symbol_attribute \
        elf_lx_merge_symbol_attribute
#define elf_backend_action_discarded \
        elf_lx_action_discarded

#define elf_backend_plt_readonly	1
#define elf_backend_want_plt_sym	0
#define elf_backend_plt_alignment	5
#define elf_backend_got_header_size	12
#define elf_backend_plt_header_size	PLT_HEADER_SIZE
#define elf_backend_want_got_plt	1
#define elf_backend_may_use_rel_p	1
#define elf_backend_may_use_rela_p	1
#define elf_backend_default_use_rela_p	1
#define elf_backend_want_dynbss		1
#define elf_backend_copy_indirect_symbol elf_lx_hash_copy_indirect
#define elf_backend_hide_symbol		elf_lx_hash_hide_symbol
#define elf_backend_reloc_type_class	elf_lx_reloc_type_class
#define elf_backend_rela_normal		1
#define elf_backend_can_gc_sections	1

#define bfd_elf32_bfd_copy_private_bfd_data    lx_elf_copy_private_flags

#define elf_backend_print_symbol_all \
  lx_elf_print_symbol_all

#include "elf32-target.h"



