/* K1-specific support for NN-bit ELF.
   Copyright (C) 2009-2016 Free Software Foundation, Inc.
   Contributed by ARM Ltd.

   Copyright (C) 2018 Kalray
   
   This file is part of BFD, the Binary File Descriptor library.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING3. If not,
   see <http://www.gnu.org/licenses/>.  */

#include "sysdep.h"
#include "bfd.h"
#include "libiberty.h"
#include "libbfd.h"
#include "bfd_stdint.h"
#include "elf-bfd.h"
#include "bfdlink.h"
#include "objalloc.h"
#include "elf/k1c.h"
#include "elfxx-k1.h"

#define ARCH_SIZE	NN


#define K1_DISABLE_IFUNC (1)

#if ARCH_SIZE == 64
#define K1_R(NAME)		R_K1_ ## NAME
#define K1_R_STR(NAME)	        "R_K1_" #NAME
#define HOWTO64(...)		HOWTO (__VA_ARGS__)
#define HOWTO32(...)		EMPTY_HOWTO (0)
#define LOG_FILE_ALIGN	3
#endif

#if ARCH_SIZE == 32
/* in K1 we have all relocs in the same set */
/* #define K1_R(NAME)           R_K1_P32_ ## NAME */
/* #define K1_R_STR(NAME)       "R_K1_P32_" #NAME */
#define K1_R(NAME)		R_K1_ ## NAME
#define K1_R_STR(NAME)	        "R_K1_" #NAME
#define HOWTO64(...)		EMPTY_HOWTO (0)
#define HOWTO32(...)		HOWTO (__VA_ARGS__)
#define LOG_FILE_ALIGN	2
#endif

#define IS_K1_TLS_RELOC(R_TYPE)			\
  ((R_TYPE) == BFD_RELOC_K1_S37_TPREL_LO10	\
   || (R_TYPE) == BFD_RELOC_K1_S37_TPREL_UP27	\
   || (R_TYPE) == BFD_RELOC_K1_S43_TPREL64_LO10	\
   || (R_TYPE) == BFD_RELOC_K1_S43_TPREL64_UP27	\
   || (R_TYPE) == BFD_RELOC_K1_S43_TPREL64_EX6	\
   || (R_TYPE) == BFD_RELOC_K1_S64_TPREL64_LO10	\
   || (R_TYPE) == BFD_RELOC_K1_S64_TPREL64_UP27	\
   || (R_TYPE) == BFD_RELOC_K1_S64_TPREL64_EX27	\
   )

#define IS_K1_TLS_RELAX_RELOC(R_TYPE)			\
  0

#define IS_K1_TLSDESC_RELOC(R_TYPE)			\
  0

#define ELIMINATE_COPY_RELOCS 0

/* Return size of a relocation entry.  HTAB is the bfd's
   elf_k1_link_hash_entry.  */
#define RELOC_SIZE(HTAB) (sizeof (ElfNN_External_Rela))

/* GOT Entry size - 8 bytes in ELF64 and 4 bytes in ELF32.  */
#define GOT_ENTRY_SIZE                  (ARCH_SIZE / 8)
#define PLT_ENTRY_SIZE                  (32)

#if ARCH_SIZE == 32
#define PLT_SMALL_ENTRY_SIZE            (16)
#else  /* 64 */
#define PLT_SMALL_ENTRY_SIZE            (20)
#endif

#define PLT_TLSDESC_ENTRY_SIZE          (32)

/* Encoding of the nop instruction */
#define INSN_NOP 0x00000000

#define k1_compute_jump_table_size(htab)		\
  (((htab)->root.srelplt == NULL) ? 0			\
   : (htab)->root.srelplt->reloc_count * GOT_ENTRY_SIZE)

static const bfd_byte elfNN_k1_small_plt0_entry[PLT_ENTRY_SIZE] =
{
 /* FIXME K1: no first entry, not used yet */
  0
};

/* Per function entry in a procedure linkage table looks like this
   if the distance between the PLTGOT and the PLT is < 4GB use
   these PLT entries.  */
static const bfd_byte elfNN_k1_small_plt_entry[PLT_SMALL_ENTRY_SIZE] =
{
  0x10, 0x00, 0xc4, 0x0f,       /* get $r16 = $pc     ;; */
#if ARCH_SIZE == 32
  0x10, 0x00, 0x40, 0xb0,       /* lwz $r16 = 0[$r16]   ;; */
#else
  0x10, 0x00, 0x40, 0xb8,       /* ld $r16 = 0[$r16] ;; */
#endif
  0x00, 0x00, 0x00, 0x18,       /* upper 27 bits for LSU */
  0x10, 0x00, 0xd8, 0x0f,	/* igoto $r16          ;; */
};

#define elf_info_to_howto               elfNN_k1_info_to_howto
#define elf_info_to_howto_rel           elfNN_k1_info_to_howto

#define K1_ELF_ABI_VERSION		0

/* In case we're on a 32-bit machine, construct a 64-bit "-1" value.  */
#define ALL_ONES (~ (bfd_vma) 0)

/* Indexed by the bfd interal reloc enumerators.
   Therefore, the table needs to be synced with BFD_RELOC_K1_*
   in reloc.c.   */

#define K1C
#include "elfxx-k1c-relocs.h"
#undef K1C

/* Given HOWTO, return the bfd internal relocation enumerator.  */

static bfd_reloc_code_real_type
elfNN_k1_bfd_reloc_from_howto (reloc_howto_type *howto)
{
  const int size
    = (int) ARRAY_SIZE (elf_k1_howto_table);
  const ptrdiff_t offset
    = howto - elf_k1_howto_table;

  if (offset > 0 && offset < size - 1)
    return BFD_RELOC_K1_RELOC_START + offset;

  /* if (howto == &elf_k1_howto_none) */
  /*   return BFD_RELOC_K1_NONE; */

  return BFD_RELOC_K1_RELOC_START;
}

/* Given R_TYPE, return the bfd internal relocation enumerator.  */

static bfd_reloc_code_real_type
elfNN_k1_bfd_reloc_from_type (unsigned int r_type)
{
  static bfd_boolean initialized_p = FALSE;
  /* Indexed by R_TYPE, values are offsets in the howto_table.  */
  static unsigned int offsets[R_K1_end];

  if (initialized_p == FALSE)
    {
      unsigned int i;

      for (i = 1; i < ARRAY_SIZE (elf_k1_howto_table) - 1; ++i)
	offsets[elf_k1_howto_table[i].type] = i;

      initialized_p = TRUE;
    }

  /* if (r_type == R_K1_NONE) // || r_type == R_K1_NULL) */
  /*   return BFD_RELOC_K1_NONE; */

  /* PR 17512: file: b371e70a.  */
  if (r_type >= R_K1_end)
    {
      _bfd_error_handler (_("Invalid K1 reloc number: %d"), r_type);
      bfd_set_error (bfd_error_bad_value);
      return BFD_RELOC_K1_NONE;
    }

  return BFD_RELOC_K1_RELOC_START + offsets[r_type];
}

struct elf_k1_reloc_map
{
  bfd_reloc_code_real_type from;
  bfd_reloc_code_real_type to;
};

/* Map bfd generic reloc to K1-specific reloc.  */
static const struct elf_k1_reloc_map elf_k1_reloc_map[] =
{
  {BFD_RELOC_NONE, BFD_RELOC_K1_NONE},

  /* Basic data relocations.  */
  {BFD_RELOC_CTOR, BFD_RELOC_K1_NN},
  {BFD_RELOC_64, BFD_RELOC_K1_64},
  {BFD_RELOC_32, BFD_RELOC_K1_32},
  {BFD_RELOC_16, BFD_RELOC_K1_16},

  {BFD_RELOC_64_PCREL, BFD_RELOC_K1_64_PCREL},
  {BFD_RELOC_32_PCREL, BFD_RELOC_K1_32_PCREL},
};

/* Given the bfd internal relocation enumerator in CODE, return the
   corresponding howto entry.  */

static reloc_howto_type *
elfNN_k1_howto_from_bfd_reloc (bfd_reloc_code_real_type code)
{
  unsigned int i;

  /* Convert bfd generic reloc to K1-specific reloc.  */
  if (code < BFD_RELOC_K1_RELOC_START
      || code > BFD_RELOC_K1_RELOC_END)
    for (i = 0; i < ARRAY_SIZE (elf_k1_reloc_map); i++)
      if (elf_k1_reloc_map[i].from == code)
	{
	  code = elf_k1_reloc_map[i].to;
	  break;
	}

  if (code > BFD_RELOC_K1_RELOC_START
      && code < BFD_RELOC_K1_RELOC_END)
    if (elf_k1_howto_table[code - BFD_RELOC_K1_RELOC_START].type)
      return &elf_k1_howto_table[code - BFD_RELOC_K1_RELOC_START];

  /* if (code == BFD_RELOC_K1_NONE) */
  /*   return &elfNN_k1_howto_none; */

  return NULL;
}

static reloc_howto_type *
elfNN_k1_howto_from_type (unsigned int r_type)
{
  bfd_reloc_code_real_type val;
  reloc_howto_type *howto;

#if ARCH_SIZE == 32
  if (r_type > 256)
    {
      bfd_set_error (bfd_error_bad_value);
      return NULL;
    }
#endif

  /* if (r_type == R_K1_NONE) */
  /*   return &elfNN_k1_howto_none; */

  val = elfNN_k1_bfd_reloc_from_type (r_type);
  howto = elfNN_k1_howto_from_bfd_reloc (val);

  if (howto != NULL)
    return howto;

  bfd_set_error (bfd_error_bad_value);
  return NULL;
}

static void
elfNN_k1_info_to_howto (bfd *abfd ATTRIBUTE_UNUSED, arelent *bfd_reloc,
			     Elf_Internal_Rela *elf_reloc)
{
  unsigned int r_type;

  r_type = ELFNN_R_TYPE (elf_reloc->r_info);
  bfd_reloc->howto = elfNN_k1_howto_from_type (r_type);
}

static reloc_howto_type *
elfNN_k1_reloc_type_lookup (bfd *abfd ATTRIBUTE_UNUSED,
				 bfd_reloc_code_real_type code)
{
  reloc_howto_type *howto = elfNN_k1_howto_from_bfd_reloc (code);

  if (howto != NULL)
    return howto;

  bfd_set_error (bfd_error_bad_value);
  return NULL;
}

static reloc_howto_type *
elfNN_k1_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED,
				 const char *r_name)
{
  unsigned int i;

  for (i = 1; i < ARRAY_SIZE (elf_k1_howto_table) - 1; ++i)
    if (elf_k1_howto_table[i].name != NULL
	&& strcasecmp (elf_k1_howto_table[i].name, r_name) == 0)
      return &elf_k1_howto_table[i];

  return NULL;
}

#define TARGET_LITTLE_SYM               k1_elfNN_vec
#define TARGET_LITTLE_NAME              "elfNN-k1"

/* The name of the dynamic interpreter.  This is put in the .interp
   section.  */
#define ELF_DYNAMIC_INTERPRETER     "/lib/ld.so.1"

/* FIXME K1 NEW */
/* This must be changed if we want to use the stub mecanism */
/* It should be disabled ATM */
#define K1_MAX_FWD_BRANCH_OFFSET \
  (((1 << 25) - 1) << 2)
#define K1_MAX_BWD_BRANCH_OFFSET \
  (-((1 << 25) << 2))

static int
k1_valid_branch_p (bfd_vma value, bfd_vma place)
{
  bfd_signed_vma offset = (bfd_signed_vma) (value - place);
  return (offset <= K1_MAX_FWD_BRANCH_OFFSET
	  && offset >= K1_MAX_BWD_BRANCH_OFFSET);
}

/* Section name for stubs is the associated section name plus this
   string.  */
#define STUB_SUFFIX ".stub"

enum elf_k1_stub_type
{
  k1_stub_none,
  /* Original aarch64 has several stub types */
};

struct elf_k1_stub_hash_entry
{
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

  enum elf_k1_stub_type stub_type;

  /* The symbol table entry, if any, that this was derived from.  */
  struct elf_k1_link_hash_entry *h;

  /* Destination symbol type */
  unsigned char st_type;

  /* Where this stub is being called from, or, in the case of combined
     stub sections, the first input section in the group.  */
  asection *id_sec;

  /* The name for the local symbol at the start of this stub.  The
     stub name in the hash table has to be unique; this does not, so
     it can be friendlier.  */
  char *output_name;

  /* The instruction which caused this stub to be generated (only valid for
     erratum 835769 workaround stubs at present).  */
  uint32_t veneered_insn;
};

/* Used to build a map of a section.  This is required for mixed-endian
   code/data.  */

typedef struct elf_elf_section_map
{
  bfd_vma vma;
  char type;
}
elf_k1_section_map;


typedef struct _k1_elf_section_data
{
  struct bfd_elf_section_data elf;
  unsigned int mapcount;
  unsigned int mapsize;
  elf_k1_section_map *map;
}
_k1_elf_section_data;

#define elf_k1_section_data(sec) \
  ((_k1_elf_section_data *) elf_section_data (sec))

/* The size of the thread control block which is defined to be two pointers.  */
#define TCB_SIZE	(ARCH_SIZE/8)*2

struct elf_k1_local_symbol
{
  unsigned int got_type;
  bfd_signed_vma got_refcount;
  bfd_vma got_offset;

  /* Offset of the GOTPLT entry reserved for the TLS descriptor. The
     offset is from the end of the jump table and reserved entries
     within the PLTGOT.

     The magic value (bfd_vma) -1 indicates that an offset has not be
     allocated.  */
  bfd_vma tlsdesc_got_jump_table_offset;
};

struct elf_k1_obj_tdata
{
  struct elf_obj_tdata root;

  /* local symbol descriptors */
  struct elf_k1_local_symbol *locals;

  /* Zero to warn when linking objects with incompatible enum sizes.  */
  int no_enum_size_warning;

  /* Zero to warn when linking objects with incompatible wchar_t sizes.  */
  int no_wchar_size_warning;
};

#define elf_k1_tdata(bfd)				\
  ((struct elf_k1_obj_tdata *) (bfd)->tdata.any)

#define elf_k1_locals(bfd) (elf_k1_tdata (bfd)->locals)

#define is_k1_elf(bfd)				\
  (bfd_get_flavour (bfd) == bfd_target_elf_flavour	\
   && elf_tdata (bfd) != NULL				\
   && elf_object_id (bfd) == K1_ELF_DATA)

static bfd_boolean
elfNN_k1_mkobject (bfd *abfd)
{
  return bfd_elf_allocate_object (abfd, sizeof (struct elf_k1_obj_tdata),
				  K1_ELF_DATA);
}

#define elf_k1_hash_entry(ent) \
  ((struct elf_k1_link_hash_entry *)(ent))

#define GOT_UNKNOWN    0
#define GOT_NORMAL     1

/* Disabled untested and unused TLS */
/* #define GOT_TLS_GD     2 */
/* #define GOT_TLS_IE     4 */
/* #define GOT_TLSDESC_GD 8 */

/* Disabled untested and unused TLS */
//#define GOT_TLS_GD_ANY_P(type)	((type & GOT_TLS_GD) || (type & GOT_TLSDESC_GD))

/* K1 ELF linker hash entry.  */
struct elf_k1_link_hash_entry
{
  struct elf_link_hash_entry root;

  /* Track dynamic relocs copied for this symbol.  */
  struct elf_dyn_relocs *dyn_relocs;

  /* Since PLT entries have variable size, we need to record the
     index into .got.plt instead of recomputing it from the PLT
     offset.  */
  bfd_signed_vma plt_got_offset;

  /* Bit mask representing the type of GOT entry(s) if any required by
     this symbol.  */
  unsigned int got_type;

  /* A pointer to the most recently used stub hash entry against this
     symbol.  */
  struct elf_k1_stub_hash_entry *stub_cache;

  /* Offset of the GOTPLT entry reserved for the TLS descriptor.  The offset
     is from the end of the jump table and reserved entries within the PLTGOT.

     The magic value (bfd_vma) -1 indicates that an offset has not
     be allocated.  */
  bfd_vma tlsdesc_got_jump_table_offset;
};

static unsigned int
elfNN_k1_symbol_got_type (struct elf_link_hash_entry *h,
			       bfd *abfd,
			       unsigned long r_symndx)
{
  if (h)
    return elf_k1_hash_entry (h)->got_type;

  if (! elf_k1_locals (abfd))
    return GOT_UNKNOWN;

  return elf_k1_locals (abfd)[r_symndx].got_type;
}

/* Get the K1 elf linker hash table from a link_info structure.  */
#define elf_k1_hash_table(info)					\
  ((struct elf_k1_link_hash_table *) ((info)->hash))

#define k1_stub_hash_lookup(table, string, create, copy)		\
  ((struct elf_k1_stub_hash_entry *)				\
   bfd_hash_lookup ((table), (string), (create), (copy)))

/* K1 ELF linker hash table.  */
struct elf_k1_link_hash_table
{
  /* The main hash table.  */
  struct elf_link_hash_table root;

  /* Nonzero to force PIC branch veneers.  */
  int pic_veneer;

  /* The number of bytes in the initial entry in the PLT.  */
  bfd_size_type plt_header_size;

  /* The number of bytes in the subsequent PLT etries.  */
  bfd_size_type plt_entry_size;

  /* Short-cuts to get to dynamic linker sections.  */
  asection *sdynbss;
  asection *srelbss;

  /* Small local sym cache.  */
  struct sym_cache sym_cache;

  /* For convenience in allocate_dynrelocs.  */
  bfd *obfd;

  /* The amount of space used by the reserved portion of the sgotplt
     section, plus whatever space is used by the jump slots.  */
  bfd_vma sgotplt_jump_table_size;

  /* The stub hash table.  */
  struct bfd_hash_table stub_hash_table;

  /* Linker stub bfd.  */
  bfd *stub_bfd;

  /* Linker call-backs.  */
  asection *(*add_stub_section) (const char *, asection *);
  void (*layout_sections_again) (void);

  /* Array to keep track of which stub sections have been created, and
     information on stub grouping.  */
  struct map_stub
  {
    /* This is the section to which stubs in the group will be
       attached.  */
    asection *link_sec;
    /* The stub section.  */
    asection *stub_sec;
  } *stub_group;

  /* Assorted information used by elfNN_k1_size_stubs.  */
  unsigned int bfd_count;
  unsigned int top_index;
  asection **input_list;

  /* The offset into splt of the PLT entry for the TLS descriptor
     resolver.  Special values are 0, if not necessary (or not found
     to be necessary yet), and -1 if needed but not determined
     yet.  */
  bfd_vma tlsdesc_plt;

  /* The GOT offset for the lazy trampoline.  Communicated to the
     loader via DT_TLSDESC_GOT.  The magic value (bfd_vma) -1
     indicates an offset is not allocated.  */
  bfd_vma dt_tlsdesc_got;

  /* Used by local STT_GNU_IFUNC symbols.  */
  htab_t loc_hash_table;
  void * loc_hash_memory;
};

/* Create an entry in an K1 ELF linker hash table.  */

