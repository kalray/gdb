#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"
#include "elf-bfd.h"
#include <elf/k1.h>

#include "elfxx-k1.h"

/* void k1_elf_info_to_howto (bfd *, arelent *, Elf_Internal_Rela *); */

/* #define K1DP_K1IO_K1BDP_K1BIO */
/* #include "elf32-k1.def" */
/* #undef K1DP_K1IO_K1BDP_K1BIO */

#define DEFAULT_STACK_SIZE 0x20000

#define ABI_64_P(abfd) \
  (get_elf_backend_data (abfd)->s->elfclass == ELFCLASS64)

/* struct k1_reloc_map */
/* { */
/*   bfd_reloc_code_real_type bfd_reloc_val; */
/*   unsigned int k1_reloc_val; */
/* }; */


const struct k1_reloc_map k1_reloc_map[]=
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
  { BFD_RELOC_K1_GOTOFF,     R_K1_GOTOFF },
  { BFD_RELOC_K1_GOT,        R_K1_GOT },
  { BFD_RELOC_K1_10_GPREL,   R_K1_10_GPREL },
  { BFD_RELOC_K1_16_GPREL,   R_K1_16_GPREL },
  { BFD_RELOC_K1_COPY,       R_K1_COPY },
  { BFD_RELOC_K1_JMP_SLOT,   R_K1_JMP_SLOT },
  { BFD_RELOC_K1_RELATIVE,   R_K1_RELATIVE },
  { BFD_RELOC_K1_EXTEND6,    R_K1_EXTEND6 },
  { BFD_RELOC_K1_HI27,       R_K1_HI27 },
  { BFD_RELOC_K1_ELO10,      R_K1_ELO10 },

/*  { BFD_RELOC_K1_PCREL_LO10, R_K1_PCREL_LO10 },
  { BFD_RELOC_K1_PCREL_HI22, R_K1_PCREL_HI22 }, */
};

const int k1_reloc_map_len = sizeof(k1_reloc_map) / sizeof (struct k1_reloc_map);


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
                                 sizeof (struct k1_elf_link_hash_entry));
      if (entry == NULL)
        return entry;
    }
  /* Call the allocation method of the superclass.  */
  entry = _bfd_elf_link_hash_newfunc (entry, table, string);
  if (entry != NULL)
    {
      struct k1_elf_link_hash_entry *eh;

      eh = (struct k1_elf_link_hash_entry *) entry;
      eh->dyn_relocs = NULL;
    }

  return entry;
}

/* Create a k1 ELF linker hash table.  */

struct bfd_link_hash_table *
k1_elf_link_hash_table_create (bfd *abfd)
{
  struct k1_elf_link_hash_table *ret;
  bfd_size_type amt = sizeof (struct k1_elf_link_hash_table);

  ret = (struct k1_elf_link_hash_table *) bfd_zmalloc (amt);
  if (ret == NULL)
    return NULL;
#ifdef BFD64
  if (ABI_64_P (abfd))
    {
      ret->bytes_per_rela = sizeof (Elf64_External_Rela);
      /* ret->dtpoff_reloc = R_TILEGX_TLS_DTPOFF64; */
      /* ret->dtpmod_reloc = R_TILEGX_TLS_DTPMOD64; */
      /* ret->tpoff_reloc = R_TILEGX_TLS_TPOFF64; */
      /* ret->r_info = tilegx_elf_r_info_64; */
      /* ret->r_symndx = tilegx_elf_r_symndx_64; */
      /* ret->dynamic_interpreter = ELF64_DYNAMIC_INTERPRETER; */
      /* ret->put_word = tilegx_put_word_64; */
    }
  else
#endif
    {
      ret->bytes_per_rela = sizeof (Elf64_External_Rela);
    }
  
  if (!_bfd_elf_link_hash_table_init (&ret->elf, abfd, link_hash_newfunc,
                                      sizeof (struct k1_elf_link_hash_entry),
                                      K1_ELF_DATA))
    {
      free (ret);
      return NULL;
    }

  return &ret->elf.root;
}

/* Update the got entry reference counts for the section being removed.  */

