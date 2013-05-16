#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"
#include "elf-bfd.h"
#include <elf/k1.h>

#define K1_MACH_MASK 0x0000000f

static reloc_howto_type* k1_reloc_type_lookup (bfd *, bfd_reloc_code_real_type);
static reloc_howto_type* k1_reloc_name_lookup (bfd *, const char *);
static void k1_elf_info_to_howto (bfd *, arelent *, Elf_Internal_Rela *);

#define K1DP_K1IO
#include "elf32-k1.def"
#undef K1DP_K1IO

#define DEFAULT_STACK_SIZE 0x20000

struct k1_reloc_map
{
  bfd_reloc_code_real_type bfd_reloc_val;
  unsigned int k1_reloc_val;
};

static const struct k1_reloc_map k1_reloc_map[] =
{
  { BFD_RELOC_NONE,          R_K1_NONE },
  { BFD_RELOC_16,            R_K1_16 },
  { BFD_RELOC_32,            R_K1_32 },
  { BFD_RELOC_K1_17_PCREL,   R_K1_17_PCREL },
  { BFD_RELOC_K1_18_PCREL,   R_K1_18_PCREL },
  { BFD_RELOC_K1_27_PCREL,   R_K1_27_PCREL },
  { BFD_RELOC_K1_32_PCREL,   R_K1_32_PCREL },
  { BFD_RELOC_K1_LO10,       R_K1_LO10 },
  { BFD_RELOC_K1_HI22,       R_K1_HI22 },
  { BFD_RELOC_K1_GPREL_LO10, R_K1_GPREL_LO10 },
  { BFD_RELOC_K1_GPREL_HI22, R_K1_GPREL_HI22 },
  { BFD_RELOC_K1_TPREL_LO10, R_K1_TPREL_LO10 },
  { BFD_RELOC_K1_TPREL_HI22, R_K1_TPREL_HI22 },
  { BFD_RELOC_K1_TPREL_32,   R_K1_TPREL_32 },
  { BFD_RELOC_K1_GOTOFF_LO10,R_K1_GOTOFF_LO10 },
  { BFD_RELOC_K1_GOTOFF_HI22,R_K1_GOTOFF_HI22 },
  { BFD_RELOC_K1_GOT_LO10,   R_K1_GOT_LO10 },
  { BFD_RELOC_K1_GOT_HI22,   R_K1_GOT_HI22 },
  { BFD_RELOC_K1_GLOB_DAT,   R_K1_GLOB_DAT },
  { BFD_RELOC_K1_PLT_LO10,   R_K1_PLT_LO10 },
  { BFD_RELOC_K1_PLT_HI22,   R_K1_PLT_HI22 },
  { BFD_RELOC_K1_FUNCDESC,   R_K1_FUNCDESC },
  { BFD_RELOC_K1_FUNCDESC_GOT_LO10,   R_K1_FUNCDESC_GOT_LO10 },
  { BFD_RELOC_K1_FUNCDESC_GOT_HI22,   R_K1_FUNCDESC_GOT_HI22 },
  { BFD_RELOC_K1_FUNCDESC_GOTOFF_LO10,   R_K1_FUNCDESC_GOTOFF_LO10 },
  { BFD_RELOC_K1_FUNCDESC_GOTOFF_HI22,   R_K1_FUNCDESC_GOTOFF_HI22 },
  { BFD_RELOC_K1_FUNCDESC_VALUE,    R_K1_FUNCDESC_VALUE },
  { BFD_RELOC_K1_GOTOFF,    R_K1_GOTOFF },
  { BFD_RELOC_K1_GOT,    R_K1_GOT },
  { BFD_RELOC_K1_10_GPREL,    R_K1_10_GPREL },
  { BFD_RELOC_K1_16_GPREL,    R_K1_16_GPREL },
/*  { BFD_RELOC_K1_PCREL_LO10, R_K1_PCREL_LO10 },
  { BFD_RELOC_K1_PCREL_HI22, R_K1_PCREL_HI22 }, */
};

static reloc_howto_type* k1_reloc_name_lookup (bfd *abfd ATTRIBUTE_UNUSED, const char *r_name){
  unsigned int i;
  for (i = 0; i < sizeof(k1_reloc_map) / sizeof (struct k1_reloc_map); i++){
    if (elf32_k1_howto_table[i].name != NULL
        && strcasecmp (elf32_k1_howto_table[i].name, r_name) == 0){
      return &elf32_k1_howto_table[i];
    }
  }
  return NULL;
}

static reloc_howto_type* k1_reloc_type_lookup (bfd * abfd ATTRIBUTE_UNUSED,
                                               bfd_reloc_code_real_type code){
  unsigned int i;
  for (i = 0; i < sizeof(k1_reloc_map) / sizeof (struct k1_reloc_map); i++){
    if (k1_reloc_map[i].bfd_reloc_val == code){
      return & elf32_k1_howto_table[k1_reloc_map[i].k1_reloc_val];
    }
  }
  return NULL;
}

static void k1_elf_info_to_howto (bfd *abfd ATTRIBUTE_UNUSED,
                                  arelent *cache_ptr,
                                  Elf_Internal_Rela *dst){
  unsigned int r;
  r = ELF32_R_TYPE (dst->r_info);

  BFD_ASSERT (r < (unsigned int) R_K1_max);

  cache_ptr->howto = &elf32_k1_howto_table[r];
}


/* The k1 linker (like many others) needs to keep track of
   the number of relocs that it decides to copy as dynamic relocs in
   check_relocs for each symbol. This is so that it can later discard
   them if they are found to be unnecessary.  We store the information
   in a field extending the regular ELF linker hash table.  */

struct elf32_k1_dyn_relocs
{
  struct elf32_k1_dyn_relocs *next;

  /* The input section of the reloc.  */
  asection *sec;

  /* Total number of relocs copied for the input section.  */
  bfd_size_type count;

  /* Number of pc-relative relocs copied for the input section.  */
  bfd_size_type pc_count;
};

/* ELF linker hash entry.  */

struct elf32_k1_link_hash_entry
{
  struct elf_link_hash_entry elf;

  /* Track dynamic relocs copied for this symbol.  */
  struct elf32_k1_dyn_relocs *dyn_relocs;

};

#define elf32_k1_hash_entry(ent) ((struct elf32_k1_link_hash_entry *)(ent))

/* ELF linker hash table.  */

struct elf32_k1_link_hash_table
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


  /* Small local sym to section mapping cache.  */
  struct sym_cache sym_sec;

   /* A hash table holding information about which symbols were
     referenced with which PIC-related relocations.  */
  struct htab *relocs_info;
  /* Summary reloc information collected by
     _k1fdpic_count_got_plt_entries.  */
  struct _k1fdpic_dynamic_got_info *g;
};

/* Get the ELF linker hash table from a link_info structure.  */

#define elf32_k1_hash_table(p)                          \
  (elf_hash_table_id ((struct elf_link_hash_table *) ((p)->hash)) \
  == K1_ELF_DATA ? ((struct elf32_k1_link_hash_table *) ((p)->hash)) : NULL)

#define k1fdpic_relocs_info(info) \
  (elf32_k1_hash_table (info)->relocs_info)
#define k1fdpic_got_section(info) \
  (elf32_k1_hash_table (info)->sgot)
#define k1fdpic_gotrel_section(info) \
  (elf32_k1_hash_table (info)->sgotrel)
#define k1fdpic_gotfixup_section(info) \
  (elf32_k1_hash_table (info)->sgotfixup)
#define k1fdpic_plt_section(info) \
  (elf32_k1_hash_table (info)->splt)
#define k1fdpic_pltrel_section(info) \
  (elf32_k1_hash_table (info)->spltrel)
#define k1fdpic_got_initial_offset(info) \
  (elf32_k1_hash_table (info)->got0)
#define k1fdpic_plt_initial_offset(info) \
  (elf32_k1_hash_table (info)->plt0)
#define k1fdpic_dynamic_got_plt_info(info) \
  (elf32_k1_hash_table (info)->g)
/* Create an entry in a k1 ELF linker hash table.  */


#define LZPLT_RESOLVER_EXTRA 10
#define LZPLT_NORMAL_SIZE 6
#define LZPLT_ENTRIES 1362

#define K1FDPIC_LZPLT_BLOCK_SIZE ((bfd_vma) LZPLT_NORMAL_SIZE * LZPLT_ENTRIES + LZPLT_RESOLVER_EXTRA)
#define K1FDPIC_LZPLT_RESOLV_LOC (LZPLT_NORMAL_SIZE * LZPLT_ENTRIES / 2)

static struct bfd_hash_entry *
link_hash_newfunc (struct bfd_hash_entry *entry,
                   struct bfd_hash_table *table,
                   const char *string)
{
  /* Allocate the structure if it has not already been allocated by a
     subclass.  */
  if (entry == NULL)
    {
      entry = bfd_hash_allocate (table,
                                 sizeof (struct elf32_k1_link_hash_entry));
      if (entry == NULL)
        return entry;
    }

  /* Call the allocation method of the superclass.  */
  entry = _bfd_elf_link_hash_newfunc (entry, table, string);
  if (entry != NULL)
    {
      struct elf32_k1_link_hash_entry *eh;

      eh = (struct elf32_k1_link_hash_entry *) entry;
      eh->dyn_relocs = NULL;
    }

  return entry;
}

/* Create a k1 ELF linker hash table.  */

static struct bfd_link_hash_table *
k1_elf_link_hash_table_create (bfd *abfd)
{
  struct elf32_k1_link_hash_table *ret;
  bfd_size_type amt = sizeof (struct elf32_k1_link_hash_table);

  ret = (struct elf32_k1_link_hash_table *) bfd_zmalloc (amt);
  if (ret == NULL)
    return NULL;

  if (!_bfd_elf_link_hash_table_init (&ret->elf, abfd, link_hash_newfunc,
                                      sizeof (struct elf32_k1_link_hash_entry),
                                      K1_ELF_DATA))
    {
      free (ret);
      return NULL;
    }

  return &ret->elf.root;
}


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

/* Compute a hash with the key fields of an k1fdpic_relocs_info entry.  */
static hashval_t
k1fdpic_relocs_info_hash (const void *entry_)
{
  const struct k1fdpic_relocs_info *entry = entry_;

  return (entry->symndx == -1
          ? (long) entry->d.h->root.root.hash
          : entry->symndx + (long) entry->d.abfd->id * 257) + entry->addend;
}

/* Test whether the key fields of two k1fdpic_relocs_info entries are
   identical.  */
static int
k1fdpic_relocs_info_eq (const void *entry1, const void *entry2)
{
  const struct k1fdpic_relocs_info *e1 = entry1;
  const struct k1fdpic_relocs_info *e2 = entry2;

  return e1->symndx == e2->symndx && e1->addend == e2->addend
    && (e1->symndx == -1 ? e1->d.h == e2->d.h : e1->d.abfd == e2->d.abfd);
}

/* Find or create an entry in a hash table HT that matches the key
   fields of the given ENTRY.  If it's not found, memory for a new
   entry is allocated in ABFD's obstack.  */
static struct k1fdpic_relocs_info *
k1fdpic_relocs_info_find (struct htab *ht,
                           bfd *abfd,
                           const struct k1fdpic_relocs_info *entry,
                           enum insert_option insert)
{
  struct k1fdpic_relocs_info **loc;

  if (!ht)
    return NULL;

  loc = (struct k1fdpic_relocs_info **) htab_find_slot (ht, entry, insert);

  if (! loc)
    return NULL;

  if (*loc)
    return *loc;

  *loc = bfd_zalloc (abfd, sizeof (**loc));

  if (! *loc)
    return *loc;

  (*loc)->symndx = entry->symndx;
  (*loc)->d = entry->d;
  (*loc)->addend = entry->addend;
  (*loc)->plt_entry = (bfd_vma)-1;
  (*loc)->lzplt_entry = (bfd_vma)-1;

  return *loc;
}

/* Obtain the address of the entry in HT associated with H's symbol +
   addend, creating a new entry if none existed.  ABFD is only used
   for memory allocation purposes.  */
inline static struct k1fdpic_relocs_info *
k1fdpic_relocs_info_for_global (struct htab *ht,
                                 bfd *abfd,
                                 struct elf_link_hash_entry *h,
                                 bfd_vma addend,
                                 enum insert_option insert)
{
  struct k1fdpic_relocs_info entry;

  entry.symndx = -1;
  entry.d.h = h;
  entry.addend = addend;

  return k1fdpic_relocs_info_find (ht, abfd, &entry, insert);

}

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



/* Obtain the address of the entry in HT associated with the SYMNDXth
   local symbol of the input bfd ABFD, plus the addend, creating a new
   entry if none existed.  */
inline static struct k1fdpic_relocs_info *
k1fdpic_relocs_info_for_local (struct htab *ht,
                                bfd *abfd,
                                long symndx,
                                bfd_vma addend,
                                enum insert_option insert)
{
  struct k1fdpic_relocs_info entry;

  entry.symndx = symndx;
  entry.d.abfd = abfd;
  entry.addend = addend;

  return k1fdpic_relocs_info_find (ht, abfd, &entry, insert);
}

/* Merge fields set by check_relocs() of two entries that end up being
   mapped to the same (presumably global) symbol.  */

inline static void
k1fdpic_pic_merge_early_relocs_info (struct k1fdpic_relocs_info *e2,
                                     struct k1fdpic_relocs_info const *e1)
{
  e2->gothilo |= e1->gothilo;
  e2->fd |= e1->fd;
  e2->fdgothilo |= e1->fdgothilo;
  e2->fdgotoffhilo |= e1->fdgotoffhilo;
  e2->gotoff |= e1->gotoff;
  e2->call |= e1->call;
  e2->sym |= e1->sym;
}


/* Compute the total GOT size required by each symbol in each range.
   Symbols may require up to 4 words in the GOT: an entry pointing to
   the symbol, an entry pointing to its function descriptor, and a
   private function descriptors taking two words.  */

static void
_k1fdpic_count_nontls_entries (struct k1fdpic_relocs_info *entry,
                                 struct _k1fdpic_dynamic_got_info *dinfo)
{
  /* Allocate space for a GOT entry pointing to the symbol.  */
  if (entry->gothilo)
    dinfo->gothilo += 4;
  else
    entry->relocs32--;
  entry->relocs32++;

  /* Allocate space for a GOT entry pointing to the function
     descriptor.  */
  if (entry->fdgothilo)
    dinfo->gothilo += 4;
  else
    entry->relocsfd--;
  entry->relocsfd++;

  /* Decide whether we need a PLT entry, a function descriptor in the
     GOT, and a lazy PLT entry for this symbol.  */
  entry->plt = entry->call
    && entry->symndx == -1 && ! K1FDPIC_SYM_LOCAL (dinfo->info, entry->d.h)
    && elf_hash_table (dinfo->info)->dynamic_sections_created;
  entry->privfd = entry->plt
    || entry->fdgotoffhilo
    || ((entry->fd || entry->fdgothilo)
        && (entry->symndx != -1
            || K1FDPIC_FUNCDESC_LOCAL (dinfo->info, entry->d.h)));

    
/* TODO: NO LAZY FOR NOW
  entry->lazyplt = entry->privfd
    && entry->symndx == -1 && ! K1FDPIC_SYM_LOCAL (dinfo->info, entry->d.h)
    && ! (dinfo->info->flags & DF_BIND_NOW)
    && elf_hash_table (dinfo->info)->dynamic_sections_created;
    */
  entry->lazyplt = 0;

  /* Allocate space for a function descriptor.  */
//   if (entry->fdgotoffhilo)
//     dinfo->fdhilo += 8;
  if (entry->privfd && entry->plt)
    dinfo->fdplt += 8;
  else if (entry->privfd)
    dinfo->fdhilo += 8;
  else
    entry->relocsfdv--;
  entry->relocsfdv++;

  if (entry->lazyplt)
    dinfo->lzplt += LZPLT_NORMAL_SIZE;

}



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



/* Compute the number of dynamic relocations and fixups that a symbol
   requires, and add (or subtract) from the grand and per-symbol
   totals.  */

static void
_k1fdpic_count_relocs_fixups (struct k1fdpic_relocs_info *entry,
                                struct _k1fdpic_dynamic_got_info *dinfo,
                                bfd_boolean subtract)
{
  bfd_vma relocs = 0, fixups = 0;

  if (!dinfo->info->executable || dinfo->info->pie)
    relocs = entry->relocs32 + entry->relocsfd + entry->relocsfdv;
  else
    {
      if (entry->symndx != -1 || K1FDPIC_SYM_LOCAL (dinfo->info, entry->d.h))
        {
          if (entry->symndx != -1
              || entry->d.h->root.type != bfd_link_hash_undefweak)
            fixups += entry->relocs32 + 2 * entry->relocsfdv;
        }
      else
        relocs += entry->relocs32 + entry->relocsfdv;

      if (entry->symndx != -1
          || K1FDPIC_FUNCDESC_LOCAL (dinfo->info, entry->d.h))
        {
          if (entry->symndx != -1
              || entry->d.h->root.type != bfd_link_hash_undefweak)
            fixups += entry->relocsfd;
        }
      else
        relocs += entry->relocsfd;
    }

  if (subtract)
    {
      relocs = - relocs;
      fixups = - fixups;
    }

  entry->dynrelocs += relocs;
  entry->fixups += fixups;
  dinfo->relocs += relocs;
  dinfo->fixups += fixups;
}


