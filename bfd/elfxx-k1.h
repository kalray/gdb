#include "elf/common.h"
#include "elf/internal.h"

#if 0
#define DPRINT(X) fprintf(stderr, "==> " X "\n")
#else
#define DPRINT(X)
#endif

/* The name of the dynamic interpreter.  This is put in the .interp
   section.  */
#define ELF_DYNAMIC_INTERPRETER "/lib/ld.so"

#define K1_MACH_MASK 0x0000000f

/* The same in PIC */
#define PLT_ENTRY_SIZE          16
#define PLT_SMALL_ENTRY_SIZE     16

#ifdef BFD64
#define ELF_R_SYM(bfd, i)					\
  (ABI_64_P (bfd) ? ELF64_R_SYM (i) : ELF32_R_SYM (i))
#define ELF_R_TYPE(bfd, i)					\
  (ABI_64_P (bfd) ? ELF64_R_TYPE (i) : ELF32_R_TYPE (i))
#define ELF_R_INFO(bfd, s, t)					\
  (ABI_64_P (bfd) ? ELF64_R_INFO (s, t) : ELF32_R_INFO (s, t))
#else
#define ELF_R_SYM(bfd, i)					\
  (ELF32_R_SYM (i))
#define ELF_R_TYPE(bfd, i)					\
  (ELF32_R_TYPE (i))
#define ELF_R_INFO(bfd, s, t)					\
  (ELF32_R_INFO (s, t))
#endif

bfd_boolean k1_merge_private_bfd_data (bfd *ibfd, bfd *obfd);

bfd_boolean k1_adjust_dynamic_symbol (struct bfd_link_info *info,
				      struct elf_link_hash_entry *h);

bfd_boolean k1_gc_sweep_hook (bfd * abfd,
			      struct bfd_link_info *info,
			      asection * sec,
			      const Elf_Internal_Rela * relocs);


int
_k1fdpic_resolve_final_relocs_info (void **entryp, void *p);

extern reloc_howto_type* k1_reloc_type_lookup (bfd * abfd ATTRIBUTE_UNUSED,
                                               bfd_reloc_code_real_type code);
extern reloc_howto_type* k1_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED, const char *r_name);
extern void k1_elf_info_to_howto (bfd *abfd ATTRIBUTE_UNUSED,
                                  arelent *cache_ptr,
                                  Elf_Internal_Rela *dst);
extern bfd_boolean
elf32_k1_is_target_special_symbol (bfd * abfd ATTRIBUTE_UNUSED, asymbol * sym);

extern struct bfd_link_hash_table *
k1_elf_link_hash_table_create (bfd *abfd);

bfd_boolean
k1_allocate_dynrelocs (struct elf_link_hash_entry *h, void * dat);

bfd_boolean
k1_size_dynamic_sections (bfd * output_bfd,
			  struct bfd_link_info *info);

bfd_boolean
k1_elf32_fdpic_create_got_section (bfd *abfd, struct bfd_link_info *info);

extern const bfd_target bfd_elf32_k1_linux_vec;
#define IS_FDPIC(bfd) ((bfd)->xvec == &bfd_elf32_k1_linux_vec)

bfd_boolean
k1_elf_relocate_section
    (bfd *                   output_bfd ATTRIBUTE_UNUSED,
     struct bfd_link_info *  info,
     bfd *                   input_bfd,
     asection *              input_section,
     bfd_byte *              contents,
     Elf_Internal_Rela *     relocs,
     Elf_Internal_Sym *      local_syms,
     asection **             local_sections);

bfd_boolean
k1_gc_sweep_hook (bfd * abfd,
                    struct bfd_link_info *info,
                    asection * sec,
		  const Elf_Internal_Rela * relocs);

bfd_boolean
k1_adjust_dynamic_symbol (struct bfd_link_info *info,
			  struct elf_link_hash_entry *h);
bfd_boolean
k1_create_dynamic_sections (bfd *dynobj, struct bfd_link_info *info);

bfd_boolean
k1_finish_dynamic_sections (bfd * output_bfd ATTRIBUTE_UNUSED,
			    struct bfd_link_info *info);

bfd_boolean
k1_finish_dynamic_symbol (bfd * output_bfd,
                                struct bfd_link_info *info,
                                struct elf_link_hash_entry *h,
			  Elf_Internal_Sym * sym);

bfd_boolean
k1_size_dynamic_sections (bfd * output_bfd ATTRIBUTE_UNUSED,
			  struct bfd_link_info *info);