static struct bfd_hash_entry *
elfNN_k1_link_hash_newfunc (struct bfd_hash_entry *entry,
				 struct bfd_hash_table *table,
				 const char *string)
{
  struct elf_k1_link_hash_entry *ret =
    (struct elf_k1_link_hash_entry *) entry;

  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (ret == NULL)
    ret = bfd_hash_allocate (table,
			     sizeof (struct elf_k1_link_hash_entry));
  if (ret == NULL)
    return (struct bfd_hash_entry *) ret;

  /* Call the allocation method of the superclass.  */
  ret = ((struct elf_k1_link_hash_entry *)
	 _bfd_elf_link_hash_newfunc ((struct bfd_hash_entry *) ret,
				     table, string));
  if (ret != NULL)
    {
      ret->dyn_relocs = NULL;
      ret->got_type = GOT_UNKNOWN;
      ret->plt_got_offset = (bfd_vma) - 1;
      ret->stub_cache = NULL;
      ret->tlsdesc_got_jump_table_offset = (bfd_vma) - 1;
    }

  return (struct bfd_hash_entry *) ret;
}

/* Initialize an entry in the stub hash table.  */

static struct bfd_hash_entry *
stub_hash_newfunc (struct bfd_hash_entry *entry,
		   struct bfd_hash_table *table, const char *string)
{
  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (entry == NULL)
    {
      entry = bfd_hash_allocate (table,
				 sizeof (struct
					 elf_k1_stub_hash_entry));
      if (entry == NULL)
	return entry;
    }

  /* Call the allocation method of the superclass.  */
  entry = bfd_hash_newfunc (entry, table, string);
  if (entry != NULL)
    {
      struct elf_k1_stub_hash_entry *eh;

      /* Initialize the local fields.  */
      eh = (struct elf_k1_stub_hash_entry *) entry;
      eh->stub_sec = NULL;
      eh->stub_offset = 0;
      eh->target_value = 0;
      eh->target_section = NULL;
      eh->stub_type = k1_stub_none;
      eh->h = NULL;
      eh->id_sec = NULL;
    }

  return entry;
}

/* Compute a hash of a local hash entry.  We use elf_link_hash_entry
  for local symbol so that we can handle local STT_GNU_IFUNC symbols
  as global symbol.  We reuse indx and dynstr_index for local symbol
  hash since they aren't used by global symbols in this backend.  */

static hashval_t
elfNN_k1_local_htab_hash (const void *ptr)
{
  struct elf_link_hash_entry *h
    = (struct elf_link_hash_entry *) ptr;
  return ELF_LOCAL_SYMBOL_HASH (h->indx, h->dynstr_index);
}

/* Compare local hash entries.  */

static int
elfNN_k1_local_htab_eq (const void *ptr1, const void *ptr2)
{
  struct elf_link_hash_entry *h1
     = (struct elf_link_hash_entry *) ptr1;
  struct elf_link_hash_entry *h2
    = (struct elf_link_hash_entry *) ptr2;

  return h1->indx == h2->indx && h1->dynstr_index == h2->dynstr_index;
}

/* Find and/or create a hash entry for local symbol.  */

static struct elf_link_hash_entry *
elfNN_k1_get_local_sym_hash (struct elf_k1_link_hash_table *htab,
				  bfd *abfd, const Elf_Internal_Rela *rel,
				  bfd_boolean create)
{
  struct elf_k1_link_hash_entry e, *ret;
  asection *sec = abfd->sections;
  hashval_t h = ELF_LOCAL_SYMBOL_HASH (sec->id,
				       ELFNN_R_SYM (rel->r_info));
  void **slot;

  e.root.indx = sec->id;
  e.root.dynstr_index = ELFNN_R_SYM (rel->r_info);
  slot = htab_find_slot_with_hash (htab->loc_hash_table, &e, h,
				   create ? INSERT : NO_INSERT);

  if (!slot)
    return NULL;

  if (*slot)
    {
      ret = (struct elf_k1_link_hash_entry *) *slot;
      return &ret->root;
    }

  ret = (struct elf_k1_link_hash_entry *)
	objalloc_alloc ((struct objalloc *) htab->loc_hash_memory,
			sizeof (struct elf_k1_link_hash_entry));
  if (ret)
    {
      memset (ret, 0, sizeof (*ret));
      ret->root.indx = sec->id;
      ret->root.dynstr_index = ELFNN_R_SYM (rel->r_info);
      ret->root.dynindx = -1;
      *slot = ret;
    }
  return &ret->root;
}

/* Copy the extra info we tack onto an elf_link_hash_entry.  */

static void
elfNN_k1_copy_indirect_symbol (struct bfd_link_info *info,
				    struct elf_link_hash_entry *dir,
				    struct elf_link_hash_entry *ind)
{
  struct elf_k1_link_hash_entry *edir, *eind;

  edir = (struct elf_k1_link_hash_entry *) dir;
  eind = (struct elf_k1_link_hash_entry *) ind;

  if (eind->dyn_relocs != NULL)
    {
      if (edir->dyn_relocs != NULL)
	{
	  struct elf_dyn_relocs **pp;
	  struct elf_dyn_relocs *p;

	  /* Add reloc counts against the indirect sym to the direct sym
	     list.  Merge any entries against the same section.  */
	  for (pp = &eind->dyn_relocs; (p = *pp) != NULL;)
	    {
	      struct elf_dyn_relocs *q;

	      for (q = edir->dyn_relocs; q != NULL; q = q->next)
		if (q->sec == p->sec)
		  {
		    q->pc_count += p->pc_count;
		    q->count += p->count;
		    *pp = p->next;
		    break;
		  }
	      if (q == NULL)
		pp = &p->next;
	    }
	  *pp = edir->dyn_relocs;
	}

      edir->dyn_relocs = eind->dyn_relocs;
      eind->dyn_relocs = NULL;
    }

  if (ind->root.type == bfd_link_hash_indirect)
    {
      /* Copy over PLT info.  */
      if (dir->got.refcount <= 0)
	{
	  edir->got_type = eind->got_type;
	  eind->got_type = GOT_UNKNOWN;
	}
    }

  _bfd_elf_link_hash_copy_indirect (info, dir, ind);
}

/* Destroy an K1 elf linker hash table.  */

static void
elfNN_k1_link_hash_table_free (bfd *obfd)
{
  struct elf_k1_link_hash_table *ret
    = (struct elf_k1_link_hash_table *) obfd->link.hash;

  if (ret->loc_hash_table)
    htab_delete (ret->loc_hash_table);
  if (ret->loc_hash_memory)
    objalloc_free ((struct objalloc *) ret->loc_hash_memory);

  bfd_hash_table_free (&ret->stub_hash_table);
  _bfd_elf_link_hash_table_free (obfd);
}

/* Create an K1 elf linker hash table.  */

static struct bfd_link_hash_table *
elfNN_k1_link_hash_table_create (bfd *abfd)
{
  struct elf_k1_link_hash_table *ret;
  bfd_size_type amt = sizeof (struct elf_k1_link_hash_table);

  ret = bfd_zmalloc (amt);
  if (ret == NULL)
    return NULL;

  if (!_bfd_elf_link_hash_table_init
      (&ret->root, abfd, elfNN_k1_link_hash_newfunc,
       sizeof (struct elf_k1_link_hash_entry), K1_ELF_DATA))
    {
      free (ret);
      return NULL;
    }

  ret->plt_header_size = PLT_ENTRY_SIZE;
  ret->plt_entry_size = PLT_SMALL_ENTRY_SIZE;
  ret->obfd = abfd;
  ret->dt_tlsdesc_got = (bfd_vma) - 1;

  if (!bfd_hash_table_init (&ret->stub_hash_table, stub_hash_newfunc,
			    sizeof (struct elf_k1_stub_hash_entry)))
    {
      _bfd_elf_link_hash_table_free (abfd);
      return NULL;
    }

  ret->loc_hash_table = htab_try_create (1024,
					 elfNN_k1_local_htab_hash,
					 elfNN_k1_local_htab_eq,
					 NULL);
  ret->loc_hash_memory = objalloc_create ();
  if (!ret->loc_hash_table || !ret->loc_hash_memory)
    {
      elfNN_k1_link_hash_table_free (abfd);
      return NULL;
    }
  ret->root.root.hash_table_free = elfNN_k1_link_hash_table_free;

  return &ret->root.root;
}

static bfd_boolean
k1_relocate (unsigned int r_type, bfd *input_bfd, asection *input_section,
		  bfd_vma offset, bfd_vma value)
{
  reloc_howto_type *howto;
  bfd_vma place;

  howto = elfNN_k1_howto_from_type (r_type);
  place = (input_section->output_section->vma + input_section->output_offset
	   + offset);

  r_type = elfNN_k1_bfd_reloc_from_type (r_type);
  value = _bfd_k1_elf_resolve_relocation (r_type, place, value, 0, FALSE);
  return _bfd_k1_elf_put_addend (input_bfd,
				      input_section->contents + offset, r_type,
				      howto, value);
}

/* Determine the type of stub needed, if any, for a call.  */

static enum elf_k1_stub_type
k1_type_of_stub (struct bfd_link_info *info ATTRIBUTE_UNUSED,
		      asection *input_sec ATTRIBUTE_UNUSED,
		      const Elf_Internal_Rela *rel ATTRIBUTE_UNUSED,
		      asection *sym_sec ATTRIBUTE_UNUSED,
		      unsigned char st_type ATTRIBUTE_UNUSED,
		      struct elf_k1_link_hash_entry *hash ATTRIBUTE_UNUSED,
		      bfd_vma destination ATTRIBUTE_UNUSED)
{
  enum elf_k1_stub_type stub_type = k1_stub_none;

  /* Disable stub selection for now */
  /* Check elfnn-aarch64.c */
  return stub_type;
}

/* Build a name for an entry in the stub hash table.  */

static char *
elfNN_k1_stub_name (const asection *input_section,
			 const asection *sym_sec,
			 const struct elf_k1_link_hash_entry *hash,
			 const Elf_Internal_Rela *rel)
{
  char *stub_name;
  bfd_size_type len;

  if (hash)
    {
      len = 8 + 1 + strlen (hash->root.root.root.string) + 1 + 16 + 1;
      stub_name = bfd_malloc (len);
      if (stub_name != NULL)
	snprintf (stub_name, len, "%08x_%s+%" BFD_VMA_FMT "x",
		  (unsigned int) input_section->id,
		  hash->root.root.root.string,
		  rel->r_addend);
    }
  else
    {
      len = 8 + 1 + 8 + 1 + 8 + 1 + 16 + 1;
      stub_name = bfd_malloc (len);
      if (stub_name != NULL)
	snprintf (stub_name, len, "%08x_%x:%x+%" BFD_VMA_FMT "x",
		  (unsigned int) input_section->id,
		  (unsigned int) sym_sec->id,
		  (unsigned int) ELFNN_R_SYM (rel->r_info),
		  rel->r_addend);
    }

  return stub_name;
}

/* Look up an entry in the stub hash.  Stub entries are cached because
   creating the stub name takes a bit of time.  */

static struct elf_k1_stub_hash_entry *
elfNN_k1_get_stub_entry (const asection *input_section,
			      const asection *sym_sec,
			      struct elf_link_hash_entry *hash,
			      const Elf_Internal_Rela *rel,
			      struct elf_k1_link_hash_table *htab)
{
  struct elf_k1_stub_hash_entry *stub_entry;
  struct elf_k1_link_hash_entry *h =
    (struct elf_k1_link_hash_entry *) hash;
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
      && h->stub_cache->h == h && h->stub_cache->id_sec == id_sec)
    {
      stub_entry = h->stub_cache;
    }
  else
    {
      char *stub_name;

      stub_name = elfNN_k1_stub_name (id_sec, sym_sec, h, rel);
      if (stub_name == NULL)
	return NULL;

      stub_entry = k1_stub_hash_lookup (&htab->stub_hash_table,
					     stub_name, FALSE, FALSE);
      if (h != NULL)
	h->stub_cache = stub_entry;

      free (stub_name);
    }

  return stub_entry;
}


/* Create a stub section.  */

static asection *
_bfd_k1_create_stub_section (asection *section,
				  struct elf_k1_link_hash_table *htab)
{
  size_t namelen;
  bfd_size_type len;
  char *s_name;

  namelen = strlen (section->name);
  len = namelen + sizeof (STUB_SUFFIX);
  s_name = bfd_alloc (htab->stub_bfd, len);
  if (s_name == NULL)
    return NULL;

  memcpy (s_name, section->name, namelen);
  memcpy (s_name + namelen, STUB_SUFFIX, sizeof (STUB_SUFFIX));
  return (*htab->add_stub_section) (s_name, section);
}


/* Find or create a stub section for a link section.

   Fix or create the stub section used to collect stubs attached to
   the specified link section.  */

static asection *
_bfd_k1_get_stub_for_link_section (asection *link_section,
					struct elf_k1_link_hash_table *htab)
{
  if (htab->stub_group[link_section->id].stub_sec == NULL)
    htab->stub_group[link_section->id].stub_sec
      = _bfd_k1_create_stub_section (link_section, htab);
  return htab->stub_group[link_section->id].stub_sec;
}


/* Find or create a stub section in the stub group for an input
   section.  */

static asection *
_bfd_k1_create_or_find_stub_sec (asection *section,
				      struct elf_k1_link_hash_table *htab)
{
  asection *link_sec = htab->stub_group[section->id].link_sec;
  return _bfd_k1_get_stub_for_link_section (link_sec, htab);
}


/* Add a new stub entry in the stub group associated with an input
   section to the stub hash.  Not all fields of the new stub entry are
   initialised.  */

static struct elf_k1_stub_hash_entry *
_bfd_k1_add_stub_entry_in_group (const char *stub_name,
				      asection *section,
				      struct elf_k1_link_hash_table *htab)
{
  asection *link_sec;
  asection *stub_sec;
  struct elf_k1_stub_hash_entry *stub_entry;

  link_sec = htab->stub_group[section->id].link_sec;
  stub_sec = _bfd_k1_create_or_find_stub_sec (section, htab);

  /* Enter this entry into the linker stub hash table.  */
  stub_entry = k1_stub_hash_lookup (&htab->stub_hash_table, stub_name,
					 TRUE, FALSE);
  if (stub_entry == NULL)
    {
      (*_bfd_error_handler) (_("%s: cannot create stub entry %s"),
			     section->owner, stub_name);
      return NULL;
    }

  stub_entry->stub_sec = stub_sec;
  stub_entry->stub_offset = 0;
  stub_entry->id_sec = link_sec;

  return stub_entry;
}

/* Add a new stub entry in the final stub section to the stub hash.
   Not all fields of the new stub entry are initialised.  */

static struct elf_k1_stub_hash_entry *
_bfd_k1_add_stub_entry_after (const char *stub_name,
				   asection *link_section,
				   struct elf_k1_link_hash_table *htab)
{
  asection *stub_sec;
  struct elf_k1_stub_hash_entry *stub_entry;

  stub_sec = _bfd_k1_get_stub_for_link_section (link_section, htab);
  stub_entry = k1_stub_hash_lookup (&htab->stub_hash_table, stub_name,
					 TRUE, FALSE);
  if (stub_entry == NULL)
    {
      (*_bfd_error_handler) (_("cannot create stub entry %s"), stub_name);
      return NULL;
    }

  stub_entry->stub_sec = stub_sec;
  stub_entry->stub_offset = 0;
  stub_entry->id_sec = link_section;

  return stub_entry;
}


static bfd_boolean
k1_build_one_stub (struct bfd_hash_entry *gen_entry ATTRIBUTE_UNUSED,
			void *in_arg ATTRIBUTE_UNUSED)
{
  /* No need for stub in K1 at the moment, this code can stay commented harmlessly */
  /* Check elfnn-aarch64.c */
  return TRUE;
}

/* As above, but don't actually build the stub.  Just bump offset so
   we know stub section sizes.  */

static bfd_boolean
k1_size_one_stub (struct bfd_hash_entry *gen_entry ATTRIBUTE_UNUSED,
		       void *in_arg ATTRIBUTE_UNUSED)
{
  return TRUE;
}

/* Add an entry to the code/data map for section SEC.  */

static void
elfNN_k1_section_map_add (asection *sec, char type, bfd_vma vma)
{
  struct _k1_elf_section_data *sec_data =
    elf_k1_section_data (sec);
  unsigned int newidx;

  if (sec_data->map == NULL)
    {
      sec_data->map = bfd_malloc (sizeof (elf_k1_section_map));
      sec_data->mapcount = 0;
      sec_data->mapsize = 1;
    }

  newidx = sec_data->mapcount++;

  if (sec_data->mapcount > sec_data->mapsize)
    {
      sec_data->mapsize *= 2;
      sec_data->map = bfd_realloc_or_free
	(sec_data->map, sec_data->mapsize * sizeof (elf_k1_section_map));
    }

  if (sec_data->map)
    {
      sec_data->map[newidx].vma = vma;
      sec_data->map[newidx].type = type;
    }
}

static bfd_vma
k1_calculate_got_entry_vma (struct elf_link_hash_entry *h,
				 struct elf_k1_link_hash_table
				 *globals, struct bfd_link_info *info,
				 bfd_vma value, bfd *output_bfd,
				 bfd_boolean *unresolved_reloc_p)
{
  bfd_vma off = (bfd_vma) - 1;
  asection *basegot = globals->root.sgot;
  bfd_boolean dyn = globals->root.dynamic_sections_created;

  if (h != NULL)
    {
      BFD_ASSERT (basegot != NULL);
      off = h->got.offset;
      BFD_ASSERT (off != (bfd_vma) - 1);
      if (!WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn, bfd_link_pic (info), h)
	  || (bfd_link_pic (info)
	      && SYMBOL_REFERENCES_LOCAL (info, h))
	  || (ELF_ST_VISIBILITY (h->other)
	      && h->root.type == bfd_link_hash_undefweak))
	{
	  /* This is actually a static link, or it is a -Bsymbolic link
	     and the symbol is defined locally.  We must initialize this
	     entry in the global offset table.  Since the offset must
	     always be a multiple of 8 (4 in the case of ILP32), we use
	     the least significant bit to record whether we have
	     initialized it already.
	     When doing a dynamic link, we create a .rel(a).got relocation
	     entry to initialize the value.  This is done in the
	     finish_dynamic_symbol routine.  */
	  if ((off & 1) != 0)
	    off &= ~1;
	  else
	    {
	      bfd_put_NN (output_bfd, value, basegot->contents + off);
	      h->got.offset |= 1;
	    }
	}
      else
	*unresolved_reloc_p = FALSE;

      /* K1 notes : This could go away if we have PC-rel sometime.
       * Original AARCH64 code is using PC-rel load/store and thus 
       * directly builds the offset to the GOT entry
       * On K1, we can't do this, and we don't want to compute a PC-rel
       * for each access. We only need the offset within the GOT here
       */
      /* off = off + basegot->output_section->vma + basegot->output_offset; */
    }

  return off;
}