/* Compute the total GOT and PLT size required by each symbol in each range. *
   Symbols may require up to 4 words in the GOT: an entry pointing to
   the symbol, an entry pointing to its function descriptor, and a
   private function descriptors taking two words.  */

static int
_k1fdpic_count_got_plt_entries (void **entryp, void *dinfo_)
{
  struct k1fdpic_relocs_info *entry = *entryp;
  struct _k1fdpic_dynamic_got_info *dinfo = dinfo_;

  _k1fdpic_count_nontls_entries (entry, dinfo);

  _k1fdpic_count_relocs_fixups (entry, dinfo, FALSE);

  return 1;
}


/* Add a dynamic relocation to the SRELOC section.  */

inline static bfd_vma
_k1fdpic_add_dyn_reloc (bfd *output_bfd, asection *sreloc, bfd_vma offset,
                         int reloc_type, long dynindx, bfd_vma addend,
                         struct k1fdpic_relocs_info *entry)
{
  Elf_Internal_Rela outrel;
  bfd_vma reloc_offset;

  outrel.r_offset = offset;
  outrel.r_info = ELF32_R_INFO (dynindx, reloc_type);
  outrel.r_addend = addend;

  reloc_offset = sreloc->reloc_count * sizeof (Elf32_External_Rel);
  BFD_ASSERT (reloc_offset < sreloc->size);
  bfd_elf32_swap_reloc_out (output_bfd, &outrel,
                            sreloc->contents + reloc_offset);
  sreloc->reloc_count++;

  /* If the entry's index is zero, this relocation was probably to a
     linkonce section that got discarded.  We reserved a dynamic
     relocation, but it was for another entry than the one we got at
     the time of emitting the relocation.  Unfortunately there's no
     simple way for us to catch this situation, since the relocation
     is cleared right before calling relocate_section, at which point
     we no longer know what the relocation used to point to.  */
  if (entry->symndx)
    {
      BFD_ASSERT (entry->dynrelocs > 0);
      entry->dynrelocs--;
    }

  return reloc_offset;
}

/* Add a fixup to the ROFIXUP section.  */

static bfd_vma
_k1fdpic_add_rofixup (bfd *output_bfd, asection *rofixup, bfd_vma offset,
                       struct k1fdpic_relocs_info *entry)
{
  bfd_vma fixup_offset;

  if (rofixup->flags & SEC_EXCLUDE)
    return -1;
  

  fixup_offset = rofixup->reloc_count * 4;
  if (rofixup->contents)
    {
      BFD_ASSERT (fixup_offset < rofixup->size);
      bfd_put_32 (output_bfd, offset, rofixup->contents + fixup_offset);
      
    }
  rofixup->reloc_count++;

  if (entry && entry->symndx)
    {
      /* See discussion about symndx == 0 in _k1fdpic_add_dyn_reloc
         above.  */
      BFD_ASSERT (entry->fixups > 0);
      entry->fixups--;
    }

  return fixup_offset;
}

/* Follow indirect and warning hash entries so that each got entry
   points to the final symbol definition.  P must point to a pointer
   to the hash table we're traversing.  Since this traversal may
   modify the hash table, we set this pointer to NULL to indicate
   we've made a potentially-destructive change to the hash table, so
   the traversal must be restarted.  */
static int
_k1fdpic_resolve_final_relocs_info (void **entryp, void *p)
{
  struct k1fdpic_relocs_info *entry = *entryp;
  htab_t *htab = p;

  if (entry->symndx == -1)
    {
      struct elf_link_hash_entry *h = entry->d.h;
      struct k1fdpic_relocs_info *oentry;

      while (h->root.type == bfd_link_hash_indirect
             || h->root.type == bfd_link_hash_warning)
        h = (struct elf_link_hash_entry *)h->root.u.i.link;

      if (entry->d.h == h)
        return 1;

      oentry = k1fdpic_relocs_info_for_global (*htab, 0, h, entry->addend,
                                                NO_INSERT);

      if (oentry)
        {
          /* Merge the two entries.  */
          k1fdpic_pic_merge_early_relocs_info (oentry, entry);
          htab_clear_slot (*htab, entryp);
          return 1;
        }

      entry->d.h = h;

      /* If we can't find this entry with the new bfd hash, re-insert
         it, and get the traversal restarted.  */
      if (! htab_find (*htab, entry))
        {
          htab_clear_slot (*htab, entryp);
          entryp = htab_find_slot (*htab, entry, INSERT);
          if (! *entryp)
            *entryp = entry;
          /* Abort the traversal, since the whole table may have
             moved, and leave it up to the parent to restart the
             process.  */
          *(htab_t *)p = NULL;
          return 0;
        }
    }

  return 1;
}


static bfd_boolean
elf32_k1_is_target_special_symbol (bfd * abfd ATTRIBUTE_UNUSED, asymbol * sym)
{
  return sym->name && sym->name[0] == '.' && sym->name[1] == 'L';
}


static bfd_vma elf32_k1_gp_base (bfd *output_bfd, struct bfd_link_info *info)
{
    bfd_vma res = _bfd_get_gp_value (output_bfd);
    struct elf_link_hash_entry *gp;
    bfd_vma gp_val = (bfd_vma) -1;

    /* Point GP at _data_start symbol */
    
    if (res) return res;

    gp = elf_link_hash_lookup (elf_hash_table (info), "_data_start", FALSE,
                                 FALSE, FALSE);
    if (gp)
      gp_val = gp->root.u.def.value;

    _bfd_set_gp_value (output_bfd, gp_val);
    return gp_val;
}

/* The name of the dynamic interpreter.  This is put in the .interp
   section.  */

#define ELF_DYNAMIC_INTERPRETER "/lib/ld.so"

/* The size in bytes of an entry in the procedure linkage table */
#define PLT_HEADER_SIZE         32
#define PLT_MIN_ENTRY_SIZE      16
#define PLT_FULL_ENTRY_SIZE     16


/* PLT templates for (FD)PIC ABI */

// static const bfd_vma plt_min_entry[PLT_MIN_ENTRY_SIZE / 4] =
//   {
//     /* goto .PLT0             */ 0x31000000,
//     /* mov $r0.9=0            */ 0x08000240,
//     /* imml 0              ;; */ 0x95000000,
//     /* nop                 ;; */ NOP_BUNDLE
//   };

static const bfd_vma fdpic_abi_plt_full_entry[PLT_FULL_ENTRY_SIZE] =
  {
    /* add $r20 = $r14, 0 (0x0) ;; */ 0x6250000e,
    /* lw $r21 = 0 (0x0)[$r20]  ;; */ 0x24540014,
    /* lw $r14 = 4 (0x4)[$r20]  ;; */ 0x24380114,
    /* igoto $r21               ;; */ 0x00114015,
  };


/* Functions for dealing with the e_flags field.  */

/* Merge backend specific data from an object file to the output
   object file when linking.  */

static bfd_boolean
elf_k1_merge_private_bfd_data (bfd *ibfd, bfd *obfd)
{
  unsigned out_mach, in_mach;
  flagword out_flags, in_flags;

  /* Check if we have the same endianess.  */
  if (!_bfd_generic_verify_endian_match (ibfd, obfd))
    return FALSE;

  /* Don't even pretend to support mixed-format linking.  */
  if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour
      || bfd_get_flavour (obfd) != bfd_target_elf_flavour)
    return FALSE;

  out_flags = elf_elfheader (obfd)->e_flags;
  in_flags = elf_elfheader (ibfd)->e_flags;

  if (!elf_flags_init (obfd)) {
#if 0
    /* [JV] I took this from ARM but I do not understand it... */
    /* If the input is the default architecture and had the default
       flags then do not bother setting the flags for the output
       architecture, instead allow future merges to do this.  If no
       future merges ever set these flags then they will retain their
       uninitialised values, which surprise surprise, correspond
       to the default values.  */
    if (bfd_get_arch_info (ibfd)->the_default
	&& elf_elfheader (ibfd)->e_flags == 0)
      return TRUE;
#endif

    elf_flags_init (obfd) = TRUE;
    elf_elfheader (obfd)->e_flags = in_flags;
    
    if (bfd_get_arch (obfd) == bfd_get_arch (ibfd)
	&& bfd_get_arch_info (obfd)->the_default)
      return bfd_set_arch_mach (obfd, bfd_get_arch (ibfd), bfd_get_mach (ibfd));
    
    return TRUE;
  }
  
  out_mach = out_flags & K1_MACH_MASK;
  in_mach = in_flags & K1_MACH_MASK;
  if (out_mach != in_mach) {
    (*_bfd_error_handler)
      (_("%B: incompatible machine type. Output is 0x%x. Input is 0x%x"),
       ibfd, out_mach, in_mach);
    bfd_set_error (bfd_error_wrong_format);
    return FALSE;
  }

  return TRUE;
}


static bfd_boolean
elf_k1_set_private_flags (bfd *abfd, flagword flags)
{
  BFD_ASSERT (!elf_flags_init (abfd)
	      || elf_elfheader (abfd)->e_flags == flags);

  elf_elfheader (abfd)->e_flags |= flags;
  elf_flags_init (abfd) = TRUE;

  return TRUE;
}

static bfd_boolean
elf_k1_print_private_bfd_data (bfd *abfd, void *farg)
{
  FILE *f = (FILE *) farg;
  flagword e_flags = elf_elfheader (abfd)->e_flags;

  fprintf (f, "\nK1 header:\n");
  switch(e_flags & K1_MACH_MASK) {
  case bfd_mach_k1dp:
    fprintf (f, "\nMachine     = k1dp\n");
    break;
  case bfd_mach_k1io:
    fprintf (f, "\nMachine     = k1io\n");
    break;
  default:
    fprintf (f, "\nMachine Id  = 0x%x\n", e_flags & K1_MACH_MASK);
  }

  return _bfd_elf_print_private_bfd_data (abfd, farg);
}

/* The final processing done just before writing out an K1 ELF object
   file.  This gets the K1 architecture right based on the machine
   number.  */

static void
elf_k1_final_write_processing (bfd *abfd,
				   bfd_boolean linker ATTRIBUTE_UNUSED)
{
  int mach;
  unsigned long val;

  switch (mach = bfd_get_mach (abfd))
    {
    case bfd_mach_k1dp:
    case bfd_mach_k1io:
      val = mach;
      break;
    default:
      return;
    }

  elf_elfheader (abfd)->e_flags &=  (~ K1_MACH_MASK);
  elf_elfheader (abfd)->e_flags |= val;
}

/* Set the right machine number for an K1 ELF file.  */

static bfd_boolean
elf_k1_object_p (bfd *abfd)
{
  int mach = elf_elfheader (abfd)->e_flags & K1_MACH_MASK;

  switch (mach) {
    case bfd_mach_k1dp:
    case bfd_mach_k1io:
      break;
    default:
      return FALSE;
    }

  (void) bfd_default_set_arch_mach (abfd, bfd_arch_k1, mach);
  return TRUE;
}


/* Find the segment number in which OSEC, and output section, is
   located.  */

static unsigned
_k1fdpic_osec_to_segment (bfd *output_bfd, asection *osec)
{
  Elf_Internal_Phdr *p = _bfd_elf_find_segment_containing_section (output_bfd, osec);

  return (p != NULL) ? p - elf_tdata (output_bfd)->phdr : -1;
}

inline static bfd_boolean
_k1fdpic_osec_readonly_p (bfd *output_bfd, asection *osec)
{
  unsigned seg = _k1fdpic_osec_to_segment (output_bfd, osec);

  return ! (elf_tdata (output_bfd)->phdr[seg].p_flags & PF_W);
}


/* Generate relocations for GOT entries, function descriptors, and
   code for PLT and lazy PLT entries.  */

inline static bfd_boolean
_k1fdpic_emit_got_relocs_plt_entries (struct k1fdpic_relocs_info *entry,
                                        bfd *output_bfd,
                                        struct bfd_link_info *info,
                                        asection *sec,
                                        Elf_Internal_Sym *sym,
                                        bfd_vma addend)