bfd_boolean
k1_gc_sweep_hook (bfd * abfd,
		  struct bfd_link_info *info,
		  asection * sec,
		  const Elf_Internal_Rela * relocs)
{
  Elf_Internal_Shdr *symtab_hdr;
  struct elf_link_hash_entry **sym_hashes;
  bfd_signed_vma *local_got_refcounts;
  const Elf_Internal_Rela *rel, *relend;
  bfd *dynobj;
  asection *sgot;
  asection *srelgot;
  asection *splt;
  asection *srelplt;
  asection *sgotplt;

  struct k1_elf_link_hash_table *htab;
  htab = k1_elf_hash_table (info);
  
  dynobj = elf_hash_table (info)->dynobj;
  if (dynobj == NULL)
    return TRUE;

  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (abfd);
  local_got_refcounts = elf_local_got_refcounts (abfd);

  sgot = bfd_get_section_by_name (dynobj, ".got");
  srelgot = bfd_get_section_by_name (dynobj, ".rela.got");
  splt = bfd_get_section_by_name (dynobj, ".plt");
  sgotplt = bfd_get_section_by_name (dynobj, ".got.plt");
  srelplt = bfd_get_section_by_name (dynobj, ".rela.plt");

  relend = relocs + sec->reloc_count;
  for (rel = relocs; rel < relend; rel++)
    {
      unsigned long r_symndx;
      struct elf_link_hash_entry *h = NULL;

      switch (ELF_R_TYPE (abfd, rel->r_info))
        {
        case R_K1_GOT:
          r_symndx = ELF_R_SYM (abfd, rel->r_info);
          if (r_symndx >= symtab_hdr->sh_info)
            {
              h = sym_hashes[r_symndx - symtab_hdr->sh_info];
              if (h->got.refcount > 0)
                {
                  --h->got.refcount;
                  if (h->got.refcount == 0)
                    {
                      /* We don't need the .got entry any more.  */
                      sgot->size -= 4;
                      srelgot->size -= k1_elf_rela_bytes(htab);
                    }
                }
            }
          else if (local_got_refcounts != NULL)
            {
              if (local_got_refcounts[r_symndx] > 0)
                {
                  --local_got_refcounts[r_symndx];
                  if (local_got_refcounts[r_symndx] == 0)
                    {
                      /* We don't need the .got entry any more.  */
                      sgot->size -= 4;
                      srelgot->size -= k1_elf_rela_bytes(htab);
                    }
                }
            }
          case R_K1_32:
          case R_K1_27_PCREL:
            if (info->shared && h == NULL)
              break;
            if (h != NULL)
            {
              if (h->plt.refcount > 0)
		{
		  --h->plt.refcount;
		  if (h->plt.refcount == 0)
		    {
		      sgotplt->size -=4;
		      srelplt->size -= k1_elf_rela_bytes(htab);
		    }
		}
            }
          break;
        default:
          break;
        }
    }
  return TRUE;
}

/* Merge backend specific data from an object file to the output
   object file when linking.  */

bfd_boolean
k1_merge_private_bfd_data (bfd *ibfd, bfd *obfd)
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

  if (in_flags & ELF_K1_FDPIC)
    in_flags &= ~ELF_K1_PIC;

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

  if (((in_flags & ELF_K1_FDPIC) == 0) != (! IS_FDPIC (obfd)))
    {
      if (IS_FDPIC (obfd))
        (*_bfd_error_handler)
          (_("%s: cannot link non-fdpic object file into fdpic executable"),
            bfd_get_filename (ibfd));
      else
        (*_bfd_error_handler)
          (_("%s: cannot link fdpic object file into non-fdpic executable"),
            bfd_get_filename (ibfd));
      bfd_set_error (bfd_error_bad_value);
      return FALSE;
    }

  return TRUE;
}


/* Compute the number of dynamic relocations and fixups that a symbol
   requires, and add (or subtract) from the grand and per-symbol
   totals.  */
bfd_boolean
k1_adjust_dynamic_symbol (struct bfd_link_info *info,
                                struct elf_link_hash_entry *h)
{
  struct k1_elf_link_hash_table *htab;
  struct k1_elf_link_hash_entry * eh;
  struct k1_elf_dyn_relocs *p;
  asection *sdynbss, *s;
  unsigned int power_of_two;
  bfd *dynobj;

  htab = k1_elf_hash_table (info);
  if (htab == NULL)
    return FALSE;

