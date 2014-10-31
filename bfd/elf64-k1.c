// FIXME Header part
// Copyright Kalray
// Marc Poulhiès

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"
#include "elf-bfd.h"
#include <elf/k1.h>

#include "elfxx-k1.h"
#define K1A_K1B
#include "elf64-k1.def"
#undef K1A_KB

/* The size in bytes of an entry in the procedure linkage table FDPIC */
#define PLT_HEADER_SIZE         32
#define PLT_MIN_ENTRY_SIZE      16
#define PLT_FULL_ENTRY_SIZE     20

static const bfd_vma plt_small_entry[PLT_ENTRY_SIZE] =
  {
      /* get $r14 = $pc     ;; */      0x00700380,
      /* lw $r9 = 0[$r14]   ;; */      0xa424000e,
                                       0x18000000,
      /* igoto $r9          ;; */      0x00114009,
  };

//FIXME first experiment: no linker for linux yet.
//extern const bfd_target bfd_elf32_k1_linux_vec;
//#define IS_FDPIC(bfd) ((bfd)->xvec == &bfd_elf32_k1_linux_vec)
//#define IS_FDPIC(bfd) (0)

static reloc_howto_type* k1_elf64_reloc_type_lookup (bfd *, bfd_reloc_code_real_type);
static reloc_howto_type* k1_elf64_reloc_name_lookup (bfd *, const char *);

K1_INFO_TO_HOWTO_DEF(64)
K1_RELOC_NAME_LOOKUP_DEF(64)
K1_RELOC_TYPE_LOOKUP_DEF(64)