{
  bfd_vma fd_lazy_rel_offset = (bfd_vma)-1;
  int dynindx = -1;

  if (entry->done)
    return TRUE;
  entry->done = 1;

  if (entry->got_entry || entry->fdgot_entry || entry->fd_entry)
    {
      /* If the symbol is dynamic, consider it for dynamic
         relocations, otherwise decay to section + offset.  */
      if (entry->symndx == -1 && entry->d.h->dynindx != -1)
        dynindx = entry->d.h->dynindx;
      else
        {
          if (sec
              && sec->output_section
              && ! bfd_is_abs_section (sec->output_section)
              && ! bfd_is_und_section (sec->output_section))
            dynindx = elf_section_data (sec->output_section)->dynindx;
          else
            dynindx = 0;
        }
    }

  /* Generate relocation for GOT entry pointing to the symbol.  */
  if (entry->got_entry)
    {
      int idx = dynindx;
      bfd_vma ad = addend;

      /* If the symbol is dynamic but binds locally, use
         section+offset.  */
      if (sec && (entry->symndx != -1
                  || K1FDPIC_SYM_LOCAL (info, entry->d.h)))
        {
          if (entry->symndx == -1)
            ad += entry->d.h->root.u.def.value;
          else
            ad += sym->st_value;
          ad += sec->output_offset;
          if (sec->output_section && elf_section_data (sec->output_section))
            idx = elf_section_data (sec->output_section)->dynindx;
          else
            idx = 0;
        }

      /* If we're linking an executable at a fixed address, we can
         omit the dynamic relocation as long as the symbol is local to
         this module.  */
      if (info->executable && !info->pie
          && (entry->symndx != -1
              || K1FDPIC_SYM_LOCAL (info, entry->d.h)))
        {
          if (sec)
            ad += sec->output_section->vma;
          if (entry->symndx != -1
              || entry->d.h->root.type != bfd_link_hash_undefweak) {
            _k1fdpic_add_rofixup (output_bfd,
                                   k1fdpic_gotfixup_section (info),
                                   k1fdpic_got_section (info)->output_section->vma
                                   + k1fdpic_got_section (info)->output_offset
                                   + k1fdpic_got_initial_offset (info)
                                   + entry->got_entry, entry);
          }
        }
      else
        _k1fdpic_add_dyn_reloc (output_bfd, k1fdpic_gotrel_section (info),
                                 _bfd_elf_section_offset
                                 (output_bfd, info,
                                  k1fdpic_got_section (info),
                                  k1fdpic_got_initial_offset (info)
                                  + entry->got_entry)
                                 + k1fdpic_got_section (info)
                                 ->output_section->vma
                                 + k1fdpic_got_section (info)->output_offset,
                                 R_K1_GLOB_DAT, idx, ad, entry);

      bfd_put_32 (output_bfd, ad,
                  k1fdpic_got_section (info)->contents
                  + k1fdpic_got_initial_offset (info)
                  + entry->got_entry);
    }

  /* Generate relocation for GOT entry pointing to a canonical
     function descriptor.  */
  if (entry->fdgot_entry)
    {
      int reloc, idx;
      bfd_vma ad = 0;

      if (! (entry->symndx == -1
             && entry->d.h->root.type == bfd_link_hash_undefweak
             && K1FDPIC_SYM_LOCAL (info, entry->d.h)))
        {
          /* If the symbol is dynamic and there may be dynamic symbol
             resolution because we are, or are linked with, a shared
             library, emit a FUNCDESC relocation such that the dynamic
             linker will allocate the function descriptor.  If the
             symbol needs a non-local function descriptor but binds
             locally (e.g., its visibility is protected, emit a
             dynamic relocation decayed to section+offset.  */
          if (entry->symndx == -1
              && ! K1FDPIC_FUNCDESC_LOCAL (info, entry->d.h)
              && K1FDPIC_SYM_LOCAL (info, entry->d.h)
              && !(info->executable && !info->pie))
            {
              reloc = R_K1_FUNCDESC;
              idx = elf_section_data (entry->d.h->root.u.def.section
                                      ->output_section)->dynindx;
              ad = entry->d.h->root.u.def.section->output_offset
                + entry->d.h->root.u.def.value;
            }
          else if (entry->symndx == -1
                   && ! K1FDPIC_FUNCDESC_LOCAL (info, entry->d.h))
            {
              reloc = R_K1_FUNCDESC;
              idx = dynindx;
              ad = addend;
              if (ad)
                return FALSE;
            }
          else
            {
              /* Otherwise, we know we have a private function descriptor,
                 so reference it directly.  */
              if (elf_hash_table (info)->dynamic_sections_created)
                BFD_ASSERT (entry->privfd);
              reloc = R_K1_GLOB_DAT;
              idx = elf_section_data (k1fdpic_got_section (info)
                                      ->output_section)->dynindx;
              ad = k1fdpic_got_section (info)->output_offset
                + k1fdpic_got_initial_offset (info) + entry->fd_entry;
            }

          /* If there is room for dynamic symbol resolution, emit the
             dynamic relocation.  However, if we're linking an
             executable at a fixed location, we won't have emitted a
             dynamic symbol entry for the got section, so idx will be
             zero, which means we can and should compute the address
             of the private descriptor ourselves.  */
          if (info->executable && !info->pie
              && (entry->symndx != -1
                  || K1FDPIC_FUNCDESC_LOCAL (info, entry->d.h)))
            {
              ad += k1fdpic_got_section (info)->output_section->vma;
              _k1fdpic_add_rofixup (output_bfd,
                                     k1fdpic_gotfixup_section (info),
                                     k1fdpic_got_section (info)
                                     ->output_section->vma
                                     + k1fdpic_got_section (info)
                                     ->output_offset
                                     + k1fdpic_got_initial_offset (info)
                                     + entry->fdgot_entry, entry);
            }
          else
            _k1fdpic_add_dyn_reloc (output_bfd,
                                     k1fdpic_gotrel_section (info),
                                     _bfd_elf_section_offset
                                     (output_bfd, info,
                                      k1fdpic_got_section (info),
                                      k1fdpic_got_initial_offset (info)
                                      + entry->fdgot_entry)
                                     + k1fdpic_got_section (info)
                                     ->output_section->vma
                                     + k1fdpic_got_section (info)
                                     ->output_offset,
                                     reloc, idx, ad, entry);
        }

      bfd_put_32 (output_bfd, ad,
                  k1fdpic_got_section (info)->contents
                  + k1fdpic_got_initial_offset (info)
                  + entry->fdgot_entry);
    }

  /* Generate relocation to fill in a private function descriptor in
     the GOT.  */
  if (entry->fd_entry)
    {
      int idx = dynindx;
      bfd_vma ad = addend;
      bfd_vma ofst;
      long lowword, highword;

      /* If the symbol is dynamic but binds locally, use
         section+offset.  */
      if (sec && (entry->symndx != -1
                  || K1FDPIC_SYM_LOCAL (info, entry->d.h)))
        {
          if (entry->symndx == -1)
            ad += entry->d.h->root.u.def.value;
          else
            ad += sym->st_value;
          ad += sec->output_offset;
          if (sec->output_section && elf_section_data (sec->output_section))
            idx = elf_section_data (sec->output_section)->dynindx;
          else
            idx = 0;
        }

      /* If we're linking an executable at a fixed address, we can
         omit the dynamic relocation as long as the symbol is local to
         this module.  */
      if (info->executable && !info->pie
          && (entry->symndx != -1 || K1FDPIC_SYM_LOCAL (info, entry->d.h)))
        {
          if (sec)
            ad += sec->output_section->vma;
          ofst = 0;
          if (entry->symndx != -1
              || entry->d.h->root.type != bfd_link_hash_undefweak)
            {
              _k1fdpic_add_rofixup (output_bfd,
                                     k1fdpic_gotfixup_section (info),
                                     k1fdpic_got_section (info)
                                     ->output_section->vma
                                     + k1fdpic_got_section (info)
                                     ->output_offset
                                     + k1fdpic_got_initial_offset (info)
                                     + entry->fd_entry, entry);
              _k1fdpic_add_rofixup (output_bfd,
                                     k1fdpic_gotfixup_section (info),
                                     k1fdpic_got_section (info)
                                     ->output_section->vma
                                     + k1fdpic_got_section (info)
                                     ->output_offset
                                     + k1fdpic_got_initial_offset (info)
                                     + entry->fd_entry + 4, entry);
            }
        }
      else
        {
          ofst
            = _k1fdpic_add_dyn_reloc (output_bfd,
                                        entry->lazyplt
                                        ? k1fdpic_pltrel_section (info)
                                        : k1fdpic_gotrel_section (info),
                                        _bfd_elf_section_offset
                                        (output_bfd, info,
                                         k1fdpic_got_section (info),
                                         k1fdpic_got_initial_offset (info)
                                         + entry->fd_entry)
                                        + k1fdpic_got_section (info)
                                        ->output_section->vma
                                        + k1fdpic_got_section (info)
                                        ->output_offset,
                                        R_K1_FUNCDESC_VALUE, idx, ad, entry);
        }

      /* If we've omitted the dynamic relocation, just emit the fixed
         addresses of the symbol and of the local GOT base offset.  */
      if (info->executable && !info->pie && sec && sec->output_section)
        {
          lowword = ad;
          highword = k1fdpic_got_section (info)->output_section->vma
            + k1fdpic_got_section (info)->output_offset
            + k1fdpic_got_initial_offset (info);
        }
      else if (entry->lazyplt)
        {
          if (ad)
            return FALSE;

          fd_lazy_rel_offset = ofst;

          /* A function descriptor used for lazy or local resolving is
             initialized such that its high word contains the output
             section index in which the PLT entries are located, and
             the low word contains the address of the lazy PLT entry
             entry point, that must be within the memory region
             assigned to that section.  */
          lowword = entry->lzplt_entry + 4
            + k1fdpic_plt_section (info)->output_offset
            + k1fdpic_plt_section (info)->output_section->vma;
          highword = _k1fdpic_osec_to_segment
            (output_bfd, k1fdpic_plt_section (info)->output_section);
        }
      else
        {
          /* A function descriptor for a local function gets the index
             of the section.  For a non-local function, it's
             disregarded.  */
          lowword = ad;
          if (sec == NULL
              || (entry->symndx == -1 && entry->d.h->dynindx != -1
                  && entry->d.h->dynindx == idx))
            highword = 0;
          else
            highword = _k1fdpic_osec_to_segment
              (output_bfd, sec->output_section);
        }

      bfd_put_32 (output_bfd, lowword,
                  k1fdpic_got_section (info)->contents
                  + k1fdpic_got_initial_offset (info)
                  + entry->fd_entry);
      bfd_put_32 (output_bfd, highword,
                  k1fdpic_got_section (info)->contents
                  + k1fdpic_got_initial_offset (info)
                  + entry->fd_entry + 4);
    }

  /* Generate code for the PLT entry.  */
  if (entry->plt_entry != (bfd_vma) -1)
    {
      abort ();
//       bfd_byte *plt_code = k1fdpic_plt_section (info)->contents
//         + entry->plt_entry;
// 
//       BFD_ASSERT (entry->fd_entry);
// 
//       /* Figure out what kind of PLT entry we need, depending on the
//          location of the function descriptor within the GOT.  */
//       if (entry->fd_entry >= -(1 << (18 - 1))
//           && entry->fd_entry + 4 < (1 << (18 - 1)))
//         {
//           /* P1 = [P3 + fd_entry]; P3 = [P3 + fd_entry + 4] */
//           bfd_put_32 (output_bfd,
//                       0xe519 | ((entry->fd_entry << 14) & 0xFFFF0000),
//                       plt_code);
//           bfd_put_32 (output_bfd,
//                       0xe51b | (((entry->fd_entry + 4) << 14) & 0xFFFF0000),
//                       plt_code + 4);
//           plt_code += 8;
//         }
//       else
//         {
//           /* P1.L = fd_entry; P1.H = fd_entry;
//              P3 = P3 + P1;
//              P1 = [P3];
//              P3 = [P3 + 4];  */
//           bfd_put_32 (output_bfd,
//                       0xe109 | (entry->fd_entry << 16),
//                       plt_code);
//           bfd_put_32 (output_bfd,
//                       0xe149 | (entry->fd_entry & 0xFFFF0000),
//                       plt_code + 4);
//           bfd_put_16 (output_bfd, 0x5ad9, plt_code + 8);
//           bfd_put_16 (output_bfd, 0x9159, plt_code + 10);
//           bfd_put_16 (output_bfd, 0xac5b, plt_code + 12);
//           plt_code += 14;
//         }
//       /* JUMP (P1) */
//       bfd_put_16 (output_bfd, 0x0051, plt_code);
    }

  /* Generate code for the lazy PLT entry.  */
  if (entry->lzplt_entry != (bfd_vma) -1)
    {
      abort ();
//       bfd_byte *lzplt_code = k1fdpic_plt_section (info)->contents
//         + entry->lzplt_entry;
//       bfd_vma resolverStub_addr;
// 
//       bfd_put_32 (output_bfd, fd_lazy_rel_offset, lzplt_code);
//       lzplt_code += 4;

//       resolverStub_addr = entry->lzplt_entry / K1FDPIC_LZPLT_BLOCK_SIZE
//         * K1FDPIC_LZPLT_BLOCK_SIZE + K1FDPIC_LZPLT_RESOLV_LOC;
//       if (resolverStub_addr >= k1fdpic_plt_initial_offset (info))
//         resolverStub_addr = k1fdpic_plt_initial_offset (info) - LZPLT_NORMAL_SIZE - LZPLT_RESOLVER_EXTRA;

//       if (entry->lzplt_entry == resolverStub_addr)
//         {
//           /* This is a lazy PLT entry that includes a resolver call.
//              P2 = [P3];
//              R3 = [P3 + 4];
//              JUMP (P2);  */
//           bfd_put_32 (output_bfd,
//                       0xa05b915a,
//                       lzplt_code);
//           bfd_put_16 (output_bfd, 0x0052, lzplt_code + 4);
//         }
//       else
//         {
//           /* JUMP.S  resolverStub */
//           bfd_put_16 (output_bfd,
//                       0x2000
//                       | (((resolverStub_addr - entry->lzplt_entry)
//                           / 2) & (((bfd_vma)1 << 12) - 1)),
//                       lzplt_code);
//         }
    }

  return TRUE;
}


/* Copied from elf32-mt.c as this implementation seemd the most clean,
   simple and generic. */