  /* If this is a function, put it in the procedure linkage table.  We
     will fill in the contents of the procedure linkage table later,
     when we know the address of the .got section.  */
  if (h->type == STT_FUNC || h->needs_plt)
    {
      if (h->plt.refcount < 0
          || SYMBOL_CALLS_LOCAL (info, h)
          || (ELF_ST_VISIBILITY (h->other) != STV_DEFAULT
              && h->root.type == bfd_link_hash_undefweak))
        {
          /* This case can occur if we saw a PLT reloc in an input
             file, but the symbol was never referred to by a dynamic
             object, or if all references were garbage collected.  In
             such a case, we don't actually need to build a procedure
             linkage table, and we can just do a PC32 reloc instead.  */
          h->plt.offset = (bfd_vma) -1;
          h->needs_plt = 0;
        }
      return TRUE;
    }
  else
    /* It's possible that we incorrectly decided a .plt reloc was
       needed for an R_K1_27_PCREL reloc to a non-function sym in
       check_relocs.  We can't decide accurately between function and
       non-function syms in check-relocs;  Objects loaded later in
       the link may change h->type.  So fix it now.  */
    h->plt.offset = (bfd_vma) -1;

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

  /* This is a reference to a symbol defined by a dynamic object which
     is not a function.  */

  /* If we are creating a shared library, we must presume that the
     only references to the symbol are via the global offset table.
     For such cases we need not do anything here; the relocations will
     be handled correctly by relocate_section.  */
  if (info->shared)
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

  eh = (struct k1_elf_link_hash_entry *) h;
  for (p = eh->dyn_relocs; p != NULL; p = p->next)
    {
      s = p->sec->output_section;
      if (s != NULL && (s->flags & SEC_READONLY) != 0)
        break;
    }

  /* If we didn't find any dynamic relocs in read-only sections, then
     we'll be keeping the dynamic relocs and avoiding the copy reloc.  */
  if (p == NULL)
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

  /* We must generate a R_K1_COPY reloc to tell the dynamic linker to
     copy the initial value out of the dynamic object and into the
     runtime process image. */
  dynobj = htab->elf.dynobj;
  BFD_ASSERT (dynobj != NULL);
  if ((h->root.u.def.section->flags & SEC_ALLOC) != 0)
    {
      htab->srelbss->size += sizeof (Elf64_External_Rela);
      h->needs_copy = 1;
    }

  /* We need to figure out the alignment required for this symbol.  I
     have no idea how ELF linkers handle this.  */
  power_of_two = bfd_log2 (h->size);
  if (power_of_two > 3)
    power_of_two = 3;

  sdynbss = htab->sdynbss;
  /* Apply the required alignment.  */
  sdynbss->size = BFD_ALIGN (sdynbss->size, (bfd_size_type) (1 << power_of_two));
  if (power_of_two > bfd_get_section_alignment (dynobj, sdynbss))
    {
      if (! bfd_set_section_alignment (dynobj, sdynbss, power_of_two))
        return FALSE;
    }

  /* Define the symbol as being at this point in the section.  */
  h->root.u.def.section = sdynbss;
  h->root.u.def.value = sdynbss->size;

  /* Increment the section size to make room for the symbol.  */
  sdynbss->size += h->size;
  return TRUE;
}


/* Allocate space in .plt, .got and associated reloc sections for
   dynamic relocs.  */

bfd_boolean
k1_allocate_dynrelocs (struct elf_link_hash_entry *h, void * dat)
{
  struct bfd_link_info *info;
  struct k1_elf_link_hash_table *htab;
  struct k1_elf_link_hash_entry *eh;
  struct k1_elf_dyn_relocs *p;

  if (h->root.type == bfd_link_hash_indirect)
    return TRUE;

  if (h->root.type == bfd_link_hash_warning)
    /* When warning symbols are created, they **replace** the "real"
       entry in the hash table, thus we never get to see the real
       symbol in a hash traversal.  So look at it now.  */
    h = (struct elf_link_hash_entry *) h->root.u.i.link;

  info = (struct bfd_link_info *) dat;
  htab = k1_elf_hash_table (info);
  if (htab == NULL)
    return FALSE;

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
            s->size =  PLT_ENTRY_SIZE;

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
          s->size +=  PLT_ENTRY_SIZE;

          /* We also need to make an entry in the .got.plt section, which
             will be placed in the .got section by the linker script.  */
          htab->sgotplt->size += 4;

          /* We also need to make an entry in the .rel.plt section.  */
          htab->srelplt->size += k1_elf_rela_bytes (htab); //sizeof (Elf32_External_Rela);
          /*fprintf(stderr, "  srelplt size: %d\n", htab->elf.srelplt->size);*/
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

/*  if (h->got.refcount > 0
    && info->executable
    && h->dynindx == -1 )
     h->got.offset = (bfd_vma) -1;
  else */
  if (h->got.refcount > 0)
    {
      asection *s;
      /* Make sure this symbol is output as a dynamic symbol.
         Undefined weak syms won't yet be marked as dynamic.  */
      if (h->dynindx == -1
          && !h->forced_local && !info->shared)
        {
          if (! bfd_elf_link_record_dynamic_symbol (info, h))
            return FALSE;
        }

      s = htab->sgot;
      h->got.offset = s->size;
      s->size += 4;
      htab->srelgot->size += k1_elf_rela_bytes (htab); //sizeof (Elf32_External_Rela);
    }
  else
    h->got.offset = (bfd_vma) -1;

  eh = (struct k1_elf_link_hash_entry *) h;
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
          struct k1_elf_dyn_relocs **pp;

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
      sreloc->size += p->count * k1_elf_rela_bytes (htab); //sizeof (Elf32_External_Rela);
    }

  return TRUE;
}