static unsigned int
k1_reloc_got_type (bfd_reloc_code_real_type r_type)
{
  switch (r_type)
    {
      /* Extracted with:
	 awk 'match ($0, /HOWTO.*R_(K1.*_GOT(OFF)?(64)?_.*),/,ary) {print "case BFD_RELOC_" ary[1] ":";}' elfxx-k1c.def
	 */
    case BFD_RELOC_K1_S37_GOTOFF_LO10:
    case BFD_RELOC_K1_S37_GOTOFF_UP27:
    case BFD_RELOC_K1_S37_GOT_LO10:
    case BFD_RELOC_K1_S37_GOT_UP27:
    case BFD_RELOC_K1_S43_GOTOFF64_LO10:
    case BFD_RELOC_K1_S43_GOTOFF64_UP27:
    case BFD_RELOC_K1_S43_GOTOFF64_EX6:
    case BFD_RELOC_K1_S43_GOT64_LO10:
    case BFD_RELOC_K1_S43_GOT64_UP27:
    case BFD_RELOC_K1_S43_GOT64_EX6:
      return GOT_NORMAL;
 
    default:
      break;
    }
  return GOT_UNKNOWN;
}

static bfd_boolean
k1_can_relax_tls (bfd *input_bfd ATTRIBUTE_UNUSED,
		       struct bfd_link_info *info ATTRIBUTE_UNUSED,
		       bfd_reloc_code_real_type r_type ATTRIBUTE_UNUSED,
		       struct elf_link_hash_entry *h ATTRIBUTE_UNUSED,
		       unsigned long r_symndx ATTRIBUTE_UNUSED)
{
  if (! IS_K1_TLS_RELAX_RELOC (r_type))
    return FALSE;

  /* Relaxing hook. Disabled on K1. */
  /* See elfnn-aarch64.c */
  return TRUE;
}

/* Given the relocation code R_TYPE, return the relaxed bfd reloc
   enumerator.  */

static bfd_reloc_code_real_type
k1_tls_transition (bfd *input_bfd,
			struct bfd_link_info *info,
			unsigned int r_type,
			struct elf_link_hash_entry *h,
			unsigned long r_symndx)
{
  bfd_reloc_code_real_type bfd_r_type
    = elfNN_k1_bfd_reloc_from_type (r_type);

  if (! k1_can_relax_tls (input_bfd, info, bfd_r_type, h, r_symndx))
    return bfd_r_type;

  return bfd_r_type;
}

/* Return the base VMA address which should be subtracted from real addresses
   when resolving R_K1_TLS_* relocation.  */

static bfd_vma
dtpoff_base (struct bfd_link_info *info)
{
  /* If tls_sec is NULL, we should have signalled an error already.  */
  BFD_ASSERT (elf_hash_table (info)->tls_sec != NULL);
  return elf_hash_table (info)->tls_sec->vma;
}

/* Return the base VMA address which should be subtracted from real addresses
   when resolving R_K1_TLS_* relocations.  */

static bfd_vma
tpoff_base (struct bfd_link_info *info)
{
  struct elf_link_hash_table *htab = elf_hash_table (info);

  /* If tls_sec is NULL, we should have signalled an error already.  */
  BFD_ASSERT (htab->tls_sec != NULL);

  bfd_vma base = align_power ((bfd_vma) TCB_SIZE,
			      htab->tls_sec->alignment_power);
  return htab->tls_sec->vma - base;
}

static bfd_vma *
symbol_got_offset_ref (bfd *input_bfd, struct elf_link_hash_entry *h,
		       unsigned long r_symndx)
{
  /* Calculate the address of the GOT entry for symbol
     referred to in h.  */
  if (h != NULL)
    return &h->got.offset;
  else
    {
      /* local symbol */
      struct elf_k1_local_symbol *l;

      l = elf_k1_locals (input_bfd);
      return &l[r_symndx].got_offset;
    }
}

static void
symbol_got_offset_mark (bfd *input_bfd, struct elf_link_hash_entry *h,
			unsigned long r_symndx)
{
  bfd_vma *p;
  p = symbol_got_offset_ref (input_bfd, h, r_symndx);
  *p |= 1;
}

static int
symbol_got_offset_mark_p (bfd *input_bfd, struct elf_link_hash_entry *h,
			  unsigned long r_symndx)
{
  bfd_vma value;
  value = * symbol_got_offset_ref (input_bfd, h, r_symndx);
  return value & 1;
}

static bfd_vma
symbol_got_offset (bfd *input_bfd, struct elf_link_hash_entry *h,
		   unsigned long r_symndx)
{
  bfd_vma value;
  value = * symbol_got_offset_ref (input_bfd, h, r_symndx);
  value &= ~1;
  return value;
}

static bfd_vma *
symbol_tlsdesc_got_offset_ref (bfd *input_bfd, struct elf_link_hash_entry *h,
			       unsigned long r_symndx)
{
  /* Calculate the address of the GOT entry for symbol
     referred to in h.  */
  if (h != NULL)
    {
      struct elf_k1_link_hash_entry *eh;
      eh = (struct elf_k1_link_hash_entry *) h;
      return &eh->tlsdesc_got_jump_table_offset;
    }
  else
    {
      /* local symbol */
      struct elf_k1_local_symbol *l;

      l = elf_k1_locals (input_bfd);
      return &l[r_symndx].tlsdesc_got_jump_table_offset;
    }
}

static void
symbol_tlsdesc_got_offset_mark (bfd *input_bfd, struct elf_link_hash_entry *h,
				unsigned long r_symndx)
{
  bfd_vma *p;
  p = symbol_tlsdesc_got_offset_ref (input_bfd, h, r_symndx);
  *p |= 1;
}

static int
symbol_tlsdesc_got_offset_mark_p (bfd *input_bfd,
				  struct elf_link_hash_entry *h,
				  unsigned long r_symndx)
{
  bfd_vma value;
  value = * symbol_tlsdesc_got_offset_ref (input_bfd, h, r_symndx);
  return value & 1;
}

static bfd_vma
symbol_tlsdesc_got_offset (bfd *input_bfd, struct elf_link_hash_entry *h,
			  unsigned long r_symndx)
{
  bfd_vma value;
  value = * symbol_tlsdesc_got_offset_ref (input_bfd, h, r_symndx);
  value &= ~1;
  return value;
}

/* FIXME: Leaving commented until cleaning done */
/* static bfd_boolean */
/* elfNN_k1_write_section (bfd *output_bfd  ATTRIBUTE_UNUSED, */
/* 			     struct bfd_link_info *link_info, */
/* 			     asection *sec, */
/* 			     bfd_byte *contents) */

/* { */
/*   struct elf_k1_link_hash_table *globals = */
/*     elf_k1_hash_table (link_info); */

/*   if (globals == NULL) */
/*     return FALSE; */

/*   return FALSE; */
/* } */

/* Perform a relocation as part of a final link.  */
static bfd_reloc_status_type
elfNN_k1_final_link_relocate (reloc_howto_type *howto,
				   bfd *input_bfd,
				   bfd *output_bfd,
				   asection *input_section,
				   bfd_byte *contents,
				   Elf_Internal_Rela *rel,
				   bfd_vma value,
				   struct bfd_link_info *info,
				   asection *sym_sec,
				   struct elf_link_hash_entry *h,
				   bfd_boolean *unresolved_reloc_p,
				   bfd_boolean save_addend,
				   bfd_vma *saved_addend,
				   Elf_Internal_Sym *sym)
{
  Elf_Internal_Shdr *symtab_hdr;
  unsigned int r_type = howto->type;
  bfd_reloc_code_real_type bfd_r_type
    = elfNN_k1_bfd_reloc_from_howto (howto);
  bfd_reloc_code_real_type new_bfd_r_type;
  unsigned long r_symndx;
  bfd_byte *hit_data = contents + rel->r_offset;
  bfd_vma place, off;
  bfd_signed_vma signed_addend;
  struct elf_k1_link_hash_table *globals;
  bfd_boolean weak_undef_p;
  asection *base_got;

  globals = elf_k1_hash_table (info);

  symtab_hdr = &elf_symtab_hdr (input_bfd);

  BFD_ASSERT (is_k1_elf (input_bfd));

  r_symndx = ELFNN_R_SYM (rel->r_info);

  /* It is possible to have linker relaxations on some TLS access
     models.  Update our information here.  */
  new_bfd_r_type = k1_tls_transition (input_bfd, info, r_type, h, r_symndx);
  if (new_bfd_r_type != bfd_r_type)
    {
      bfd_r_type = new_bfd_r_type;
      howto = elfNN_k1_howto_from_bfd_reloc (bfd_r_type);
      BFD_ASSERT (howto != NULL);
      r_type = howto->type;
    }

  place = input_section->output_section->vma
    + input_section->output_offset + rel->r_offset;

  /* Get addend, accumulating the addend for consecutive relocs
     which refer to the same offset.  */
  signed_addend = saved_addend ? *saved_addend : 0;
  signed_addend += rel->r_addend;

  weak_undef_p = (h ? h->root.type == bfd_link_hash_undefweak
		  : bfd_is_und_section (sym_sec));

  /* Original AARCH64 code has code for IFUNC here */
#if ! K1_DISABLE_IFUNC
#error IFUNC not ready
#endif

  switch (bfd_r_type)
    {
    case BFD_RELOC_K1_NN:
#if ARCH_SIZE == 64
    case BFD_RELOC_K1_32:
#endif
    case BFD_RELOC_K1_S37_LO10:
    case BFD_RELOC_K1_S37_UP27:

    case BFD_RELOC_K1_S43_LO10:
    case BFD_RELOC_K1_S43_UP27:
    case BFD_RELOC_K1_S43_EX6:

    case BFD_RELOC_K1_S64_LO10:
    case BFD_RELOC_K1_S64_UP27:
    case BFD_RELOC_K1_S64_EX27:


      /* When generating a shared object or relocatable executable, these
         relocations are copied into the output file to be resolved at
         run time.  */
      if (((bfd_link_pic (info) == TRUE)
	    || globals->root.is_relocatable_executable)
	  && (input_section->flags & SEC_ALLOC)
	  && (h == NULL
	      || ELF_ST_VISIBILITY (h->other) == STV_DEFAULT
	      || h->root.type != bfd_link_hash_undefweak))
	{
	  Elf_Internal_Rela outrel;
	  bfd_byte *loc;
	  bfd_boolean skip, relocate;
	  asection *sreloc;

	  *unresolved_reloc_p = FALSE;

	  skip = FALSE;
	  relocate = FALSE;

	  outrel.r_addend = signed_addend;
	  outrel.r_offset =
	    _bfd_elf_section_offset (output_bfd, info, input_section,
				     rel->r_offset);
	  if (outrel.r_offset == (bfd_vma) - 1)
	    skip = TRUE;
	  else if (outrel.r_offset == (bfd_vma) - 2)
	    {
	      skip = TRUE;
	      relocate = TRUE;
	    }

	  outrel.r_offset += (input_section->output_section->vma
			      + input_section->output_offset);

	  if (skip)
	    memset (&outrel, 0, sizeof outrel);
	  else if (h != NULL
		   && h->dynindx != -1
		   && (!bfd_link_pic (info) || !info->symbolic || !h->def_regular))
	    outrel.r_info = ELFNN_R_INFO (h->dynindx, r_type);
	  else if (bfd_r_type == BFD_RELOC_K1_32
		   || bfd_r_type == BFD_RELOC_K1_64)
	    {
	      int symbol;

	      /* On SVR4-ish systems, the dynamic loader cannot
		 relocate the text and data segments independently,
		 so the symbol does not matter.  */
	      symbol = 0;
	      outrel.r_info = ELFNN_R_INFO (symbol, K1_R (RELATIVE));
	      outrel.r_addend += value;
	    }
	  else if (/* h != NULL */
		   /* && h->dynindx != -1 */
		   /* &&  */(bfd_link_pic (info) && info->symbolic  /* && h->def_regular */))
	    {

	      /* if ( h != NULL && h->dynindx != -1 && h->def_regular){ */
		
	      /* 	if (h->root.root.string) */
	      /* 	  name = h->root.root.string; */
	      /* 	else */
	      /* 	  name = bfd_elf_sym_name (input_bfd, symtab_hdr, sym, */
	      /* 				   NULL); */
	      /* } */
	      goto ugly_exit;
	    }
	  else
	    {
	      /* We may endup here from bad input code trying to
		 insert relocation on symbols within code.  We do not
		 want that currently, and such code should use GOT +
		 K1_32/64 reloc that translate in K1_RELATIVE
	      */
		const char *name;
		if (h && h->root.root.string)
		  name = h->root.root.string;
		else
		  name = bfd_elf_sym_name (input_bfd, symtab_hdr, sym,
					   NULL);

		(*_bfd_error_handler)
		  (_("%B: invalid relocation %s against `%s' at 0x%lx in section `%A'."),
		   input_bfd, input_section, howto->name, name,
		   rel->r_offset);
		return bfd_reloc_notsupported;
	    }

	  sreloc = elf_section_data (input_section)->sreloc;
	  if (sreloc == NULL || sreloc->contents == NULL)
	    return bfd_reloc_notsupported;

	  loc = sreloc->contents + sreloc->reloc_count++ * RELOC_SIZE (globals);
	  bfd_elfNN_swap_reloca_out (output_bfd, &outrel, loc);

	  if (sreloc->reloc_count * RELOC_SIZE (globals) > sreloc->size)
	    {
	      /* Sanity to check that we have previously allocated
		 sufficient space in the relocation section for the
		 number of relocations we actually want to emit.  */
	      abort ();
	    }

	  /* If this reloc is against an external symbol, we do not want to
	     fiddle with the addend.  Otherwise, we need to include the symbol
	     value so that it becomes an addend for the dynamic reloc.  */
	  if (!relocate)
	    return bfd_reloc_ok;

	  return _bfd_final_link_relocate (howto, input_bfd, input_section,
					   contents, rel->r_offset, value,
					   signed_addend);
	}
      /* else */
      /* 	value += signed_addend; */
    ugly_exit:


      /* FIXME K1 : this was added for K1*/
      return _bfd_final_link_relocate (howto, input_bfd, input_section,
				       contents, rel->r_offset, value,
				       signed_addend);
      break;

    case BFD_RELOC_K1_17_PCREL:
    case BFD_RELOC_K1_27_PCREL:
      {
	/*
	 * BCU insn are always first in a bundle, so there is no need
	 * to correct the address using offset within bundle
	 */

	asection *splt = globals->root.splt;
	bfd_boolean via_plt_p =
	  splt != NULL && h != NULL && h->plt.offset != (bfd_vma) - 1;

	/* A call to an undefined weak symbol is converted to a jump to
	   the next instruction unless a PLT entry will be created.
	   The jump to the next instruction is optimized as a NOP.
	   Do the same for local undefined symbols.  */
	if (weak_undef_p && ! via_plt_p)
	  {
	    bfd_putl32 (INSN_NOP, hit_data);
	    return bfd_reloc_ok;
	  }

	/* If the call goes through a PLT entry, make sure to
	   check distance to the right destination address.  */
	if (via_plt_p)
	  {
	    value = (splt->output_section->vma
		     + splt->output_offset + h->plt.offset);
	    *unresolved_reloc_p = FALSE;
	  }

	/* If the target symbol is global and marked as a function the
	   relocation applies a function call or a tail call.  In this
	   situation we can veneer out of range branches.  The veneers
	   use IP0 and IP1 hence cannot be used arbitrary out of range
	   branches that occur within the body of a function.  */
	if (h && h->type == STT_FUNC)
	  {
	    /* Check if a stub has to be inserted because the destination
	       is too far away.  */
	    if (! k1_valid_branch_p (value, place))
	      {
		/* The target is out of reach, so redirect the branch to
		   the local stub for this function.  */
		struct elf_k1_stub_hash_entry *stub_entry;
		stub_entry = elfNN_k1_get_stub_entry (input_section,
							   sym_sec, h,
							   rel, globals);
		if (stub_entry != NULL)
		  value = (stub_entry->stub_offset
			   + stub_entry->stub_sec->output_offset
			   + stub_entry->stub_sec->output_section->vma);
	      }
	  }
      }

      /* FALLTHROUGH */

      /* PCREL 32 are used in dwarf2 table for exception handling */
    case BFD_RELOC_K1_32_PCREL:
      return _bfd_final_link_relocate (howto, input_bfd, input_section,
				       contents, rel->r_offset, value,
				       signed_addend);
      break;

    case BFD_RELOC_K1_S37_TPREL_LO10:
    case BFD_RELOC_K1_S37_TPREL_UP27:

    case BFD_RELOC_K1_S43_TPREL64_LO10:
    case BFD_RELOC_K1_S43_TPREL64_UP27:
    case BFD_RELOC_K1_S43_TPREL64_EX6:

    case BFD_RELOC_K1_S64_TPREL64_LO10:
    case BFD_RELOC_K1_S64_TPREL64_UP27:
    case BFD_RELOC_K1_S64_TPREL64_EX27:

    case BFD_RELOC_K1_TPREL64_64:
    case BFD_RELOC_K1_TPREL_32:
      {
	if(bfd_link_pic (info))
	  {
	    /* TBD: Manage dynamic relocations. */
	    *unresolved_reloc_p = FALSE;
	  }
	else
	  {
	    asection *tls_sec = elf_hash_table (info)->tls_sec;
	    if (tls_sec == NULL)
	      {
		const char *name;		
		if (h->root.root.string)
		  name = h->root.root.string;
		else
		  name = bfd_elf_sym_name (input_bfd, symtab_hdr, sym,
					   NULL);

		(*_bfd_error_handler)
		  (_("%B: missing TLS section for relocation %s against `%s' at 0x%lx in section `%A'."),
		   input_bfd, input_section, howto->name, name,
		   rel->r_offset);
		return FALSE;
	      }
	    value -=  tls_sec->vma;
	  }
	return _bfd_final_link_relocate (howto, input_bfd, input_section,
					 contents, rel->r_offset, value,
					 signed_addend);
      }
      break;

    /* case BFD_RELOC_K1_TLS_GD_LO10: */
    /* case BFD_RELOC_K1_TLS_GD_HI22: */
      /* FIXME Add 64 TLS_GD relocation handling */

    /* case BFD_RELOC_K1_TLS_IE64_LO10: */
    /* case BFD_RELOC_K1_TLS_IE64_HI27: */
    /* case BFD_RELOC_K1_TLS_IE64_EXTEND6: */

    /* case BFD_RELOC_K1_TLS_IE_LO10: */
    /* case BFD_RELOC_K1_TLS_IE_HI22: */
    /*   if (globals->root.sgot == NULL) */
    /* 	return bfd_reloc_notsupported; */
    /*   if (h != NULL) { */
    /* 	value = (symbol_got_offset (input_bfd, h, r_symndx) */
    /* 		 + globals->root.sgot->output_section->vma */
    /* 		 + globals->root.sgot->output_offset); */

    /* 	_bfd_final_link_relocate (howto, input_bfd, input_section, */
    /* 				  contents, rel->r_offset, value, */
    /* 				  signed_addend); */
    /* 	/\* value = _bfd_k1_elf_resolve_relocation (bfd_r_type, place, value, *\/ */
    /* 	/\* 						   0, weak_undef_p); *\/ */
    /*   } */
    /*   *unresolved_reloc_p = FALSE; */
    /*   break; */

    case BFD_RELOC_K1_S37_GOTADDR_UP27:
    case BFD_RELOC_K1_S37_GOTADDR_LO10:

    case BFD_RELOC_K1_S43_GOTADDR_UP27:
    case BFD_RELOC_K1_S43_GOTADDR_EX6:
    case BFD_RELOC_K1_S43_GOTADDR_LO10:

    case BFD_RELOC_K1_S64_GOTADDR_UP27:
    case BFD_RELOC_K1_S64_GOTADDR_EX27:
    case BFD_RELOC_K1_S64_GOTADDR_LO10:
      {
	if (globals->root.sgot == NULL)
	  BFD_ASSERT (h != NULL);

	int bundle_offset = 0;

#ifdef UGLY_DEBUG
	printf("GOTADDR: rel@ %x", rel->r_offset);
#endif
	/* find address of bundle as PC rel are computed using it
	   instead of addr of the relocation */
	if (rel->r_offset != 0){
	  BFD_ASSERT(rel->r_offset >= 4);

	  /* start with previous word */
	  int bundle_end_found = 0;
	  int data;
	  unsigned int data_idx = 0;
	  bfd_vma current_offset;
	  for(data_idx = 1 ; data_idx <= rel->r_offset/4; data_idx++){
	    current_offset = rel->r_offset - 4*data_idx;
	    data = bfd_get_32(input_bfd, contents + current_offset);

	    bundle_end_found = !(1<<31 & data);

	    if (bundle_end_found)
	      break;
	  }

	  if (bundle_end_found){
	    bundle_offset = (current_offset + 4) - rel->r_offset;
	  } else {
	    bundle_offset = -rel->r_offset;
	  }
	}
#ifdef UGLY_DEBUG
	printf(", bundle offset: %d\n", bundle_offset);
#endif
	value = globals->root.sgot->output_section->vma
	  + globals->root.sgot->output_offset;
#ifdef UGLY_DEBUG
	printf("  value(got) %x (%x +%x)\n", value,
	       globals->root.sgot->output_section->vma,
	       globals->root.sgot->output_offset);
#endif

	return _bfd_final_link_relocate (howto, input_bfd, input_section,
					 contents, rel->r_offset, value,
					 -bundle_offset);
      }
      break;

    case BFD_RELOC_K1_S37_GOTOFF_LO10:
    case BFD_RELOC_K1_S37_GOTOFF_UP27:

    case BFD_RELOC_K1_GOTOFF:
    case BFD_RELOC_K1_GOTOFF64:

    case BFD_RELOC_K1_S43_GOTOFF64_LO10:
    case BFD_RELOC_K1_S43_GOTOFF64_UP27:
    case BFD_RELOC_K1_S43_GOTOFF64_EX6:

      {
	asection *basegot = globals->root.sgot;
	/* BFD_ASSERT(h == NULL); */
	BFD_ASSERT(globals->root.sgot != NULL);
	value -= basegot->output_section->vma + basegot->output_offset;
	return _bfd_final_link_relocate (howto, input_bfd, input_section,
					 contents, rel->r_offset, value,
					 signed_addend);
      }
      break;

    case BFD_RELOC_K1_S37_GOT_LO10:
    case BFD_RELOC_K1_S37_GOT_UP27:

    case BFD_RELOC_K1_GOT:
    case BFD_RELOC_K1_GOT64:

    case BFD_RELOC_K1_S43_GOT64_LO10:
    case BFD_RELOC_K1_S43_GOT64_UP27:
    case BFD_RELOC_K1_S43_GOT64_EX6:

      if (globals->root.sgot == NULL)
	BFD_ASSERT (h != NULL);

      if (h != NULL)
	{
	  value = k1_calculate_got_entry_vma (h, globals, info, value,
						   output_bfd,
						   unresolved_reloc_p);
#ifdef UGLY_DEBUG
	  printf("GOT_LO/HI for %s, value %x\n", h->root.root.string, value);
#endif

	  /* value = _bfd_k1_elf_resolve_relocation (bfd_r_type, place, value, */
	  /* 					       0, weak_undef_p); */
	  return _bfd_final_link_relocate (howto, input_bfd, input_section,
					   contents, rel->r_offset, value,
					   signed_addend);
	}
      else
	{
#ifdef UGLY_DEBUG
	  printf("GOT_LO/HI with h NULL, initial value %x\n", value);
#endif

	bfd_vma addend = 0;
	struct elf_k1_local_symbol *locals
	  = elf_k1_locals (input_bfd);

	if (locals == NULL)
	  {
	    int howto_index = bfd_r_type - BFD_RELOC_K1_RELOC_START;
	    (*_bfd_error_handler)
	      (_("%B: Local symbol descriptor table be NULL when applying "
		 "relocation %s against local symbol"),
	       input_bfd, elf_k1_howto_table[howto_index].name);
	    abort ();
	  }

	off = symbol_got_offset (input_bfd, h, r_symndx);
	base_got = globals->root.sgot;
	bfd_vma got_entry_addr = (base_got->output_section->vma
				  + base_got->output_offset + off);

	if (!symbol_got_offset_mark_p (input_bfd, h, r_symndx))
	  {
	    bfd_put_64 (output_bfd, value, base_got->contents + off);

	    if (bfd_link_pic (info))
	      {
		asection *s;
		Elf_Internal_Rela outrel;

		/* For local symbol, we have done absolute relocation in static
		   linking stageh. While for share library, we need to update
		   the content of GOT entry according to the share objects
		   loading base address. So we need to generate a
		   R_AARCH64_RELATIVE reloc for dynamic linker.  */
		s = globals->root.srelgot;
		if (s == NULL)
		  abort ();

		outrel.r_offset = got_entry_addr;
		outrel.r_info = ELFNN_R_INFO (0, K1_R (RELATIVE));
		outrel.r_addend = value;
		elf_append_rela (output_bfd, s, &outrel);
	      }

	    symbol_got_offset_mark (input_bfd, h, r_symndx);
	  }

	/* Update the relocation value to GOT entry addr as we have transformed
	   the direct data access into indirect data access through GOT.  */
	value = got_entry_addr;

	return _bfd_final_link_relocate (howto, input_bfd, input_section,
					 contents, rel->r_offset, off,
					 addend);
	}
      break;

    default:
      return bfd_reloc_notsupported;
    }

  if (saved_addend)
    *saved_addend = value;

  /* Only apply the final relocation in a sequence.  */
  if (save_addend)
    return bfd_reloc_continue;

  return _bfd_k1_elf_put_addend (input_bfd, hit_data, bfd_r_type,
				      howto, value);
}