bfd_boolean
k1_check_relocs (bfd * abfd,
		   struct bfd_link_info *info,
		   asection *sec,
		 const Elf_Internal_Rela *relocs);

unsigned int
k1_bfd_elf_action_discarded (asection *sec);

bfd_boolean
elf_k1_merge_private_bfd_data (bfd *ibfd, bfd *obfd);

bfd_boolean
elf_k1_set_private_flags (bfd *abfd, flagword flags);

bfd_boolean
elf_k1_print_private_bfd_data (bfd *abfd, void *farg);

bfd_boolean
elf_k1_copy_private_bfd_data(bfd *ibfd, bfd *obfd);

void
elf_k1_final_write_processing (bfd *abfd,
			       bfd_boolean linker ATTRIBUTE_UNUSED);
bfd_boolean
elf_k1_object_p (bfd *abfd);

bfd_vma
k1_plt_sym_val (bfd_vma i, const asection *plt,
		const arelent *rel ATTRIBUTE_UNUSED);
bfd_boolean
k1fdpic_gc_sweep_hook (bfd *abfd,
			 struct bfd_link_info *info,
			 asection *sec,
		       const Elf_Internal_Rela *relocs);

bfd_boolean
_k1fdpic_link_omit_section_dynsym (bfd *output_bfd ATTRIBUTE_UNUSED,
                                    struct bfd_link_info *info ATTRIBUTE_UNUSED,
				   asection *p);

bfd_boolean
elf_k1_always_size_sections(bfd *output_bfd, struct bfd_link_info *info);

bfd_boolean
elf_k1_modify_program_headers (bfd *output_bfd,
			       struct bfd_link_info *info);

bfd_boolean
k1fdpic_elf_relocate_section
    (bfd *                   output_bfd ATTRIBUTE_UNUSED,
     struct bfd_link_info *  info,
     bfd *                   input_bfd,
     asection *              input_section,
     bfd_byte *              contents,
     Elf_Internal_Rela *     relocs,
     Elf_Internal_Sym *      local_syms,
     asection **             local_sections);

bfd_boolean
k1fdpic_check_relocs (bfd * abfd,
                 struct bfd_link_info *info,
                 asection *sec,
		      const Elf_Internal_Rela *relocs);

bfd_boolean
k1fdpic_size_dynamic_sections (bfd *output_bfd ATTRIBUTE_UNUSED,
			       struct bfd_link_info *info);

hashval_t
k1fdpic_relocs_info_hash (const void *entry_);

int
k1fdpic_relocs_info_eq (const void *entry1, const void *entry2);

bfd_boolean
k1fdpic_adjust_dynamic_symbol (struct bfd_link_info *info,
			       struct elf_link_hash_entry *h);

bfd_boolean
k1fdpic_finish_dynamic_sections (bfd *output_bfd,
				 struct bfd_link_info *info);
bfd_boolean
k1fdpic_finish_dynamic_symbol
(bfd *output_bfd ATTRIBUTE_UNUSED,
 struct bfd_link_info *info ATTRIBUTE_UNUSED,
 struct elf_link_hash_entry *h ATTRIBUTE_UNUSED,
 Elf_Internal_Sym *sym ATTRIBUTE_UNUSED);

int
_k1fdpic_assign_got_entries (void **entryp, void *info_);

int
_k1fdpic_count_got_plt_entries (void **entryp, void *dinfo_);

int
_k1fdpic_reset_got_plt_entries (void **entryp, void *ignore ATTRIBUTE_UNUSED);

unsigned
_k1fdpic_osec_to_segment (bfd *output_bfd, asection *osec);

/* The k1 linker (like many others) needs to keep track of
   the number of relocs that it decides to copy as dynamic relocs in
   check_relocs for each symbol. This is so that it can later discard
   them if they are found to be unnecessary.  We store the information
   in a field extending the regular ELF linker hash table.  */

struct k1_elf_dyn_relocs
{
  struct k1_elf_dyn_relocs *next;

  /* The input section of the reloc.  */
  asection *sec;

  /* Total number of relocs copied for the input section.  */
  bfd_size_type count;

  /* Number of pc-relative relocs copied for the input section.  */
  bfd_size_type pc_count;
};

/* ELF linker hash entry.  */

struct k1_elf_link_hash_entry
{
  struct elf_link_hash_entry elf;

  /* Track dynamic relocs copied for this symbol.  */
  struct k1_elf_dyn_relocs *dyn_relocs;
  

};


/* This structure is used to collect the number of entries present in
   each addressable range of the got.  */