static bfd_boolean
k1_elf_relocate_section
    (bfd *                   output_bfd ATTRIBUTE_UNUSED,
     struct bfd_link_info *  info,
     bfd *                   input_bfd,
     asection *              input_section,
     bfd_byte *              contents,
     Elf_Internal_Rela *     relocs,
     Elf_Internal_Sym *      local_syms,
     asection **             local_sections)
{  
  Elf_Internal_Shdr *           symtab_hdr;
  struct elf_link_hash_entry ** sym_hashes;  
  Elf_Internal_Rela *           rel;
  Elf_Internal_Rela *           relend;
  struct elf32_k1_link_hash_table *htab;
  bfd *dynobj;
  bfd_vma *local_got_offsets;

  unsigned isec_segment, got_segment, plt_segment,
    check_segment[2];
  /*int silence_segment_error = !(info->shared || info->pie);*/


  isec_segment = _k1fdpic_osec_to_segment (output_bfd,
                                             input_section->output_section);
  if (k1fdpic_got_section (info))
    got_segment = _k1fdpic_osec_to_segment (output_bfd,
                                              k1fdpic_got_section (info)
                                              ->output_section);
  else
    got_segment = -1;
  
  if (elf_hash_table (info)->dynamic_sections_created)
    plt_segment = _k1fdpic_osec_to_segment (output_bfd,
                                              k1fdpic_plt_section (info)
                                              ->output_section);
  else
    plt_segment = -1;

  

  symtab_hdr = & elf_tdata (input_bfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (input_bfd);
  relend     = relocs + input_section->reloc_count;
  dynobj     = elf_hash_table (info)->dynobj;    
  
  local_got_offsets = elf_local_got_offsets (input_bfd);
  htab = elf32_k1_hash_table (info);

  if (htab == NULL)
    return FALSE;

  for (rel = relocs; rel < relend; rel ++)
    {
      reloc_howto_type             *howto;
      unsigned long                r_symndx;
      Elf_Internal_Sym             *sym;
      asection                     *sec;
      struct elf_link_hash_entry   *h;
      bfd_vma                      relocation;
      bfd_reloc_status_type        r;
      const char                   *name = NULL;
      int                          r_type;
      asection                     *osec;
      struct k1fdpic_relocs_info   *picrel;
      
      bfd_vma orig_addend = rel->r_addend;
      
      r_type = ELF32_R_TYPE (rel->r_info);

      r_symndx = ELF32_R_SYM (rel->r_info);

      howto  = elf32_k1_howto_table + ELF32_R_TYPE (rel->r_info);
      h      = NULL;
      sym    = NULL;
      sec    = NULL;
      
      if (!input_section->use_rela_p) {
	  bfd_byte *where = contents + rel->r_offset;

	  switch (howto->size) {
	  case 0:
	      rel->r_addend = bfd_get_8 (input_bfd, where);
	      rel->r_addend = (rel->r_addend ^ 0x80) - 0x80;
	      break;
	  case 1:
	      rel->r_addend = bfd_get_16 (input_bfd, where);
	      rel->r_addend = (rel->r_addend ^ 0x8000) - 0x8000;
	      break;
	  case 2:
	      rel->r_addend = bfd_get_32 (input_bfd, where);
	      rel->r_addend = (rel->r_addend ^ 0x80000000) - 0x80000000;
	      break;
	  default:
	      abort ();
	  }
      }

      if (r_symndx < symtab_hdr->sh_info)
	{
	  sym = local_syms + r_symndx;
	  osec = sec = local_sections [r_symndx];

	  name = bfd_elf_string_from_elf_section
	    (input_bfd, symtab_hdr->sh_link, sym->st_name);
	  name = (name == NULL) ? bfd_section_name (input_bfd, sec) : name;

	  relocation = _bfd_elf_rela_local_sym (output_bfd, sym, &sec, rel);
	}
      else
	{
	  bfd_boolean unresolved_reloc;
	  bfd_boolean warned;

	  RELOC_FOR_GLOBAL_SYMBOL (info, input_bfd, input_section, rel,
				   r_symndx, symtab_hdr, sym_hashes,
				   h, sec, relocation,
				   unresolved_reloc, warned);

	  name = h->root.root.string;
          osec = sec;
	}

      if (sec != NULL && elf_discarded_section (sec))
	{
	    RELOC_AGAINST_DISCARDED_SECTION (info, input_bfd, input_section, rel,
					     relend, howto, contents);
	    continue;
	}

      if (info->relocatable)
	continue;

      if (h != NULL
          && (h->root.type == bfd_link_hash_defined
              || h->root.type == bfd_link_hash_defweak)
          && !K1FDPIC_SYM_LOCAL (info, h))
        {
          osec = sec = NULL;
          relocation = 0;
        }

      /* Finally, the sole K1-specific part.  */
      switch (r_type)
        {
	    /* Handle K1 specific things here */
	case R_K1_TPREL_LO10:
	case R_K1_TPREL_HI22:
	case R_K1_TPREL_32:
	    relocation -=  elf_hash_table (info)->tls_sec->vma;
	    break;
	case R_K1_10_GPREL:
	case R_K1_16_GPREL:
	case R_K1_GPREL_LO10:
	case R_K1_GPREL_HI22:
	    relocation -=  elf32_k1_gp_base (output_bfd, info);
	    break;

        case R_K1_27_PCREL:
	case R_K1_GOT:
        case R_K1_GOT_LO10:
        case R_K1_GOT_HI22:
	case R_K1_GOTOFF:
        case R_K1_GOTOFF_LO10:
        case R_K1_GOTOFF_HI22:
        case R_K1_FUNCDESC:
	case R_K1_FUNCDESC_GOT_LO10:
        case R_K1_FUNCDESC_GOT_HI22:
        case R_K1_FUNCDESC_GOTOFF_LO10:
        case R_K1_FUNCDESC_GOTOFF_HI22:
        case R_K1_FUNCDESC_VALUE:
        case R_K1_PLT_LO10:
        case R_K1_PLT_HI22:
        case R_K1_GLOB_DAT:
           if (h != NULL)
            picrel = k1fdpic_relocs_info_for_global (k1fdpic_relocs_info
                                                       (info), input_bfd, h,
                                                       orig_addend, INSERT);
          else
            /* In order to find the entry we created before, we must
               use the original addend, not the one that may have been
               modified by _bfd_elf_rela_local_sym().  */
            picrel = k1fdpic_relocs_info_for_local (k1fdpic_relocs_info
                                                      (info), input_bfd, r_symndx,
                                                      orig_addend, INSERT);
          if (! picrel)
            return FALSE;

          if (!_k1fdpic_emit_got_relocs_plt_entries (picrel, output_bfd, info,
                                                       osec, sym,
                                                       rel->r_addend))
            {
              (*_bfd_error_handler)
                (_("%B: relocation at `%A+0x%x' references symbol `%s' with nonzero addend"),
                 input_bfd, input_section, rel->r_offset, name);
              return FALSE;

            }
            break;
        default:
          picrel = NULL;
          if (h && ! K1FDPIC_SYM_LOCAL (info, h)
            && _bfd_elf_section_offset (output_bfd, info, input_section,
              rel->r_offset) != (bfd_vma) -1)
            {
              info->callbacks->warning
                (info, _("relocation references symbol not defined in the module"),
                 name, input_bfd, input_section, rel->r_offset);
              return FALSE;
            }
          break;
        }

        switch (r_type)
        {
          case R_K1_27_PCREL:
	    check_segment[0] = isec_segment;
            if (picrel->plt)
            {
              relocation = k1fdpic_plt_section (info)->output_section->vma
                + k1fdpic_plt_section (info)->output_offset
                + picrel->plt_entry;
              check_segment[1] = plt_segment;
            }
          /* We don't want to warn on calls to undefined weak symbols,
             as calls to them must be protected by non-NULL tests
             anyway, and unprotected calls would invoke undefined
             behavior.  */
            else if (picrel->symndx == -1
                   && picrel->d.h->root.type == bfd_link_hash_undefweak)
              check_segment[1] = check_segment[0];
            else
              check_segment[1] = sec
                ? _k1fdpic_osec_to_segment (output_bfd, sec->output_section)
                : (unsigned)-1;
            break;
	  case R_K1_GOT:
	    relocation = k1fdpic_got_section (info)->output_section->vma
			  + picrel->got_entry;
	    check_segment[0] = check_segment[1] = got_segment;
	    break;
          case R_K1_GOT_HI22:
          case R_K1_GOT_LO10:	  
            relocation = picrel->got_entry;
            check_segment[0] = check_segment[1] = got_segment;
            break;

	  case R_K1_FUNCDESC_GOT_HI22:
	  case R_K1_FUNCDESC_GOT_LO10:
	    relocation = picrel->fdgot_entry;
	    check_segment[0] = check_segment[1] = got_segment;
	    break;
          case R_K1_GOTOFF_LO10:
          case R_K1_GOTOFF_HI22:
	  case R_K1_GOTOFF:
            relocation -= k1fdpic_got_section (info)->output_section->vma
              + k1fdpic_got_section (info)->output_offset
              + k1fdpic_got_initial_offset (info);
            check_segment[0] = got_segment;
            check_segment[1] = sec
              ? _k1fdpic_osec_to_segment (output_bfd, sec->output_section)
              : (unsigned)-1;
            break;
	  case R_K1_FUNCDESC_GOTOFF_LO10:
	  case R_K1_FUNCDESC_GOTOFF_HI22:
	    relocation = picrel->fd_entry;
	    check_segment[0] = check_segment[1] = got_segment;
	  break;
          case R_K1_FUNCDESC:
            {
              int dynindx;
              bfd_vma addend = rel->r_addend;

              if (! (h && h->root.type == bfd_link_hash_undefweak
                    && K1FDPIC_SYM_LOCAL (info, h)))
                {
                /* If the symbol is dynamic and there may be dynamic
                   symbol resolution because we are or are linked with a
                   shared library, emit a FUNCDESC relocation such that
                   the dynamic linker will allocate the function
                   descriptor.  If the symbol needs a non-local function
                   descriptor but binds locally (e.g., its visibility is
                   protected, emit a dynamic relocation decayed to
                   section+offset.  */
                  if (h && ! K1FDPIC_FUNCDESC_LOCAL (info, h)
                      && K1FDPIC_SYM_LOCAL (info, h)
                      && !(info->executable && !info->pie))
                    {
                      dynindx = elf_section_data (h->root.u.def.section
                                                  ->output_section)->dynindx;
                      addend += h->root.u.def.section->output_offset
                        + h->root.u.def.value;
                    }
                  else if (h && ! K1FDPIC_FUNCDESC_LOCAL (info, h))
                    {
                      if (addend)
                        {
                          info->callbacks->warning
                            (info, _("R_K1_FUNCDESC references dynamic symbol with nonzero addend"),
                            name, input_bfd, input_section, rel->r_offset);
                          return FALSE;
                        }
                      dynindx = h->dynindx;
                    }
                  else
                    {                      
                      /* Otherwise, we know we have a private function
                        descriptor, so reference it directly.  */
                      BFD_ASSERT (picrel->privfd);
                      r_type = R_K1_GLOB_DAT;// R_K1_BYTE4_DATA;
                      dynindx = elf_section_data (k1fdpic_got_section (info)
                                                  ->output_section)->dynindx;
                      addend = k1fdpic_got_section (info)->output_offset
                        + k1fdpic_got_initial_offset (info)
                        + picrel->fd_entry;
                    }

                  /* If there is room for dynamic symbol resolution, emit
                    the dynamic relocation.  However, if we're linking an
                    executable at a fixed location, we won't have emitted a
                    dynamic symbol entry for the got section, so idx will
                    be zero, which means we can and should compute the
                    address of the private descriptor ourselves.  */
                  if (info->executable && !info->pie
                      && (!h || K1FDPIC_FUNCDESC_LOCAL (info, h)))
                    {
                      bfd_vma offset;
  
                      addend += k1fdpic_got_section (info)->output_section->vma;
                      if ((bfd_get_section_flags (output_bfd,
                                                  input_section->output_section)
                          & (SEC_ALLOC | SEC_LOAD)) == (SEC_ALLOC | SEC_LOAD))
                        {
                          if (_k1fdpic_osec_readonly_p (output_bfd,
                                                        input_section
                                                        ->output_section))
                            {
                              info->callbacks->warning
                                (info,
                                _("cannot emit fixups in read-only section"),
                                name, input_bfd, input_section, rel->r_offset);
                              return FALSE;
                            }
  
                          offset = _bfd_elf_section_offset
                            (output_bfd, info,
                            input_section, rel->r_offset);

                          if (offset != (bfd_vma)-1) {
                            _k1fdpic_add_rofixup (output_bfd,
                                                    k1fdpic_gotfixup_section
                                                    (info),
                                                    offset + input_section
                                                    ->output_section->vma
                                                    + input_section->output_offset,
                                                    picrel);
                          }
                        }
                    }
                  else if ((bfd_get_section_flags (output_bfd,
                                                  input_section->output_section)
                            & (SEC_ALLOC | SEC_LOAD)) == (SEC_ALLOC | SEC_LOAD))
                    {
                      bfd_vma offset;

                      if (_k1fdpic_osec_readonly_p (output_bfd,
                                                    input_section
                                                    ->output_section))
                        {
                          info->callbacks->warning
                            (info,
                            _("cannot emit dynamic relocations in read-only section"),
                            name, input_bfd, input_section, rel->r_offset);
                          return FALSE;
                        }
                      offset = _bfd_elf_section_offset (output_bfd, info,
                                                        input_section, rel->r_offset);
  
                      if (offset != (bfd_vma)-1)
                        _k1fdpic_add_dyn_reloc (output_bfd,
                                                  k1fdpic_gotrel_section (info),
                                                  offset + input_section
                                                  ->output_section->vma
                                                  + input_section->output_offset,
                                                  r_type,
                                                  dynindx, addend, picrel);
                    }
                  else
                    addend += k1fdpic_got_section (info)->output_section->vma;
                }
  
              /* We want the addend in-place because dynamic
                relocations are REL.  Setting relocation to it should
                arrange for it to be installed.  */
              relocation = addend - rel->r_addend;
              }
            check_segment[0] = check_segment[1] = got_segment;
            break;
          case R_K1_GLOB_DAT:
          case R_K1_FUNCDESC_VALUE:
             {
            int dynindx;
            bfd_vma addend = rel->r_addend;
            bfd_vma offset;
            offset = _bfd_elf_section_offset (output_bfd, info,
                                              input_section, rel->r_offset);

            /* If the symbol is dynamic but binds locally, use
               section+offset.  */
            if (h && ! K1FDPIC_SYM_LOCAL (info, h))
              {
                if (addend && r_type == R_K1_FUNCDESC_VALUE)
                  {
                    info->callbacks->warning
                      (info, _("R_K1_FUNCDESC_VALUE references dynamic symbol with nonzero addend"),
                       name, input_bfd, input_section, rel->r_offset);
                    return FALSE;
                  }
                dynindx = h->dynindx;
              }
            else
              {
                if (h)
                  addend += h->root.u.def.value;
                else
                  addend += sym->st_value;
                if (osec)
                  addend += osec->output_offset;
                if (osec && osec->output_section
                    && ! bfd_is_abs_section (osec->output_section)
                    && ! bfd_is_und_section (osec->output_section))
                  dynindx = elf_section_data (osec->output_section)->dynindx;
                else
                  dynindx = 0;
              }

            /* If we're linking an executable at a fixed address, we
               can omit the dynamic relocation as long as the symbol
               is defined in the current link unit (which is implied
               by its output section not being NULL).  */
            if (info->executable && !info->pie
                && (!h || K1FDPIC_SYM_LOCAL (info, h)))
              {
                if (osec)
                  addend += osec->output_section->vma;
                if (/*IS_FDPIC (input_bfd)
                    && */(bfd_get_section_flags (output_bfd,
                                               input_section->output_section)
                        & (SEC_ALLOC | SEC_LOAD)) == (SEC_ALLOC | SEC_LOAD))
                  {
                    if (_k1fdpic_osec_readonly_p (output_bfd,
                                                   input_section
                                                   ->output_section))
                      {
                        info->callbacks->warning
                          (info,
                           _("cannot emit fixups in read-only section"),
                           name, input_bfd, input_section, rel->r_offset);
                        return FALSE;
                      }
                    if (!h || h->root.type != bfd_link_hash_undefweak)
                      {
                        if (offset != (bfd_vma)-1)
                          {
                            _k1fdpic_add_rofixup (output_bfd,
                                                    k1fdpic_gotfixup_section
                                                    (info),
                                                    offset + input_section
                                                    ->output_section->vma
                                                    + input_section->output_offset,
                                                    picrel);

                            if (r_type == R_K1_FUNCDESC_VALUE) {
                              _k1fdpic_add_rofixup
                                (output_bfd,
                                 k1fdpic_gotfixup_section (info),
                                 offset + input_section->output_section->vma
                                 + input_section->output_offset + 4, picrel);
                            }
                          }
                      }
                  }
              }
            else
              {
                if ((bfd_get_section_flags (output_bfd,
                                            input_section->output_section)
                     & (SEC_ALLOC | SEC_LOAD)) == (SEC_ALLOC | SEC_LOAD))
                  {
                    if (_k1fdpic_osec_readonly_p (output_bfd,
                                                   input_section
                                                   ->output_section))
                      {
                        info->callbacks->warning
                          (info,
                           _("cannot emit dynamic relocations in read-only section"),
                           name, input_bfd, input_section, rel->r_offset);
                        return FALSE;
                      }

                    if (offset != (bfd_vma)-1)
                      _k1fdpic_add_dyn_reloc (output_bfd,
                                                k1fdpic_gotrel_section (info),
                                                offset
                                                + input_section->output_section->vma
                                                + input_section->output_offset,
                                                r_type, dynindx, addend, picrel);
                  }
                else if (osec)
                  addend += osec->output_section->vma;
                /* We want the addend in-place because dynamic
                   relocations are REL.  Setting relocation to it
                   should arrange for it to be installed.  */
                relocation = addend - rel->r_addend;
              }

            if (r_type == R_K1_FUNCDESC_VALUE)
              {
                /* If we've omitted the dynamic relocation, just emit
                   the fixed addresses of the symbol and of the local
                   GOT base offset.  */
                if (info->executable && !info->pie
                    && (!h || K1FDPIC_SYM_LOCAL (info, h)))
                  bfd_put_32 (output_bfd,
                              k1fdpic_got_section (info)->output_section->vma
                              + k1fdpic_got_section (info)->output_offset
                              + k1fdpic_got_initial_offset (info),
                              contents + rel->r_offset + 4);
                else
                  /* A function descriptor used for lazy or local
                     resolving is initialized such that its high word
                     contains the output section index in which the
                     PLT entries are located, and the low word
                     contains the offset of the lazy PLT entry entry
                     point into that section.  */
                  bfd_put_32 (output_bfd,
                              h && ! K1FDPIC_SYM_LOCAL (info, h)
                              ? 0
                              : _k1fdpic_osec_to_segment (output_bfd,
                                                            sec
                                                            ->output_section),
                              contents + rel->r_offset + 4);
              }
          }
          check_segment[0] = check_segment[1] = got_segment;
          break;

        default:
          check_segment[0] = isec_segment;
          check_segment[1] = sec
            ? _k1fdpic_osec_to_segment (output_bfd, sec->output_section)
            : (unsigned)-1;
          break;

        }

#if 0
        if (check_segment[0] != check_segment[1] && IS_FDPIC (output_bfd))
        {
#if 1 /* If you take this out, remove the #error from fdpic-static-6.d
         in the ld testsuite.  */
          /* This helps catch problems in GCC while we can't do more
             than static linking.  The idea is to test whether the
             input file basename is crt0.o only once.  */
          if (silence_segment_error == 1)
            silence_segment_error =
              (strlen (input_bfd->filename) == 6
               && strcmp (input_bfd->filename, "crt0.o") == 0)
              || (strlen (input_bfd->filename) > 6
                  && strcmp (input_bfd->filename
                             + strlen (input_bfd->filename) - 7,
                             "/crt0.o") == 0)
              ? -1 : 0;
#endif
          if (!silence_segment_error
              /* We don't want duplicate errors for undefined
                 symbols.  */
              && !(picrel && picrel->symndx == -1
                   && picrel->d.h->root.type == bfd_link_hash_undefined))
            info->callbacks->warning
              (info,
               (info->shared || info->pie)
               ? _("relocations between different segments are not supported")
               : _("warning: relocation references a different segment"),
               name, input_bfd, input_section, rel->r_offset);
          if (!silence_segment_error && (info->shared || info->pie))
            return FALSE;
          elf_elfheader (output_bfd)->e_flags |= EF_K1_PIC;
        }
#endif//0

      switch (r_type)
        {
        case R_K1_27_PCREL:
        if (/*! IS_FDPIC (output_bfd) ||*/ ! picrel->plt)
            break;
          /* Fall through.  */

          /* When referencing a GOT entry, a function descriptor or a
             PLT, we don't want the addend to apply to the reference,
             but rather to the referenced symbol.  The actual entry
             will have already been created taking the addend into
             account, so cancel it out here.  */
	case R_K1_GOT:
        case R_K1_GOT_LO10:
        case R_K1_GOT_HI22:
	case R_K1_GOTOFF:
        case R_K1_GOTOFF_LO10:
        case R_K1_GOTOFF_HI22:
//         case R_K1_FUNCDESC:
	case R_K1_FUNCDESC_GOT_LO10:
        case R_K1_FUNCDESC_GOT_HI22:
        case R_K1_FUNCDESC_GOTOFF_LO10:
        case R_K1_FUNCDESC_GOTOFF_HI22:
        case R_K1_FUNCDESC_VALUE:
        case R_K1_PLT_LO10:
        case R_K1_PLT_HI22:
          relocation -= rel->r_addend;
          break;

        default:
          break;
        }

      /* Generic relocation */
      r = _bfd_final_link_relocate (howto, input_bfd, input_section,
                                    contents, rel->r_offset,
                                    relocation, rel->r_addend);

      if (r != bfd_reloc_ok)
	{
	  const char * msg = (const char *) NULL;

	  switch (r)
	    {
	    case bfd_reloc_overflow:
	      r = info->callbacks->reloc_overflow
		(info, (h ? &h->root : NULL), name, howto->name, (bfd_vma) 0,
		 input_bfd, input_section, rel->r_offset);
	      break;
	      
	    case bfd_reloc_undefined:
	      r = info->callbacks->undefined_symbol
		(info, name, input_bfd, input_section, rel->r_offset, TRUE);
	      break;
	      
	    case bfd_reloc_outofrange:
	      msg = _("internal error: out of range error");
	      break;

	    case bfd_reloc_dangerous:
	      msg = _("internal error: dangerous relocation");
	      break;

	    default:
	      msg = _("internal error: unknown error");
	      break;
	    }

	  if (msg)
	    r = info->callbacks->warning
	      (info, msg, name, input_bfd, input_section, rel->r_offset);

	  if (! r)
	    return FALSE;
	}
    }

  return TRUE;
}

/* Update the relocation information for the relocations of the section
   being removed.  */

static bfd_boolean
k1fdpic_gc_sweep_hook (bfd *abfd,
			 struct bfd_link_info *info,
			 asection *sec,
			 const Elf_Internal_Rela *relocs)
{
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes, **sym_hashes_end;
  const Elf_Internal_Rela *rel;
  const Elf_Internal_Rela *rel_end;
  struct k1fdpic_relocs_info *picrel;

//   BFD_ASSERT (IS_FDPIC (abfd));

  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (abfd);
  sym_hashes_end = sym_hashes + symtab_hdr->sh_size/sizeof(Elf32_External_Sym);
  if (!elf_bad_symtab (abfd))
    sym_hashes_end -= symtab_hdr->sh_info;

  rel_end = relocs + sec->reloc_count;
  for (rel = relocs; rel < rel_end; rel++)
    {
      struct elf_link_hash_entry *h;
      unsigned long r_symndx;

      r_symndx = ELF32_R_SYM (rel->r_info);
      if (r_symndx < symtab_hdr->sh_info)
        h = NULL;
      else
        h = sym_hashes[r_symndx - symtab_hdr->sh_info];

      if (h != NULL)
	picrel = k1fdpic_relocs_info_for_global (k1fdpic_relocs_info (info),
						   abfd, h,
						   rel->r_addend, NO_INSERT);
      else
	picrel = k1fdpic_relocs_info_for_local (k1fdpic_relocs_info
						  (info), abfd, r_symndx,
						  rel->r_addend, NO_INSERT);

      if (!picrel)
	return TRUE;

      switch (ELF32_R_TYPE (rel->r_info))
        {
	case R_K1_27_PCREL:	
	  picrel->call--;
	  break;

	case R_K1_FUNCDESC_VALUE:
	  picrel->relocsfdv--;
	  if (bfd_get_section_flags (abfd, sec) & SEC_ALLOC)
	    picrel->relocs32++;
	  /* Fall through.  */

	case R_K1_GLOB_DAT:
	  picrel->sym--;
	  if (bfd_get_section_flags (abfd, sec) & SEC_ALLOC)
	    picrel->relocs32--;
	  break;

	case R_K1_GOT:
	case R_K1_GOT_HI22:
	case R_K1_GOT_LO10:
	  picrel->gothilo--;
	  break;

	case R_K1_FUNCDESC_GOT_HI22:
	case R_K1_FUNCDESC_GOT_LO10:
	  picrel->fdgothilo--;
	  break;

	case R_K1_GOTOFF:
	case R_K1_GOTOFF_HI22:
	case R_K1_GOTOFF_LO10:
	  picrel->gotoff--;
	  break;

	case R_K1_FUNCDESC_GOTOFF_HI22:
	case R_K1_FUNCDESC_GOTOFF_LO10:
	  picrel->fdgotoffhilo--;
	  break;

	case R_K1_FUNCDESC:
	  picrel->fd--;
	  picrel->relocsfd--;
	  break;

	default:
	  break;
        }
    }

  return TRUE;
}

/* We need dynamic symbols for every section, since segments can
   relocate independently.  */
static bfd_boolean
_k1fdpic_link_omit_section_dynsym (bfd *output_bfd ATTRIBUTE_UNUSED,
                                    struct bfd_link_info *info ATTRIBUTE_UNUSED,
                                    asection *p)
{
  switch (elf_section_data (p)->this_hdr.sh_type)
    {
    case SHT_PROGBITS:
    case SHT_NOBITS:
      /* If sh_type is yet undecided, assume it could be
         SHT_PROGBITS/SHT_NOBITS.  */
    case SHT_NULL:
      return FALSE;

      /* There shouldn't be section relative relocations
         against any other section.  */
    default:
      return TRUE;
    }
}

/* Create .got, .gotplt, and .rela.got sections in DYNOBJ, and set up
   shortcuts to them in our hash table.  */

static bfd_boolean
k1_create_got_section (bfd *abfd, struct bfd_link_info *info)
{
  flagword flags, pltflags;
  asection *s;
  struct elf_link_hash_entry *h;
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  int ptralign;


  /* This function may be called more than once.  */
  s = bfd_get_section_by_name (abfd, ".got");
  if (s != NULL && (s->flags & SEC_LINKER_CREATED) != 0)
    return TRUE;

  /* Machine specific: although pointers are 32-bits wide, we want the
     GOT to be aligned to a 64-bit boundary, such that function
     descriptors in it can be accessed with 64-bit loads and
     stores.  */
  ptralign = 3;

  flags = (SEC_ALLOC | SEC_LOAD | SEC_HAS_CONTENTS | SEC_IN_MEMORY
           | SEC_LINKER_CREATED);
  pltflags = flags;

  s = bfd_make_section_with_flags (abfd, ".got", flags);
  if (s == NULL
      || !bfd_set_section_alignment (abfd, s, ptralign))
    return FALSE;

  if (bed->want_got_plt)
    {
      s = bfd_make_section_with_flags (abfd, ".got.plt", flags);
      if (s == NULL
          || !bfd_set_section_alignment (abfd, s, ptralign))
        return FALSE;
    }

  if (bed->want_got_sym)
    {
      /* Define the symbol _GLOBAL_OFFSET_TABLE_ at the start of the .got
         (or .got.plt) section.  We don't do this in the linker script
         because we don't want to define the symbol if we are not creating
         a global offset table.  */
      h = _bfd_elf_define_linkage_sym (abfd, info, s, "_GLOBAL_OFFSET_TABLE_");
      elf_hash_table (info)->hgot = h;
      if (h == NULL)
        return FALSE;

      /* Machine-specific: we want the symbol for executables as
         well.  */
      if (! bfd_elf_link_record_dynamic_symbol (info, h))
        return FALSE;
    }

  /* The first bit of the global offset table is the header.  */
      s->size += bed->got_header_size;

  /* This is the machine-specific part.  Create and initialize section
     data for the got.  */
//   if (IS_FDPIC (abfd))
//     {
      k1fdpic_got_section (info) = s;
      k1fdpic_relocs_info (info) = htab_try_create (1,
                                                      k1fdpic_relocs_info_hash,
                                                      k1fdpic_relocs_info_eq,
                                                      (htab_del) NULL);
      if (! k1fdpic_relocs_info (info))
        return FALSE;

      s = bfd_make_section_with_flags (abfd, ".rel.got",
                                       (flags | SEC_READONLY));
      if (s == NULL
          || ! bfd_set_section_alignment (abfd, s, 2))
        return FALSE;

      k1fdpic_gotrel_section (info) = s;

      /* Machine-specific.  */
      s = bfd_make_section_with_flags (abfd, ".rofixup",
                                       (flags | SEC_READONLY));
      if (s == NULL
          || ! bfd_set_section_alignment (abfd, s, 2))
        return FALSE;

      k1fdpic_gotfixup_section (info) = s;
//     }

  pltflags |= SEC_CODE;
  if (bed->plt_not_loaded)
    pltflags &= ~ (SEC_CODE | SEC_LOAD | SEC_HAS_CONTENTS);
  if (bed->plt_readonly)
    pltflags |= SEC_READONLY;

  s = bfd_make_section_with_flags (abfd, ".plt", pltflags);
  if (s == NULL
      || ! bfd_set_section_alignment (abfd, s, bed->plt_alignment))
    return FALSE;
  /* K1-specific: remember it.  */
  k1fdpic_plt_section (info) = s;

  if (bed->want_plt_sym)
    {
      /* Define the symbol _PROCEDURE_LINKAGE_TABLE_ at the start of the
         .plt section.  */
      struct bfd_link_hash_entry *bh = NULL;

      if (! (_bfd_generic_link_add_one_symbol
             (info, abfd, "_PROCEDURE_LINKAGE_TABLE_", BSF_GLOBAL, s, 0, NULL,
              FALSE, get_elf_backend_data (abfd)->collect, &bh)))
        return FALSE;
      h = (struct elf_link_hash_entry *) bh;
      h->def_regular = 1;
      h->type = STT_OBJECT;

      if (! info->executable
          && ! bfd_elf_link_record_dynamic_symbol (info, h))
        return FALSE;
    }

  /* K1-specific: we want rel relocations for the plt.  */
  s = bfd_make_section_with_flags (abfd, ".rel.plt", flags | SEC_READONLY);
  if (s == NULL
      || ! bfd_set_section_alignment (abfd, s, bed->s->log_file_align))
    return FALSE;
  /* K1-specific: remember it.  */
  k1fdpic_pltrel_section (info) = s;

  return TRUE;
}


/* Look through the relocs for a section during the first phase.  */

static bfd_boolean
k1_check_relocs (bfd * abfd,
                 struct bfd_link_info *info,
                 asection *sec,
                 const Elf_Internal_Rela *relocs)
{
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  const Elf_Internal_Rela *rel;
  const Elf_Internal_Rela *rel_end;
  bfd *dynobj;
  struct k1fdpic_relocs_info *picrel;  
  
  if (info->relocatable)
    return TRUE;

  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (abfd);

  dynobj = elf_hash_table (info)->dynobj;
  rel_end = relocs + sec->reloc_count;
  for (rel = relocs; rel < rel_end; rel++)
    {
      struct elf_link_hash_entry *h;
      unsigned long r_symndx;

      r_symndx = ELF32_R_SYM (rel->r_info);
      if (r_symndx < symtab_hdr->sh_info)
        h = NULL;
      else
        h = sym_hashes[r_symndx - symtab_hdr->sh_info];
      switch (ELF32_R_TYPE (rel->r_info))
        {
	case R_K1_GOT:
        case R_K1_GOT_LO10:
        case R_K1_GOT_HI22:
	case R_K1_GOTOFF:
        case R_K1_GOTOFF_LO10:
        case R_K1_GOTOFF_HI22:
        case R_K1_PLT_LO10:
        case R_K1_PLT_HI22:
        case R_K1_FUNCDESC:
        case R_K1_FUNCDESC_VALUE:
        case R_K1_FUNCDESC_GOTOFF_LO10:
        case R_K1_FUNCDESC_GOTOFF_HI22:
	case R_K1_FUNCDESC_GOT_LO10:
        case R_K1_FUNCDESC_GOT_HI22:
        case R_K1_27_PCREL:
        case R_K1_10_GPREL:
        case R_K1_16_GPREL:
        case R_K1_GPREL_HI22:
        case R_K1_GPREL_LO10:
        case R_K1_GLOB_DAT:
//           if (! IS_FDPIC (abfd))
//             goto bad_reloc;
          /* Fall through.  */
//         case R_K1_PCREL24:
//         case R_K1_PCREL24_JUMP_L:
//         case R_K1_BYTE4_DATA:
          if (/*IS_FDPIC (abfd) && */! dynobj)
            {
              elf_hash_table (info)->dynobj = dynobj = abfd;
              if (! k1_create_got_section (abfd, info))
                return FALSE;
            }

          if (h != NULL)
            {
              if (h->dynindx == -1)
                switch (ELF_ST_VISIBILITY (h->other))
                  {
                  case STV_INTERNAL:
                  case STV_HIDDEN:
                    break;
                  default:
                    bfd_elf_link_record_dynamic_symbol (info, h);
                    break;
                  }
              picrel
                = k1fdpic_relocs_info_for_global (k1fdpic_relocs_info (info),
                                                   abfd, h,
                                                   rel->r_addend, INSERT);
            }
          else {
            picrel = k1fdpic_relocs_info_for_local (k1fdpic_relocs_info
                                                     (info), abfd, r_symndx,
                                                     rel->r_addend, INSERT);
          }
          
          if (! picrel)
            return FALSE;
          break;

        default:
          picrel = NULL;
          break;
        }

      switch (ELF32_R_TYPE (rel->r_info))
        {
        case R_K1_27_PCREL:        
//           if (IS_FDPIC (abfd))
            picrel->call++;
          break;

        case R_K1_FUNCDESC_VALUE:
          picrel->relocsfdv++;
          if (bfd_get_section_flags (abfd, sec) & SEC_ALLOC)
            picrel->relocs32--;
            
          /* Fall through.  */

        case R_K1_GLOB_DAT:
//           if (! IS_FDPIC (abfd))
//             break;

          picrel->sym++;
          if (bfd_get_section_flags (abfd, sec) & SEC_ALLOC)
            picrel->relocs32++;
          break;

	case R_K1_GOT:
        case R_K1_GOT_LO10:
        case R_K1_GOT_HI22:
          picrel->gothilo++;
          break;

	case R_K1_FUNCDESC_GOT_HI22:
	case R_K1_FUNCDESC_GOT_LO10:
	  picrel->fdgothilo++;
	  break;
  
        case R_K1_PLT_LO10:
        case R_K1_PLT_HI22:
          break;

	case R_K1_GOTOFF:
        case R_K1_GOTOFF_LO10:
        case R_K1_GOTOFF_HI22:
          picrel->gotoff++;
          break;

        case R_K1_FUNCDESC_GOTOFF_LO10:
        case R_K1_FUNCDESC_GOTOFF_HI22:
          picrel->fdgotoffhilo++;
          break;

        case R_K1_FUNCDESC:
          picrel->fd++;
          picrel->relocsfd++;
          break;

        /* This relocation describes the C++ object vtable hierarchy.
           Reconstruct it for later use during GC.  */
//         case R_K1_GNU_VTINHERIT:
//           if (!bfd_elf_gc_record_vtinherit (abfd, sec, h, rel->r_offset))
//             return FALSE;
//           break;

        /* This relocation describes which C++ vtable entries are actually
           used.  Record for later use during GC.  */
//         case R_K1_GNU_VTENTRY:
//           BFD_ASSERT (h != NULL);
//           if (h != NULL
//               && !bfd_elf_gc_record_vtentry (abfd, sec, h, rel->r_addend))
//             return FALSE;
//           break;

        case R_K1_16:
        case R_K1_32:
        case R_K1_17_PCREL:
        case R_K1_18_PCREL:
        case R_K1_32_PCREL:
        case R_K1_LO10:
        case R_K1_HI22:
        case R_K1_10_GPREL:
        case R_K1_16_GPREL:
        case R_K1_GPREL_HI22:
        case R_K1_GPREL_LO10:
        case R_K1_TPREL_32:
        case R_K1_TPREL_HI22:
        case R_K1_TPREL_LO10:
        break;

        default:
/*
        bad_reloc:
*/
          (*_bfd_error_handler)
            (_("%B: unsupported relocation type %i"),
             abfd, ELF32_R_TYPE (rel->r_info));
          return FALSE;
        break;
        }
    }
    
  return TRUE;
}

static bfd_boolean
k1_elf_create_dynamic_sections (bfd *abfd, struct bfd_link_info *info)
{
#if 0
  struct elf32_k1_link_hash_table *htab = elf32_k1_hash_table (info);

  if (htab == NULL)
    return FALSE;

  if (!htab->sgot && !create_got_section (dynobj, info))
    return FALSE;

  if (!_bfd_elf_create_dynamic_sections (dynobj, info))
    return FALSE;

  htab->splt = bfd_get_section_by_name (dynobj, ".plt");
  htab->srelplt = bfd_get_section_by_name (dynobj, ".rela.plt");
  htab->sdynbss = bfd_get_section_by_name (dynobj, ".dynbss");
  if (!info->shared)
    htab->srelbss = bfd_get_section_by_name (dynobj, ".rela.bss");

  if (!htab->splt || !htab->srelplt || !htab->sdynbss
      || (!info->shared && !htab->srelbss))
    abort ();

  return TRUE;
#endif

   /* This is mostly copied from
     elflink.c:_bfd_elf_create_dynamic_sections().  */
  flagword flags;
  asection *s;
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);

  flags = (SEC_ALLOC | SEC_LOAD | SEC_HAS_CONTENTS | SEC_IN_MEMORY
           | SEC_LINKER_CREATED);

  /* We need to create .plt, .rel[a].plt, .got, .got.plt, .dynbss, and
     .rel[a].bss sections.  */

  /* K1-specific: we want to create the GOT in the K1 way.  */
  if (! k1_create_got_section (abfd, info))
    return FALSE;

  /* K1-specific: make sure we created everything we wanted.  */
  BFD_ASSERT (k1fdpic_got_section (info) && k1fdpic_gotrel_section (info)
              /* && k1fdpic_gotfixup_section (info) */
              && k1fdpic_plt_section (info)
              && k1fdpic_pltrel_section (info));

  if (bed->want_dynbss)
    {
      /* The .dynbss section is a place to put symbols which are defined
         by dynamic objects, are referenced by regular objects, and are
         not functions.  We must allocate space for them in the process
         image and use a R_*_COPY reloc to tell the dynamic linker to
         initialize them at run time.  The linker script puts the .dynbss
         section into the .bss section of the final image.  */
      s = bfd_make_section_with_flags (abfd, ".dynbss",
                                       SEC_ALLOC | SEC_LINKER_CREATED);
      if (s == NULL)
        return FALSE;

      /* The .rel[a].bss section holds copy relocs.  This section is not
     normally needed.  We need to create it here, though, so that the
     linker will map it to an output section.  We can't just create it
     only if we need it, because we will not know whether we need it
     until we have seen all the input files, and the first time the
     main linker code calls BFD after examining all the input files
     (size_dynamic_sections) the input sections have already been
     mapped to the output sections.  If the section turns out not to
     be needed, we can discard it later.  We will never need this
     section when generating a shared object, since they do not use
     copy relocs.  */
      if (! info->shared)
        {
          s = bfd_make_section_with_flags (abfd,
                                           ".rela.bss",
                                           flags | SEC_READONLY);
          if (s == NULL
              || ! bfd_set_section_alignment (abfd, s, bed->s->log_file_align))
            return FALSE;
        }
    }

  return TRUE;
}