/* Relocate a K1 ELF section.  */

static bfd_boolean
elfNN_k1_relocate_section (bfd *output_bfd,
				struct bfd_link_info *info,
				bfd *input_bfd,
				asection *input_section,
				bfd_byte *contents,
				Elf_Internal_Rela *relocs,
				Elf_Internal_Sym *local_syms,
				asection **local_sections)
{
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  Elf_Internal_Rela *rel;
  Elf_Internal_Rela *relend;
  const char *name;
  struct elf_k1_link_hash_table *globals;
  bfd_boolean save_addend = FALSE;
  bfd_vma addend = 0;

  globals = elf_k1_hash_table (info);

  symtab_hdr = &elf_symtab_hdr (input_bfd);
  sym_hashes = elf_sym_hashes (input_bfd);

  rel = relocs;
  relend = relocs + input_section->reloc_count;
  for (; rel < relend; rel++)
    {
      unsigned int r_type;
      bfd_reloc_code_real_type bfd_r_type;
      bfd_reloc_code_real_type relaxed_bfd_r_type;
      reloc_howto_type *howto;
      unsigned long r_symndx;
      Elf_Internal_Sym *sym;
      asection *sec;
      struct elf_link_hash_entry *h;
      bfd_vma relocation;
      bfd_reloc_status_type r;
      arelent bfd_reloc;
      char sym_type;
      bfd_boolean unresolved_reloc = FALSE;
      char *error_message = NULL;

      r_symndx = ELFNN_R_SYM (rel->r_info);
      r_type = ELFNN_R_TYPE (rel->r_info);

      bfd_reloc.howto = elfNN_k1_howto_from_type (r_type);
      howto = bfd_reloc.howto;

      if (howto == NULL)
	{
	  (*_bfd_error_handler)
	    (_("%B: unrecognized relocation (0x%x) in section `%A'"),
	     input_bfd, input_section, r_type);
	  return FALSE;
	}
      bfd_r_type = elfNN_k1_bfd_reloc_from_howto (howto);

      h = NULL;
      sym = NULL;
      sec = NULL;

      if (r_symndx < symtab_hdr->sh_info)
	{
	  sym = local_syms + r_symndx;
	  sym_type = ELFNN_ST_TYPE (sym->st_info);
	  sec = local_sections[r_symndx];

	  /* An object file might have a reference to a local
	     undefined symbol.  This is a draft object file, but we
	     should at least do something about it.  */
	  if (r_type != R_K1_NONE
	      && r_type != R_K1_S37_GOTADDR_LO10
	      && r_type != R_K1_S37_GOTADDR_UP27
	      && r_type != R_K1_S64_GOTADDR_LO10
	      && r_type != R_K1_S64_GOTADDR_UP27
	      && r_type != R_K1_S64_GOTADDR_EX27
	      && r_type != R_K1_S43_GOTADDR_LO10
	      && r_type != R_K1_S43_GOTADDR_UP27
	      && r_type != R_K1_S43_GOTADDR_EX6
	      && bfd_is_und_section (sec)
	      && ELF_ST_BIND (sym->st_info) != STB_WEAK)
	    {
	      if (!info->callbacks->undefined_symbol
		  (info, bfd_elf_string_from_elf_section
		   (input_bfd, symtab_hdr->sh_link, sym->st_name),
		   input_bfd, input_section, rel->r_offset, TRUE))
		return FALSE;
	    }

	  relocation = _bfd_elf_rela_local_sym (output_bfd, sym, &sec, rel);
  /* Original AARCH64 code has code for IFUNC here */
#if ! K1_DISABLE_IFUNC
#error IFUNC not ready
#endif
	}
      else
	{
	  bfd_boolean warned, ignored;

	  RELOC_FOR_GLOBAL_SYMBOL (info, input_bfd, input_section, rel,
				   r_symndx, symtab_hdr, sym_hashes,
				   h, sec, relocation,
				   unresolved_reloc, warned, ignored);

	  sym_type = h->type;
	}

      if (sec != NULL && discarded_section (sec))
	RELOC_AGAINST_DISCARDED_SECTION (info, input_bfd, input_section,
					 rel, 1, relend, howto, 0, contents);

      if (bfd_link_relocatable (info))
	continue;

      if (h != NULL)
	name = h->root.root.string;
      else
	{
	  name = (bfd_elf_string_from_elf_section
		  (input_bfd, symtab_hdr->sh_link, sym->st_name));
	  if (name == NULL || *name == '\0')
	    name = bfd_section_name (input_bfd, sec);
	}

      if (r_symndx != 0
	  && r_type != R_K1_NONE
	  && (h == NULL
	      || h->root.type == bfd_link_hash_defined
	      || h->root.type == bfd_link_hash_defweak)
	  && IS_K1_TLS_RELOC (bfd_r_type) != (sym_type == STT_TLS))
	{
	  (*_bfd_error_handler)
	    ((sym_type == STT_TLS
	      ? _("%B(%A+0x%lx): %s used with TLS symbol %s")
	      : _("%B(%A+0x%lx): %s used with non-TLS symbol %s")),
	     input_bfd,
	     input_section, (long) rel->r_offset, howto->name, name);
	}

      /* Original aarch64 has relaxation handling for TLS here. */
      r = bfd_reloc_continue;

      /* There may be multiple consecutive relocations for the
         same offset.  In that case we are supposed to treat the
         output of each relocation as the addend for the next.  */
      if (rel + 1 < relend
	  && rel->r_offset == rel[1].r_offset
	  && ELFNN_R_TYPE (rel[1].r_info) != R_K1_NONE)

	save_addend = TRUE;
      else
	save_addend = FALSE;

      if (r == bfd_reloc_continue)
	r = elfNN_k1_final_link_relocate (howto, input_bfd, output_bfd,
					       input_section, contents, rel,
					       relocation, info, sec,
					       h, &unresolved_reloc,
					       save_addend, &addend, sym);

      /* Disabling TLS IE/GD untested code */
      /* switch (elfNN_k1_bfd_reloc_from_type (r_type)) */
      /* 	{ */

      /* 	case BFD_RELOC_K1_TLS_IE64_LO10: */
      /* 	case BFD_RELOC_K1_TLS_IE64_HI27: */
      /* 	case BFD_RELOC_K1_TLS_IE64_EXTEND6: */

      /* 	case BFD_RELOC_K1_TLS_IE_LO10: */
      /* 	case BFD_RELOC_K1_TLS_IE_HI22: */
      /* 	  if (! symbol_got_offset_mark_p (input_bfd, h, r_symndx)) */
      /* 	    { */
      /* 	      bfd_boolean need_relocs = FALSE; */
      /* 	      bfd_byte *loc; */
      /* 	      int indx; */
      /* 	      bfd_vma off; */

      /* 	      off = symbol_got_offset (input_bfd, h, r_symndx); */

      /* 	      indx = h && h->dynindx != -1 ? h->dynindx : 0; */

      /* 	      need_relocs = */
      /* 		(bfd_link_pic (info) || indx != 0) && */
      /* 		(h == NULL */
      /* 		 || ELF_ST_VISIBILITY (h->other) == STV_DEFAULT */
      /* 		 || h->root.type != bfd_link_hash_undefweak); */

      /* 	      BFD_ASSERT (globals->root.srelgot != NULL); */

      /* 	      if (need_relocs) */
      /* 		{ */
      /* 		  Elf_Internal_Rela rela; */

      /* 		  if (indx == 0) */
      /* 		    rela.r_addend = relocation - dtpoff_base (info); */
      /* 		  else */
      /* 		    rela.r_addend = 0; */

      /* 		  /\* FIXME need for better 64bits relocs support ! *\/ */
      /* 		  rela.r_info = ELFNN_R_INFO (indx, K1_R (TLS_TPOFF32)); */
      /* 		  rela.r_offset = globals->root.sgot->output_section->vma + */
      /* 		    globals->root.sgot->output_offset + off; */

      /* 		  loc = globals->root.srelgot->contents; */
      /* 		  loc += globals->root.srelgot->reloc_count++ */
      /* 		    * RELOC_SIZE (htab); */

      /* 		  bfd_elfNN_swap_reloca_out (output_bfd, &rela, loc); */

      /* 		  bfd_put_NN (output_bfd, rela.r_addend, */
      /* 			      globals->root.sgot->contents + off); */
      /* 		} */
      /* 	      else */
      /* 		bfd_put_NN (output_bfd, relocation - tpoff_base (info), */
      /* 			    globals->root.sgot->contents + off); */

      /* 	      symbol_got_offset_mark (input_bfd, h, r_symndx); */
      /* 	    } */
      /* 	  break; */


      /* 	case BFD_RELOC_K1_TLS_GD_LO10: */
      /* 	case BFD_RELOC_K1_TLS_GD_HI22: */
      /* 	  if (! symbol_got_offset_mark_p (input_bfd, h, r_symndx)) */
      /* 	    { */
      /* 	      bfd_boolean need_relocs = FALSE; */
      /* 	      bfd_byte *loc; */
      /* 	      int indx; */
      /* 	      bfd_vma off; */

      /* 	      off = symbol_got_offset (input_bfd, h, r_symndx); */
      /* 	      indx = h && h->dynindx != -1 ? h->dynindx : 0; */

      /* 	      need_relocs = */
      /* 		(bfd_link_pic (info) || indx != 0) && */
      /* 		(h == NULL */
      /* 		 || ELF_ST_VISIBILITY (h->other) == STV_DEFAULT */
      /* 		 || h->root.type != bfd_link_hash_undefweak); */

      /* 	      BFD_ASSERT (globals->root.srelgot != NULL); */

      /* 	      if (need_relocs) */
      /* 		{ */
      /* 		  Elf_Internal_Rela rela; */
      /* 		  rela.r_info = ELFNN_R_INFO (indx, K1_R (TLS_DTPMOD32)); */
      /* 		  rela.r_addend = 0; */
      /* 		  rela.r_offset = globals->root.sgot->output_section->vma + */
      /* 		    globals->root.sgot->output_offset + off; */


      /* 		  loc = globals->root.srelgot->contents; */
      /* 		  loc += globals->root.srelgot->reloc_count++ */
      /* 		    * RELOC_SIZE (htab); */
      /* 		  bfd_elfNN_swap_reloca_out (output_bfd, &rela, loc); */

      /* 		  if (indx == 0) */
      /* 		    { */
      /* 		      bfd_put_NN (output_bfd, */
      /* 				  relocation - dtpoff_base (info), */
      /* 				  globals->root.sgot->contents + off */
      /* 				  + GOT_ENTRY_SIZE); */
      /* 		    } */
      /* 		  else */
      /* 		    { */
      /* 		      /\* This TLS symbol is global. We emit a */
      /* 			 relocation to fixup the tls offset at load */
      /* 			 time.  *\/ */
      /* 		      rela.r_info = */
      /* 			ELFNN_R_INFO (indx, K1_R (TLS_DTPOFF32)); */
      /* 		      rela.r_addend = 0; */
      /* 		      rela.r_offset = */
      /* 			(globals->root.sgot->output_section->vma */
      /* 			 + globals->root.sgot->output_offset + off */
      /* 			 + GOT_ENTRY_SIZE); */

      /* 		      loc = globals->root.srelgot->contents; */
      /* 		      loc += globals->root.srelgot->reloc_count++ */
      /* 			* RELOC_SIZE (globals); */
      /* 		      bfd_elfNN_swap_reloca_out (output_bfd, &rela, loc); */
      /* 		      bfd_put_NN (output_bfd, (bfd_vma) 0, */
      /* 				  globals->root.sgot->contents + off */
      /* 				  + GOT_ENTRY_SIZE); */
      /* 		    } */
      /* 		} */
      /* 	      else */
      /* 		{ */
      /* 		  bfd_put_NN (output_bfd, (bfd_vma) 1, */
      /* 			      globals->root.sgot->contents + off); */
      /* 		  bfd_put_NN (output_bfd, */
      /* 			      relocation - dtpoff_base (info), */
      /* 			      globals->root.sgot->contents + off */
      /* 			      + GOT_ENTRY_SIZE); */
      /* 		} */

      /* 	      symbol_got_offset_mark (input_bfd, h, r_symndx); */
      /* 	    } */
      /* 	  break; */

      /* 	default: */
      /* 	  break; */
      /* 	} */

      /* Dynamic relocs are not propagated for SEC_DEBUGGING sections
         because such sections are not SEC_ALLOC and thus ld.so will
         not process them.  */
      if (unresolved_reloc
	  && !((input_section->flags & SEC_DEBUGGING) != 0
	       && h->def_dynamic)
	  && _bfd_elf_section_offset (output_bfd, info, input_section,
				      +rel->r_offset) != (bfd_vma) - 1)
	{
	  (*_bfd_error_handler)
	    (_
	     ("%B(%A+0x%lx): unresolvable %s relocation against symbol `%s'"),
	     input_bfd, input_section, (long) rel->r_offset, howto->name,
	     h->root.root.string);
	  return FALSE;
	}

      if (r != bfd_reloc_ok && r != bfd_reloc_continue)
	{
	  bfd_reloc_code_real_type real_r_type
	    = elfNN_k1_bfd_reloc_from_type (r_type);

	  switch (r)
	    {
	    case bfd_reloc_overflow:
	      if (!(*info->callbacks->reloc_overflow)
		  (info, (h ? &h->root : NULL), name, howto->name, (bfd_vma) 0,
		   input_bfd, input_section, rel->r_offset))
		return FALSE;
	      /* Original aarch64 code had a check for alignement correctness */
	      break;

	    case bfd_reloc_undefined:
	      if (!((*info->callbacks->undefined_symbol)
		    (info, name, input_bfd, input_section,
		     rel->r_offset, TRUE)))
		return FALSE;
	      break;

	    case bfd_reloc_outofrange:
	      error_message = _("out of range");
	      goto common_error;

	    case bfd_reloc_notsupported:
	      error_message = _("unsupported relocation");
	      goto common_error;

	    case bfd_reloc_dangerous:
	      /* error_message should already be set.  */
	      goto common_error;

	    default:
	      error_message = _("unknown error");
	      /* Fall through.  */

	    common_error:
	      BFD_ASSERT (error_message != NULL);
	      if (!((*info->callbacks->reloc_dangerous)
		    (info, error_message, input_bfd, input_section,
		     rel->r_offset)))
		return FALSE;
	      break;
	    }
	}

      if (!save_addend)
	addend = 0;
    }

  return TRUE;
}