struct _k1fdpic_dynamic_got_info
{
  /* Several bits of information about the current link.  */
  struct bfd_link_info *info;
  /* Total size needed for GOT entries.  */
  bfd_vma gothilo;
  /* Total size needed for function descriptor entries.  */
  bfd_vma fd, fdhilo;
  /* Total size needed function descriptor entries referenced in PLT
     entries, that would be profitable to place in offsets close to
     the PIC register.  */
  bfd_vma fdplt;
  /* Total size needed by lazy PLT entries.  */
  bfd_vma lzplt;
  /* Number of relocations carried over from input object files.  */
  unsigned long relocs;
  /* Number of fixups introduced by relocations in input object files.  */
  unsigned long fixups;
};

/* This structure is used to assign offsets to got entries, function
   descriptors, plt entries and lazy plt entries.  */

struct _k1fdpic_dynamic_got_plt_info
{
  /* Summary information collected with _k1fdpic_count_got_plt_entries.  */
  struct _k1fdpic_dynamic_got_info g;

  /* For each addressable range, we record a MAX (positive) and MIN
     (negative) value.  CUR is used to assign got entries, and it's
     incremented from an initial positive value to MAX, then from MIN
     to FDCUR (unless FDCUR wraps around first).  FDCUR is used to
     assign function descriptors, and it's decreased from an initial
     non-positive value to MIN, then from MAX down to CUR (unless CUR
     wraps around first).  All of MIN, MAX, CUR and FDCUR always point
     to even words.  ODD, if non-zero, indicates an odd word to be
     used for the next got entry, otherwise CUR is used and
     incremented by a pair of words, wrapping around when it reaches
     MAX.  FDCUR is decremented (and wrapped) before the next function
     descriptor is chosen.  FDPLT indicates the number of remaining
     slots that can be used for function descriptors used only by PLT
     entries.  */
  struct _k1fdpic_dynamic_got_alloc_data
  {
    bfd_signed_vma max, cur, odd, fdcur, min;
    bfd_vma fdplt;
  } gothilo;
};

bfd_signed_vma
_k1fdpic_get_fd_entry (struct _k1fdpic_dynamic_got_alloc_data *gad);


/* #define elf32_k1_hash_entry(ent) ((struct k1_elf_link_hash_entry *)(ent)) */


/* ELF linker hash table.  */

struct k1_elf_link_hash_table
{
  struct elf_link_hash_table elf;

  /* Short-cuts to get to dynamic linker sections.  */
  asection *sgot;
  asection *sgotplt;
  asection *srelgot;
  asection *splt;
  asection *srelplt;
  asection *sdynbss;
  asection *srelbss;    
  asection *sgotfixup;
  /* A pointer to the .rel.got section.  */
  asection *sgotrel;
  /* A pointer to the .rel.plt section.  */
  asection *spltrel;
  /* GOT base offset.  */
  bfd_vma got0;
  /* Location of the first non-lazy PLT entry, i.e., the number of
     bytes taken by lazy PLT entries.  */
  bfd_vma plt0;

  /* force emitting of GOT when we use GOTOFF and there is not other
   * reference to global offset table */
  bfd_boolean needs_got;

  /* Small local sym to section mapping cache.  */
  struct sym_cache sym_sec;

   /* A hash table holding information about which symbols were
     referenced with which PIC-related relocations.  */
  struct htab *relocs_info;
  /* Summary reloc information collected by
     _k1fdpic_count_got_plt_entries.  */
  struct _k1fdpic_dynamic_got_info *g;

  int bytes_per_rela;
};

/* Get the ELF linker hash table from a link_info structure.  */

#define k1_elf_hash_table(p)                          \
  (elf_hash_table_id ((struct elf_link_hash_table *) ((p)->hash)) \
  == K1_ELF_DATA ? ((struct k1_elf_link_hash_table *) ((p)->hash)) : NULL)

/* The size of an external RELA relocation.  */
#define k1_elf_rela_bytes(htab) \
  ((htab)->bytes_per_rela)

#define k1fdpic_relocs_info(info) \
  (k1_elf_hash_table (info)->relocs_info)
#define k1fdpic_got_section(info) \
  (k1_elf_hash_table (info)->sgot)
#define k1fdpic_gotrel_section(info) \
  (k1_elf_hash_table (info)->sgotrel)
#define k1fdpic_gotfixup_section(info) \
  (k1_elf_hash_table (info)->sgotfixup)
#define k1fdpic_plt_section(info) \
  (k1_elf_hash_table (info)->splt)
#define k1fdpic_pltrel_section(info) \
  (k1_elf_hash_table (info)->spltrel)