#if 0
/* Allocate space in .plt, .got and associated reloc sections for
   dynamic relocs.  */

static bfd_boolean
allocate_dynrelocs (struct elf_link_hash_entry *h, void * dat)
{
  struct bfd_link_info *info;
  struct elf32_k1_link_hash_table *htab;
  struct elf32_k1_link_hash_entry *eh;
  struct elf32_k1_dyn_relocs *p;

  if (h->root.type == bfd_link_hash_indirect)
    return TRUE;

  if (h->root.type == bfd_link_hash_warning)
    /* When warning symbols are created, they **replace** the "real"
       entry in the hash table, thus we never get to see the real
       symbol in a hash traversal.  So look at it now.  */
    h = (struct elf_link_hash_entry *) h->root.u.i.link;

  info = (struct bfd_link_info *) dat;
  htab = elf32_k1_hash_table (info);
  if (htab == NULL)
    return FALSE;

  if (htab->elf.dynamic_sections_created
      && h->plt.refcount > 0)
    {
      fprintf(stderr, "==>allocate_dynrelocs -> plt_ref_count > 0\n");
      /* Make sure this symbol is output as a dynamic symbol.
         Undefined weak syms won't yet be marked as dynamic.  */
      if (h->dynindx == -1
          && !h->forced_local)
        {
          if (! bfd_elf_link_record_dynamic_symbol (info, h))
            return FALSE;
        }

      if (WILL_CALL_FINISH_DYNAMIC_SYMBOL (1, info->shared, h))
        {
          asection *s = htab->splt;

          /* The first entry in .plt is reserved.  */
          if (s->size == 0)
            s->size =  PLT_FULL_ENTRY_SIZE;

          h->plt.offset = s->size;

          /* If this symbol is not defined in a regular file, and we are
             not generating a shared library, then set the symbol to this
             location in the .plt.  This is required to make function
             pointers compare as equal between the normal executable and
             the shared library.  */
          if (! info->shared
              && !h->def_regular)
            {
              h->root.u.def.section = s;
              h->root.u.def.value = h->plt.offset;
            }

          /* Make room for this entry.  */
          s->size +=  PLT_FULL_ENTRY_SIZE;

          /* We also need to make an entry in the .got.plt section, which
             will be placed in the .got section by the linker script.  */
          htab->sgotplt->size += 8;

          /* We also need to make an entry in the .rel.plt section.  */
          htab->srelplt->size += sizeof (Elf32_External_Rela);
          fprintf(stderr, "  srelplt size: %d\n", htab->srelplt->size);
        }
      else
        {
          h->plt.offset = (bfd_vma) -1;
          h->needs_plt = 0;
        }
    }
  else
    {
      h->plt.offset = (bfd_vma) -1;
      h->needs_plt = 0;
    }

  if (h->got.refcount > 0)
    {
      asection *s;
      /* Make sure this symbol is output as a dynamic symbol.
         Undefined weak syms won't yet be marked as dynamic.  */
      if (h->dynindx == -1
          && !h->forced_local)
        {
          if (! bfd_elf_link_record_dynamic_symbol (info, h))
            return FALSE;
        }

      s = htab->sgot;
      h->got.offset = s->size;
      s->size += 4;
      htab->srelgot->size += sizeof (Elf32_External_Rela);
    }
  else
    h->got.offset = (bfd_vma) -1;

  eh = (struct elf32_k1_link_hash_entry *) h;
  if (eh->dyn_relocs == NULL)
    return TRUE;

  /* In the shared -Bsymbolic case, discard space allocated for
     dynamic pc-relative relocs against symbols which turn out to be
     defined in regular objects.  For the normal shared case, discard
     space for pc-relative relocs that have become local due to symbol
     visibility changes.  */

  if (info->shared)
    {
      if (h->def_regular
          && (h->forced_local
              || info->symbolic))
        {
          struct elf32_k1_dyn_relocs **pp;

          for (pp = &eh->dyn_relocs; (p = *pp) != NULL; )
            {
              p->count -= p->pc_count;
              p->pc_count = 0;
              if (p->count == 0)
                *pp = p->next;
              else
                pp = &p->next;
            }
        }
    }
  else
    {
      /* For the non-shared case, discard space for relocs against
         symbols which turn out to need copy relocs or are not
         dynamic.  */

      if (!h->non_got_ref
          && ((h->def_dynamic
               && !h->def_regular)
              || (htab->elf.dynamic_sections_created
                  && (h->root.type == bfd_link_hash_undefweak
                      || h->root.type == bfd_link_hash_undefined))))
        {
          /* Make sure this symbol is output as a dynamic symbol.
             Undefined weak syms won't yet be marked as dynamic.  */
          if (h->dynindx == -1
              && !h->forced_local)
            {
              if (! bfd_elf_link_record_dynamic_symbol (info, h))
                return FALSE;
            }

          /* If that succeeded, we know we'll be keeping all the
             relocs.  */
          if (h->dynindx != -1)
            goto keep;
        }

      eh->dyn_relocs = NULL;

    keep: ;
    }

  /* Finally, allocate space.  */
  for (p = eh->dyn_relocs; p != NULL; p = p->next)
    {
      asection *sreloc = elf_section_data (p->sec)->sreloc;
      sreloc->size += p->count * sizeof (Elf32_External_Rela);
    }

  return TRUE;
}