bfd_boolean
k1_elf32_fdpic_create_got_section (bfd *abfd, struct bfd_link_info *info)
{
  flagword flags, pltflags;
  asection *s;
  struct elf_link_hash_entry *h;
  struct bfd_link_hash_entry *bh;
  const struct elf_backend_data *bed = get_elf_backend_data (abfd);
  int ptralign;
  int offset;


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
   if (IS_FDPIC (abfd))
    {
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
      offset = -2048;
      flags = BSF_GLOBAL;
    }
  else
    {
      offset = 2048;
      flags = BSF_GLOBAL | BSF_WEAK;
    }

      /* Define _gp in .rofixup, for FDPIC.  If it
     turns out that we're linking with a different linker script, the
     linker script will override it.  */
  bh = NULL;
  if (!(_bfd_generic_link_add_one_symbol
        (info, abfd, "_gp", flags, s, offset, (const char *) NULL, FALSE,
         bed->collect, &bh)))
    return FALSE;
  h = (struct elf_link_hash_entry *) bh;
  h->def_regular = 1;
  h->type = STT_OBJECT;
  h->other = STV_HIDDEN;

  /* Machine-specific: we want the symbol for executables as well.  */
  if (IS_FDPIC (abfd) && ! bfd_elf_link_record_dynamic_symbol (info, h))
    return FALSE;

//   if (!IS_FDPIC (abfd))
//     return TRUE;

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
      bh = NULL;

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


bfd_vma
k1_gp_base (bfd *output_bfd, struct bfd_link_info *info)
{
    bfd_vma res = _bfd_get_gp_value (output_bfd);
    struct elf_link_hash_entry *gp;
    bfd_vma gp_val = (bfd_vma) -1;

    /* Point GP at _data_start symbol */
    
    if (res) return res;

    if (IS_FDPIC(output_bfd))
      {
        gp = elf_link_hash_lookup (elf_hash_table (info), "_gp", FALSE, FALSE, TRUE);
        if (gp)
          gp_val = gp->root.u.def.value
                 + gp->root.u.def.section->output_section->vma
                 + gp->root.u.def.section->output_offset;
      }
    else
      {
        gp = elf_link_hash_lookup (elf_hash_table (info), "_data_start", FALSE,
                                 FALSE, FALSE);
        if (gp)
          gp_val = gp->root.u.def.value
                 + gp->root.u.def.section->output_section->vma
                 + gp->root.u.def.section->output_offset;
      }

    _bfd_set_gp_value (output_bfd, gp_val);
    return gp_val;
}

bfd_boolean
k1_size_dynamic_sections (bfd * output_bfd ATTRIBUTE_UNUSED,
                                struct bfd_link_info *info)
{
  struct k1_elf_link_hash_table *htab;
  bfd *dynobj;
  asection *s;
  bfd *ibfd;

  htab = k1_elf_hash_table (info);
  dynobj = htab->elf.dynobj;
  
  BFD_ASSERT (dynobj != NULL);
  
  DPRINT("size_dynamic_sections");

  if (htab->elf.dynamic_sections_created)
    {
      DPRINT("dynsec created");
      /* Set the contents of the .interp section to the interpreter.  */
      if (info->executable)
        {
          s = bfd_get_section_by_name (dynobj, ".interp");
          BFD_ASSERT (s != NULL);
          s->size = sizeof ELF_DYNAMIC_INTERPRETER;
          s->contents = (unsigned char *) ELF_DYNAMIC_INTERPRETER;
        }
    }
  else
    {
      /* We may have created entries in the .rela.got section.
         However, if we are not creating the dynamic sections, we will
         not actually use these entries.  Reset the size of .rela.got,
         which will cause it to get stripped from the output file
         below.  */
      s = htab->srelgot;
      if (s != NULL)
        s->size = 0;
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

      for (s = ibfd->sections; s != NULL; s = s->next)
        {
          struct k1_elf_dyn_relocs *p;

          for (p = ((struct k1_elf_dyn_relocs *)
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
                  srel->size += p->count * k1_elf_rela_bytes (htab);
                  if ((p->sec->output_section->flags & SEC_READONLY) != 0)
                    info->flags |= DF_TEXTREL;
                }
            }
        }

      /* Check if we have local got references at all */
      local_got = elf_local_got_refcounts (ibfd);
      if (!local_got)
        continue;

      symtab_hdr = &elf_symtab_hdr (ibfd);
      locsymcount = symtab_hdr->sh_info;
      end_local_got = local_got + locsymcount;
      s = htab->sgot;
      srel = htab->srelgot;

      /* Make sure we have created the .got and .rela.got sections
       * before.
       */
      BFD_ASSERT (s != NULL && srel != NULL);

      for (; local_got < end_local_got; ++local_got)
        {
          if (*local_got > 0)
            {
              *local_got = s->size;
              s->size += 4;
              if (info->shared)
                srel->size += k1_elf_rela_bytes (htab);
            }
            else
              *local_got = (bfd_vma) -1;
        }
    }

  /* Allocate global sym .plt and .got entries, and space for global
     sym dynamic relocs.  */
  elf_link_hash_traverse (&htab->elf, k1_allocate_dynrelocs, info);
  
  if (htab->sgotplt)
    {
      struct elf_link_hash_entry *got;
      got = elf_link_hash_lookup (&htab->elf,
                                  "_GLOBAL_OFFSET_TABLE_",
                                  FALSE, FALSE, FALSE);

      /* Don't allocate .got.plt section if there are no GOT nor PLT
         entries and there is no reference to _GLOBAL_OFFSET_TABLE_.  */
      if ((got == NULL
           || !got->ref_regular_nonweak)
          && (htab->sgotplt->size
              == get_elf_backend_data (output_bfd)->got_header_size)
          && (htab->splt == NULL
              || htab->splt->size == 0)
          && (htab->sgot == NULL
              || htab->sgot->size == 0))
        htab->sgotplt->size = 0;
    }

 /* We now have determined the sizes of the various dynamic sections.
     Allocate memory for them.  */
  for (s = dynobj->sections; s != NULL; s = s->next)
    {
      const char *name;
      bfd_boolean strip_section = FALSE;

      if ((s->flags & SEC_LINKER_CREATED) == 0)
        continue;

      name = bfd_get_section_name (dynobj, s);
      if (strncmp (name, ".rela", 5) == 0)
        {
          if (s->size == 0) strip_section = TRUE;
          else s->reloc_count = 0;
        }
      else if (s != htab->splt && s != htab->sgot && s != htab->sgotplt)
        continue;

      if (strip_section)
        {
          s->flags |= SEC_EXCLUDE;
          continue;
        }

      if ((s->flags & SEC_HAS_CONTENTS) == 0 ||
          (s->size == 0 && s != htab->sgotplt && !htab->needs_got))
        continue;

      /* Allocate memory for the section contents.  We use bfd_zalloc
         here in case unused entries are not reclaimed before the
         section's contents are written out.  This should not happen,
         but this way if it does, we get a R_K1_NONE reloc instead
         of garbage.  */
      s->contents = (bfd_byte *) bfd_zalloc (dynobj, s->size);
      /*fprintf(stderr, ">> sname: %s, addr: 0x%lx, size: 0x%lx\n", name, s->contents, s->size);*/
      if (s->contents == NULL && s->size != 0)
        return FALSE;
    }
    

  if (htab->elf.dynamic_sections_created)
    {
      /* Add some entries to the .dynamic section.  We fill in the
         values later, in k1_finish_dynamic_sections, but we
         must add the entries now so that we get the correct size for
         the .dynamic section.  The DT_DEBUG entry is filled in by the
         dynamic linker and used by the debugger.  */
#define add_dynamic_entry(TAG, VAL) \
  _bfd_elf_add_dynamic_entry (info, TAG, VAL)

      if (info->executable)
        {
          if (!add_dynamic_entry (DT_DEBUG, 0))
            return FALSE;
        }
        
      if (htab->splt->size != 0)
        {
          if (!add_dynamic_entry (DT_PLTGOT, 0)
              || !add_dynamic_entry (DT_PLTRELSZ, 0)
              || !add_dynamic_entry (DT_PLTREL, DT_RELA)
              || !add_dynamic_entry (DT_JMPREL, 0))
            return FALSE;
        }

      if (!add_dynamic_entry (DT_RELA, 0)
          || !add_dynamic_entry (DT_RELASZ, 0)
          || !add_dynamic_entry (DT_RELAENT, k1_elf_rela_bytes (htab)))
            return FALSE;
        
      if ((info->flags & DF_TEXTREL) != 0)
        {
          if (!add_dynamic_entry (DT_TEXTREL, 0))
            return FALSE;
        }       
    }
#undef add_dynamic_entry

  return TRUE;
}

/* Compute a hash with the key fields of an k1fdpic_relocs_info entry.  */
hashval_t
k1fdpic_relocs_info_hash (const void *entry_)
{
  const struct k1fdpic_relocs_info *entry = entry_;

  return (entry->symndx == -1
          ? (long) entry->d.h->root.root.hash
          : entry->symndx + (long) entry->d.abfd->id * 257) + entry->addend;
}

/* Test whether the key fields of two k1fdpic_relocs_info entries are
   identical.  */
int
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
struct k1fdpic_relocs_info *
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
struct k1fdpic_relocs_info *
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


/* Obtain the address of the entry in HT associated with the SYMNDXth
   local symbol of the input bfd ABFD, plus the addend, creating a new
   entry if none existed.  */
struct k1fdpic_relocs_info *
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

void
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






/* Add a dynamic relocation to the SRELOC section.  */

bfd_vma
k1_elf32_fdpic_add_dyn_reloc (bfd *output_bfd, asection *sreloc, bfd_vma offset,
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

bfd_vma
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
      /* See discussion about symndx == 0 in k1_elf32_fdpic_add_dyn_reloc
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
int
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


bfd_boolean
elf32_k1_is_target_special_symbol (bfd * abfd ATTRIBUTE_UNUSED, asymbol * sym)
{
  return sym->name && sym->name[0] == '.' && sym->name[1] == 'L';
}



/* Functions for dealing with the e_flags field.  */

bfd_boolean
elf_k1_set_private_flags (bfd *abfd, flagword flags)
{
  BFD_ASSERT (!elf_flags_init (abfd)
	      || elf_elfheader (abfd)->e_flags == flags);

  elf_elfheader (abfd)->e_flags |= flags;
  elf_flags_init (abfd) = TRUE;

  return TRUE;
}

bfd_boolean
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
  case bfd_mach_k1bdp:
	fprintf (f, "\nMachine     = k1bdp\n");
	break;
  case bfd_mach_k1bio:
	fprintf (f, "\nMachine     = k1bio\n");
	break;
  case bfd_mach_k1bdp_64:
	fprintf (f, "\nMachine     = k1bdp:64\n");
	break;
  case bfd_mach_k1bio_64:
	fprintf (f, "\nMachine     = k1bio:64\n");
	break;

  default:
    fprintf (f, "\nMachine Id  = 0x%x\n", e_flags & K1_MACH_MASK);
  }

  if (e_flags & ELF_K1_PIC)
    fprintf(f, "\n-fpic\n");

  if (e_flags & ELF_K1_FDPIC)
    fprintf(f, "\n-mfdpic\n");

  return _bfd_elf_print_private_bfd_data (abfd, farg);
}