#define k1fdpic_got_initial_offset(info) \
  (k1_elf_hash_table (info)->got0)
#define k1fdpic_plt_initial_offset(info) \
  (k1_elf_hash_table (info)->plt0)
#define k1fdpic_dynamic_got_plt_info(info) \
  (k1_elf_hash_table (info)->g)
/* Create an entry in a k1 ELF linker hash table.  */


#define LZPLT_RESOLVER_EXTRA 10
#define LZPLT_NORMAL_SIZE 6
#define LZPLT_ENTRIES 1362

#define K1FDPIC_LZPLT_BLOCK_SIZE ((bfd_vma) LZPLT_NORMAL_SIZE * LZPLT_ENTRIES + LZPLT_RESOLVER_EXTRA)
#define K1FDPIC_LZPLT_RESOLV_LOC (LZPLT_NORMAL_SIZE * LZPLT_ENTRIES / 2)


/* Decide whether a reference to a symbol can be resolved locally or
   not.  If the symbol is protected, we want the local address, but
   its function descriptor must be assigned by the dynamic linker.  */
#define K1FDPIC_SYM_LOCAL(INFO, H) \
  (_bfd_elf_symbol_refs_local_p ((H), (INFO), 1) \
   || ! elf_hash_table (INFO)->dynamic_sections_created)
#define K1FDPIC_FUNCDESC_LOCAL(INFO, H) \
  ((H)->dynindx == -1 || ! elf_hash_table (INFO)->dynamic_sections_created)


/* This structure collects information on what kind of GOT, PLT or
   function descriptors are required by relocations that reference a
   certain symbol.  */
struct k1fdpic_relocs_info
{
  /* The index of the symbol, as stored in the relocation r_info, if
     we have a local symbol; -1 otherwise.  */
  long symndx;
  union
  {
    /* The input bfd in which the symbol is defined, if it's a local
       symbol.  */
    bfd *abfd;
    /* If symndx == -1, the hash table entry corresponding to a global
       symbol (even if it turns out to bind locally, in which case it
       should ideally be replaced with section's symndx + addend).  */
    struct elf_link_hash_entry *h;
  } d;
  /* The addend of the relocation that references the symbol.  */
  bfd_vma addend;

  /* The fields above are used to identify an entry.  The fields below
     contain information on how an entry is used and, later on, which
     locations it was assigned.  */
  /* The following field record whether the symbol+addend above was
     ever referenced with a GOT relocation.  */
  unsigned gothilo;  
  /* Whether a FUNCDESC relocation references symbol+addend.  */
  unsigned fd;
  /* Whether a FUNCDESC_GOT relocation references symbol+addend.  */
  unsigned fdgothilo;  
  /* Whether a FUNCDESC_GOTOFF relocation references symbol+addend.  */
  unsigned fdgotoffhilo;  
  /* Whether symbol+addend is referenced with GOTOFF relocations.
     The addend doesn't really matter, since we
     envision that this will only be used to check whether the symbol
     is mapped to the same segment as the got.  */
  unsigned gotoff;
  /* Whether symbol+addend is referenced by a LABEL24 relocation.  */
  unsigned call;
  /* Whether symbol+addend is referenced by a 32 or FUNCDESC_VALUE
     relocation.  */
  unsigned sym;
  /* Whether we need a PLT entry for a symbol.  Should be implied by
     something like:
     (call && symndx == -1 && ! K1FDPIC_SYM_LOCAL (info, d.h))  */
  unsigned plt:1;
  /* Whether a function descriptor should be created in this link unit
     for symbol+addend.  Should be implied by something like:
     (plt || fdgotoff
      || ((fd || fdgot)
          && (symndx != -1 || K1FDPIC_FUNCDESC_LOCAL (info, d.h))))  */
  unsigned privfd:1;
  /* Whether a lazy PLT entry is needed for this symbol+addend.
     Should be implied by something like:
     (privfd && symndx == -1 && ! K1FDPIC_SYM_LOCAL (info, d.h)
      && ! (info->flags & DF_BIND_NOW))  */
  unsigned lazyplt:1;
  /* Whether we've already emitted GOT relocations and PLT entries as
     needed for this symbol.  */
  unsigned done:1;

  /* The number of R_K1_BYTE4_DATA, R_K1_FUNCDESC and R_K1_FUNCDESC_VALUE
     relocations referencing the symbol.  */
  unsigned relocs32, relocsfd, relocsfdv;

  /* The number of .rofixups entries and dynamic relocations allocated
     for this symbol, minus any that might have already been used.  */
  unsigned fixups, dynrelocs;