#endif//0

/* Determine the positive and negative ranges to be used by each
   offset range in the GOT.  FDCUR and CUR, that must be aligned to a
   double-word boundary, are the minimum (negative) and maximum
   (positive) GOT offsets already used by previous ranges, except for
   an ODD entry that may have been left behind.  GOT and FD indicate
   the size of GOT entries and function descriptors that must be
   placed within the range from -WRAP to WRAP.  If there's room left,
   up to FDPLT bytes should be reserved for additional function
   descriptors.  */

inline static bfd_signed_vma
_k1fdpic_compute_got_alloc_data (struct _k1fdpic_dynamic_got_alloc_data *gad,
                                   bfd_signed_vma fdcur,
                                   bfd_signed_vma odd,
                                   bfd_signed_vma cur,
                                   bfd_vma got,
                                   bfd_vma fd,
                                   bfd_vma fdplt,
                                   bfd_vma wrap)
{
  bfd_signed_vma wrapmin = -wrap;

  /* Start at the given initial points.  */
  gad->fdcur = fdcur;
  gad->cur = cur;

  /* If we had an incoming odd word and we have any got entries that
     are going to use it, consume it, otherwise leave gad->odd at
     zero.  We might force gad->odd to zero and return the incoming
     odd such that it is used by the next range, but then GOT entries
     might appear to be out of order and we wouldn't be able to
     shorten the GOT by one word if it turns out to end with an
     unpaired GOT entry.  */
  if (odd && got)
    {
      gad->odd = odd;
      got -= 4;
      odd = 0;
    }
  else
    gad->odd = 0;

  /* If we're left with an unpaired GOT entry, compute its location
     such that we can return it.  Otherwise, if got doesn't require an
     odd number of words here, either odd was already zero in the
     block above, or it was set to zero because got was non-zero, or
     got was already zero.  In the latter case, we want the value of
     odd to carry over to the return statement, so we don't want to
     reset odd unless the condition below is true.  */
  if (got & 4)
    {
      odd = cur + got;
      got += 4;
    }

  /* Compute the tentative boundaries of this range.  */
  gad->max = cur + got;
  gad->min = fdcur - fd;
  gad->fdplt = 0;

  /* If function descriptors took too much space, wrap some of them
     around.  */
  if (gad->min < wrapmin)
    {
      gad->max += wrapmin - gad->min;
      gad->min = wrapmin;
    }
  /* If there is space left and we have function descriptors
     referenced in PLT entries that could take advantage of shorter
     offsets, place them here.  */
  else if (fdplt && gad->min > wrapmin)
    {
      bfd_vma fds;
      if ((bfd_vma) (gad->min - wrapmin) < fdplt)
        fds = gad->min - wrapmin;
      else
        fds = fdplt;

      fdplt -= fds;
      gad->min -= fds;
      gad->fdplt += fds;
    }

  /* If GOT entries took too much space, wrap some of them around.
     This may well cause gad->min to become lower than wrapmin.  This
     will cause a relocation overflow later on, so we don't have to
     report it here . */
  if ((bfd_vma) gad->max > wrap)
    {
      gad->min -= gad->max - wrap;
      gad->max = wrap;
    }
  /* If there is more space left, try to place some more function
     descriptors for PLT entries.  */
  else if (fdplt && (bfd_vma) gad->max < wrap)
    {
      bfd_vma fds;
      if ((bfd_vma) (wrap - gad->max) < fdplt)
        fds = wrap - gad->max;
      else
        fds = fdplt;

      fdplt -= fds;
      gad->max += fds;
      gad->fdplt += fds;
    }

  /* If odd was initially computed as an offset past the wrap point,
     wrap it around.  */
  if (odd > gad->max)
    odd = gad->min + odd - gad->max;

  /* _k1fdpic_get_got_entry() below will always wrap gad->cur if needed
     before returning, so do it here too.  This guarantees that,
     should cur and fdcur meet at the wrap point, they'll both be
     equal to min.  */
  if (gad->cur == gad->max)
    gad->cur = gad->min;

  return odd;
}



/* Compute the location of the next GOT entry, given the allocation
   data for a range.  */

inline static bfd_signed_vma
_k1fdpic_get_got_entry (struct _k1fdpic_dynamic_got_alloc_data *gad)
{
  bfd_signed_vma ret;

  if (gad->odd)
    {
      /* If there was an odd word left behind, use it.  */
      ret = gad->odd;
      gad->odd = 0;
    }
  else
    {
      /* Otherwise, use the word pointed to by cur, reserve the next
         as an odd word, and skip to the next pair of words, possibly
         wrapping around.  */
      ret = gad->cur;
      gad->odd = gad->cur + 4;
      gad->cur += 8;
      if (gad->cur == gad->max)
        gad->cur = gad->min;
    }

  return ret;
}

/* Compute the location of the next function descriptor entry in the
   GOT, given the allocation data for a range.  */

inline static bfd_signed_vma
_k1fdpic_get_fd_entry (struct _k1fdpic_dynamic_got_alloc_data *gad)
{
  /* If we're at the bottom, wrap around, and only then allocate the
     next pair of words.  */
  if (gad->fdcur == gad->min)
    gad->fdcur = gad->max;
  return gad->fdcur -= 8;
}

/* Assign GOT offsets for every GOT entry and function descriptor.
   Doing everything in a single pass is tricky.  */

static int
_k1fdpic_assign_got_entries (void **entryp, void *info_)
{
  struct k1fdpic_relocs_info *entry = *entryp;
  struct _k1fdpic_dynamic_got_plt_info *dinfo = info_;

  if (entry->gothilo)
    entry->got_entry = _k1fdpic_get_got_entry (&dinfo->gothilo);

  if (entry->fdgothilo)
    entry->fdgot_entry = _k1fdpic_get_got_entry (&dinfo->gothilo);

  if (entry->fdgotoffhilo)
    entry->fd_entry = _k1fdpic_get_fd_entry (&dinfo->gothilo);
  else if (entry->plt && dinfo->gothilo.fdplt)
    {
      dinfo->gothilo.fdplt -= 8;
      entry->fd_entry = _k1fdpic_get_fd_entry (&dinfo->gothilo);
    }
  else if (entry->plt)
    {
      dinfo->gothilo.fdplt -= 8;
      entry->fd_entry = _k1fdpic_get_fd_entry (&dinfo->gothilo);
    }
  else if (entry->privfd)
    entry->fd_entry = _k1fdpic_get_fd_entry (&dinfo->gothilo);

  return 1;
}

/* Assign GOT offsets to private function descriptors used by PLT
   entries (or referenced by 32-bit offsets), as well as PLT entries
   and lazy PLT entries.  */

static int
_k1fdpic_assign_plt_entries (void **entryp, void *info_)
{
  struct k1fdpic_relocs_info *entry = *entryp;
  struct _k1fdpic_dynamic_got_plt_info *dinfo = info_;

  /* If this symbol requires a local function descriptor, allocate
     one.  */
  if (entry->privfd && entry->fd_entry == 0)
    {
       entry->fd_entry = _k1fdpic_get_fd_entry (&dinfo->gothilo);
       dinfo->gothilo.fdplt -= 8;
    }

  if (entry->plt)
    {
      int size;

      /* We use the section's raw size to mark the location of the
         next PLT entry.  */
      entry->plt_entry = k1fdpic_plt_section (dinfo->g.info)->size;

      /* Figure out the length of this PLT entry based on the
         addressing mode we need to reach the function descriptor.  */
      BFD_ASSERT (entry->fd_entry);
      if (entry->fd_entry >= -(1 << (18 - 1))
          && entry->fd_entry + 4 < (1 << (18 - 1)))
        size = 10;
      else
        size = 16;

      k1fdpic_plt_section (dinfo->g.info)->size += size;
    }

  if (entry->lazyplt)
    {
      entry->lzplt_entry = dinfo->g.lzplt;
      dinfo->g.lzplt += LZPLT_NORMAL_SIZE;
      /* If this entry is the one that gets the resolver stub, account
         for the additional instruction.  */
      if (entry->lzplt_entry % K1FDPIC_LZPLT_BLOCK_SIZE
          == K1FDPIC_LZPLT_RESOLV_LOC)
        dinfo->g.lzplt += LZPLT_RESOLVER_EXTRA;
    }

  return 1;
}

/* Cancel out any effects of calling _k1fdpic_assign_got_entries and
   _k1fdpic_assign_plt_entries.  */

static int
_k1fdpic_reset_got_plt_entries (void **entryp, void *ignore ATTRIBUTE_UNUSED)
{
  struct k1fdpic_relocs_info *entry = *entryp;

  entry->got_entry = 0;
  entry->fdgot_entry = 0;
  entry->fd_entry = 0;
  entry->plt_entry = (bfd_vma)-1;
  entry->lzplt_entry = (bfd_vma)-1;

  return 1;
}


/* Compute the total size of the GOT, the PLT, the dynamic relocations
   section and the rofixup section.  Assign locations for GOT and PLT
   entries.  */

static bfd_boolean
_k1fdpic_size_got_plt (bfd *output_bfd,
                         struct _k1fdpic_dynamic_got_plt_info *gpinfop)
{
  bfd_signed_vma odd = 12;
//   bfd_vma limit;
  struct bfd_link_info *info = gpinfop->g.info;
  bfd *dynobj = elf_hash_table (info)->dynobj;

  memcpy (k1fdpic_dynamic_got_plt_info (info), &gpinfop->g,
          sizeof (gpinfop->g));
  
  odd = _k1fdpic_compute_got_alloc_data (&gpinfop->gothilo,
                                          0,
                                          odd,
                                          16,
                                          gpinfop->g.gothilo,
                                          gpinfop->g.fdhilo,
                                          gpinfop->g.fdplt,
                                          (bfd_vma)1 << (32-1));


  /* Now assign (most) GOT offsets.  */
  htab_traverse (k1fdpic_relocs_info (info), _k1fdpic_assign_got_entries,
                 gpinfop);

  k1fdpic_got_section (info)->size = gpinfop->gothilo.max
    - gpinfop->gothilo.min
     /* If an odd word is the last word of the GOT, we don't need this
        word to be part of the GOT.  */
    - (odd + 4 == gpinfop->gothilo.max ? 4 : 0);
  if (k1fdpic_got_section (info)->size == 0)
    k1fdpic_got_section (info)->flags |= SEC_EXCLUDE;
  else if (k1fdpic_got_section (info)->size == 12
           && ! elf_hash_table (info)->dynamic_sections_created)
    {
      k1fdpic_got_section (info)->flags |= SEC_EXCLUDE;
      k1fdpic_got_section (info)->size = 0;
    }
  else
    {
      k1fdpic_got_section (info)->contents =
        (bfd_byte *) bfd_zalloc (dynobj,
                                 k1fdpic_got_section (info)->size);
      if (k1fdpic_got_section (info)->contents == NULL)
        return FALSE;
    }

  if (elf_hash_table (info)->dynamic_sections_created)
    /* Subtract the number of lzplt entries, since those will generate
       relocations in the pltrel section.  */
    k1fdpic_gotrel_section (info)->size =
      (gpinfop->g.relocs - gpinfop->g.lzplt / LZPLT_NORMAL_SIZE)
      * get_elf_backend_data (output_bfd)->s->sizeof_rel;
  else
    BFD_ASSERT (gpinfop->g.relocs == 0);
  if (k1fdpic_gotrel_section (info)->size == 0)
    k1fdpic_gotrel_section (info)->flags |= SEC_EXCLUDE;
  else
    {
      k1fdpic_gotrel_section (info)->contents =
        (bfd_byte *) bfd_zalloc (dynobj,
                                 k1fdpic_gotrel_section (info)->size);
      if (k1fdpic_gotrel_section (info)->contents == NULL)
        return FALSE;
    }


  k1fdpic_gotfixup_section (info)->size = (gpinfop->g.fixups + 1 )* 4;
  if (k1fdpic_gotfixup_section (info)->size == 0)
    k1fdpic_gotfixup_section (info)->flags |= SEC_EXCLUDE;
  else
    {
      k1fdpic_gotfixup_section (info)->contents =
        (bfd_byte *) bfd_zalloc (dynobj,
                                 k1fdpic_gotfixup_section (info)->size);
      if (k1fdpic_gotfixup_section (info)->contents == NULL)
        return FALSE;
    }

  if (elf_hash_table (info)->dynamic_sections_created)
    k1fdpic_pltrel_section (info)->size =
      gpinfop->g.lzplt / LZPLT_NORMAL_SIZE * get_elf_backend_data (output_bfd)->s->sizeof_rel;
  if (k1fdpic_pltrel_section (info)->size == 0)
    k1fdpic_pltrel_section (info)->flags |= SEC_EXCLUDE;
  else
    {
      k1fdpic_pltrel_section (info)->contents =
        (bfd_byte *) bfd_zalloc (dynobj,
                                 k1fdpic_pltrel_section (info)->size);
      if (k1fdpic_pltrel_section (info)->contents == NULL)
        return FALSE;
    }

  /* Add 4 bytes for every block of at most 65535 lazy PLT entries,
     such that there's room for the additional instruction needed to
     call the resolver.  Since _k1fdpic_assign_got_entries didn't
     account for them, our block size is 4 bytes smaller than the real
     block size.  */
  if (elf_hash_table (info)->dynamic_sections_created)
    {
      k1fdpic_plt_section (info)->size = gpinfop->g.lzplt
        + ((gpinfop->g.lzplt + (K1FDPIC_LZPLT_BLOCK_SIZE - 4) - LZPLT_NORMAL_SIZE)
           / (K1FDPIC_LZPLT_BLOCK_SIZE - 4) * LZPLT_RESOLVER_EXTRA);
    }