/* The final processing done just before writing out an K1 ELF object
   file.  This gets the K1 architecture right based on the machine
   number.  */

void
elf_k1_final_write_processing (bfd *abfd,
				   bfd_boolean linker ATTRIBUTE_UNUSED)
{
  int mach;
  unsigned long val;

  switch (mach = bfd_get_mach (abfd))
    {
    case bfd_mach_k1dp:
    case bfd_mach_k1io:
    case bfd_mach_k1bdp:
    case bfd_mach_k1bio:
    case bfd_mach_k1bdp_64:
    case bfd_mach_k1bio_64:
      val = mach;
      break;
    default:
      return;
    }

  elf_elfheader (abfd)->e_flags &=  (~ K1_MACH_MASK);
  elf_elfheader (abfd)->e_flags |= val;
}

/* Set the right machine number for an K1 ELF file.  */

bfd_boolean
elf_k1_object_p (bfd *abfd)
{
  int mach = elf_elfheader (abfd)->e_flags & K1_MACH_MASK;

  switch (mach) {
    case bfd_mach_k1dp:
    case bfd_mach_k1io:
    case bfd_mach_k1bdp:
    case bfd_mach_k1bio:
    case bfd_mach_k1bdp_64:
    case bfd_mach_k1bio_64:
      break;
    default:
      return FALSE;
    }

  (void) bfd_default_set_arch_mach (abfd, bfd_arch_k1, mach);
  return TRUE;
}