  /* The offsets of the GOT entries assigned to symbol+addend, to the
     function descriptor's address, and to a function descriptor,
     respectively.  Should be zero if unassigned.  The offsets are
     counted from the value that will be assigned to the PIC register,
     not from the beginning of the .got section.  */
  bfd_signed_vma got_entry, fdgot_entry, fd_entry;
  /* The offsets of the PLT entries assigned to symbol+addend,
     non-lazy and lazy, respectively.  If unassigned, should be
     (bfd_vma)-1.  */
  bfd_vma plt_entry, lzplt_entry;
};

void
_k1fdpic_count_nontls_entries (struct k1fdpic_relocs_info *entry,
			       struct _k1fdpic_dynamic_got_info *dinfo);
bfd_boolean
k1_create_got_section (bfd *dynobj, struct bfd_link_info *info);

bfd_signed_vma
_k1fdpic_compute_got_alloc_data (struct _k1fdpic_dynamic_got_alloc_data *gad,
                                   bfd_signed_vma fdcur,
                                   bfd_signed_vma odd,
                                   bfd_signed_vma cur,
                                   bfd_vma got,
                                   bfd_vma fd,
                                   bfd_vma fdplt,
				 bfd_vma wrap);
bfd_vma
k1_elf_gp_base (bfd *output_bfd, struct bfd_link_info *info);

struct k1fdpic_relocs_info *
k1fdpic_relocs_info_find (struct htab *ht,
                           bfd *abfd,
                           const struct k1fdpic_relocs_info *entry,
			  enum insert_option insert);

struct k1fdpic_relocs_info *
k1fdpic_relocs_info_for_global (struct htab *ht,
                                 bfd *abfd,
                                 struct elf_link_hash_entry *h,
                                 bfd_vma addend,
				enum insert_option insert);

struct k1fdpic_relocs_info *
k1fdpic_relocs_info_for_local (struct htab *ht,
                                bfd *abfd,
                                long symndx,
                                bfd_vma addend,
			       enum insert_option insert);

bfd_boolean
_k1fdpic_osec_readonly_p (bfd *output_bfd, asection *osec);

bfd_vma
_k1fdpic_add_rofixup (bfd *output_bfd, asection *rofixup, bfd_vma offset,
		      struct k1fdpic_relocs_info *entry);

bfd_vma
k1_elf32_fdpic_add_dyn_reloc (bfd *output_bfd, asection *sreloc, bfd_vma offset,
                         int reloc_type, long dynindx, bfd_vma addend,
			      struct k1fdpic_relocs_info *entry);

struct k1_reloc_map
{
  bfd_reloc_code_real_type bfd_reloc_val;
  unsigned int k1_reloc_val;
};


extern const struct k1_reloc_map k1_reloc_map[];
extern const int k1_reloc_map_len;


#define K1_RELOC_NAME_LOOKUP_DEF(size) \
static reloc_howto_type* k1_elf ## size ## _reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED, const char *r_name){ \
  unsigned int i; \
  for (i = 0; i < k1_reloc_map_len; i++){ \
    if (elf ## size ## _k1_howto_table[i].name != NULL \
        && strcasecmp (elf ## size ## _k1_howto_table[i].name, r_name) == 0){ \
      return &elf ## size ## _k1_howto_table[i]; \
    } \
  } \
  return NULL; \
}

#define K1_RELOC_TYPE_LOOKUP_DEF(size) \
  static reloc_howto_type* k1_elf ## size ##_reloc_type_lookup (bfd * abfd ATTRIBUTE_UNUSED, \
                 bfd_reloc_code_real_type code){ \
  unsigned int i; \
  for (i = 0; i < k1_reloc_map_len; i++){ \
    if (k1_reloc_map[i].bfd_reloc_val == code){ \
      return & elf ## size ##_k1_howto_table[k1_reloc_map[i].k1_reloc_val]; \
    } \
  } \
  return NULL; \
}

#define K1_INFO_TO_HOWTO_DEF(size) \
static void k1_elf ## size ## _info_to_howto (bfd *abfd ATTRIBUTE_UNUSED, \
                                  arelent *cache_ptr,  \
                                  Elf_Internal_Rela *dst){ \
  unsigned int r; \
  r = ELF## size ##_R_TYPE (dst->r_info); \
  BFD_ASSERT (r < (unsigned int) R_K1_max); \
  cache_ptr->howto = &elf ## size ##_k1_howto_table[r]; \
}

bfd_vma k1_gp_base (bfd *output_bfd, struct bfd_link_info *info);