  /* Reset it, such that _k1fdpic_assign_plt_entries() can use it to
     actually assign lazy PLT entries addresses.  */
  gpinfop->g.lzplt = 0;

  /* Save information that we're going to need to generate GOT and PLT
     entries.  */
  k1fdpic_got_initial_offset (info) = -gpinfop->gothilo.min;

  if (get_elf_backend_data (output_bfd)->want_got_sym)
    elf_hash_table (info)->hgot->root.u.def.value
      = k1fdpic_got_initial_offset (info);

  if (elf_hash_table (info)->dynamic_sections_created)
    k1fdpic_plt_initial_offset (info) =
      k1fdpic_plt_section (info)->size;

  htab_traverse (k1fdpic_relocs_info (info), _k1fdpic_assign_plt_entries,
                 gpinfop);

  /* Allocate the PLT section contents only after
     _k1fdpic_assign_plt_entries has a chance to add the size of the
     non-lazy PLT entries.  */
  if (k1fdpic_plt_section (info)->size == 0)
    k1fdpic_plt_section (info)->flags |= SEC_EXCLUDE;
  else
    {
      k1fdpic_plt_section (info)->contents =
        (bfd_byte *) bfd_zalloc (dynobj,
                                 k1fdpic_plt_section (info)->size);
      if (k1fdpic_plt_section (info)->contents == NULL)
        return FALSE;
    }

  return TRUE;
}


/* Set the sizes of the dynamic sections.  */

static bfd_boolean
k1_elf_size_dynamic_sections (bfd *output_bfd ATTRIBUTE_UNUSED,
                                      struct bfd_link_info *info)
{
#if 0
  bfd *dynobj;
  asection *s;
  bfd *ibfd;
  struct elf_link_hash_table *htab = elf_hash_table (info);

  fprintf (stderr, "==> size dynamic sections\n");
  if (htab == NULL)
    return FALSE;

  dynobj = htab->dynobj;
  BFD_ASSERT (dynobj != NULL);

  if (htab->dynamic_sections_created)
    {
      /* Set the contents of the .interp section to the interpreter.  */
      if (info->executable)
        {
          s = bfd_get_section_by_name (dynobj, ".interp");
          if (s == NULL)
            abort ();
          s->size = sizeof ELF_DYNAMIC_INTERPRETER;
          s->contents = (unsigned char *) ELF_DYNAMIC_INTERPRETER;
        }
    }

  /* Set up .got offsets for local syms, and space for local dynamic
     relocs.  */
  for (ibfd = info->input_bfds; ibfd != NULL; ibfd = ibfd->link_next)
    {
      bfd_signed_vma *local_got;
      bfd_signed_vma *end_local_got;
      bfd_size_type locsymcount;
      Elf_Internal_Shdr *symtab_hdr;
      asection *srel;

      if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour)
        continue;

      for (s = ibfd->sections; s != NULL; s = s->next)
        {
          struct elf32_k1_dyn_relocs *p;

          for (p = ((struct elf32_k1_dyn_relocs *)
                    elf_section_data (s)->local_dynrel);
               p != NULL;
               p = p->next)
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
                  srel->size += p->count * sizeof (Elf32_External_Rela);
                  if ((p->sec->output_section->flags & SEC_READONLY) != 0)
                    info->flags |= DF_TEXTREL;
                }
            }
        }

      local_got = elf_local_got_refcounts (ibfd);
      if (!local_got)
        continue;

      symtab_hdr = &elf_tdata (ibfd)->symtab_hdr;
      locsymcount = symtab_hdr->sh_info;
      end_local_got = local_got + locsymcount;
      s = htab->sgot;
      srel = htab->srelgot;

      for (; local_got < end_local_got; ++local_got)
        {
          if (*local_got > 0)
            {
              *local_got = s->size;
              s->size += 4;
              if (info->shared)
                srel->size += sizeof (Elf32_External_Rela);
            }
          else
            *local_got = (bfd_vma) -1;
        }
    }

     /* Allocate global sym .plt and .got entries, and space for global
     sym dynamic relocs.  */
  elf_link_hash_traverse (elf_hash_table (info), allocate_dynrelocs, info);

//   if (elf_hash_table (info)->dynamic_sections_created)
//     {
      /* Make space for the trailing nop in .plt.  */
//       if (htab->splt->size > 0)
//         htab->splt->size += 4;
//     }

     /* The check_relocs and adjust_dynamic_symbol entry points have
     determined the sizes of the various dynamic sections.  Allocate
     memory for them.  */
  for (s = dynobj->sections; s != NULL; s = s->next)
    {
      const char *name;
      bfd_boolean strip = FALSE;

      if ((s->flags & SEC_LINKER_CREATED) == 0)
        continue;

      /* It's OK to base decisions on the section name, because none
         of the dynobj section names depend upon the input files.  */
      name = bfd_get_section_name (dynobj, s);

      if (strncmp (name, ".rela", 5) == 0)
        {
          if (s->size == 0)
            {
              /* If we don't need this section, strip it from the
                 output file.  This is to handle .rela.bss and
                 .rela.plt.  We must create it in
                 create_dynamic_sections, because it must be created
                 before the linker maps input sections to output
                 sections.  The linker does that before
                 adjust_dynamic_symbol is called, and it is that
                 function which decides whether anything needs to go
                 into these sections.  */
              strip = TRUE;
            }
          else
            {
              /* We use the reloc_count field as a counter if we need
                 to copy relocs into the output file.  */
              s->reloc_count = 0;
            }
        }
      else if (s != htab->splt && s != htab->sgot && s != htab->sgotplt)
        {
          /* It's not one of our sections, so don't allocate space.  */
          continue;
        }

      if (strip)
        {
          s->flags |= SEC_EXCLUDE;
          continue;
        }

      /* Allocate memory for the section contents.  */
      /* FIXME: This should be a call to bfd_alloc not bfd_zalloc.
         Unused entries should be reclaimed before the section's contents
         are written out, but at the moment this does not happen.  Thus in
         order to prevent writing out garbage, we initialise the section's
         contents to zero.  */
      s->contents = (bfd_byte *) bfd_zalloc (dynobj, s->size);
      if (s->contents == NULL && s->size != 0)
        return FALSE;
    }
    
    if (elf_hash_table (info)->dynamic_sections_created)
    {
      /* Add some entries to the .dynamic section.  We fill in the
         values later, in microblaze_elf_finish_dynamic_sections, but we
         must add the entries now so that we get the correct size for
         the .dynamic section.  The DT_DEBUG entry is filled in by the
         dynamic linker and used by the debugger.  */
#define add_dynamic_entry(TAG, VAL)                     \
      _bfd_elf_add_dynamic_entry (info, TAG, VAL)

      if (info->executable)
        {
          if (!add_dynamic_entry (DT_DEBUG, 0))
            return FALSE;
        }

      if (!add_dynamic_entry (DT_RELA, 0)
          || !add_dynamic_entry (DT_RELASZ, 0)
          || !add_dynamic_entry (DT_RELAENT, sizeof (Elf32_External_Rela)))
        return FALSE;

      if (htab->splt->size != 0)
        {
          if (!add_dynamic_entry (DT_PLTGOT, 0)
              || !add_dynamic_entry (DT_PLTRELSZ, 0)
              || !add_dynamic_entry (DT_PLTREL, DT_RELA)
              || !add_dynamic_entry (DT_JMPREL, 0)
              || !add_dynamic_entry (DT_BIND_NOW, 1))
            return FALSE;
        }

      if (info->flags & DF_TEXTREL)
        {
          if (!add_dynamic_entry (DT_TEXTREL, 0))
            return FALSE;
        }
    }
#undef add_dynamic_entry
  return TRUE;

#endif//0
  struct elf_link_hash_table *htab;
  bfd *dynobj;
  asection *s;
  struct _k1fdpic_dynamic_got_plt_info gpinfo;

  htab = elf_hash_table (info);
  dynobj = htab->dynobj;
  BFD_ASSERT (dynobj != NULL);

  if (htab->dynamic_sections_created)
    {
      /* Set the contents of the .interp section to the interpreter.  */
      if (info->executable)
        {
          s = bfd_get_section_by_name (dynobj, ".interp");
          BFD_ASSERT (s != NULL);
          s->size = sizeof ELF_DYNAMIC_INTERPRETER;
          s->contents = (bfd_byte *) ELF_DYNAMIC_INTERPRETER;
        }
    }

  memset (&gpinfo, 0, sizeof (gpinfo));
  gpinfo.g.info = info;

  for (;;)
    {
      htab_t relocs = k1fdpic_relocs_info (info);

      htab_traverse (relocs, _k1fdpic_resolve_final_relocs_info, &relocs);

      if (relocs == k1fdpic_relocs_info (info))
        break;
    }

  htab_traverse (k1fdpic_relocs_info (info), _k1fdpic_count_got_plt_entries,
                 &gpinfo.g);

  /* Allocate space to save the summary information, we're going to
     use it if we're doing relaxations.  */
  k1fdpic_dynamic_got_plt_info (info) = bfd_alloc (dynobj, sizeof (gpinfo.g));

  if (!_k1fdpic_size_got_plt (output_bfd, &gpinfo))
      return FALSE;

  if (elf_hash_table (info)->dynamic_sections_created)
    {
      if (k1fdpic_got_section (info)->size)
        if (!_bfd_elf_add_dynamic_entry (info, DT_PLTGOT, 0))
          return FALSE;

      if (k1fdpic_pltrel_section (info)->size)
        if (!_bfd_elf_add_dynamic_entry (info, DT_PLTRELSZ, 0)
            || !_bfd_elf_add_dynamic_entry (info, DT_PLTREL, DT_REL)
            || !_bfd_elf_add_dynamic_entry (info, DT_JMPREL, 0))
          return FALSE;

      if (k1fdpic_gotrel_section (info)->size)
        if (!_bfd_elf_add_dynamic_entry (info, DT_REL, 0)
            || !_bfd_elf_add_dynamic_entry (info, DT_RELSZ, 0)
            || !_bfd_elf_add_dynamic_entry (info, DT_RELENT,
                                            sizeof (Elf32_External_Rel)))
          return FALSE;
    }

  s = bfd_get_section_by_name (dynobj, ".dynbss");
  if (s && s->size == 0)
    s->flags |= SEC_EXCLUDE;

  s = bfd_get_section_by_name (dynobj, ".rela.bss");
  if (s && s->size == 0)
    s->flags |= SEC_EXCLUDE;

  return TRUE;
  
}

static bfd_boolean
k1_elf_adjust_dynamic_symbol (struct bfd_link_info *info,
                              struct elf_link_hash_entry *h)
{
    bfd * dynobj;

  dynobj = elf_hash_table (info)->dynobj;

  /* Make sure we know what is going on here.  */
  BFD_ASSERT (dynobj != NULL
              && (h->u.weakdef != NULL
                  || (h->def_dynamic
                      && h->ref_regular
                      && !h->def_regular)));

  /* If this is a weak symbol, and there is a real definition, the
     processor independent code will have arranged for us to see the
     real definition first, and we can just use the same value.  */
  if (h->u.weakdef != NULL)
    {
      BFD_ASSERT (h->u.weakdef->root.type == bfd_link_hash_defined
                  || h->u.weakdef->root.type == bfd_link_hash_defweak);
      h->root.u.def.section = h->u.weakdef->root.u.def.section;
      h->root.u.def.value = h->u.weakdef->root.u.def.value;
    }
  return TRUE;
}


/* Finish up dynamic symbol handling.  We set the contents of various
   dynamic sections here.  */

/* Perform any actions needed for dynamic symbols.  */

static bfd_boolean
k1_elf_finish_dynamic_symbol
(bfd *output_bfd ATTRIBUTE_UNUSED,
 struct bfd_link_info *info ATTRIBUTE_UNUSED,
 struct elf_link_hash_entry *h ATTRIBUTE_UNUSED,
 Elf_Internal_Sym *sym ATTRIBUTE_UNUSED)
{
  return TRUE;
}

#if 0
static bfd_boolean
k1_elf_finish_dynamic_symbol (bfd *output_bfd,
                              struct bfd_link_info *info,
                              struct elf_link_hash_entry *h,
                              Elf_Internal_Sym *sym)
{
  struct elf32_k1_link_hash_table *htab;

  htab = elf32_k1_hash_table (info);
  if (htab == NULL)
    return FALSE;

  if (h->plt.offset != (bfd_vma) -1)
    {
      asection *splt;
      asection *srela;
      asection *sgotplt;
      Elf_Internal_Rela rela;
      bfd_byte *loc;
      bfd_vma plt_index;
      bfd_vma got_offset;
      bfd_vma got_addr;
      int i;

      const bfd_vma *template = fdpic_abi_plt_full_entry;

#ifdef DEBUG
      fprintf(stderr, "==> Finish dynamic symbol -> PLT entry\n");
#endif//DEBUG

            /* This symbol has an entry in the procedure linkage table.  Set
         it up.  */
      BFD_ASSERT (h->dynindx != -1);

      splt = htab->splt;
      srela = htab->srelplt;
      sgotplt = htab->sgotplt;
      BFD_ASSERT (splt != NULL && srela != NULL && sgotplt != NULL);

      plt_index = h->plt.offset / PLT_FULL_ENTRY_SIZE - 1; /* first entry reserved.  */
      got_offset = (plt_index + 3) * 4; /* 3 reserved ???  */
      got_addr = got_offset;

      /* For non-PIC objects we need absolute address of the GOT entry.  */
      if (!info->shared)
        got_addr += htab->sgotplt->output_section->vma + sgotplt->output_offset;

      for (i = 0; i < (PLT_FULL_ENTRY_SIZE / 4); i++)
        bfd_put_32(output_bfd, template[i], splt->contents + h->plt.offset + (4*i));

/*
 * TODO: can we have offsets bigger than 10bits
 */      
      _bfd_final_link_relocate (elf32_k1_howto_table + R_K1_LO10,
                                output_bfd, splt, splt->contents,
                                h->plt.offset, got_addr, 0);

      /* Fill in the entry in the .rela.plt section.  */
      rela.r_offset = (sgotplt->output_section->vma
                       + sgotplt->output_offset
                       + got_offset);
      rela.r_info = ELF32_R_INFO (h->dynindx, R_K1_FUNCDESC);
      rela.r_addend = 0;
      loc = srela->contents;
      loc += plt_index * sizeof (Elf32_External_Rela);
      bfd_elf32_swap_reloca_out (output_bfd, &rela, loc);

      fprintf(stderr, " got.addr:     0x%x\n\n", got_addr);
            
    }

    if (h->got.offset != (bfd_vma) -1)
    {
      asection *sgot;
      asection *srela;
      Elf_Internal_Rela rela;
      bfd_byte *loc;

#ifdef DEBUG
      fprintf(stderr, "==> Finish dynamic symbol -> GOT entry\n");
#endif//DEBUG

      /* This symbol has an entry in the global offset table.  Set it
         up.  */

      sgot = htab->sgot;
      srela = htab->srelgot;
      BFD_ASSERT (sgot != NULL && srela != NULL);

      rela.r_offset = (sgot->output_section->vma
                       + sgot->output_offset
                       + (h->got.offset &~ (bfd_vma) 1));

   
      rela.r_info = ELF32_R_INFO (h->dynindx, R_K1_GLOB_DAT);
      rela.r_addend = 0;  

      bfd_put_32 (output_bfd, (bfd_vma) 0,
                  sgot->contents + (h->got.offset &~ (bfd_vma) 1));
      loc = srela->contents;
      loc += srela->reloc_count++ * sizeof (Elf32_External_Rela);
      bfd_elf32_swap_reloca_out (output_bfd, &rela, loc);
    }
  
  /* Mark some specially defined symbols as absolute.  */
  if (strcmp (h->root.root.string, "_DYNAMIC") == 0
      || strcmp (h->root.root.string, "_GLOBAL_OFFSET_TABLE_") == 0
      || strcmp (h->root.root.string, "_PROCEDURE_LINKAGE_TABLE_") == 0)
      sym->st_shndx = SHN_ABS;

  return TRUE;
}
#endif//0

/* Finish up the dynamic sections.  */

static bfd_boolean
k1_elf_finish_dynamic_sections (bfd *output_bfd,
                                        struct bfd_link_info *info)
{
  bfd *dynobj;
  asection *sdyn;

  dynobj = elf_hash_table (info)->dynobj;

  if (k1fdpic_got_section (info))
    {
      BFD_ASSERT (k1fdpic_gotrel_section (info)->size
                  == (k1fdpic_gotrel_section (info)->reloc_count
                      * sizeof (Elf32_External_Rel)));

      if (k1fdpic_gotfixup_section (info))
        {
          struct elf_link_hash_entry *hgot = elf_hash_table (info)->hgot;
          bfd_vma got_value = hgot->root.u.def.value
            + hgot->root.u.def.section->output_section->vma
            + hgot->root.u.def.section->output_offset;

          _k1fdpic_add_rofixup (output_bfd, k1fdpic_gotfixup_section (info),
                                 got_value, 0);

          if (k1fdpic_gotfixup_section (info)->size
              != (k1fdpic_gotfixup_section (info)->reloc_count * 4))
            {
              (*_bfd_error_handler)
                ("LINKER BUG: .rofixup section size mismatch");
              return FALSE;
            }
        }
    }
  if (elf_hash_table (info)->dynamic_sections_created)
    {
      BFD_ASSERT (k1fdpic_pltrel_section (info)->size
                  == (k1fdpic_pltrel_section (info)->reloc_count
                      * sizeof (Elf32_External_Rel)));
    }

