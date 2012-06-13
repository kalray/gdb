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
  { BFD_RELOC_K1_GOTOFF_LO10, R_K1_GOTOFF_LO10 },
  { BFD_RELOC_K1_GOTOFF_HI22, R_K1_GOTOFF_HI22 },
  { BFD_RELOC_K1_GOT_LO10,   R_K1_GOT_LO10 },
  { BFD_RELOC_K1_GOT_HI22,   R_K1_GOT_HI22 },
  { BFD_RELOC_K1_GLOB_DAT,   R_K1_GLOB_DAT },
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

  /* Small local sym to section mapping cache.  */
  struct sym_cache sym_sec;
};

/* Get the ELF linker hash table from a link_info structure.  */

#define elf32_k1_hash_table(p)                          \
  (elf_hash_table_id ((struct elf_link_hash_table *) ((p)->hash)) \
  == K1_ELF_DATA ? ((struct elf32_k1_link_hash_table *) ((p)->hash)) : NULL)

/* Create an entry in a k1 ELF linker hash table.  */

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


static bfd_boolean
elf32_k1_is_target_special_symbol (bfd * abfd ATTRIBUTE_UNUSED, asymbol * sym)
{
  return sym->name && sym->name[0] == '.' && sym->name[1] == 'L';
}


static bfd_vma elf32_k1_gp_base (bfd *output_bfd)
{
    bfd_vma res = _bfd_get_gp_value (output_bfd);
    bfd_vma min_vma = (bfd_vma) -1;
    asection *os;
      
    if (res) return res;
    
    /* Point GP at the lowest data section */
    for (os = output_bfd->sections; os ; os = os->next) {
	if ((os->flags & (SEC_ALLOC | SEC_DATA)) == (SEC_ALLOC | SEC_DATA)
	    && os->size > 0) {
	    if (os->vma < min_vma)
		min_vma = os->vma;
	}
    }

    _bfd_set_gp_value (output_bfd, min_vma);
    return min_vma;
}

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
  struct elf32_k1_link_hash_table *htab;
  Elf_Internal_Shdr *           symtab_hdr;
  struct elf_link_hash_entry ** sym_hashes;
  Elf_Internal_Rela *           rel;
  Elf_Internal_Rela *           relend;
  bfd *dynobj;
  bfd_vma *local_got_offsets;

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
      reloc_howto_type *           howto;
      unsigned long                r_symndx;
      Elf_Internal_Sym *           sym;
      asection *                   sec;
      struct elf_link_hash_entry * h;
      bfd_vma                      relocation;
      bfd_reloc_status_type        r;
      const char *                 name = NULL;
      int                          r_type;
      
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
	  sec = local_sections [r_symndx];

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
	}

      if (sec != NULL && elf_discarded_section (sec))
	{
	    RELOC_AGAINST_DISCARDED_SECTION (info, input_bfd, input_section, rel,
					     relend, howto, contents);
	    continue;
	}

      if (info->relocatable)
	continue;

      /* Finally, the sole K1-specific part.  */
      switch (r_type)
        {
	    /* Handle K1 specific things here */
	case R_K1_TPREL_LO10:
	case R_K1_TPREL_HI22:
	case R_K1_TPREL_32:
	    relocation -=  elf_hash_table (info)->tls_sec->vma;
	    break;
	case R_K1_GPREL_LO10:
	case R_K1_GPREL_HI22:
	    relocation -=  elf32_k1_gp_base (output_bfd);
	    break;
        case R_K1_GOT_LO10:
        case R_K1_GOT_HI22:
          if (htab->sgot == NULL)
            abort ();

          if (h != NULL)
            {
              bfd_vma off = h->got.offset;
              if (htab->sgotplt == NULL || off == (bfd_vma) -1)
                abort();
#ifdef DEBUG
              fprintf(stderr, " value: 0x%x got_addr: 0x%x got_off: 0x%x\n", relocation + rel->r_addend,
                      htab->sgot->contents, off);
#endif//DEBUG

              /* The LSB indicates whether we've already
                       created relocation.  */
              if ((off & 1) != 0)
                off &= ~1;
              else
                {
                  bfd_put_32 (output_bfd, relocation + rel->r_addend,
                              htab->sgot->contents + off);
                  h->got.offset |= 1;
                }

              relocation = htab->sgot->output_section->vma
                         + htab->sgot->output_offset
                         + off
                         - htab->sgotplt->output_section->vma
                         - htab->sgotplt->output_offset;
#ifdef DEBUG
              fprintf(stderr, "    new value: 0x%x\n", relocation);
#endif//DEBUG
            } // local got offsets
            else
              abort (); /* ??? */
          break;
        case R_K1_GOTOFF_LO10:
        case R_K1_GOTOFF_HI22:
          if (htab->sgotplt != NULL)
            {
#ifdef DEBUG
              fprintf(stderr, " old_reloc:       0x%x\n"
                              " addend:          0x%x\n"
                              " offset:          0x%x\n"
                              " section_vma:     0x%x\n"
                              " sgot_out_offset: 0x%x\n"
                              " x:               0x%x\n",
                      relocation,
                      rel->r_addend,
                      rel->r_offset,
                      htab->sgotplt->output_section->vma,
                      htab->sgotplt->output_offset,
                      relocation-htab->sgotplt->output_section->vma
                     );
#endif//DEBUG

              relocation += rel->r_addend;
              relocation -= htab->sgotplt->output_section->vma
                      + htab->sgotplt->output_offset;
#ifdef DEBUG
              fprintf(stderr, " new value:       0x%x\n", relocation);
#endif//DEBUG
            }
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

/* Create .got, .gotplt, and .rela.got sections in DYNOBJ, and set up
   shortcuts to them in our hash table.  */

static bfd_boolean
create_got_section (bfd *dynobj, struct bfd_link_info *info)
{
  struct elf32_k1_link_hash_table *htab;

  if (! _bfd_elf_create_got_section (dynobj, info))
    return FALSE;
  htab = elf32_k1_hash_table (info);
  if (htab == NULL)
    return FALSE;

  htab->sgot = bfd_get_section_by_name (dynobj, ".got");
  htab->sgotplt = bfd_get_section_by_name (dynobj, ".got.plt");
  if (!htab->sgot || !htab->sgotplt)
    return FALSE;

  htab->srelgot = bfd_get_section_by_name (dynobj, ".rela.got");

  if (htab->srelgot == NULL
      || ! bfd_set_section_flags (dynobj, htab->srelgot, SEC_ALLOC
                                  | SEC_LOAD
                                  | SEC_HAS_CONTENTS
                                  | SEC_IN_MEMORY
                                  | SEC_LINKER_CREATED
                                  | SEC_READONLY)
      || ! bfd_set_section_alignment (dynobj, htab->srelgot, 2))
    return FALSE;

  return TRUE;
}


/* Look through the relocs for a section during the first phase.  */

static bfd_boolean
k1_check_relocs (bfd * abfd,
                 struct bfd_link_info *info,
                 asection *sec,
                 const Elf_Internal_Rela *relocs)
{
  Elf_Internal_Shdr *           symtab_hdr;
  struct elf_link_hash_entry ** sym_hashes;
  struct elf_link_hash_entry ** sym_hashes_end;
  const Elf_Internal_Rela *     rel;
  const Elf_Internal_Rela *     rel_end;
  struct elf32_k1_link_hash_table *htab;

  if (info->relocatable)
    return TRUE;

  htab = elf32_k1_hash_table (info);
  if (htab == NULL)
    return FALSE;

  symtab_hdr = & elf_tdata (abfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (abfd);
  sym_hashes_end = sym_hashes + symtab_hdr->sh_size / sizeof (Elf32_External_Sym);
  if (!elf_bad_symtab (abfd))
    sym_hashes_end -= symtab_hdr->sh_info;

  rel_end = relocs + sec->reloc_count;

  for (rel = relocs; rel < rel_end; rel++)
    {
      unsigned int r_type;
      struct elf_link_hash_entry * h;
      unsigned long r_symndx;

      r_symndx = ELF32_R_SYM (rel->r_info);
      r_type = ELF32_R_TYPE (rel->r_info);

      if (r_symndx < symtab_hdr->sh_info)
        h = NULL;
      else
        h = sym_hashes [r_symndx - symtab_hdr->sh_info];

      switch (r_type)
        {
            /* This relocation requires .got entry.  */
        case R_K1_GOT_HI22:
        case R_K1_GOT_LO10:
          if (htab->sgot == NULL)
            {
              if (htab->elf.dynobj == NULL)
                htab->elf.dynobj = abfd;
              if (!create_got_section (htab->elf.dynobj, info))
                return FALSE;
            }
          if (h != NULL)
            {
              h->got.refcount += 1;
            }
          else
            {
              bfd_signed_vma *local_got_refcounts;

              /* This is a global offset table entry for a local symbol.  */
              local_got_refcounts = elf_local_got_refcounts (abfd);
              if (local_got_refcounts == NULL)
                {
                  bfd_size_type size;

                  size = symtab_hdr->sh_info;
                  size *= sizeof (bfd_signed_vma);
                  local_got_refcounts = bfd_zalloc (abfd, size);
                  if (local_got_refcounts == NULL)
                    return FALSE;
                  elf_local_got_refcounts (abfd) = local_got_refcounts;
                }
              local_got_refcounts[r_symndx] += 1;
            }
          break;
        }
    }
  return TRUE;
}

static bfd_boolean
k1_elf_create_dynamic_sections (bfd *dynobj, struct bfd_link_info *info)
{
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
}



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

#if 0
/* TODO: AP: No PLT for now */
  if (htab->elf.dynamic_sections_created
      && h->plt.refcount > 0)
    {
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
            s->size = PLT_ENTRY_SIZE;

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
          s->size += PLT_ENTRY_SIZE;

          /* We also need to make an entry in the .got.plt section, which
             will be placed in the .got section by the linker script.  */
          htab->sgotplt->size += 4;

          /* We also need to make an entry in the .rel.plt section.  */
          htab->srelplt->size += sizeof (Elf32_External_Rela);
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
#endif//0

  h->plt.offset = (bfd_vma) -1;
  h->needs_plt = 0;

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



/* Set the sizes of the dynamic sections.  */

static bfd_boolean
k1_elf_size_dynamic_sections (bfd *output_bfd ATTRIBUTE_UNUSED,
                                      struct bfd_link_info *info)
{
  bfd *dynobj;
  asection *s;
  bfd *ibfd;
  struct elf_link_hash_table *htab = elf_hash_table (info);

  if (htab == NULL)
    return FALSE;

  dynobj = htab->dynobj;
  BFD_ASSERT (dynobj != NULL);

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

  if (elf_hash_table (info)->dynamic_sections_created)
    {
      /* Make space for the trailing nop in .plt.  */
      if (htab->splt->size > 0)
        htab->splt->size += 4;
    }

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
}

static bfd_boolean
k1_elf_adjust_dynamic_symbol (struct bfd_link_info *info,
                              struct elf_link_hash_entry *h)
{
  return TRUE;
}


/* Finish up dynamic symbol handling.  We set the contents of various
   dynamic sections here.  */

static bfd_boolean
k1_elf_finish_dynamic_symbol (bfd *output_bfd,
                              struct bfd_link_info *info,
                              struct elf_link_hash_entry *h,
                              Elf_Internal_Sym *sym)
{  
  /* Mark some specially defined symbols as absolute.  */
  if (strcmp (h->root.root.string, "_DYNAMIC") == 0
      || strcmp (h->root.root.string, "_GLOBAL_OFFSET_TABLE_") == 0
      || strcmp (h->root.root.string, "_PROCEDURE_LINKAGE_TABLE_") == 0)
      sym->st_shndx = SHN_ABS;

  return TRUE;
}

/* Finish up the dynamic sections.  */

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
//           memset (splt->contents, 0, PLT_ENTRY_SIZE);
//           bfd_put_32 (output_bfd, (bfd_vma) 0x80000000 /* nop.  */,
//                       splt->contents + splt->size - 4);
//         }
//
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
#define ELF_MAXPAGESIZE                         64
#define bfd_elf32_bfd_reloc_type_lookup         k1_reloc_type_lookup
#define bfd_elf32_bfd_reloc_name_lookup         k1_reloc_name_lookup
#define elf_info_to_howto                       k1_elf_info_to_howto
#define bfd_elf32_bfd_is_target_special_symbol  elf32_k1_is_target_special_symbol

#define bfd_elf32_bfd_link_hash_table_create    k1_elf_link_hash_table_create

#define elf_backend_can_gc_sections             1
#define elf_backend_relocate_section            k1_elf_relocate_section
#define elf_backend_rela_normal                 1



#define elf_backend_want_got_plt                1
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

#include "elf32-target.h"