static bfd_boolean
k1_elf32_check_relocs (bfd * abfd,
		   struct bfd_link_info *info,
		   asection *sec,
                   const Elf_Internal_Rela *relocs)
{
  bfd *dynobj;
  Elf_Internal_Shdr *symtab_hdr;
  struct k1_elf_link_hash_table *htab;
  struct elf_link_hash_entry **sym_hashes;
  const Elf_Internal_Rela *rel;
  const Elf_Internal_Rela *rel_end;
  //asection *sgot;
  //asection *srelgot;
  asection *sreloc = NULL;
  
  DPRINT("check_relocs");
  
  if (info->relocatable)
    return TRUE;

  dynobj = elf_hash_table (info)->dynobj;
  symtab_hdr = &elf_tdata (abfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (abfd);
  htab = k1_elf_hash_table(info);
  rel_end = relocs + sec->reloc_count;
  for (rel = relocs; rel < rel_end; rel++)
    {
      unsigned long r_symndx;
      struct elf_link_hash_entry *h;
      int r_type = ELF64_R_TYPE (rel->r_info);
      Elf_Internal_Sym *sym;
      const char *name = NULL;

      r_symndx = ELF64_R_SYM (rel->r_info);
      if (r_symndx < symtab_hdr->sh_info)
      {
          /* A local symbol.  */
        sym = bfd_sym_from_r_symndx (&htab->sym_sec,
                                        abfd, r_symndx);
        if (sym == NULL)
          return FALSE;
        h= NULL;
      }
      else {
        sym = NULL;
        h = sym_hashes[r_symndx - symtab_hdr->sh_info];
	struct elf_link_hash_entry * h2 = h;

	while (h2->root.type == bfd_link_hash_indirect
	       || h2->root.type == bfd_link_hash_warning)
	  h2 = (struct elf_link_hash_entry *) h2->root.u.i.link;

	name = h->root.root.string;
      }

      switch (r_type)
	{
	  /* 32bits */
	case R_K1_LO10:
	case R_K1_HI22:

	  /* 64 bits */
	case R_K1_EXTEND6:
	case R_K1_HI27:
	case R_K1_ELO10:

	  /*
	   * if some reloc uses special symbol named "_gp_disp" and
	   * GOT not yet created, create it
	   */
	  if (htab->sgot == NULL && name != NULL && strcmp (name, "_gp_disp") == 0){
            if (htab->elf.dynobj == NULL)
	      htab->elf.dynobj = dynobj = abfd;
	    if (!k1_create_got_section (dynobj, info)){
	      return FALSE;
	    }
	  }
	  break;

        case R_K1_GOTOFF:
        case R_K1_GOTOFF_HI22:
        case R_K1_GOTOFF_LO10:

        case R_K1_GOTOFF64:
        case R_K1_GOTOFF64_HI27:
        case R_K1_GOTOFF64_EXTEND6:
        case R_K1_GOTOFF64_LO10:

          htab->needs_got = TRUE;
          /*fallthrough */
        case R_K1_GOT:
        case R_K1_GOT_HI22:
        case R_K1_GOT_LO10:
        case R_K1_GLOB_DAT:

        case R_K1_GOT64:
        case R_K1_GOT64_HI27:
        case R_K1_GOT64_LO10:
        case R_K1_GLOB_DAT64:

          if (htab->sgot == NULL)
          {
            if (htab->elf.dynobj == NULL)	    
	      htab->elf.dynobj = dynobj = abfd;
	    if (!k1_create_got_section (dynobj, info))
	      return FALSE;
          }
	  break;
        default:
          break;
        }
        
      switch (r_type)
        {
        case R_K1_GOT:
        case R_K1_GOT_HI22:
        case R_K1_GOT_LO10:

        case R_K1_GOT64:
        case R_K1_GOT64_HI27:
        case R_K1_GOT64_LO10:
          if (h != NULL)
              h->got.refcount += 1;
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
          
        case R_K1_PLT_HI22:
        case R_K1_PLT_LO10:

        case R_K1_PLT64_HI27:
        case R_K1_PLT64_LO10:
        case R_K1_PLT64_EXTEND6:

        case R_K1_27_PCREL:
          /* If this is a local symbol, we resolve it directly without
             creating a procedure linkage table entry.  */
          if (h != NULL)
            {
              h->needs_plt = 1;
              h->plt.refcount += 1;
            }
          break;

        case R_K1_32:
	case R_K1_64:
           if (h != NULL && !info->shared)
            {
              /* If this reloc is in a read-only section, we might
                 need a copy reloc.  We can't check reliably at this
                 stage whether the section is read-only, as input
                 sections have not yet been mapped to output sections.
                 Tentatively set the flag for now, and correct in
                 adjust_dynamic_symbol.  */
              h->non_got_ref = 1;

              /* We may need a .plt entry if the function this reloc
                 refers to is in a shared lib.  */;
//               h->plt.refcount += 1;
//               h->needs_plt = 1;
              h->pointer_equality_needed = 1;
            }
            
             /* If we are creating a shared library, and this is a reloc
             against a global symbol, or a non PC relative reloc
             against a local symbol, then we need to copy the reloc
             into the shared library.  However, if we are linking with
             -Bsymbolic, we do not need to copy a reloc against a
             global symbol which is defined in an object we are
             including in the link (i.e., DEF_REGULAR is set).  At
             this point we have not seen all the input files, so it is
             possible that DEF_REGULAR is not set now but will be set
             later (it is never cleared).  In case of a weak definition,
             DEF_REGULAR may be cleared later by a strong definition in
             a shared library.  We account for that possibility below by
             storing information in the relocs_copied field of the hash
             table entry.  A similar situation occurs when creating
             shared libraries and symbol visibility changes render the
             symbol local.

             If on the other hand, we are creating an executable, we
             may need to keep relocations for symbols satisfied by a
             dynamic library if we manage to avoid copy relocs for the
             symbol.  */
          if ((info->shared
               && (sec->flags & SEC_ALLOC) != 0)
               || (!info->shared
                  && (sec->flags & SEC_ALLOC) != 0
                  && h != NULL
                  && (h->root.type == bfd_link_hash_defweak
                      || !h->def_regular)))
            {
              struct k1_elf_dyn_relocs *p;
              struct k1_elf_dyn_relocs **head;

              /* We must copy these reloc types into the output file.
                 Create a reloc section in dynobj and make room for
                 this reloc.  */
              if (sreloc == NULL)
                {
                  if (htab->elf.dynobj == NULL)
                    htab->elf.dynobj = abfd;
                  dynobj = htab->elf.dynobj;

                  sreloc = _bfd_elf_make_dynamic_reloc_section
                    (sec, dynobj, 2, abfd, /*rela?*/ TRUE);
                  
                  if (sreloc == NULL)
                    return FALSE;
                }

              /* If this is a global symbol, we count the number of
                 relocations we need for this symbol.  */
              if (h != NULL)
                {
                  head = &((struct k1_elf_link_hash_entry *) h)->dyn_relocs;
                }
              else
                {
                  /* Track dynamic relocs needed for local syms too.
                     We really need local syms available to do this
                     easily.  Oh well.  */
                  void **vpp;
                  asection *s;

                  sym = bfd_sym_from_r_symndx (&htab->sym_sec,
                                                abfd, r_symndx);
                  if (sym == NULL)
                    return FALSE;

                  s = bfd_section_from_elf_index (abfd, sym->st_shndx);
                  if (s == NULL)
                    s = sec;

                  vpp = &elf_section_data (s)->local_dynrel;
                  head = (struct k1_elf_dyn_relocs **)vpp;
                }

              p = *head;
              if (p == NULL || p->sec != sec)
                {
                  bfd_size_type amt = sizeof *p;
                  p = (struct k1_elf_dyn_relocs *) bfd_alloc (dynobj,
                                                           amt);
                  if (p == NULL)
                    return FALSE;
                  p->next = *head;
                  *head = p;
                  p->sec = sec;
                  p->count = 0;
                  p->pc_count = 0;
                }

              p->count += 1;
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


static bfd_boolean
k1_elf64_relocate_section
    (bfd *                   output_bfd ATTRIBUTE_UNUSED,
     struct bfd_link_info *  info,
     bfd *                   input_bfd,
     asection *              input_section,
     bfd_byte *              contents,
     Elf_Internal_Rela *     relocs,
     Elf_Internal_Sym *      local_syms,
     asection **             local_sections)
{
  bfd *dynobj;
  Elf_Internal_Shdr *           symtab_hdr;
  struct elf_link_hash_entry ** sym_hashes;
  Elf_Internal_Rela *           rel;
  Elf_Internal_Rela *           relend;
  bfd_vma *local_got_offsets;
  struct k1_elf_link_hash_table *htab;
  asection *sreloc;

  htab = k1_elf_hash_table (info);
  if (htab == NULL)
    return FALSE;

  dynobj = htab->elf.dynobj;
  symtab_hdr = & elf_tdata (input_bfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (input_bfd);
  relend     = relocs + input_section->reloc_count;
  local_got_offsets = elf_local_got_offsets (input_bfd);

  sreloc = elf_section_data (input_section)->sreloc;


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
      bfd_boolean                  gp_disp_p;
      
      DPRINT("k1_elf64_relocate_section");
      
      r_type = ELF64_R_TYPE (rel->r_info);

      r_symndx = ELF64_R_SYM (rel->r_info);

      howto  = elf64_k1_howto_table + r_type;
      h      = NULL;
      sym    = NULL;
      sec    = NULL;
      gp_disp_p = FALSE;
      
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
	  case 4:
	    BFD_ASSERT(0 && "64 bits relocs not handled");
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
/*           DPRINT ("reloc for global symbol"); */
          /* Use this instead of RELOC_FOR_GLOBAL_SYMBOL cause
           *  we have to support our magic symbol _gp_disp
           */

            /* It seems this can happen with erroneous or unsupported
            input (mixing a.out and elf in an archive, for example.)  */
            if (sym_hashes == NULL)
              return FALSE;

            h = sym_hashes[r_symndx - symtab_hdr->sh_info];

            while (h->root.type == bfd_link_hash_indirect
              || h->root.type == bfd_link_hash_warning)
              h = (struct elf_link_hash_entry *) h->root.u.i.link;

            relocation = 0;

            name = h->root.root.string;
            if (strcmp (name, "_gp_disp") == 0)
            {
              DPRINT("got gp_disp");
              gp_disp_p = TRUE;
            }

            if (h->root.type == bfd_link_hash_defined
                || h->root.type == bfd_link_hash_defweak)
            {
              sec = h->root.u.def.section;
              if (sec != NULL)
                relocation = (h->root.u.def.value
                          + sec->output_section->vma
                          + sec->output_offset);
            }
          else if (h->root.type == bfd_link_hash_undefweak)
            ;
          else if (info->unresolved_syms_in_objects == RM_IGNORE
               && ELF_ST_VISIBILITY (h->other) == STV_DEFAULT)
            ;
          else if (!info->relocatable && !gp_disp_p)
          {
            bfd_boolean err;
            err = (info->unresolved_syms_in_objects == RM_GENERATE_ERROR
                   || ELF_ST_VISIBILITY (h->other) != STV_DEFAULT);
            if (!info->callbacks->undefined_symbol (info,
                                                    h->root.root.string,
                                                    input_bfd,
                                                    input_section,
                                                    rel->r_offset, err))
            return FALSE;
          }
	}

      if (sec != NULL && discarded_section (sec))
	{
	    RELOC_AGAINST_DISCARDED_SECTION (info, input_bfd, input_section, rel,
					     1, relend, howto, 0, contents);
	    continue;
	}

      if (info->relocatable)
	continue;

      /* Finally, the sole K1-specific part.  */
      switch (r_type)
        {
        case R_K1_32:
	case R_K1_64:
          if ((input_section->flags & SEC_ALLOC) == 0)
            break;
          if ((info->shared
               && (h == NULL
                   || ELF_ST_VISIBILITY (h->other) == STV_DEFAULT
                   || h->root.type != bfd_link_hash_undefweak))
              || (!info->shared
                  && h != NULL
                  && h->dynindx != -1
                  && !h->non_got_ref
                  && ((h->def_dynamic
                       && !h->def_regular)
                      || h->root.type == bfd_link_hash_undefweak
                      || h->root.type == bfd_link_hash_undefined)))
          {
              Elf_Internal_Rela outrel;
              bfd_byte *loc;
              bfd_boolean skip, relocate;

              /* When generating a shared object, these relocations
                 are copied into the output file to be resolved at run
                 time.  */

              skip = FALSE;
              relocate = FALSE;

              outrel.r_offset =
                _bfd_elf_section_offset (output_bfd, info, input_section,
                                         rel->r_offset);
              if (outrel.r_offset == (bfd_vma) -1)
                skip = TRUE;
              else if (outrel.r_offset == (bfd_vma) -2)
                skip = TRUE, relocate = TRUE;
              outrel.r_offset += (input_section->output_section->vma
                                  + input_section->output_offset);

              if (skip)
                memset (&outrel, 0, sizeof (outrel));
              else if (h != NULL
                       && h->dynindx != -1
                       && (!info->shared
                           || !SYMBOLIC_BIND (info, h)
                           || !h->def_regular))
                outrel.r_info = ELF64_R_INFO (h->dynindx, r_type);
              else
                {
                  /* This symbol is local, or marked to become local.  */
                  relocate = TRUE;
                  outrel.r_info = ELF64_R_INFO (0, R_K1_RELATIVE);
                }

              BFD_ASSERT (sreloc != NULL && sreloc->contents != NULL);

              loc = sreloc->contents;
              loc += sreloc->reloc_count++ * sizeof (Elf64_External_Rela);

              bfd_elf32_swap_reloc_out (output_bfd, &outrel, loc);

              /* If this reloc is against an external symbol, we do
                 not want to fiddle with the addend.  Otherwise, we
                 need to include the symbol value so that it becomes
                 an addend for the dynamic reloc.  */
              if (!relocate)
                continue;
              }
                break;

	    /* Handle K1 specific things here */
        case R_K1_HI22:
        case R_K1_LO10:

        case R_K1_HI27:
        case R_K1_EXTEND6:
        case R_K1_ELO10:

          if (gp_disp_p)
          {
            bfd_vma got_value;
            bfd_vma p;

	    /* current bundle address */
            p = (input_section->output_section->vma
              + input_section->output_offset
              + rel->r_offset);

            BFD_ASSERT (htab->sgotplt != NULL);
            got_value = htab->sgotplt->output_section->vma
                      + htab->sgotplt->output_offset;

            /* we have to make a correction against the bundle
             * start address, not our (the instruction word)
             * address, hence the +4 and +8
             */
            if (r_type == R_K1_LO10 || r_type == R_K1_ELO10 || r_type == R_K1_EXTEND6)
              relocation = got_value - p + 4;
            else
              relocation = got_value - p + 8;
          }
          break;

	case R_K1_TPREL_LO10:
	case R_K1_TPREL_HI22:
	case R_K1_TPREL_32:

	case R_K1_TPREL64_EXTEND6:
	case R_K1_TPREL64_ELO10:
	case R_K1_TPREL64_HI27:
	case R_K1_TPREL64_64:

	    relocation -=  htab->elf.tls_sec->vma;
	    break;

	case R_K1_10_GPREL:
	case R_K1_16_GPREL:
	case R_K1_GPREL_LO10:
	case R_K1_GPREL_HI22:
            relocation -=  k1_gp_base (output_bfd, info);
	    break;

	case R_K1_GOTOFF:
	case R_K1_GOTOFF_HI22:
	case R_K1_GOTOFF_LO10:

        case R_K1_GOTOFF64:
        case R_K1_GOTOFF64_HI27:
        case R_K1_GOTOFF64_EXTEND6:
        case R_K1_GOTOFF64_LO10:

	  BFD_ASSERT (htab->sgotplt != NULL);
	  relocation -= htab->sgotplt->output_section->vma
            + htab->sgotplt->output_offset;
	  break;

	case R_K1_GOT:
	case R_K1_GOT_HI22:
	case R_K1_GOT_LO10:

        case R_K1_GOT64:
        case R_K1_GOT64_HI27:
        case R_K1_GOT64_LO10:
        case R_K1_GLOB_DAT64:

          if (htab->sgot == NULL)
                  abort ();
        {
          bfd_vma off;
          if (h == NULL)
            {
              BFD_ASSERT (local_got_offsets != NULL);
              off = local_got_offsets[r_symndx];
              BFD_ASSERT (off != (bfd_vma) - 1);

              /* The offset must always be a multiple of 4.  We use
                 the least significant bit to record whether we have
                 already generated the necessary reloc.  */
              if ((off & 1) != 0)
                off &= ~1;
              else
                {
                  bfd_put_32 (output_bfd, relocation, 
                              htab->sgot->contents + off);
                  if (info->shared)
                    {
                      Elf_Internal_Rela outrel;
                      bfd_byte *loc;
                      BFD_ASSERT (htab->srelgot != NULL);

                      outrel.r_offset = (htab->sgot->output_section->vma
                                         + htab->sgot->output_offset + off);
                      outrel.r_info = ELF64_R_INFO (0, R_K1_RELATIVE);
                      outrel.r_addend = relocation;

                      loc = htab->srelgot->contents;
                      loc +=
                        htab->srelgot->reloc_count++ * sizeof (Elf64_External_Rela);
                      bfd_elf32_swap_reloca_out (output_bfd, &outrel, loc);
                    }
                  local_got_offsets[r_symndx] |= 1;
                }
              }
            else
              {
		bfd_boolean dyn;
		off = h->got.offset;
		BFD_ASSERT (off != (bfd_vma) - 1);
		dyn = htab->elf.dynamic_sections_created;

		if (!WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn, info->shared, h)
		    || (info->shared
			&& (info->symbolic
			    || h->dynindx == -1
			    || h->forced_local)
			&& h->def_regular))
		  {
		    /* This is actually a static link, or it is a
		       -Bsymbolic link and the symbol is defined
		       locally, or the symbol was forced to be local
		       because of a version file..  We must initialize
		       this entry in the global offset table.  Since
		       the offset must always be a multiple of 4, we
		       use the least significant bit to record whether
		       we have initialized it already.

		       When doing a dynamic link, we create a .rela.got
		       relocation entry to initialize the value.  This
		       is done in the finish_dynamic_symbol routine.  */
		    if ((off & 1) != 0)
		      off &= ~1;
		    else
		      {
			bfd_put_32 (output_bfd, relocation,
				    htab->sgot->contents + off);
			h->got.offset |= 1;
		      }
		  }
	      }
	      relocation = htab->sgot->output_section->vma
                      + htab->sgot->output_offset + off
                      - htab->sgotplt->output_section->vma
                      - htab->sgotplt->output_offset;
      }
	  break;
          
        case R_K1_PLT_HI22:
        case R_K1_PLT_LO10:

        case R_K1_PLT64_HI27:
        case R_K1_PLT64_LO10:
        case R_K1_PLT64_EXTEND6:

        case R_K1_27_PCREL:
          /* Relocation is to the entry for this symbol in the
             procedure linkage table.  */

          /* Resolve a PLT reloc against a local symbol directly,
             without using the procedure linkage table.  */
          if (h == NULL)
            break;
          
          if (h->plt.offset == (bfd_vma) -1
              || htab->splt == NULL)
            {
              /* We didn't make a PLT entry for this symbol.  This
                 happens when statically linking PIC code, or when
                 using -Bsymbolic.  */
              break;
            }
          
          relocation = (htab->splt->output_section->vma
                        + htab->splt->output_offset
                        + h->plt.offset);
          rel->r_addend = 0;
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


/* Finish up dynamic symbol handling.  We set the contents of various
   dynamic sections here.  */

static bfd_boolean
k1_elf32_finish_dynamic_symbol (bfd * output_bfd,
                                struct bfd_link_info *info,
                                struct elf_link_hash_entry *h,
                                Elf_Internal_Sym * sym)
{
  struct k1_elf_link_hash_table *htab;

  htab = k1_elf_hash_table (info);
  if (htab == NULL)
    return FALSE;

  if (h->plt.offset != (bfd_vma) -1)
    {
      bfd_vma plt_index;
      bfd_vma got_offset;
      Elf_Internal_Rela rel;
      bfd_byte *loc;
      asection *plt, *gotplt, *relplt, *got;

      plt = htab->splt;
      gotplt = htab->sgotplt;
      relplt = htab->srelplt;
      got = htab->sgot;

      /* This symbol has an entry in the procedure linkage table.  Set
         it up.  */
      BFD_ASSERT (h->dynindx != -1);

      /* Get the index in the procedure linkage table which
         corresponds to this symbol.  This is the index of this symbol
         in all the symbols for which we are making plt entries.  The
         first entry in the procedure linkage table is reserved.

         Get the offset into the .got table of the entry that
         corresponds to this function.  Each .got entry is 4 bytes.
         The first three are reserved.

         For static executables, we don't reserve anything.  */

      plt_index = h->plt.offset / PLT_ENTRY_SIZE - 1;
      got_offset = (plt_index + 3) * 4;

      /* Fill in the entry in the procedure linkage table.  */
        {
          int i;
          const bfd_vma *template = plt_small_entry;
          bfd_vma pcgotoffset = got->output_section->vma + gotplt->output_offset +got_offset;

          pcgotoffset -= plt->output_section->vma + plt->output_offset + h->plt.offset;

          BFD_ASSERT(plt->contents != NULL);
          for (i = 0; i < (PLT_SMALL_ENTRY_SIZE / 4); ++i)
            bfd_put_32(output_bfd, template[i], plt->contents + h->plt.offset + (4*i));
          
          _bfd_final_link_relocate (elf64_k1_howto_table + R_K1_LO10,
                                output_bfd, plt,
                                plt->contents + h->plt.offset + 4,
                                0, pcgotoffset, 0);
          _bfd_final_link_relocate (elf64_k1_howto_table + R_K1_HI22,
                                output_bfd, plt,
                                plt->contents + h->plt.offset + 8,
                                0, pcgotoffset, 0);
         }

      /* Fill in the entry in the global offset table.  */
      bfd_put_32 (output_bfd,
                  (plt->output_section->vma
                   + plt->output_offset
                   + h->plt.offset
                   + 6),
                  gotplt->contents + got_offset);

      /* Fill in the entry in the .rel.plt section.  */
      rel.r_offset = (gotplt->output_section->vma
                      + gotplt->output_offset
                      + got_offset);
      
      rel.r_info = ELF64_R_INFO (h->dynindx, R_K1_JMP_SLOT);
      loc = relplt->contents + plt_index * sizeof (Elf64_External_Rela);
      bfd_elf32_swap_reloc_out (output_bfd, &rel, loc);

      if (!h->def_regular)
        {
          /* Mark the symbol as undefined, rather than as defined in
             the .plt section.  Leave the value if there were any
             relocations where pointer equality matters (this is a clue
             for the dynamic linker, to make function pointer
             comparisons work between an application and shared
             library), otherwise set it to zero.  If a function is only
             called from a binary, there is no need to slow down
             shared libraries because of that.  */
          sym->st_shndx = SHN_UNDEF;
          if (!h->pointer_equality_needed)
            sym->st_value = 0;
        }
    }
    
    
/* ====================================== */    
    
  
  if (h->got.offset != (bfd_vma) - 1)
    {
      bfd *dynobj;
      asection *sgot;
      asection *srela;
      Elf_Internal_Rela rela;
      bfd_byte *loc;
      dynobj = htab->elf.dynobj;

      /* This symbol has an entry in the global offset table.
         Set it up.  */
      sgot = htab->sgot;
      srela = htab->srelgot;
      BFD_ASSERT (sgot != NULL && srela != NULL);

      rela.r_offset = (sgot->output_section->vma
                       + sgot->output_offset
                       + (h->got.offset & ~(bfd_vma) 1));

      /* If this is a -Bsymbolic link, and the symbol is defined
         locally, we just want to emit a RELATIVE reloc.  Likewise if
         the symbol was forced to be local because of a version file.
         The entry in the global offset table will already have been
         initialized in the relocate_section function.  */
      if (info->shared
          && (info->symbolic
              || h->dynindx == -1 || h->forced_local) && h->def_regular)
        {
          asection *sec = h->root.u.def.section;
          rela.r_info = ELF64_R_INFO (0, R_K1_RELATIVE);
          rela.r_addend = (h->root.u.def.value
                           + sec->output_section->vma
                           + sec->output_offset);
        }
      else
        {
          rela.r_info = ELF64_R_INFO (h->dynindx, R_K1_GLOB_DAT);
          rela.r_addend = 0;
        }
      bfd_put_32 (output_bfd, (bfd_vma) 0,
                      sgot->contents + (h->got.offset & ~(bfd_vma) 1));
      loc = srela->contents;
      loc += srela->reloc_count++ * sizeof (Elf64_External_Rela);
      bfd_elf32_swap_reloca_out (output_bfd, &rela, loc);
    }

  if (h->needs_copy)
    {
      abort();
    }
  /* Mark some specially defined symbols as absolute.  */
  if (strcmp (h->root.root.string, "_DYNAMIC") == 0
      || strcmp (h->root.root.string, "_GLOBAL_OFFSET_TABLE_") == 0
      || strcmp (h->root.root.string, "_PROCEDURE_LINKAGE_TABLE_") == 0)
    sym->st_shndx = SHN_ABS;

  return TRUE;
}

static bfd_boolean
k1_elf32_finish_dynamic_sections (bfd * output_bfd ATTRIBUTE_UNUSED,
                                  struct bfd_link_info *info)
{
  bfd *dynobj;
  asection *sdyn;  
  struct k1_elf_link_hash_table *htab;
  
  DPRINT("finish dynamic sections");
  htab = k1_elf_hash_table (info);
  if (htab == NULL)
    return FALSE;
  dynobj = htab->elf.dynobj;
  
  sdyn = bfd_get_section_by_name (dynobj, ".dynamic");

  if (htab->elf.dynamic_sections_created)
    {
      Elf64_External_Dyn * dyncon;
      Elf64_External_Dyn * dynconend;

      BFD_ASSERT (htab->splt != NULL && sdyn != NULL);

      dyncon = (Elf64_External_Dyn *) sdyn->contents;
      dynconend = (Elf64_External_Dyn *) (sdyn->contents + sdyn->size);

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
        
         /* Fill in the first entry in the procedure linkage table.  */
      if (htab->splt->size > 0)
        {
          DPRINT("We shall fill the first PLT entry here");
          
            if (info->shared)
            {
//               memcpy (htab->elf.splt->contents, elf_i386_pic_plt0_entry,
//                       sizeof (elf_i386_pic_plt0_entry));
//               memset (htab->elf.splt->contents + sizeof (elf_i386_pic_plt0_entry),
//                       htab->plt0_pad_byte,
//                       PLT_ENTRY_SIZE - sizeof (elf_i386_pic_plt0_entry));
            }
          else
            {
//               memcpy (htab->elf.splt->contents, elf_i386_plt0_entry,
//                       sizeof(elf_i386_plt0_entry));
//               memset (htab->elf.splt->contents + sizeof (elf_i386_plt0_entry),
//                       htab->plt0_pad_byte,
//                       PLT_ENTRY_SIZE - sizeof (elf_i386_plt0_entry));
//               bfd_put_32 (output_bfd,
//                           (htab->elf.sgotplt->output_section->vma
//                            + htab->elf.sgotplt->output_offset
//                            + 4),
//                           htab->elf.splt->contents + 2);
//               bfd_put_32 (output_bfd,
//                           (htab->elf.sgotplt->output_section->vma
//                            + htab->elf.sgotplt->output_offset
//                            + 8),
//                           htab->elf.splt->contents + 8);
            }
            
        }
        elf_section_data (htab->splt->output_section)->this_hdr.sh_entsize = 4;
    }
        
  if (htab->sgotplt && htab->sgotplt->size > 0)
    {
      /* Fill in the first three entries in the global offset table.  */      
      bfd_put_32 (output_bfd,
                   (sdyn == NULL ? 0
                    : sdyn->output_section->vma + sdyn->output_offset),
                      htab->sgotplt->contents);
      bfd_put_32 (output_bfd, 0, htab->sgotplt->contents + 4);
      bfd_put_32 (output_bfd, 0, htab->sgotplt->contents + 8);
      

      elf_section_data (htab->sgotplt->output_section)->this_hdr.sh_entsize = 4;
    }

  if (htab->sgot && htab->sgot->size > 0)
    elf_section_data (htab->sgot->output_section)->this_hdr.sh_entsize = 4;
    

  return TRUE;
}

// #ifndef ELF_ARCH
#define TARGET_LITTLE_SYM                       bfd_elf64_k1_vec
#define TARGET_LITTLE_NAME                      "elf64-k1"
#define ELF_ARCH                                bfd_arch_k1
// #endif//ELF_ARCH

#define ELF_TARGET_ID                           K1_ELF_DATA
#define ELF_MACHINE_CODE                        EM_K1
#define ELF_MAXPAGESIZE                         0x4000
#define bfd_elf64_bfd_reloc_type_lookup         k1_elf64_reloc_type_lookup
#define bfd_elf64_bfd_reloc_name_lookup         k1_elf64_reloc_name_lookup
#define elf_info_to_howto                       k1_elf64_info_to_howto
#define bfd_elf64_bfd_is_target_special_symbol  elf32_k1_is_target_special_symbol

#define bfd_elf64_bfd_link_hash_table_create    k1_elf_link_hash_table_create

#define elf_backend_can_gc_sections             1
#define elf_backend_relocate_section            k1_elf64_relocate_section
#define elf_backend_rela_normal                 1


#define elf_backend_want_got_plt                1
#define elf_backend_plt_readonly                1
#define elf_backend_can_refcount                1
#define elf_backend_want_plt_sym                0
#define elf_backend_got_header_size             12


#define elf_backend_gc_mark_hook                _bfd_elf_gc_mark_hook
#define elf_backend_gc_sweep_hook               k1_gc_sweep_hook

#define elf_backend_adjust_dynamic_symbol       k1_adjust_dynamic_symbol

#define elf_backend_create_dynamic_sections     k1_create_dynamic_sections
#define elf_backend_finish_dynamic_sections     k1_elf32_finish_dynamic_sections
#define elf_backend_finish_dynamic_symbol       k1_elf32_finish_dynamic_symbol
#define elf_backend_size_dynamic_sections       k1_size_dynamic_sections


#define elf_backend_check_relocs                k1_elf32_check_relocs
#define elf_backend_action_discarded            k1_bfd_elf_action_discarded


#define bfd_elf64_bfd_merge_private_bfd_data    k1_merge_private_bfd_data
#define bfd_elf64_bfd_set_private_flags         elf_k1_set_private_flags
#define bfd_elf64_bfd_print_private_bfd_data    elf_k1_print_private_bfd_data
#define bfd_elf64_bfd_copy_private_bfd_data     elf_k1_copy_private_bfd_data

#define elf_backend_final_write_processing      elf_k1_final_write_processing
#define elf_backend_object_p                    elf_k1_object_p


#define elf_backend_may_use_rel_p       1
#define elf_backend_may_use_rela_p      1

#undef elf_backend_plt_sym_val
#define elf_backend_plt_sym_val	 k1_plt_sym_val

#define elf_backend_rela_plts_and_copies_p 1

#include "elf64-target.h"