  sdyn = bfd_get_section_by_name (dynobj, ".dynamic");

  if (elf_hash_table (info)->dynamic_sections_created)
    {
      Elf32_External_Dyn * dyncon;
      Elf32_External_Dyn * dynconend;

      BFD_ASSERT (sdyn != NULL);

      dyncon = (Elf32_External_Dyn *) sdyn->contents;
      dynconend = (Elf32_External_Dyn *) (sdyn->contents + sdyn->size);

      for (; dyncon < dynconend; dyncon++)
        {
          Elf_Internal_Dyn dyn;

          bfd_elf32_swap_dyn_in (dynobj, dyncon, &dyn);

          switch (dyn.d_tag)
            {
            default:
              break;

            case DT_PLTGOT:
              dyn.d_un.d_ptr = k1fdpic_got_section (info)->output_section->vma
                + k1fdpic_got_section (info)->output_offset
                + k1fdpic_got_initial_offset (info);
              bfd_elf32_swap_dyn_out (output_bfd, &dyn, dyncon);
              break;

            case DT_JMPREL:
              dyn.d_un.d_ptr = k1fdpic_pltrel_section (info)
                ->output_section->vma
                + k1fdpic_pltrel_section (info)->output_offset;
              bfd_elf32_swap_dyn_out (output_bfd, &dyn, dyncon);
              break;

            case DT_PLTRELSZ:
              dyn.d_un.d_val = k1fdpic_pltrel_section (info)->size;
              bfd_elf32_swap_dyn_out (output_bfd, &dyn, dyncon);
              break;
            }
        }
    }

  return TRUE;
}


#if 0
static bfd_boolean
k1_elf_finish_dynamic_sections (bfd *output_bfd,
                                struct bfd_link_info *info)
{
  bfd *dynobj;
  asection *sdyn, *sgot;
  struct elf32_k1_link_hash_table *htab;

  htab = elf32_k1_hash_table (info);
  if (htab == NULL)
    return FALSE;

  dynobj = htab->elf.dynobj;

  sdyn = bfd_get_section_by_name (dynobj, ".dynamic");

  if (htab->elf.dynamic_sections_created)
    {
      asection *splt;
      Elf32_External_Dyn *dyncon, *dynconend;

      splt = bfd_get_section_by_name (dynobj, ".plt");
      BFD_ASSERT (splt != NULL && sdyn != NULL);

      dyncon = (Elf32_External_Dyn *) sdyn->contents;
      dynconend = (Elf32_External_Dyn *) (sdyn->contents + sdyn->size);
      for (; dyncon < dynconend; dyncon++)
        {
          Elf_Internal_Dyn dyn;
          const char *name;
          bfd_boolean size;

          bfd_elf32_swap_dyn_in (dynobj, dyncon, &dyn);

          switch (dyn.d_tag)
            {
            case DT_PLTGOT:   name = ".got.plt"; size = FALSE; break;
            case DT_PLTRELSZ: name = ".rela.plt"; size = TRUE; break;
            case DT_JMPREL:   name = ".rela.plt"; size = FALSE; break;
            case DT_RELA:     name = ".rela.dyn"; size = FALSE; break;
            case DT_RELASZ:   name = ".rela.dyn"; size = TRUE; break;
            default:      name = NULL; size = FALSE; break;
            }

          if (name != NULL)
            {
              asection *s;

              s = bfd_get_section_by_name (output_bfd, name);
              if (s == NULL)
                dyn.d_un.d_val = 0;
              else
                {
                  if (! size)
                    dyn.d_un.d_ptr = s->vma;
                  else
                    dyn.d_un.d_val = s->size;
                }
              bfd_elf32_swap_dyn_out (output_bfd, &dyn, dyncon);
            }
        }

//       /* Clear the first entry in the procedure linkage table,
//          and put a nop in the last four bytes.  */
//       if (splt->size > 0)
//         {
//           memset (splt->contents, 0, PLT_FULL_ENTRY_SIZE);
//           bfd_put_32 (output_bfd, (bfd_vma) 0x80000000 /* nop.  */,
//                       splt->contents + splt->size - 4);
//         }

      elf_section_data (splt->output_section)->this_hdr.sh_entsize = 4;
    }

  /* Set the first entry in the global offset table to the address of
     the dynamic section.  */
  sgot = bfd_get_section_by_name (dynobj, ".got.plt");
  if (sgot && sgot->size > 0)
    {
      if (sdyn == NULL)
        bfd_put_32 (output_bfd, (bfd_vma) 0, sgot->contents);
      else
        bfd_put_32 (output_bfd,
                    sdyn->output_section->vma + sdyn->output_offset,
                    sgot->contents);
      elf_section_data (sgot->output_section)->this_hdr.sh_entsize = 4;
    }

  if (htab->sgot && htab->sgot->size > 0)
    elf_section_data (htab->sgot->output_section)->this_hdr.sh_entsize = 4;
  return TRUE;
}

#endif//0


static bfd_boolean
elf_k1_always_size_sections(bfd *output_bfd, struct bfd_link_info *info){
  if (!info->relocatable)
  {
    struct elf_link_hash_entry *h;

    /* Force a PT_GNU_STACK segment to be created.  */
    if (! elf_tdata (output_bfd)->stack_flags)
      elf_tdata (output_bfd)->stack_flags = PF_R | PF_W | PF_X;

    /* Define __stacksize if it's not defined yet.
     * */
    h = elf_link_hash_lookup (elf_hash_table (info), "__stacksize",
        FALSE, FALSE, FALSE);
    if (! h || h->root.type != bfd_link_hash_defined
        || h->type != STT_OBJECT
        || !h->def_regular)
    {
      struct bfd_link_hash_entry *bh = NULL;

      if (!(_bfd_generic_link_add_one_symbol
            (info, output_bfd, "__stacksize",
             BSF_GLOBAL, bfd_abs_section_ptr, DEFAULT_STACK_SIZE,
             (const char *) NULL, FALSE,
             get_elf_backend_data (output_bfd)->collect, &bh)))
        return FALSE;

      h = (struct elf_link_hash_entry *) bh;
      h->def_regular = 1;
      h->type = STT_OBJECT;
    }
  }

  return TRUE;
}


/* Check whether any of the relocations was optimized away, and
   subtract it from the relocation or fixup count.  */
static bfd_boolean
_k1fdpic_check_discarded_relocs (bfd *abfd, asection *sec,
                                  struct bfd_link_info *info,

                                  bfd_boolean *changed)
{
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes, **sym_hashes_end;
  Elf_Internal_Rela *rel, *erel;

  if ((sec->flags & SEC_RELOC) == 0
      || sec->reloc_count == 0)
    return TRUE;

  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (abfd);
  sym_hashes_end = sym_hashes + symtab_hdr->sh_size/sizeof(Elf32_External_Sym);
  if (!elf_bad_symtab (abfd))
    sym_hashes_end -= symtab_hdr->sh_info;

  rel = elf_section_data (sec)->relocs;

  /* Now examine each relocation.  */
  for (erel = rel + sec->reloc_count; rel < erel; rel++)
    {
      struct elf_link_hash_entry *h;
      unsigned long r_symndx;
      struct k1fdpic_relocs_info *picrel;
      struct _k1fdpic_dynamic_got_info *dinfo;

      if (ELF32_R_TYPE (rel->r_info) != R_K1_GLOB_DAT
          && ELF32_R_TYPE (rel->r_info) != R_K1_FUNCDESC)
        continue;

      if (_bfd_elf_section_offset (sec->output_section->owner,
                                   info, sec, rel->r_offset)
          != (bfd_vma)-1)
        continue;

      r_symndx = ELF32_R_SYM (rel->r_info);
      if (r_symndx < symtab_hdr->sh_info)
        h = NULL;
      else
        {
          h = sym_hashes[r_symndx - symtab_hdr->sh_info];
          while (h->root.type == bfd_link_hash_indirect
                 || h->root.type == bfd_link_hash_warning)
            h = (struct elf_link_hash_entry *)h->root.u.i.link;
        }

      if (h != NULL)
        picrel = k1fdpic_relocs_info_for_global (k1fdpic_relocs_info (info),
                                                  abfd, h,
                                                  rel->r_addend, NO_INSERT);
      else
        picrel = k1fdpic_relocs_info_for_local (k1fdpic_relocs_info (info),
                                                 abfd, r_symndx,
                                                 rel->r_addend, NO_INSERT);

      if (! picrel)
        return FALSE;

      *changed = TRUE;
      dinfo = k1fdpic_dynamic_got_plt_info (info);

      _k1fdpic_count_relocs_fixups (picrel, dinfo, TRUE);
      if (ELF32_R_TYPE (rel->r_info) == R_K1_GLOB_DAT)
        picrel->relocs32--;
      else /* we know (ELF32_R_TYPE (rel->r_info) == R_K1_FUNCDESC) */
        picrel->relocsfd--;
      _k1fdpic_count_relocs_fixups (picrel, dinfo, FALSE);
    }

  return TRUE;
}

static bfd_boolean
k1fdpic_elf_discard_info (bfd *ibfd,
                           struct elf_reloc_cookie *cookie ATTRIBUTE_UNUSED,
                           struct bfd_link_info *info)
{
  bfd_boolean changed = FALSE;
  asection *s;
  bfd *obfd = NULL;

  /* Account for relaxation of .eh_frame section.  */
  for (s = ibfd->sections; s; s = s->next)
    if (s->sec_info_type == ELF_INFO_TYPE_EH_FRAME)
      {
        if (!_k1fdpic_check_discarded_relocs (ibfd, s, info, &changed))
          return FALSE;
        obfd = s->output_section->owner;
      }

  if (changed)
    {
      struct _k1fdpic_dynamic_got_plt_info gpinfo;

      memset (&gpinfo, 0, sizeof (gpinfo));
      memcpy (&gpinfo.g, k1fdpic_dynamic_got_plt_info (info),
              sizeof (gpinfo.g));

      /* Clear GOT and PLT assignments.  */
      htab_traverse (k1fdpic_relocs_info (info),
                     _k1fdpic_reset_got_plt_entries,
                     NULL);

      if (!_k1fdpic_size_got_plt (obfd, &gpinfo))
        return FALSE;
    }

  return TRUE;
}

static bfd_boolean
elf_k1_modify_program_headers (bfd *output_bfd,
    struct bfd_link_info *info)
{
  struct elf_obj_tdata *tdata = elf_tdata (output_bfd);
  struct elf_segment_map *m;
  Elf_Internal_Phdr *p;

  /* objcopy and strip preserve what's already there using
   *      elf_k1_copy_private_bfd_data ().  */
  if (! info)
    return TRUE;

  for (p = tdata->phdr, m = tdata->segment_map; m != NULL; m = m->next, p++)
    if (m->p_type == PT_GNU_STACK)
      break;

  if (m)
  {
    struct elf_link_hash_entry *h;

    /* Obtain the pointer to the __stacksize
     * symbol.  */
    h = elf_link_hash_lookup (elf_hash_table (info), "__stacksize",
        FALSE, FALSE, FALSE);
    if (h)
    {
      while (h->root.type == bfd_link_hash_indirect
          || h->root.type == bfd_link_hash_warning)
        h = (struct elf_link_hash_entry *) h->root.u.i.link;
      BFD_ASSERT (h->root.type == bfd_link_hash_defined);
    }

    /* Set the header p_memsz from the symbol value.  We
     *    intentionally ignore the  symbol section.  */
    if (h && h->root.type == bfd_link_hash_defined)
      p->p_memsz = h->root.u.def.value;
    else
      p->p_memsz = DEFAULT_STACK_SIZE;

    p->p_align = 8;
  }

  return TRUE;
}

/* Copy backend specific data from one object module to another.  */

static bfd_boolean
k1_elf_copy_private_bfd_data (bfd *ibfd, bfd *obfd)
{
  if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour
      || bfd_get_flavour (obfd) != bfd_target_elf_flavour)
    return TRUE;

  BFD_ASSERT (!elf_flags_init (obfd)
        || elf_elfheader (obfd)->e_flags == elf_elfheader (ibfd)->e_flags);

  elf_elfheader (obfd)->e_flags = elf_elfheader (ibfd)->e_flags;
  elf_flags_init (obfd) = TRUE;

  /* Copy object attributes.  */
  _bfd_elf_copy_obj_attributes (ibfd, obfd);

  return TRUE;
}

static bfd_boolean
elf_k1_copy_private_bfd_data(bfd *ibfd, bfd *obfd)
{
  unsigned i;

  if (bfd_get_flavour (ibfd) != bfd_target_elf_flavour
      || bfd_get_flavour (obfd) != bfd_target_elf_flavour)
    return TRUE;

  if (! k1_elf_copy_private_bfd_data (ibfd, obfd))
    return FALSE;

  if (! elf_tdata (ibfd) || ! elf_tdata (ibfd)->phdr
      || ! elf_tdata (obfd) || ! elf_tdata (obfd)->phdr)
    return TRUE;

  /* Copy the stack size.  */
  for (i = 0; i < elf_elfheader (ibfd)->e_phnum; i++)
    if (elf_tdata (ibfd)->phdr[i].p_type == PT_GNU_STACK)
    {
      Elf_Internal_Phdr *iphdr = &elf_tdata (ibfd)->phdr[i];

      for (i = 0; i < elf_elfheader (obfd)->e_phnum; i++)
        if (elf_tdata (obfd)->phdr[i].p_type == PT_GNU_STACK)
        {
          memcpy (&elf_tdata (obfd)->phdr[i], iphdr, sizeof (*iphdr));

          /* Rewrite the phdrs, since we're only called after they were first written.  */
          if (bfd_seek (obfd, (bfd_signed_vma) get_elf_backend_data (obfd)->s->sizeof_ehdr, SEEK_SET) != 0
              || get_elf_backend_data (obfd)->s->write_out_phdrs (obfd, elf_tdata (obfd)->phdr, elf_elfheader (obfd)->e_phnum) != 0)
            return FALSE;
          break;
        }

      break;
    }

  return TRUE;
}

#ifndef ELF_ARCH
#define TARGET_LITTLE_SYM                       bfd_elf32_k1_vec
#define TARGET_LITTLE_NAME                      "elf32-k1"
#define ELF_ARCH                                bfd_arch_k1
#endif//ELF_ARCH

#define ELF_TARGET_ID                           K1_ELF_DATA
#define ELF_MACHINE_CODE                        EM_K1
#define ELF_MAXPAGESIZE                         0x1000
#define bfd_elf32_bfd_reloc_type_lookup         k1_reloc_type_lookup
#define bfd_elf32_bfd_reloc_name_lookup         k1_reloc_name_lookup
#define elf_info_to_howto                       k1_elf_info_to_howto
#define bfd_elf32_bfd_is_target_special_symbol  elf32_k1_is_target_special_symbol

#define bfd_elf32_bfd_link_hash_table_create    k1_elf_link_hash_table_create

#define elf_backend_can_gc_sections             1
#define elf_backend_relocate_section            k1_elf_relocate_section
#define elf_backend_rela_normal                 1



#define elf_backend_want_got_plt                0
#define elf_backend_plt_readonly                1
#define elf_backend_want_plt_sym                0
#define elf_backend_got_header_size             12



#define elf_backend_adjust_dynamic_symbol       k1_elf_adjust_dynamic_symbol
#define elf_backend_create_dynamic_sections     k1_elf_create_dynamic_sections
#define elf_backend_finish_dynamic_sections     k1_elf_finish_dynamic_sections
#define elf_backend_finish_dynamic_symbol       k1_elf_finish_dynamic_symbol
#define elf_backend_size_dynamic_sections       k1_elf_size_dynamic_sections


#define elf_backend_check_relocs                k1_check_relocs

#define bfd_elf32_bfd_merge_private_bfd_data    elf_k1_merge_private_bfd_data
#define bfd_elf32_bfd_set_private_flags         elf_k1_set_private_flags
#define bfd_elf32_bfd_print_private_bfd_data    elf_k1_print_private_bfd_data
#define elf_backend_final_write_processing      elf_k1_final_write_processing
#define elf_backend_object_p                    elf_k1_object_p

#define bfd_elf32_bfd_copy_private_bfd_data     elf_k1_copy_private_bfd_data
#define elf_backend_always_size_sections        elf_k1_always_size_sections
#define elf_backend_modify_program_headers      elf_k1_modify_program_headers
#define elf_backend_omit_section_dynsym         _k1fdpic_link_omit_section_dynsym
#define elf_backend_discard_info                k1fdpic_elf_discard_info
#define elf_backend_gc_sweep_hook               k1fdpic_gc_sweep_hook

#define elf_backend_may_use_rel_p       1
#define elf_backend_may_use_rela_p      1

#include "elf32-target.h"