/* /\* The final processing done just before writing out an K1 ELF object */
/*    file.  This gets the K1 architecture right based on the machine */
/*    number.  *\/ */

/* void */
/* elfNN_k1_final_write_processing (bfd *abfd, */
/* 				   bfd_boolean linker ATTRIBUTE_UNUSED) */
/* { */
/*   int mach; */
/*   unsigned long val; */

/*   switch (mach = bfd_get_mach (abfd)) */
/*     { */
/*     case bfd_mach_k1dp: */
/*     case bfd_mach_k1io: */
/*     case bfd_mach_k1bdp: */
/*     case bfd_mach_k1bio: */
/*     case bfd_mach_k1bdp_64: */
/*     case bfd_mach_k1bio_64: */
/*       val = mach; */
/*       break; */
/*     default: */
/*       return; */
/*     } */

/*   elf_elfheader (abfd)->e_flags &=  (~ K1_MACH_MASK); */
/*   elf_elfheader (abfd)->e_flags |= val; */
/* } */


/* Set the right machine number.  */

static bfd_boolean
elfNN_k1_object_p (bfd *abfd)
{
  /* must be coherent with default arch in cpu-k1.c */
  int e_set = bfd_mach_k1c;

  if (elf_elfheader (abfd)->e_machine == EM_K1)
    {
#if ARCH_SIZE == 64
      e_set = bfd_mach_k1c_64;
#else
      e_set = bfd_mach_k1c;
#endif
    }
  return bfd_default_set_arch_mach (abfd, bfd_arch_k1, e_set);
    
}

/* Function to keep K1 specific flags in the ELF header.  */

static bfd_boolean
elfNN_k1_set_private_flags (bfd *abfd, flagword flags)
{
  if (elf_flags_init (abfd) && elf_elfheader (abfd)->e_flags != flags)
    {
    }
  else
    {
      elf_elfheader (abfd)->e_flags = flags;
      elf_flags_init (abfd) = TRUE;
    }

  return TRUE;
}

/* Merge backend specific data from an object file to the output
   object file when linking.  */

static bfd_boolean
elfNN_k1_merge_private_bfd_data (bfd *ibfd, bfd *obfd)
{
  flagword out_flags;
  flagword in_flags;
  bfd_boolean flags_compatible = TRUE;
  asection *sec;

  /* Check if we have the same endianess.  */
  if (!_bfd_generic_verify_endian_match (ibfd, obfd))
    return FALSE;

  if (!is_k1_elf (ibfd) || !is_k1_elf (obfd))
    return TRUE;

  /* The input BFD must have had its flags initialised.  */
  /* The following seems bogus to me -- The flags are initialized in
     the assembler but I don't think an elf_flags_init field is
     written into the object.  */
  /* BFD_ASSERT (elf_flags_init (ibfd)); */

  in_flags = elf_elfheader (ibfd)->e_flags;
  out_flags = elf_elfheader (obfd)->e_flags;

  if (!elf_flags_init (obfd))
    {
      /* If the input is the default architecture and had the default
         flags then do not bother setting the flags for the output
         architecture, instead allow future merges to do this.  If no
         future merges ever set these flags then they will retain their
         uninitialised values, which surprise surprise, correspond
         to the default values.  */
      if (bfd_get_arch_info (ibfd)->the_default
	  && elf_elfheader (ibfd)->e_flags == 0)
	return TRUE;

      elf_flags_init (obfd) = TRUE;
      elf_elfheader (obfd)->e_flags = in_flags;

      if (bfd_get_arch (obfd) == bfd_get_arch (ibfd)
	  && bfd_get_arch_info (obfd)->the_default)
	return bfd_set_arch_mach (obfd, bfd_get_arch (ibfd),
				  bfd_get_mach (ibfd));

      return TRUE;
    }

  /* Identical flags must be compatible.  */
  if (in_flags == out_flags)
    return TRUE;

  /* Check to see if the input BFD actually contains any sections.  If
     not, its flags may not have been initialised either, but it
     cannot actually cause any incompatiblity.  Do not short-circuit
     dynamic objects; their section list may be emptied by
     elf_link_add_object_symbols.

     Also check to see if there are no code sections in the input.
     In this case there is no need to check for code specific flags.
     XXX - do we need to worry about floating-point format compatability
     in data sections ?  */
  if (!(ibfd->flags & DYNAMIC))
    {
      bfd_boolean null_input_bfd = TRUE;
      bfd_boolean only_data_sections = TRUE;

      for (sec = ibfd->sections; sec != NULL; sec = sec->next)
	{
	  if ((bfd_get_section_flags (ibfd, sec)
	       & (SEC_LOAD | SEC_CODE | SEC_HAS_CONTENTS))
	      == (SEC_LOAD | SEC_CODE | SEC_HAS_CONTENTS))
	    only_data_sections = FALSE;

	  null_input_bfd = FALSE;
	  break;
	}

      if (null_input_bfd || only_data_sections)
	return TRUE;
    }

  return flags_compatible;
}

/* Display the flags field.  */

static bfd_boolean
elfNN_k1_print_private_bfd_data (bfd *abfd, void *ptr)
{
  FILE *file = (FILE *) ptr;
  unsigned long flags;

  BFD_ASSERT (abfd != NULL && ptr != NULL);

  /* Print normal ELF private data.  */
  _bfd_elf_print_private_bfd_data (abfd, ptr);

  flags = elf_elfheader (abfd)->e_flags;
  /* Ignore init flag - it may not be set, despite the flags field
     containing valid data.  */

  /* xgettext:c-format */
  fprintf (file, _("private flags = %lx:"), elf_elfheader (abfd)->e_flags);

  if (flags)
    fprintf (file, _("<Unrecognised flag bits set>"));

  fputc ('\n', file);

  return TRUE;
}

/* Update the got entry reference counts for the section being removed.  */

static bfd_boolean
elfNN_k1_gc_sweep_hook (bfd *abfd,
			     struct bfd_link_info *info,
			     asection *sec,
			     const Elf_Internal_Rela * relocs)
{
  struct elf_k1_link_hash_table *htab;
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  struct elf_k1_local_symbol *locals;
  const Elf_Internal_Rela *rel, *relend;

  if (bfd_link_relocatable (info))
    return TRUE;

  htab = elf_k1_hash_table (info);

  if (htab == NULL)
    return FALSE;

  elf_section_data (sec)->local_dynrel = NULL;

  symtab_hdr = &elf_symtab_hdr (abfd);
  sym_hashes = elf_sym_hashes (abfd);

  locals = elf_k1_locals (abfd);

  relend = relocs + sec->reloc_count;
  for (rel = relocs; rel < relend; rel++)
    {
      unsigned long r_symndx;
      unsigned int r_type;
      struct elf_link_hash_entry *h = NULL;

      r_symndx = ELFNN_R_SYM (rel->r_info);

      if (r_symndx >= symtab_hdr->sh_info)
	{

	  h = sym_hashes[r_symndx - symtab_hdr->sh_info];
	  while (h->root.type == bfd_link_hash_indirect
		 || h->root.type == bfd_link_hash_warning)
	    h = (struct elf_link_hash_entry *) h->root.u.i.link;
        }
      else
	{
	  Elf_Internal_Sym *isym;

	  /* A local symbol.  */
	  isym = bfd_sym_from_r_symndx (&htab->sym_cache,
					abfd, r_symndx);

  /* Original AARCH64 code has code for IFUNC here */
#if ! K1_DISABLE_IFUNC
#error IFUNC not ready
#endif
	}

      if (h)
	{
	  struct elf_k1_link_hash_entry *eh;
	  struct elf_dyn_relocs **pp;
	  struct elf_dyn_relocs *p;

	  eh = (struct elf_k1_link_hash_entry *) h;

	  for (pp = &eh->dyn_relocs; (p = *pp) != NULL; pp = &p->next)
	    if (p->sec == sec)
	      {
		/* Everything must go for SEC.  */
		*pp = p->next;
		break;
	      }
	}

      r_type = ELFNN_R_TYPE (rel->r_info);
      switch (k1_tls_transition (abfd,info, r_type, h ,r_symndx))
	{
	default:
	  break;
	}
    }

  return TRUE;
}

/* Adjust a symbol defined by a dynamic object and referenced by a
   regular object.  The current definition is in some section of the
   dynamic object, but we're not including those sections.  We have to
   change the definition to something the rest of the link can
   understand.	*/

static bfd_boolean
elfNN_k1_adjust_dynamic_symbol (struct bfd_link_info *info,
				     struct elf_link_hash_entry *h)
{
  struct elf_k1_link_hash_table *htab;
  asection *s;

  /* If this is a function, put it in the procedure linkage table.  We
     will fill in the contents of the procedure linkage table later,
     when we know the address of the .got section.  */
  if (h->type == STT_FUNC || h->type == STT_GNU_IFUNC || h->needs_plt)
    {
      if (h->plt.refcount <= 0
	  || (h->type != STT_GNU_IFUNC
	      && (SYMBOL_CALLS_LOCAL (info, h)
		  || (ELF_ST_VISIBILITY (h->other) != STV_DEFAULT
		      && h->root.type == bfd_link_hash_undefweak))))
	{
	  /* This case can occur if we saw a CALL26 reloc in
	     an input file, but the symbol wasn't referred to
	     by a dynamic object or all references were
	     garbage collected. In which case we can end up
	     resolving.  */
	  h->plt.offset = (bfd_vma) - 1;
	  h->needs_plt = 0;
	}

      return TRUE;
    }
  else
    /* Otherwise, reset to -1.  */
    h->plt.offset = (bfd_vma) - 1;


  /* If this is a weak symbol, and there is a real definition, the
     processor independent code will have arranged for us to see the
     real definition first, and we can just use the same value.  */
  if (h->u.weakdef != NULL)
    {
      BFD_ASSERT (h->u.weakdef->root.type == bfd_link_hash_defined
		  || h->u.weakdef->root.type == bfd_link_hash_defweak);
      h->root.u.def.section = h->u.weakdef->root.u.def.section;
      h->root.u.def.value = h->u.weakdef->root.u.def.value;
      if (ELIMINATE_COPY_RELOCS || info->nocopyreloc)
	h->non_got_ref = h->u.weakdef->non_got_ref;
      return TRUE;
    }

  /* If we are creating a shared library, we must presume that the
     only references to the symbol are via the global offset table.
     For such cases we need not do anything here; the relocations will
     be handled correctly by relocate_section.  */
  if (bfd_link_pic (info))
    return TRUE;

  /* If there are no references to this symbol that do not use the
     GOT, we don't need to generate a copy reloc.  */
  if (!h->non_got_ref)
    return TRUE;

  /* If -z nocopyreloc was given, we won't generate them either.  */
  if (info->nocopyreloc)
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

  htab = elf_k1_hash_table (info);

  /* We must generate a R_K1_COPY reloc to tell the dynamic linker
     to copy the initial value out of the dynamic object and into the
     runtime process image.  */
  if ((h->root.u.def.section->flags & SEC_ALLOC) != 0 && h->size != 0)
    {
      htab->srelbss->size += RELOC_SIZE (htab);
      h->needs_copy = 1;
    }

  s = htab->sdynbss;

  return _bfd_elf_adjust_dynamic_copy (info, h, s);

}

static bfd_boolean
elfNN_k1_allocate_local_symbols (bfd *abfd, unsigned number)
{
  struct elf_k1_local_symbol *locals;
  locals = elf_k1_locals (abfd);
  if (locals == NULL)
    {
      locals = (struct elf_k1_local_symbol *)
	bfd_zalloc (abfd, number * sizeof (struct elf_k1_local_symbol));
      if (locals == NULL)
	return FALSE;
      elf_k1_locals (abfd) = locals;
    }
  return TRUE;
}

/* Create the .got section to hold the global offset table.  */

static bfd_boolean
k1_elf_create_got_section (bfd *abfd, struct bfd_link_info *info)
{
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  flagword flags;
  asection *s;
  struct elf_link_hash_entry *h;
  struct elf_link_hash_table *htab = elf_hash_table (info);

  /* This function may be called more than once.  */
  s = bfd_get_linker_section (abfd, ".got");
  if (s != NULL)
    return TRUE;

  flags = bed->dynamic_sec_flags;

  s = bfd_make_section_anyway_with_flags (abfd,
					  (bed->rela_plts_and_copies_p
					   ? ".rela.got" : ".rel.got"),
					  (bed->dynamic_sec_flags
					   | SEC_READONLY));
  if (s == NULL
      || ! bfd_set_section_alignment (abfd, s, bed->s->log_file_align))
    return FALSE;
  htab->srelgot = s;

  s = bfd_make_section_anyway_with_flags (abfd, ".got", flags);
  if (s == NULL
      || !bfd_set_section_alignment (abfd, s, bed->s->log_file_align))
    return FALSE;
  htab->sgot = s;
  htab->sgot->size += GOT_ENTRY_SIZE;

  if (bed->want_got_sym)
    {
      /* Define the symbol _GLOBAL_OFFSET_TABLE_ at the start of the .got
	 (or .got.plt) section.  We don't do this in the linker script
	 because we don't want to define the symbol if we are not creating
	 a global offset table.  */
      h = _bfd_elf_define_linkage_sym (abfd, info, s,
				       "_GLOBAL_OFFSET_TABLE_");
      elf_hash_table (info)->hgot = h;
      if (h == NULL)
	return FALSE;
    }

  if (bed->want_got_plt)
    {
      s = bfd_make_section_anyway_with_flags (abfd, ".got.plt", flags);
      if (s == NULL
	  || !bfd_set_section_alignment (abfd, s,
					 bed->s->log_file_align))
	return FALSE;
      htab->sgotplt = s;
    }

  /* The first bit of the global offset table is the header.  */
  htab->sgot->size += bed->got_header_size;

  /* we still need to handle got content when doing static link with PIC */
  if (bfd_link_executable (info) && !bfd_link_pic (info)) {
    htab->dynobj = abfd;
  }

  return TRUE;
}

/* Look through the relocs for a section during the first phase.  */