/* Find the segment number in which OSEC, and output section, is
   located.  */

 unsigned
_k1fdpic_osec_to_segment (bfd *output_bfd, asection *osec)
{
  Elf_Internal_Phdr *p = _bfd_elf_find_segment_containing_section (output_bfd, osec);

  return (p != NULL) ? p - elf_tdata (output_bfd)->phdr : -1;
}

bfd_boolean
_k1fdpic_osec_readonly_p (bfd *output_bfd, asection *osec)
{
  unsigned seg = _k1fdpic_osec_to_segment (output_bfd, osec);

  return ! (elf_tdata (output_bfd)->phdr[seg].p_flags & PF_W);
}


/* Generate relocations for GOT entries, function descriptors, and
   code for PLT and lazy PLT entries.  */


/* Copied from elf32-mt.c as this implementation seemd the most clean,
   simple and generic. */





/* Update the relocation information for the relocations of the section
   being removed.  */



/* We need dynamic symbols for every section, since segments can
   relocate independently.  */
bfd_boolean
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

bfd_boolean
k1_create_got_section (bfd *dynobj, struct bfd_link_info *info)
{
  struct k1_elf_link_hash_table *htab;

  if (! _bfd_elf_create_got_section (dynobj, info))
    return FALSE;
  htab = k1_elf_hash_table (info);
  if (htab == NULL)
    return FALSE;

  htab->sgot = bfd_get_section_by_name (dynobj, ".got");
  htab->sgotplt = bfd_get_section_by_name (dynobj, ".got.plt");
  if (!htab->sgot || !htab->sgotplt)
    return FALSE;

  htab->srelgot = bfd_get_section_by_name (dynobj, ".rela.got");
  
  return TRUE;
}

/* Create .got, .gotplt, and .rela.got sections in DYNOBJ, and set up
   shortcuts to them in our hash table.  */


/* Look through the relocs for a section during the first phase, and
   allocate space in the global offset table or procedure linkage
   table.  */





bfd_boolean
k1_create_dynamic_sections (bfd *dynobj, struct bfd_link_info *info)
{
  struct k1_elf_link_hash_table *htab;

  DPRINT("create_dynamic_sections");

  htab = k1_elf_hash_table (info);
  if (htab == NULL)
    return FALSE;

  if (!htab->sgot && ! k1_create_got_section (dynobj, info))
    return FALSE;

  if (!_bfd_elf_create_dynamic_sections (dynobj, info))
    return FALSE;

  htab->srelplt = bfd_get_section_by_name (dynobj, ".rela.plt");
  htab->splt = bfd_get_section_by_name (dynobj, ".plt");

  htab->sdynbss = bfd_get_section_by_name (dynobj, ".dynbss");
  if (!info->shared)
    htab->srelbss = bfd_get_section_by_name (dynobj, ".rela.bss");
  
  if (!htab->sdynbss
      || (!info->shared && !htab->srelbss))
    abort ();
  if (!htab->splt || !htab->srelplt || !htab->sdynbss
      || (!info->shared && !htab->srelbss))
    abort ();

  return TRUE;
}

/* Return address for Ith PLT stub in section PLT, for relocation REL
 or (bfd_vma) -1 if it should not be included. */

bfd_vma
k1_plt_sym_val (bfd_vma i, const asection *plt,
	 const arelent *rel ATTRIBUTE_UNUSED)
{
    if (rel->howto->type != R_K1_JMP_SLOT)
        return (bfd_vma)-1;

    return plt->vma + 16 + i*16;
}


/* Determine the positive and negative ranges to be used by each
   offset range in the GOT.  FDCUR and CUR, that must be aligned to a
   double-word boundary, are the minimum (negative) and maximum
   (positive) GOT offsets already used by previous ranges, except for
   an ODD entry that may have been left behind.  GOT and FD indicate
   the size of GOT entries and function descriptors that must be
   placed within the range from -WRAP to WRAP.  If there's room left,
   up to FDPLT bytes should be reserved for additional function
   descriptors.  */