static bfd_boolean
elfNN_k1_check_relocs (bfd *abfd, struct bfd_link_info *info,
			    asection *sec, const Elf_Internal_Rela *relocs)
{
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  const Elf_Internal_Rela *rel;
  const Elf_Internal_Rela *rel_end;
  asection *sreloc;

  struct elf_k1_link_hash_table *htab;

  if (bfd_link_relocatable (info))
    return TRUE;

  BFD_ASSERT (is_k1_elf (abfd));

  htab = elf_k1_hash_table (info);
  sreloc = NULL;

  symtab_hdr = &elf_symtab_hdr (abfd);
  sym_hashes = elf_sym_hashes (abfd);

  rel_end = relocs + sec->reloc_count;
  for (rel = relocs; rel < rel_end; rel++)
    {
      struct elf_link_hash_entry *h;
      unsigned long r_symndx;
      unsigned int r_type;
      bfd_reloc_code_real_type bfd_r_type;
      Elf_Internal_Sym *isym;

      r_symndx = ELFNN_R_SYM (rel->r_info);
      r_type = ELFNN_R_TYPE (rel->r_info);

      if (r_symndx >= NUM_SHDR_ENTRIES (symtab_hdr))
	{
	  (*_bfd_error_handler) (_("%B: bad symbol index: %d"), abfd,
				 r_symndx);
	  return FALSE;
	}

      if (r_symndx < symtab_hdr->sh_info)
	{
	  /* A local symbol.  */
	  isym = bfd_sym_from_r_symndx (&htab->sym_cache,
					abfd, r_symndx);
	  if (isym == NULL)
	    return FALSE;

	  /* Original AARCH64 code has code for IFUNC here */
#if ! K1_DISABLE_IFUNC
#error IFUNC not ready
#endif
	    h = NULL;
	}
      else
	{
	  h = sym_hashes[r_symndx - symtab_hdr->sh_info];
	  while (h->root.type == bfd_link_hash_indirect
		 || h->root.type == bfd_link_hash_warning)
	    h = (struct elf_link_hash_entry *) h->root.u.i.link;

	  /* PR15323, ref flags aren't set for references in the same
	     object.  */
	  h->root.non_ir_ref = 1;
	}

      /* Could be done earlier, if h were already available.  */
      bfd_r_type = k1_tls_transition (abfd, info, r_type, h, r_symndx);

      if (h != NULL)
	{
	  /* Create the ifunc sections for static executables.  If we
	     never see an indirect function symbol nor we are building
	     a static executable, those sections will be empty and
	     won't appear in output.  */
	  switch (bfd_r_type)
	    {
	    default:
	      break;
	    }

	  /* It is referenced by a non-shared object. */
	  h->ref_regular = 1;
	  h->root.non_ir_ref = 1;
	}

      switch (bfd_r_type)
	{

	case BFD_RELOC_K1_S43_LO10:
	case BFD_RELOC_K1_S43_UP27:
	case BFD_RELOC_K1_S43_EX6:

	case BFD_RELOC_K1_S37_LO10:
	case BFD_RELOC_K1_S37_UP27:

	case BFD_RELOC_K1_S64_LO10:
	case BFD_RELOC_K1_S64_UP27:
	case BFD_RELOC_K1_S64_EX27:

	case BFD_RELOC_K1_32:
	case BFD_RELOC_K1_64:

	  /* We don't need to handle relocs into sections not going into
	     the "real" output.  */
	  if ((sec->flags & SEC_ALLOC) == 0)
	    break;

	  if (h != NULL)
	    {
	      if (!bfd_link_pic (info))
		h->non_got_ref = 1;

	      h->plt.refcount += 1;
	      h->pointer_equality_needed = 1;
	    }

	  /* No need to do anything if we're not creating a shared
	     object.  */
	  if (! bfd_link_pic (info))
	    break;

	  {
	    struct elf_dyn_relocs *p;
	    struct elf_dyn_relocs **head;

	    /* We must copy these reloc types into the output file.
	       Create a reloc section in dynobj and make room for
	       this reloc.  */
	    if (sreloc == NULL)
	      {
		if (htab->root.dynobj == NULL)
		  htab->root.dynobj = abfd;

		sreloc = _bfd_elf_make_dynamic_reloc_section
		  (sec, htab->root.dynobj, LOG_FILE_ALIGN, abfd, /*rela? */ TRUE);

		if (sreloc == NULL)
		  return FALSE;
	      }

	    /* If this is a global symbol, we count the number of
	       relocations we need for this symbol.  */
	    if (h != NULL)
	      {
		struct elf_k1_link_hash_entry *eh;
		eh = (struct elf_k1_link_hash_entry *) h;
		head = &eh->dyn_relocs;
	      }
	    else
	      {
		/* Track dynamic relocs needed for local syms too.
		   We really need local syms available to do this
		   easily.  Oh well.  */

		asection *s;
		void **vpp;

		isym = bfd_sym_from_r_symndx (&htab->sym_cache,
					      abfd, r_symndx);
		if (isym == NULL)
		  return FALSE;

		s = bfd_section_from_elf_index (abfd, isym->st_shndx);
		if (s == NULL)
		  s = sec;

		/* Beware of type punned pointers vs strict aliasing
		   rules.  */
		vpp = &(elf_section_data (s)->local_dynrel);
		head = (struct elf_dyn_relocs **) vpp;
	      }

	    p = *head;
	    if (p == NULL || p->sec != sec)
	      {
		bfd_size_type amt = sizeof *p;
		p = ((struct elf_dyn_relocs *)
		     bfd_zalloc (htab->root.dynobj, amt));
		if (p == NULL)
		  return FALSE;
		p->next = *head;
		*head = p;
		p->sec = sec;
	      }

	    p->count += 1;

	  }
	  break;

	case BFD_RELOC_K1_S37_GOT_LO10:
	case BFD_RELOC_K1_S37_GOT_UP27:

	case BFD_RELOC_K1_S37_GOTOFF_LO10:
	case BFD_RELOC_K1_S37_GOTOFF_UP27:

	case BFD_RELOC_K1_S43_GOT64_LO10:
	case BFD_RELOC_K1_S43_GOT64_UP27:
	case BFD_RELOC_K1_S43_GOT64_EX6:

	case BFD_RELOC_K1_S43_GOTOFF64_LO10:
	case BFD_RELOC_K1_S43_GOTOFF64_UP27:
	case BFD_RELOC_K1_S43_GOTOFF64_EX6:

	  {
	    unsigned got_type;
	    unsigned old_got_type;

	    got_type = k1_reloc_got_type (bfd_r_type);

	    if (h)
	      {
		h->got.refcount += 1;
		old_got_type = elf_k1_hash_entry (h)->got_type;
	      }
	    else
	      {
		struct elf_k1_local_symbol *locals;

		if (!elfNN_k1_allocate_local_symbols
		    (abfd, symtab_hdr->sh_info))
		  return FALSE;

		locals = elf_k1_locals (abfd);
		BFD_ASSERT (r_symndx < symtab_hdr->sh_info);
		locals[r_symndx].got_refcount += 1;
		old_got_type = locals[r_symndx].got_type;
	      }

	    /* If a variable is accessed with both general dynamic TLS
	       methods, two slots may be created.  */
	    /* Disabled untested and unused TLS */
	    /* if (GOT_TLS_GD_ANY_P (old_got_type) && GOT_TLS_GD_ANY_P (got_type)) */
	    /*   got_type |= old_got_type; */

	    /* We will already have issued an error message if there
	       is a TLS/non-TLS mismatch, based on the symbol type.
	       So just combine any TLS types needed.  */
	    if (old_got_type != GOT_UNKNOWN && old_got_type != GOT_NORMAL
		&& got_type != GOT_NORMAL)
	      got_type |= old_got_type;

	    /* If the symbol is accessed by both IE and GD methods, we
	       are able to relax.  Turn off the GD flag, without
	       messing up with any other kind of TLS types that may be
	       involved.  */
	    /* Disabled untested and unused TLS */
	    /* if ((got_type & GOT_TLS_IE) && GOT_TLS_GD_ANY_P (got_type)) */
	    /*   got_type &= ~ (GOT_TLSDESC_GD | GOT_TLS_GD); */

	    if (old_got_type != got_type)
	      {
		if (h != NULL)
		  elf_k1_hash_entry (h)->got_type = got_type;
		else
		  {
		    struct elf_k1_local_symbol *locals;
		    locals = elf_k1_locals (abfd);
		    BFD_ASSERT (r_symndx < symtab_hdr->sh_info);
		    locals[r_symndx].got_type = got_type;
		  }
	      }

	    if (htab->root.dynobj == NULL)
	      htab->root.dynobj = abfd;
	    if (! k1_elf_create_got_section (htab->root.dynobj, info))
	      return FALSE;
	    break;
	  }

	case BFD_RELOC_K1_S64_GOTADDR_LO10:
	case BFD_RELOC_K1_S64_GOTADDR_UP27:
	case BFD_RELOC_K1_S64_GOTADDR_EX27:

	case BFD_RELOC_K1_S43_GOTADDR_LO10:
	case BFD_RELOC_K1_S43_GOTADDR_UP27:
	case BFD_RELOC_K1_S43_GOTADDR_EX6:

	case BFD_RELOC_K1_S37_GOTADDR_LO10:
	case BFD_RELOC_K1_S37_GOTADDR_UP27:

	    if (htab->root.dynobj == NULL)
	      htab->root.dynobj = abfd;
	    if (! k1_elf_create_got_section (htab->root.dynobj, info))
	      return FALSE;
	    break;

	case BFD_RELOC_K1_27_PCREL:
	case BFD_RELOC_K1_17_PCREL:
	  /* If this is a local symbol then we resolve it
	     directly without creating a PLT entry.  */
	  if (h == NULL)
	    continue;

	  h->needs_plt = 1;
	  if (h->plt.refcount <= 0)
	    h->plt.refcount = 1;
	  else
	    h->plt.refcount += 1;
	  break;

	default:
	  break;
	}
    }

  return TRUE;
}

static void
elfNN_k1_post_process_headers (bfd *abfd,
				    struct bfd_link_info *link_info)
{
  Elf_Internal_Ehdr *i_ehdrp;	/* ELF file header, internal form.  */

  i_ehdrp = elf_elfheader (abfd);
  i_ehdrp->e_ident[EI_ABIVERSION] = K1_ELF_ABI_VERSION;

  _bfd_elf_post_process_headers (abfd, link_info);
}

static enum elf_reloc_type_class
elfNN_k1_reloc_type_class (const struct bfd_link_info *info ATTRIBUTE_UNUSED,
				const asection *rel_sec ATTRIBUTE_UNUSED,
				const Elf_Internal_Rela *rela)
{
  switch ((int) ELFNN_R_TYPE (rela->r_info))
    {
    case K1_R (RELATIVE):
      return reloc_class_relative;
    case K1_R (JMP_SLOT):
      return reloc_class_plt;
    case K1_R (COPY):
      return reloc_class_copy;
    default:
      return reloc_class_normal;
    }
}

/* A structure used to record a list of sections, independently
   of the next and prev fields in the asection structure.  */
typedef struct section_list
{
  asection *sec;
  struct section_list *next;
  struct section_list *prev;
}
section_list;



typedef struct
{
  void *finfo;
  struct bfd_link_info *info;
  asection *sec;
  int sec_shndx;
  int (*func) (void *, const char *, Elf_Internal_Sym *,
	       asection *, struct elf_link_hash_entry *);
} output_arch_syminfo;

enum map_symbol_type
{
  K1_MAP_INSN,
  K1_MAP_DATA
};


/* Output a single mapping symbol.  */

static bfd_boolean
elfNN_k1_output_map_sym (output_arch_syminfo *osi,
			      enum map_symbol_type type, bfd_vma offset)
{
  static const char *names[2] = { "$x", "$d" };
  Elf_Internal_Sym sym;

  sym.st_value = (osi->sec->output_section->vma
		  + osi->sec->output_offset + offset);
  sym.st_size = 0;
  sym.st_other = 0;
  sym.st_info = ELF_ST_INFO (STB_LOCAL, STT_NOTYPE);
  sym.st_shndx = osi->sec_shndx;
  return osi->func (osi->finfo, names[type], &sym, osi->sec, NULL) == 1;
}

/* Output a single local symbol for a generated stub.  */

static bfd_boolean
elfNN_k1_output_stub_sym (output_arch_syminfo *osi, const char *name,
			       bfd_vma offset, bfd_vma size)
{
  Elf_Internal_Sym sym;

  sym.st_value = (osi->sec->output_section->vma
		  + osi->sec->output_offset + offset);
  sym.st_size = size;
  sym.st_other = 0;
  sym.st_info = ELF_ST_INFO (STB_LOCAL, STT_FUNC);
  sym.st_shndx = osi->sec_shndx;
  return osi->func (osi->finfo, name, &sym, osi->sec, NULL) == 1;
}

static bfd_boolean
k1_map_one_stub (struct bfd_hash_entry *gen_entry, void *in_arg)
{
  struct elf_k1_stub_hash_entry *stub_entry;
  asection *stub_sec;
  bfd_vma addr;
  char *stub_name;
  output_arch_syminfo *osi;

  /* Massage our args to the form they really have.  */
  stub_entry = (struct elf_k1_stub_hash_entry *) gen_entry;
  osi = (output_arch_syminfo *) in_arg;

  stub_sec = stub_entry->stub_sec;

  /* Ensure this stub is attached to the current section being
     processed.  */
  if (stub_sec != osi->sec)
    return TRUE;

  addr = (bfd_vma) stub_entry->stub_offset;

  stub_name = stub_entry->output_name;

  switch (stub_entry->stub_type)
    {

    default:
      abort ();
    }

  return TRUE;
}

/* Output mapping symbols for linker generated sections.  */

static bfd_boolean
elfNN_k1_output_arch_local_syms (bfd *output_bfd,
				      struct bfd_link_info *info,
				      void *finfo,
				      int (*func) (void *, const char *,
						   Elf_Internal_Sym *,
						   asection *,
						   struct elf_link_hash_entry
						   *))
{
  output_arch_syminfo osi;
  struct elf_k1_link_hash_table *htab;

  htab = elf_k1_hash_table (info);

  osi.finfo = finfo;
  osi.info = info;
  osi.func = func;

  /* Long calls stubs.  */
  if (htab->stub_bfd && htab->stub_bfd->sections)
    {
      asection *stub_sec;

      for (stub_sec = htab->stub_bfd->sections;
	   stub_sec != NULL; stub_sec = stub_sec->next)
	{
	  /* Ignore non-stub sections.  */
	  if (!strstr (stub_sec->name, STUB_SUFFIX))
	    continue;

	  osi.sec = stub_sec;

	  osi.sec_shndx = _bfd_elf_section_from_bfd_section
	    (output_bfd, osi.sec->output_section);

	  /* The first instruction in a stub is always a branch.  */
	  if (!elfNN_k1_output_map_sym (&osi, K1_MAP_INSN, 0))
	    return FALSE;

	  bfd_hash_traverse (&htab->stub_hash_table, k1_map_one_stub,
			     &osi);
	}
    }

  /* Finally, output mapping symbols for the PLT.  */
  if (!htab->root.splt || htab->root.splt->size == 0)
    return TRUE;

  osi.sec_shndx = _bfd_elf_section_from_bfd_section
    (output_bfd, htab->root.splt->output_section);
  osi.sec = htab->root.splt;

  elfNN_k1_output_map_sym (&osi, K1_MAP_INSN, 0);

  return TRUE;

}

/* Allocate target specific section data.  */

static bfd_boolean
elfNN_k1_new_section_hook (bfd *abfd, asection *sec)
{
  if (!sec->used_by_bfd)
    {
      _k1_elf_section_data *sdata;
      bfd_size_type amt = sizeof (*sdata);

      sdata = bfd_zalloc (abfd, amt);
      if (sdata == NULL)
	return FALSE;
      sec->used_by_bfd = sdata;
    }

  return _bfd_elf_new_section_hook (abfd, sec);
}

/* Create dynamic sections. This is different from the ARM backend in that
   the got, plt, gotplt and their relocation sections are all created in the
   standard part of the bfd elf backend.  */

static bfd_boolean
elfNN_k1_create_dynamic_sections (bfd *dynobj,
				       struct bfd_link_info *info)
{
  struct elf_k1_link_hash_table *htab;

  /* We need to create .got section.  */
  if (!k1_elf_create_got_section (dynobj, info))
    return FALSE;

  if (!_bfd_elf_create_dynamic_sections (dynobj, info))
    return FALSE;

  htab = elf_k1_hash_table (info);
  htab->sdynbss = bfd_get_linker_section (dynobj, ".dynbss");
  if (!bfd_link_pic (info))
    htab->srelbss = bfd_get_linker_section (dynobj, ".rela.bss");

  if (!htab->sdynbss || (!bfd_link_pic (info) && !htab->srelbss))
    abort ();

  return TRUE;
}


/* Allocate space in .plt, .got and associated reloc sections for
   dynamic relocs.  */

static bfd_boolean
elfNN_k1_allocate_dynrelocs (struct elf_link_hash_entry *h, void *inf)
{
  struct bfd_link_info *info;
  struct elf_k1_link_hash_table *htab;
  struct elf_k1_link_hash_entry *eh;
  struct elf_dyn_relocs *p;

  /* An example of a bfd_link_hash_indirect symbol is versioned
     symbol. For example: __gxx_personality_v0(bfd_link_hash_indirect)
     -> __gxx_personality_v0(bfd_link_hash_defined)

     There is no need to process bfd_link_hash_indirect symbols here
     because we will also be presented with the concrete instance of
     the symbol and elfNN_k1_copy_indirect_symbol () will have been
     called to copy all relevant data from the generic to the concrete
     symbol instance.
   */
  if (h->root.type == bfd_link_hash_indirect)
    return TRUE;

  if (h->root.type == bfd_link_hash_warning)
    h = (struct elf_link_hash_entry *) h->root.u.i.link;

  info = (struct bfd_link_info *) inf;
  htab = elf_k1_hash_table (info);

  /* Original AARCH64 code has code for IFUNC here */
#if ! K1_DISABLE_IFUNC
#error IFUNC not ready
#endif
    if (htab->root.dynamic_sections_created && h->plt.refcount > 0)
    {
      /* Make sure this symbol is output as a dynamic symbol.
         Undefined weak syms won't yet be marked as dynamic.  */
      if (h->dynindx == -1 && !h->forced_local)
	{
	  if (!bfd_elf_link_record_dynamic_symbol (info, h))
	    return FALSE;
	}

      if (bfd_link_pic (info) || WILL_CALL_FINISH_DYNAMIC_SYMBOL (1, 0, h))
	{
	  asection *s = htab->root.splt;

	  /* If this is the first .plt entry, make room for the special
	     first entry.  */
	  if (s->size == 0)
	    s->size += htab->plt_header_size;

	  h->plt.offset = s->size;

	  /* If this symbol is not defined in a regular file, and we are
	     not generating a shared library, then set the symbol to this
	     location in the .plt.  This is required to make function
	     pointers compare as equal between the normal executable and
	     the shared library.  */
	  if (!bfd_link_pic (info) && !h->def_regular)
	    {
	      h->root.u.def.section = s;
	      h->root.u.def.value = h->plt.offset;
	    }

	  /* Make room for this entry. For now we only create the
	     small model PLT entries. We later need to find a way
	     of relaxing into these from the large model PLT entries.  */
	  s->size += PLT_SMALL_ENTRY_SIZE;

	  /* We also need to make an entry in the .got.plt section, which
	     will be placed in the .got section by the linker script.  */
	  htab->root.sgotplt->size += GOT_ENTRY_SIZE;

	  /* We also need to make an entry in the .rela.plt section.  */
	  htab->root.srelplt->size += RELOC_SIZE (htab);

	  /* We need to ensure that all GOT entries that serve the PLT
	     are consecutive with the special GOT slots [0] [1] and
	     [2]. Any addtional relocations, such as
	     R_AARCH64_TLSDESC, must be placed after the PLT related
	     entries.  We abuse the reloc_count such that during
	     sizing we adjust reloc_count to indicate the number of
	     PLT related reserved entries.  In subsequent phases when
	     filling in the contents of the reloc entries, PLT related
	     entries are placed by computing their PLT index (0
	     .. reloc_count). While other none PLT relocs are placed
	     at the slot indicated by reloc_count and reloc_count is
	     updated.  */

	  htab->root.srelplt->reloc_count++;
	}
      else
	{
	  h->plt.offset = (bfd_vma) - 1;
	  h->needs_plt = 0;
	}
    }
  else
    {
      h->plt.offset = (bfd_vma) - 1;
      h->needs_plt = 0;
    }

  eh = (struct elf_k1_link_hash_entry *) h;
  eh->tlsdesc_got_jump_table_offset = (bfd_vma) - 1;

  if (h->got.refcount > 0)
    {
      bfd_boolean dyn;
      unsigned got_type = elf_k1_hash_entry (h)->got_type;

      h->got.offset = (bfd_vma) - 1;

      dyn = htab->root.dynamic_sections_created;

      /* Make sure this symbol is output as a dynamic symbol.
         Undefined weak syms won't yet be marked as dynamic.  */
      if (dyn && h->dynindx == -1 && !h->forced_local)
	{
	  if (!bfd_elf_link_record_dynamic_symbol (info, h))
	    return FALSE;
	}

      if (got_type == GOT_UNKNOWN)
	{
	  (*_bfd_error_handler)
	    (_("relocation against `%s' has faulty GOT type "),
	     (h) ? h->root.root.string : "a local symbol");
	  bfd_set_error (bfd_error_bad_value);
	  return FALSE;
	}
      else if (got_type == GOT_NORMAL)
	{
#ifdef UGLY_DEBUG
	  printf("'%s'\t\t at offset %d (%x)\n",
		 (h) ? h->root.root.string : "a local symbol",
		 htab->root.sgot->size, htab->root.sgot->size);
#endif
	  h->got.offset = htab->root.sgot->size;
	  htab->root.sgot->size += GOT_ENTRY_SIZE;
	  if ((ELF_ST_VISIBILITY (h->other) == STV_DEFAULT
	       || h->root.type != bfd_link_hash_undefweak)
	      && (bfd_link_pic (info)
		  || WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn, 0, h)))
	    {
	      htab->root.srelgot->size += RELOC_SIZE (htab);
	    }
	}
      else
	{
	  (*_bfd_error_handler)
	    (_("relocation against `%s' has faulty GOT type "),
	     (h) ? h->root.root.string : "a local symbol");
	  bfd_set_error (bfd_error_bad_value);
	  return FALSE;

	  int indx;
	  /* Disabled untested and unused TLS */
	  /* if (got_type & GOT_TLSDESC_GD) */
	  /*   { */
	  /*     eh->tlsdesc_got_jump_table_offset = */
	  /* 	(htab->root.sgotplt->size */
	  /* 	 - k1_compute_jump_table_size (htab)); */
	  /*     htab->root.sgotplt->size += GOT_ENTRY_SIZE * 2; */
	  /*     h->got.offset = (bfd_vma) - 2; */
	  /*   } */
	  /* if (got_type & GOT_TLS_GD) */
	  /*   { */
	  /*     h->got.offset = htab->root.sgot->size; */
	  /*     htab->root.sgot->size += GOT_ENTRY_SIZE * 2; */
	  /*   } */

	  /* if (got_type & GOT_TLS_IE) */
	  /*   { */
	  /*     h->got.offset = htab->root.sgot->size; */
	  /*     htab->root.sgot->size += GOT_ENTRY_SIZE; */
	  /*   } */

	  indx = h && h->dynindx != -1 ? h->dynindx : 0;
	  if ((ELF_ST_VISIBILITY (h->other) == STV_DEFAULT
	       || h->root.type != bfd_link_hash_undefweak)
	      && (bfd_link_pic (info)
		  || indx != 0
		  || WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn, 0, h)))
	    {
	      BFD_ASSERT(0);
	    /* Disabled untested and unused TLS */
	      /* if (got_type & GOT_TLSDESC_GD) */
	      /* 	{ */
	      /* 	  htab->root.srelplt->size += RELOC_SIZE (htab); */
	      /* 	  /\* Note reloc_count not incremented here!  We have */
	      /* 	     already adjusted reloc_count for this relocation */
	      /* 	     type.  *\/ */

	      /* 	  /\* TLSDESC PLT is now needed, but not yet determined.  *\/ */
	      /* 	  htab->tlsdesc_plt = (bfd_vma) - 1; */
	      /* 	} */

	      /* if (got_type & GOT_TLS_GD) */
	      /* 	htab->root.srelgot->size += RELOC_SIZE (htab) * 2; */

	      /* if (got_type & GOT_TLS_IE) */
	      /* 	htab->root.srelgot->size += RELOC_SIZE (htab); */
	    }
	}
    }
  else
    {
      h->got.offset = (bfd_vma) - 1;
    }

  if (eh->dyn_relocs == NULL)
    return TRUE;

  /* In the shared -Bsymbolic case, discard space allocated for
     dynamic pc-relative relocs against symbols which turn out to be
     defined in regular objects.  For the normal shared case, discard
     space for pc-relative relocs that have become local due to symbol
     visibility changes.  */

  if (bfd_link_pic (info))
    {
      /* Relocs that use pc_count are those that appear on a call
         insn, or certain REL relocs that can generated via assembly.
         We want calls to protected symbols to resolve directly to the
         function rather than going via the plt.  If people want
         function pointer comparisons to work as expected then they
         should avoid writing weird assembly.  */
      if (SYMBOL_CALLS_LOCAL (info, h))
	{
	  struct elf_dyn_relocs **pp;

	  for (pp = &eh->dyn_relocs; (p = *pp) != NULL;)
	    {
	      p->count -= p->pc_count;
	      p->pc_count = 0;
	      if (p->count == 0)
		*pp = p->next;
	      else
		pp = &p->next;
	    }
	}

      /* Also discard relocs on undefined weak syms with non-default
         visibility.  */
      if (eh->dyn_relocs != NULL && h->root.type == bfd_link_hash_undefweak)
	{
	  if (ELF_ST_VISIBILITY (h->other) != STV_DEFAULT)
	    eh->dyn_relocs = NULL;

	  /* Make sure undefined weak symbols are output as a dynamic
	     symbol in PIEs.  */
	  else if (h->dynindx == -1
		   && !h->forced_local
		   && !bfd_elf_link_record_dynamic_symbol (info, h))
	    return FALSE;
	}

    }
  else if (ELIMINATE_COPY_RELOCS)
    {
      /* For the non-shared case, discard space for relocs against
         symbols which turn out to need copy relocs or are not
         dynamic.  */

      if (!h->non_got_ref
	  && ((h->def_dynamic
	       && !h->def_regular)
	      || (htab->root.dynamic_sections_created
		  && (h->root.type == bfd_link_hash_undefweak
		      || h->root.type == bfd_link_hash_undefined))))
	{
	  /* Make sure this symbol is output as a dynamic symbol.
	     Undefined weak syms won't yet be marked as dynamic.  */
	  if (h->dynindx == -1
	      && !h->forced_local
	      && !bfd_elf_link_record_dynamic_symbol (info, h))
	    return FALSE;

	  /* If that succeeded, we know we'll be keeping all the
	     relocs.  */
	  if (h->dynindx != -1)
	    goto keep;
	}

      eh->dyn_relocs = NULL;

    keep:;
    }

  /* Finally, allocate space.  */
  for (p = eh->dyn_relocs; p != NULL; p = p->next)
    {
      asection *sreloc;

      sreloc = elf_section_data (p->sec)->sreloc;

      BFD_ASSERT (sreloc != NULL);

      sreloc->size += p->count * RELOC_SIZE (htab);
    }

  return TRUE;
}

/* Allocate space in .plt, .got and associated reloc sections for
   ifunc dynamic relocs.  */
#if ! K1_DISABLE_IFUNC
  /* Original AARCH64 code has code for IFUNC here */
#error IFUNC not ready

static bfd_boolean
elfNN_k1_allocate_ifunc_dynrelocs (struct elf_link_hash_entry *h,
					void *inf)
{
}
#endif /* K1_DISABLE_IFUNC */

/* Allocate space in .plt, .got and associated reloc sections for
   local dynamic relocs.  */

static bfd_boolean
elfNN_k1_allocate_local_dynrelocs (void **slot, void *inf)
{
  struct elf_link_hash_entry *h
    = (struct elf_link_hash_entry *) *slot;

  if (h->type != STT_GNU_IFUNC
      || !h->def_regular
      || !h->ref_regular
      || !h->forced_local
      || h->root.type != bfd_link_hash_defined)
    abort ();

  return elfNN_k1_allocate_dynrelocs (h, inf);
}

/* Allocate space in .plt, .got and associated reloc sections for
   local ifunc dynamic relocs.  */
#if ! K1_DISABLE_IFUNC
  /* Original AARCH64 code has code for IFUNC here */
#error IFUNC not ready
static bfd_boolean
elfNN_k1_allocate_local_ifunc_dynrelocs (void **slot, void *inf)
{
}
#endif /* K1_DISABLE_IFUNC */

/* Find any dynamic relocs that apply to read-only sections.  */

static bfd_boolean
k1_readonly_dynrelocs (struct elf_link_hash_entry * h, void * inf)
{
  struct elf_k1_link_hash_entry * eh;
  struct elf_dyn_relocs * p;

  eh = (struct elf_k1_link_hash_entry *) h;
  for (p = eh->dyn_relocs; p != NULL; p = p->next)
    {
      asection *s = p->sec;

      if (s != NULL && (s->flags & SEC_READONLY) != 0)
	{
	  struct bfd_link_info *info = (struct bfd_link_info *) inf;

	  info->flags |= DF_TEXTREL;

	  /* Not an error, just cut short the traversal.  */
	  return FALSE;
	}
    }
  return TRUE;
}

/* This is the most important function of all . Innocuosly named
   though !  */
static bfd_boolean
elfNN_k1_size_dynamic_sections (bfd *output_bfd ATTRIBUTE_UNUSED,
				     struct bfd_link_info *info)
{
  struct elf_k1_link_hash_table *htab;
  bfd *dynobj;
  asection *s;
  bfd_boolean relocs;
  bfd *ibfd;

  htab = elf_k1_hash_table ((info));
  dynobj = htab->root.dynobj;

  BFD_ASSERT (dynobj != NULL);

  if (htab->root.dynamic_sections_created)
    {
      if (bfd_link_executable (info) && !info->nointerp)
	{
	  s = bfd_get_linker_section (dynobj, ".interp");
	  if (s == NULL)
	    abort ();
	  s->size = sizeof ELF_DYNAMIC_INTERPRETER;
	  s->contents = (unsigned char *) ELF_DYNAMIC_INTERPRETER;
	}
    }

  /* Set up .got offsets for local syms, and space for local dynamic
     relocs.  */
  for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link.next)
    {
      struct elf_k1_local_symbol *locals = NULL;
      Elf_Internal_Shdr *symtab_hdr;
      asection *srel;
      unsigned int i;

      if (!is_k1_elf (ibfd))
	continue;

      for (s = ibfd->sections; s != NULL; s = s->next)
	{
	  struct elf_dyn_relocs *p;

	  for (p = (struct elf_dyn_relocs *)
	       (elf_section_data (s)->local_dynrel); p != NULL; p = p->next)
	    {
	      if (!bfd_is_abs_section (p->sec)
		  && bfd_is_abs_section (p->sec->output_section))
		{
		  /* Input section has been discarded, either because
		     it is a copy of a linkonce section or due to
		     linker script /DISCARD/, so we'll be discarding
		     the relocs too.  */
		}
	      else if (p->count != 0)
		{
		  srel = elf_section_data (p->sec)->sreloc;
		  srel->size += p->count * RELOC_SIZE (htab);
		  if ((p->sec->output_section->flags & SEC_READONLY) != 0)
		    info->flags |= DF_TEXTREL;
		}
	    }
	}

      locals = elf_k1_locals (ibfd);
      if (!locals)
	continue;

      symtab_hdr = &elf_symtab_hdr (ibfd);
      srel = htab->root.srelgot;
      for (i = 0; i < symtab_hdr->sh_info; i++)
	{
	  locals[i].got_offset = (bfd_vma) - 1;
	  locals[i].tlsdesc_got_jump_table_offset = (bfd_vma) - 1;
	  if (locals[i].got_refcount > 0)
	    {
	      unsigned got_type = locals[i].got_type;
	      /* Disabled untested and unused TLS */
	      /* if (got_type & GOT_TLSDESC_GD) */
	      /* 	{ */
	      /* 	  locals[i].tlsdesc_got_jump_table_offset = */
	      /* 	    (htab->root.sgotplt->size */
	      /* 	     - k1_compute_jump_table_size (htab)); */
	      /* 	  htab->root.sgotplt->size += GOT_ENTRY_SIZE * 2; */
	      /* 	  locals[i].got_offset = (bfd_vma) - 2; */
	      /* 	} */

	      /* if (got_type & GOT_TLS_GD) */
	      /* 	{ */
	      /* 	  locals[i].got_offset = htab->root.sgot->size; */
	      /* 	  htab->root.sgot->size += GOT_ENTRY_SIZE * 2; */
	      /* 	} */

	      /* if (got_type & GOT_TLS_IE */
	      /* 	  || got_type & GOT_NORMAL) */
	      if (got_type & GOT_NORMAL)
		{
		  locals[i].got_offset = htab->root.sgot->size;
		  htab->root.sgot->size += GOT_ENTRY_SIZE;
		}

	      if (got_type == GOT_UNKNOWN)
		{
		}

	      if (bfd_link_pic (info))
		{
		  /* Disabled untested and unused TLS */
		  /* if (got_type & GOT_TLSDESC_GD) */
		  /*   { */
		  /*     htab->root.srelplt->size += RELOC_SIZE (htab); */
		  /*     /\* Note RELOC_COUNT not incremented here! *\/ */
		  /*     htab->tlsdesc_plt = (bfd_vma) - 1; */
		  /*   } */

		  /* if (got_type & GOT_TLS_GD) */
		  /*   htab->root.srelgot->size += RELOC_SIZE (htab) * 2; */
		  /* if (got_type & GOT_TLS_IE */
		  /*     || got_type & GOT_NORMAL) */

		  if (got_type & GOT_NORMAL)
		    htab->root.srelgot->size += RELOC_SIZE (htab);
		}
	    }
	  else
	    {
	      locals[i].got_refcount = (bfd_vma) - 1;
	    }
	}
    }


  /* Allocate global sym .plt and .got entries, and space for global
     sym dynamic relocs.  */
  elf_link_hash_traverse (&htab->root, elfNN_k1_allocate_dynrelocs,
			  info);

  /* Allocate global ifunc sym .plt and .got entries, and space for global
     ifunc sym dynamic relocs.  */
#if ! K1_DISABLE_IFUNC
  /* Original AARCH64 code has code for IFUNC here */
#error IFUNC not ready
#endif /* K1_DISABLE_IFUNC */

  /* Allocate .plt and .got entries, and space for local symbols.  */
  htab_traverse (htab->loc_hash_table,
		 elfNN_k1_allocate_local_dynrelocs,
		 info);

  /* Allocate .plt and .got entries, and space for local ifunc symbols.  */
#if ! K1_DISABLE_IFUNC
  /* Original AARCH64 code has code for IFUNC here */
#error IFUNC not ready
#endif /* K1_DISABLE_IFUNC */

  /* For every jump slot reserved in the sgotplt, reloc_count is
     incremented.  However, when we reserve space for TLS descriptors,
     it's not incremented, so in order to compute the space reserved
     for them, it suffices to multiply the reloc count by the jump
     slot size.  */

  if (htab->root.srelplt)
    htab->sgotplt_jump_table_size = k1_compute_jump_table_size (htab);

  if (htab->tlsdesc_plt)
    {
      if (htab->root.splt->size == 0)
	htab->root.splt->size += PLT_ENTRY_SIZE;

      htab->tlsdesc_plt = htab->root.splt->size;
      htab->root.splt->size += PLT_TLSDESC_ENTRY_SIZE;

      /* If we're not using lazy TLS relocations, don't generate the
         GOT entry required.  */
      if (!(info->flags & DF_BIND_NOW))
	{
	  htab->dt_tlsdesc_got = htab->root.sgot->size;
	  htab->root.sgot->size += GOT_ENTRY_SIZE;
	}
    }

  /* We now have determined the sizes of the various dynamic sections.
     Allocate memory for them.  */
  relocs = FALSE;
  for (s = dynobj->sections; s != NULL; s = s->next)
    {
      if ((s->flags & SEC_LINKER_CREATED) == 0)
	continue;

      if (s == htab->root.splt
	  || s == htab->root.sgot
	  || s == htab->root.sgotplt
	  || s == htab->root.iplt
	  || s == htab->root.igotplt || s == htab->sdynbss)
	{
	  /* Strip this section if we don't need it; see the
	     comment below.  */
	}
      else if (CONST_STRNEQ (bfd_get_section_name (dynobj, s), ".rela"))
	{
	  if (s->size != 0 && s != htab->root.srelplt)
	    relocs = TRUE;

	  /* We use the reloc_count field as a counter if we need
	     to copy relocs into the output file.  */
	  if (s != htab->root.srelplt)
	    s->reloc_count = 0;
	}
      else
	{
	  /* It's not one of our sections, so don't allocate space.  */
	  continue;
	}

      if (s->size == 0)
	{
	  /* If we don't need this section, strip it from the
	     output file.  This is mostly to handle .rela.bss and
	     .rela.plt.  We must create both sections in
	     create_dynamic_sections, because they must be created
	     before the linker maps input sections to output
	     sections.  The linker does that before
	     adjust_dynamic_symbol is called, and it is that
	     function which decides whether anything needs to go
	     into these sections.  */

	  s->flags |= SEC_EXCLUDE;
	  continue;
	}

      if ((s->flags & SEC_HAS_CONTENTS) == 0)
	continue;

      /* Allocate memory for the section contents.  We use bfd_zalloc
         here in case unused entries are not reclaimed before the
         section's contents are written out.  This should not happen,
         but this way if it does, we get a R_K1_NONE reloc instead
         of garbage.  */
      s->contents = (bfd_byte *) bfd_zalloc (dynobj, s->size);
      if (s->contents == NULL)
	return FALSE;
    }

  if (htab->root.dynamic_sections_created)
    {
      /* Add some entries to the .dynamic section.  We fill in the
         values later, in elfNN_k1_finish_dynamic_sections, but we
         must add the entries now so that we get the correct size for
         the .dynamic section.  The DT_DEBUG entry is filled in by the
         dynamic linker and used by the debugger.  */
#define add_dynamic_entry(TAG, VAL)			\
      _bfd_elf_add_dynamic_entry (info, TAG, VAL)

      if (bfd_link_executable (info))
	{
	  if (!add_dynamic_entry (DT_DEBUG, 0))
	    return FALSE;
	}

      if (htab->root.splt->size != 0)
	{
	  if (!add_dynamic_entry (DT_PLTGOT, 0)
	      || !add_dynamic_entry (DT_PLTRELSZ, 0)
	      || !add_dynamic_entry (DT_PLTREL, DT_RELA)
	      || !add_dynamic_entry (DT_JMPREL, 0))
	    return FALSE;

	  if (htab->tlsdesc_plt
	      && (!add_dynamic_entry (DT_TLSDESC_PLT, 0)
		  || !add_dynamic_entry (DT_TLSDESC_GOT, 0)))
	    return FALSE;
	}

      if (relocs)
	{
	  if (!add_dynamic_entry (DT_RELA, 0)
	      || !add_dynamic_entry (DT_RELASZ, 0)
	      || !add_dynamic_entry (DT_RELAENT, RELOC_SIZE (htab)))
	    return FALSE;

	  /* If any dynamic relocs apply to a read-only section,
	     then we need a DT_TEXTREL entry.  */
	  if ((info->flags & DF_TEXTREL) == 0)
	    elf_link_hash_traverse (& htab->root, k1_readonly_dynrelocs,
				    info);

	  if ((info->flags & DF_TEXTREL) != 0)
	    {
	      if (!add_dynamic_entry (DT_TEXTREL, 0))
		return FALSE;
	    }
	}
    }
#undef add_dynamic_entry

  return TRUE;
}

static inline void
elf_k1_update_plt_entry (bfd *output_bfd,
			      bfd_reloc_code_real_type r_type,
			      bfd_byte *plt_entry, bfd_vma value)
{
  reloc_howto_type *howto = elfNN_k1_howto_from_bfd_reloc (r_type);
  BFD_ASSERT(howto != NULL);
  _bfd_k1_elf_put_addend (output_bfd, plt_entry, r_type, howto, value);
}

static void
elfNN_k1_create_small_pltn_entry (struct elf_link_hash_entry *h,
				       struct elf_k1_link_hash_table
				       *htab, bfd *output_bfd,
				       struct bfd_link_info *info)
{
  bfd_byte *plt_entry;
  bfd_vma plt_index;
  bfd_vma got_offset;
  bfd_vma gotplt_entry_address;
  bfd_vma plt_entry_address;
  Elf_Internal_Rela rela;
  bfd_byte *loc;
  asection *plt, *gotplt, *relplt;

  /* When building a static executable, use .iplt, .igot.plt and
     .rela.iplt sections for STT_GNU_IFUNC symbols.  */
  if (htab->root.splt != NULL)
    {
      plt = htab->root.splt;
      gotplt = htab->root.sgotplt;
      relplt = htab->root.srelplt;
    }
  else
    {
      plt = htab->root.iplt;
      gotplt = htab->root.igotplt;
      relplt = htab->root.irelplt;
    }