bfd_signed_vma
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

bfd_signed_vma
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

int
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


/* Cancel out any effects of calling _k1fdpic_assign_got_entries and
   _k1fdpic_assign_plt_entries.  */

int
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



/* The k1 linker needs to keep track of the number of relocs that it
   decides to copy in check_relocs for each symbol.  This is so that it
   can discard PC relative relocs if it doesn't need them when linking
   with -Bsymbolic.  We store the information in a field extending the
   regular ELF linker hash table.  */

/* This structure keeps track of the number of PC relative relocs we have
   copied for a given symbol.  */

// struct k1_pcrel_relocs_copied
// {
//   /* Next section.  */
//   struct k1_pcrel_relocs_copied *next;
//   /* A section in dynobj.  */
//   asection *section;
//   /* Number of relocs copied in this section.  */
//   bfd_size_type count;
// };
// 
// /* This function is called via elf_link_hash_traverse if we are
//    creating a shared object.  In the -Bsymbolic case it discards the
//    space allocated to copy PC relative relocs against symbols which
//    are defined in regular objects.  For the normal shared case, it
//    discards space for pc-relative relocs that have become local due to
//    symbol visibility changes.  We allocated space for them in the
//    check_relocs routine, but we won't fill them in in the
//    relocate_section routine.
// 
//    We also check whether any of the remaining relocations apply
//    against a readonly section, and set the DF_TEXTREL flag in this
//    case.  */
// 
// static bfd_boolean
// k1_discard_copies (struct elf_link_hash_entry *h, PTR inf)
// {
//   struct bfd_link_info *info = (struct bfd_link_info *) inf;
//   struct k1_pcrel_relocs_copied *s;
// 
//   if (h->root.type == bfd_link_hash_warning)
//     h = (struct elf_link_hash_entry *) h->root.u.i.link;
// 
//   if (!h->def_regular || (!info->symbolic && !h->forced_local))
//     {
//       if ((info->flags & DF_TEXTREL) == 0)
//         {
//           /* Look for relocations against read-only sections.  */
//           for (s = elf32_k1_hash_entry (h)->pcrel_relocs_copied;
//                s != NULL; s = s->next)
//             if ((s->section->flags & SEC_READONLY) != 0)
//               {
//                 info->flags |= DF_TEXTREL;
//                 break;
//               }
//         }
// 
//       return TRUE;
//     }
// 
//   for (s = elf32_k1_hash_entry (h)->pcrel_relocs_copied;
//        s != NULL; s = s->next)
//     s->section->size -= s->count * sizeof (Elf32_External_Rela);
// 
//   return TRUE;
// }




/* Set the sizes of the dynamic sections.  */




/* Adjust a symbol defined by a dynamic object and referenced by a
   regular object.  The current definition is in some section of the
   dynamic object, but we're not including those sections.  We have to
   change the definition to something the rest of the link can
   understand.  */


bfd_boolean
k1fdpic_adjust_dynamic_symbol (struct bfd_link_info *info,
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

bfd_boolean
k1fdpic_finish_dynamic_symbol
(bfd *output_bfd ATTRIBUTE_UNUSED,
 struct bfd_link_info *info ATTRIBUTE_UNUSED,
 struct elf_link_hash_entry *h ATTRIBUTE_UNUSED,
 Elf_Internal_Sym *sym ATTRIBUTE_UNUSED)
{
  return TRUE;
}

/* Finish up the dynamic sections.  */


bfd_boolean
elf_k1_always_size_sections(bfd *output_bfd, struct bfd_link_info *info){
  if (!info->relocatable)
  {
    struct elf_link_hash_entry *h;

    /* Force a PT_GNU_STACK segment to be created.  */
    if (! elf_stack_flags(output_bfd))
      elf_stack_flags(output_bfd) = PF_R | PF_W | PF_X;

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




bfd_boolean
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

  for (p = tdata->phdr, m = elf_seg_map(output_bfd); m != NULL; m = m->next, p++)
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


bfd_boolean
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


/* Do ignore failed relocs against the tracepoint metadata section. */
unsigned int
k1_bfd_elf_action_discarded (asection *sec)
{
    if (strcmp ("__k1_tracepoints", sec->name) == 0)
        return 0;

    return _bfd_elf_default_action_discarded (sec);
}