  /* Get the index in the procedure linkage table which
     corresponds to this symbol.  This is the index of this symbol
     in all the symbols for which we are making plt entries.  The
     first entry in the procedure linkage table is reserved.

     Get the offset into the .got table of the entry that
     corresponds to this function.	Each .got entry is GOT_ENTRY_SIZE
     bytes. The first three are reserved for the dynamic linker.

     For static executables, we don't reserve anything.  */

  if (plt == htab->root.splt)
    {
      plt_index = (h->plt.offset - htab->plt_header_size) / htab->plt_entry_size;
      got_offset = (plt_index + 3) * GOT_ENTRY_SIZE;
    }
  else
    {
      plt_index = h->plt.offset / htab->plt_entry_size;
      got_offset = plt_index * GOT_ENTRY_SIZE;
    }

  plt_entry = plt->contents + h->plt.offset;
  plt_entry_address = plt->output_section->vma
    + plt->output_offset + h->plt.offset;
  gotplt_entry_address = gotplt->output_section->vma +
    gotplt->output_offset + got_offset;

  /* Copy in the boiler-plate for the PLTn entry.  */
  memcpy (plt_entry, elfNN_k1_small_plt_entry, PLT_SMALL_ENTRY_SIZE);

  /* Patch the loading of the GOT entry, relative to the PLT entry
   * address
   */

  /* Use 37bits offset for both 32 and 64bits mode */
  /* Fill the LO10 of of lw $r9 = 0[$r14] */
  elf_k1_update_plt_entry(output_bfd, BFD_RELOC_K1_S37_LO10,
			  plt_entry+4,
			  gotplt_entry_address - plt_entry_address);

  /* Fill the UP27 of of lw $r9 = 0[$r14] */
  elf_k1_update_plt_entry(output_bfd, BFD_RELOC_K1_S37_UP27,
			  plt_entry+8,
			  gotplt_entry_address - plt_entry_address);

  rela.r_offset = gotplt_entry_address;

  /* Original AARCH64 code has code for IFUNC here */
#if ! K1_DISABLE_IFUNC
#error IFUNC not ready
#endif
    {
      /* Fill in the entry in the .rela.plt section.  */
      rela.r_info = ELFNN_R_INFO (h->dynindx, K1_R (JMP_SLOT));
      rela.r_addend = 0;
    }

  /* Compute the relocation entry to used based on PLT index and do
     not adjust reloc_count. The reloc_count has already been adjusted
     to account for this entry.  */
  loc = relplt->contents + plt_index * RELOC_SIZE (htab);
  bfd_elfNN_swap_reloca_out (output_bfd, &rela, loc);
}

/* Size sections even though they're not dynamic.  We use it to setup
   _TLS_MODULE_BASE_, if needed.  */

static bfd_boolean
elfNN_k1_always_size_sections (bfd *output_bfd,
				    struct bfd_link_info *info)
{
  asection *tls_sec;

  if (bfd_link_relocatable (info))
    return TRUE;

  tls_sec = elf_hash_table (info)->tls_sec;

  if (tls_sec)
    {
      struct elf_link_hash_entry *tlsbase;

      tlsbase = elf_link_hash_lookup (elf_hash_table (info),
				      "_TLS_MODULE_BASE_", TRUE, TRUE, FALSE);

      if (tlsbase)
	{
	  struct bfd_link_hash_entry *h = NULL;
	  const struct elf_backend_data *bed =
	    get_elf_backend_data (output_bfd);

	  if (!(_bfd_generic_link_add_one_symbol
		(info, output_bfd, "_TLS_MODULE_BASE_", BSF_LOCAL,
		 tls_sec, 0, NULL, FALSE, bed->collect, &h)))
	    return FALSE;

	  tlsbase->type = STT_TLS;
	  tlsbase = (struct elf_link_hash_entry *) h;
	  tlsbase->def_regular = 1;
	  tlsbase->other = STV_HIDDEN;
	  (*bed->elf_backend_hide_symbol) (info, tlsbase, TRUE);
	}
    }

  return TRUE;
}

/* Finish up dynamic symbol handling.  We set the contents of various
   dynamic sections here.  */
static bfd_boolean
elfNN_k1_finish_dynamic_symbol (bfd *output_bfd,
				     struct bfd_link_info *info,
				     struct elf_link_hash_entry *h,
				     Elf_Internal_Sym *sym)
{
  struct elf_k1_link_hash_table *htab;
  htab = elf_k1_hash_table (info);

  if (h->plt.offset != (bfd_vma) - 1)
    {
      asection *plt, *gotplt, *relplt;

      /* This symbol has an entry in the procedure linkage table.  Set
         it up.  */

      /* When building a static executable, use .iplt, .igot.plt and
	 .rela.iplt sections for STT_GNU_IFUNC symbols.  */
      if (htab->root.splt != NULL)
	{
	  plt = htab->root.splt;
	  gotplt = htab->root.sgotplt;
	  relplt = htab->root.srelplt;
	}
#if ! K1_DISABLE_IFUNC
  /* Original AARCH64 code has code for IFUNC here */
#error IFUNC not ready
#endif /* K1_DISABLE_IFUNC */

      /* This symbol has an entry in the procedure linkage table.  Set
	 it up.	 */
      if ((h->dynindx == -1
	   && !((h->forced_local || bfd_link_executable (info))
		&& h->def_regular
		&& h->type == STT_GNU_IFUNC))
	  || plt == NULL
	  || gotplt == NULL
	  || relplt == NULL)
	abort ();

      elfNN_k1_create_small_pltn_entry (h, htab, output_bfd, info);
      if (!h->def_regular)
	{
	  /* Mark the symbol as undefined, rather than as defined in
	     the .plt section.  */
	  sym->st_shndx = SHN_UNDEF;
	  /* If the symbol is weak we need to clear the value.
	     Otherwise, the PLT entry would provide a definition for
	     the symbol even if the symbol wasn't defined anywhere,
	     and so the symbol would never be NULL.  Leave the value if
	     there were any relocations where pointer equality matters
	     (this is a clue for the dynamic linker, to make function
	     pointer comparisons work between an application and shared
	     library).  */
	  if (!h->ref_regular_nonweak || !h->pointer_equality_needed)
	    sym->st_value = 0;
	}
    }

  if (h->got.offset != (bfd_vma) - 1
      && elf_k1_hash_entry (h)->got_type == GOT_NORMAL)
    {
      Elf_Internal_Rela rela;
      bfd_byte *loc;

      /* This symbol has an entry in the global offset table.  Set it
         up.  */
      if (htab->root.sgot == NULL || htab->root.srelgot == NULL)
	abort ();

      rela.r_offset = (htab->root.sgot->output_section->vma
		       + htab->root.sgot->output_offset
		       + (h->got.offset & ~(bfd_vma) 1));

#ifdef UGLY_DEBUG
      printf("setting rela at offset 0x%x(0x%x + 0x%x + 0x%x) for %s\n",
	     rela.r_offset,
	     htab->root.sgot->output_section->vma,
	     htab->root.sgot->output_offset,
	     h->got.offset,
	     h->root.root.string);
#endif

#if ! K1_DISABLE_IFUNC
  /* Original AARCH64 code has code for IFUNC here */
#error IFUNC not ready
#endif /* K1_DISABLE_IFUNC */
	if (bfd_link_pic (info) && SYMBOL_REFERENCES_LOCAL (info, h))
	{
	  if (!h->def_regular)
	    return FALSE;

	  /* in case of PLT related GOT entry, it is not clear who is
	     supposed to set the LSB of GOT entry...
	     k1_calculate_got_entry_vma() would be a good candidate,
	     but it is not called currently
	     So we are commenting it ATM
	  */
	  // BFD_ASSERT ((h->got.offset & 1) != 0);
	  rela.r_info = ELFNN_R_INFO (0, K1_R (RELATIVE));
	  rela.r_addend = (h->root.u.def.value
			   + h->root.u.def.section->output_section->vma
			   + h->root.u.def.section->output_offset);
	}
      else
	{
do_glob_dat:
	  BFD_ASSERT ((h->got.offset & 1) == 0);
	  bfd_put_NN (output_bfd, (bfd_vma) 0,
		      htab->root.sgot->contents + h->got.offset);
	  rela.r_info = ELFNN_R_INFO (h->dynindx, K1_R (GLOB_DAT));
	  rela.r_addend = 0;
	}

      loc = htab->root.srelgot->contents;
      loc += htab->root.srelgot->reloc_count++ * RELOC_SIZE (htab);
      bfd_elfNN_swap_reloca_out (output_bfd, &rela, loc);
    }

  if (h->needs_copy)
    {
      Elf_Internal_Rela rela;
      bfd_byte *loc;

      /* This symbol needs a copy reloc.  Set it up.  */

      if (h->dynindx == -1
	  || (h->root.type != bfd_link_hash_defined
	      && h->root.type != bfd_link_hash_defweak)
	  || htab->srelbss == NULL)
	abort ();

      rela.r_offset = (h->root.u.def.value
		       + h->root.u.def.section->output_section->vma
		       + h->root.u.def.section->output_offset);
      rela.r_info = ELFNN_R_INFO (h->dynindx, K1_R (COPY));
      rela.r_addend = 0;
      loc = htab->srelbss->contents;
      loc += htab->srelbss->reloc_count++ * RELOC_SIZE (htab);
      bfd_elfNN_swap_reloca_out (output_bfd, &rela, loc);
    }

  /* Mark _DYNAMIC and _GLOBAL_OFFSET_TABLE_ as absolute.  SYM may
     be NULL for local symbols.  */
  if (sym != NULL
      && (h == elf_hash_table (info)->hdynamic
	  || h == elf_hash_table (info)->hgot))
    sym->st_shndx = SHN_ABS;

  return TRUE;
}

/* Finish up local dynamic symbol handling.  We set the contents of
   various dynamic sections here.  */

static bfd_boolean
elfNN_k1_finish_local_dynamic_symbol (void **slot, void *inf)
{
  struct elf_link_hash_entry *h
    = (struct elf_link_hash_entry *) *slot;
  struct bfd_link_info *info
    = (struct bfd_link_info *) inf;

  return elfNN_k1_finish_dynamic_symbol (info->output_bfd,
					      info, h, NULL);
}

static void
elfNN_k1_init_small_plt0_entry (bfd *output_bfd ATTRIBUTE_UNUSED,
				     struct elf_k1_link_hash_table
				     *htab)
{
  bfd_vma plt_got_2nd_ent;	/* Address of GOT[2].  */
  bfd_vma plt_base;


  memcpy (htab->root.splt->contents, elfNN_k1_small_plt0_entry,
	  PLT_ENTRY_SIZE);
  elf_section_data (htab->root.splt->output_section)->this_hdr.sh_entsize =
    PLT_ENTRY_SIZE;
}

static bfd_boolean
elfNN_k1_finish_dynamic_sections (bfd *output_bfd,
				       struct bfd_link_info *info)
{
  struct elf_k1_link_hash_table *htab;
  bfd *dynobj;
  asection *sdyn;

  htab = elf_k1_hash_table (info);
  dynobj = htab->root.dynobj;
  sdyn = bfd_get_linker_section (dynobj, ".dynamic");

  if (htab->root.dynamic_sections_created)
    {
      ElfNN_External_Dyn *dyncon, *dynconend;

      if (sdyn == NULL || htab->root.sgot == NULL)
	abort ();

      dyncon = (ElfNN_External_Dyn *) sdyn->contents;
      dynconend = (ElfNN_External_Dyn *) (sdyn->contents + sdyn->size);
      for (; dyncon < dynconend; dyncon++)
	{
	  Elf_Internal_Dyn dyn;
	  asection *s;

	  bfd_elfNN_swap_dyn_in (dynobj, dyncon, &dyn);

	  switch (dyn.d_tag)
	    {
	    default:
	      continue;

	    case DT_PLTGOT:
	      s = htab->root.sgotplt;
	      dyn.d_un.d_ptr = s->output_section->vma + s->output_offset;
	      break;

	    case DT_JMPREL:
	      dyn.d_un.d_ptr = htab->root.srelplt->output_section->vma;
	      break;

	    case DT_PLTRELSZ:
	      s = htab->root.srelplt;
	      dyn.d_un.d_val = s->size;
	      break;

	    case DT_RELASZ:
	      /* The procedure linkage table relocs (DT_JMPREL) should
		 not be included in the overall relocs (DT_RELA).
		 Therefore, we override the DT_RELASZ entry here to
		 make it not include the JMPREL relocs.  Since the
		 linker script arranges for .rela.plt to follow all
		 other relocation sections, we don't have to worry
		 about changing the DT_RELA entry.  */
	      if (htab->root.srelplt != NULL)
		{
		  s = htab->root.srelplt;
		  dyn.d_un.d_val -= s->size;
		}
	      break;

	    case DT_TLSDESC_PLT:
	      s = htab->root.splt;
	      dyn.d_un.d_ptr = s->output_section->vma + s->output_offset
		+ htab->tlsdesc_plt;
	      break;

	    case DT_TLSDESC_GOT:
	      s = htab->root.sgot;
	      dyn.d_un.d_ptr = s->output_section->vma + s->output_offset
		+ htab->dt_tlsdesc_got;
	      break;
	    }

	  bfd_elfNN_swap_dyn_out (output_bfd, &dyn, dyncon);
	}

    }

  /* Fill in the special first entry in the procedure linkage table.  */
  if (htab->root.splt && htab->root.splt->size > 0)
    {
      elfNN_k1_init_small_plt0_entry (output_bfd, htab);

      elf_section_data (htab->root.splt->output_section)->
	this_hdr.sh_entsize = htab->plt_entry_size;


      if (htab->tlsdesc_plt)
	{
	  /* Original aarch64 handle TLS here. */
	  BFD_ASSERT (0);
	}
    }

  if (htab->root.sgotplt)
    {
      if (bfd_is_abs_section (htab->root.sgotplt->output_section))
	{
	  (*_bfd_error_handler)
	    (_("discarded output section: `%A'"), htab->root.sgotplt);
	  return FALSE;
	}

      /* Fill in the first three entries in the global offset table.  */
      if (htab->root.sgotplt->size > 0)
	{
	  bfd_put_NN (output_bfd, (bfd_vma) 0, htab->root.sgotplt->contents);

	  /* Write GOT[1] and GOT[2], needed for the dynamic linker.  */
	  bfd_put_NN (output_bfd,
		      (bfd_vma) 0,
		      htab->root.sgotplt->contents + GOT_ENTRY_SIZE);
	  bfd_put_NN (output_bfd,
		      (bfd_vma) 0,
		      htab->root.sgotplt->contents + GOT_ENTRY_SIZE * 2);
	}

      if (htab->root.sgot)
	{
	  if (htab->root.sgot->size > 0)
	    {
	      bfd_vma addr =
		sdyn ? sdyn->output_section->vma + sdyn->output_offset : 0;
	      bfd_put_NN (output_bfd, addr, htab->root.sgot->contents);
	    }
	}

      elf_section_data (htab->root.sgotplt->output_section)->
	this_hdr.sh_entsize = GOT_ENTRY_SIZE;
    }

  if (htab->root.sgot && htab->root.sgot->size > 0)
    elf_section_data (htab->root.sgot->output_section)->this_hdr.sh_entsize
      = GOT_ENTRY_SIZE;

  /* Fill PLT and GOT entries for local STT_GNU_IFUNC symbols.  */
  htab_traverse (htab->loc_hash_table,
		 elfNN_k1_finish_local_dynamic_symbol,
		 info);

  return TRUE;
}

/* Return address for Ith PLT stub in section PLT, for relocation REL
   or (bfd_vma) -1 if it should not be included.  */

static bfd_vma
elfNN_k1_plt_sym_val (bfd_vma i, const asection *plt,
			   const arelent *rel ATTRIBUTE_UNUSED)
{
  return plt->vma + PLT_ENTRY_SIZE + i * PLT_SMALL_ENTRY_SIZE;
}

#define ELF_ARCH			bfd_arch_k1
#define ELF_MACHINE_CODE		EM_K1
#define ELF_MAXPAGESIZE			0x10000
#define ELF_MINPAGESIZE			0x1000
#define ELF_COMMONPAGESIZE		0x1000

#define bfd_elfNN_bfd_link_hash_table_create    \
  elfNN_k1_link_hash_table_create

#define bfd_elfNN_bfd_merge_private_bfd_data	\
  elfNN_k1_merge_private_bfd_data

#define bfd_elfNN_bfd_print_private_bfd_data	\
  elfNN_k1_print_private_bfd_data

#define bfd_elfNN_bfd_reloc_type_lookup		\
  elfNN_k1_reloc_type_lookup

#define bfd_elfNN_bfd_reloc_name_lookup		\
  elfNN_k1_reloc_name_lookup

#define bfd_elfNN_bfd_set_private_flags		\
  elfNN_k1_set_private_flags

#define bfd_elfNN_mkobject			\
  elfNN_k1_mkobject

#define bfd_elfNN_new_section_hook		\
  elfNN_k1_new_section_hook

#define elf_backend_adjust_dynamic_symbol	\
  elfNN_k1_adjust_dynamic_symbol

#define elf_backend_always_size_sections	\
  elfNN_k1_always_size_sections

#define elf_backend_check_relocs		\
  elfNN_k1_check_relocs

#define elf_backend_copy_indirect_symbol	\
  elfNN_k1_copy_indirect_symbol

/* Create .dynbss, and .rela.bss sections in DYNOBJ, and set up shortcuts
   to them in our hash.  */
#define elf_backend_create_dynamic_sections	\
  elfNN_k1_create_dynamic_sections

#define elf_backend_init_index_section		\
  _bfd_elf_init_2_index_sections

#define elf_backend_finish_dynamic_sections	\
  elfNN_k1_finish_dynamic_sections

#define elf_backend_finish_dynamic_symbol	\
  elfNN_k1_finish_dynamic_symbol

#define elf_backend_gc_sweep_hook		\
  elfNN_k1_gc_sweep_hook

#define elf_backend_object_p			\
  elfNN_k1_object_p

#define elf_backend_output_arch_local_syms      \
  elfNN_k1_output_arch_local_syms

#define elf_backend_plt_sym_val			\
  elfNN_k1_plt_sym_val

#define elf_backend_post_process_headers	\
  elfNN_k1_post_process_headers

#define elf_backend_relocate_section		\
  elfNN_k1_relocate_section

#define elf_backend_reloc_type_class		\
  elfNN_k1_reloc_type_class

#define elf_backend_size_dynamic_sections	\
  elfNN_k1_size_dynamic_sections

#define elf_backend_can_refcount       1
#define elf_backend_can_gc_sections    1
#define elf_backend_plt_readonly       1
#define elf_backend_want_got_plt       1
#define elf_backend_want_plt_sym       0
#define elf_backend_may_use_rel_p      0
#define elf_backend_may_use_rela_p     1
#define elf_backend_default_use_rela_p 1
#define elf_backend_rela_normal        1
#define elf_backend_got_header_size (GOT_ENTRY_SIZE * 3)
#define elf_backend_default_execstack  0
#define elf_backend_extern_protected_data 1

#include "elfNN-target.h"
