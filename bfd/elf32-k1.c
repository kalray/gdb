// FIXME Header part
// Copyright Kalray
// Marc Poulhiès

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"
#include "elf-bfd.h"
#include <elf/k1c.h>

#include "elfxx-k1.h"

/* The size in bytes of an entry in the procedure linkage table FDPIC */
#define PLT_HEADER_SIZE         32
#define PLT_MIN_ENTRY_SIZE      16
#define PLT_FULL_ENTRY_SIZE     20

extern const bfd_target k1_linux_elf32_vec;

const bfd_vma plt_small_entry_k1c_32[] =
  {
    /* get $r14 = $pc     ;; */      0x01000380,
    /* lwz $r9 = 0[$r14]  ;; */      0xa824000e,
                                     0x00000000,
    /* igoto $r9          ;; */      0x00114009,
  };
const size_t plt_entry_k1c_32_size = PLT_SIZE(plt_small_entry_k1c_32);

static reloc_howto_type* k1_elf32_reloc_type_lookup (bfd *, bfd_reloc_code_real_type);
static reloc_howto_type* k1_elf32_reloc_name_lookup (bfd *, const char *);

K1_INFO_TO_HOWTO_DEF(32)
K1_RELOC_NAME_LOOKUP_DEF(32)
K1_RELOC_TYPE_LOOKUP_DEF(32)

struct bfd_link_hash_table * k1_elf32_link_hash_table_create (bfd *abfd);

struct bfd_link_hash_table *
k1_elf32_link_hash_table_create (bfd *abfd)
{
  struct k1_elf_link_hash_table *ret;
  ret = k1_elfxx_link_hash_table_create(abfd);
  
  if (ret == NULL)
    return NULL;

  ret->bytes_per_rela = sizeof (Elf32_External_Rela);
  ret->plt_entry_size = plt_entry_k1c_32_size;
  ret->bytes_per_address = 4;

  return &ret->elf.root;
}



/* static bfd_boolean */
/* k1_elf32_fdpic_create_dynamic_sections (bfd *abfd, struct bfd_link_info *info) */
/* { */

/*    /\* This is mostly copied from */
/*      elflink.c:_bfd_elf_create_dynamic_sections().  *\/ */
/*   flagword flags; */
/*   asection *s; */
/*   const struct elf_backend_data *bed = get_elf_backend_data (abfd); */

/*   flags = (SEC_ALLOC | SEC_LOAD | SEC_HAS_CONTENTS | SEC_IN_MEMORY */
/*            | SEC_LINKER_CREATED); */

/*   /\* We need to create .plt, .rel[a].plt, .got, .got.plt, .dynbss, and */
/*      .rel[a].bss sections.  *\/ */

/*   /\* K1-specific: we want to create the GOT in the K1 way.  *\/ */
/*   if (! k1_elf32_fdpic_create_got_section (abfd, info)) */
/*     return FALSE; */

/*   /\* K1-specific: make sure we created everything we wanted.  *\/ */
/*   BFD_ASSERT (k1fdpic_got_section (info) && k1fdpic_gotrel_section (info) */
/* 	      && k1fdpic_gotfixup_section (info) */
/*               && k1fdpic_plt_section (info) */
/*               && k1fdpic_pltrel_section (info)); */

/*   if (bed->want_dynbss) */
/*     { */
/*       /\* The .dynbss section is a place to put symbols which are defined */
/*          by dynamic objects, are referenced by regular objects, and are */
/*          not functions.  We must allocate space for them in the process */
/*          image and use a R_*_COPY reloc to tell the dynamic linker to */
/*          initialize them at run time.  The linker script puts the .dynbss */
/*          section into the .bss section of the final image.  *\/ */
/*       s = bfd_make_section_with_flags (abfd, ".dynbss", */
/*                                        SEC_ALLOC | SEC_LINKER_CREATED); */
/*       if (s == NULL) */
/*         return FALSE; */

/*       /\* The .rel[a].bss section holds copy relocs.  This section is not */
/*      normally needed.  We need to create it here, though, so that the */
/*      linker will map it to an output section.  We can't just create it */
/*      only if we need it, because we will not know whether we need it */
/*      until we have seen all the input files, and the first time the */
/*      main linker code calls BFD after examining all the input files */
/*      (size_dynamic_sections) the input sections have already been */
/*      mapped to the output sections.  If the section turns out not to */
/*      be needed, we can discard it later.  We will never need this */
/*      section when generating a shared object, since they do not use */
/*      copy relocs.  *\/ */
/*       if (! bfd_link_pic(info)) */
/*         { */
/*           s = bfd_make_section_with_flags (abfd, */
/*                                            ".rela.bss", */
/*                                            flags | SEC_READONLY); */
/*           if (s == NULL */
/*               || ! bfd_set_section_alignment (abfd, s, bed->s->log_file_align)) */
/*             return FALSE; */
/*         } */
/*     } */

/*   return TRUE; */
/* } */


/* Assign GOT offsets to private function descriptors used by PLT
   entries (or referenced by 32-bit offsets), as well as PLT entries
   and lazy PLT entries.  */

/* static int */
/* k1_elf32_fdpic_assign_plt_entries (void **entryp, void *info_) */
/* { */
/*   struct k1fdpic_relocs_info *entry = *entryp; */
/*   struct _k1fdpic_dynamic_got_plt_info *dinfo = info_; */

/*   /\* If this symbol requires a local function descriptor, allocate */
/*      one.  *\/ */
/*   if (entry->privfd && entry->fd_entry == 0) */
/*     { */
/*        entry->fd_entry = _k1fdpic_get_fd_entry (&dinfo->gothilo); */
/*        dinfo->gothilo.fdplt -= 8; */
/*     } */

/*   if (entry->plt) */
/*     { */
/*       int size; */

/*       /\* We use the section's raw size to mark the location of the */
/*          next PLT entry.  *\/ */
/*       entry->plt_entry = k1fdpic_plt_section (dinfo->g.info)->size; */

/*       /\* Figure out the length of this PLT entry based on the */
/*          addressing mode we need to reach the function descriptor.  *\/ */
/*       BFD_ASSERT (entry->fd_entry); */
/* //       if (entry->fd_entry >= -(1 << (18 - 1)) */
/* //           && entry->fd_entry + 4 < (1 << (18 - 1))) */
/* //         size = 10; */
/* //       else */
/*         size = PLT_FULL_ENTRY_SIZE; */

/*       k1fdpic_plt_section (dinfo->g.info)->size += size; */
/*     } */

/*   if (entry->lazyplt) */
/*     { */
/*       abort(); */
/*       entry->lzplt_entry = dinfo->g.lzplt; */
/*       dinfo->g.lzplt += LZPLT_NORMAL_SIZE; */
/*       /\* If this entry is the one that gets the resolver stub, account */
/*          for the additional instruction.  *\/ */
/*       if (entry->lzplt_entry % K1FDPIC_LZPLT_BLOCK_SIZE */
/*           == K1FDPIC_LZPLT_RESOLV_LOC) */
/*         dinfo->g.lzplt += LZPLT_RESOLVER_EXTRA; */
/*     } */

/*   return 1; */
/* } */


/* Compute the total size of the GOT, the PLT, the dynamic relocations
   section and the rofixup section.  Assign locations for GOT and PLT
   entries.  */

/* static bfd_boolean */
/* k1_elf32_fdpic_size_got_plt (bfd *output_bfd, */
/*                          struct _k1fdpic_dynamic_got_plt_info *gpinfop) */
/* { */
/*   bfd_signed_vma odd = 12; */
/* //   bfd_vma limit; */
/*   struct bfd_link_info *info = gpinfop->g.info; */
/*   bfd *dynobj = elf_hash_table (info)->dynobj; */

/*   memcpy (k1fdpic_dynamic_got_plt_info (info), &gpinfop->g, */
/*           sizeof (gpinfop->g)); */
  
/*   odd = _k1fdpic_compute_got_alloc_data (&gpinfop->gothilo, */
/*                                           0, */
/*                                           odd, */
/*                                           16, */
/*                                           gpinfop->g.gothilo, */
/*                                           gpinfop->g.fdhilo, */
/*                                           gpinfop->g.fdplt, */
/*                                           (bfd_vma)1 << (32-1)); */


/*   /\* Now assign (most) GOT offsets.  *\/ */
/*   htab_traverse (k1fdpic_relocs_info (info), _k1fdpic_assign_got_entries, */
/*                  gpinfop); */

/*   k1fdpic_got_section (info)->size = gpinfop->gothilo.max */
/*     - gpinfop->gothilo.min */
/*      /\* If an odd word is the last word of the GOT, we don't need this */
/*         word to be part of the GOT.  *\/ */
/*     - (odd + 4 == gpinfop->gothilo.max ? 4 : 0); */
/*   if (k1fdpic_got_section (info)->size == 0) */
/*     k1fdpic_got_section (info)->flags |= SEC_EXCLUDE; */
/*   else if (k1fdpic_got_section (info)->size == 12 */
/*            && ! elf_hash_table (info)->dynamic_sections_created) */
/*     { */
/*       k1fdpic_got_section (info)->flags |= SEC_EXCLUDE; */
/*       k1fdpic_got_section (info)->size = 0; */
/*     } */
/*   else */
/*     { */
/*       k1fdpic_got_section (info)->contents = */
/*         (bfd_byte *) bfd_zalloc (dynobj, */
/*                                  k1fdpic_got_section (info)->size); */
/*       if (k1fdpic_got_section (info)->contents == NULL) */
/*         return FALSE; */
/*     } */

/*   if (elf_hash_table (info)->dynamic_sections_created) */
/*     /\* Subtract the number of lzplt entries, since those will generate */
/*        relocations in the pltrel section.  *\/ */
/*     k1fdpic_gotrel_section (info)->size = */
/*       (gpinfop->g.relocs - gpinfop->g.lzplt / LZPLT_NORMAL_SIZE) */
/*       * get_elf_backend_data (output_bfd)->s->sizeof_rela; */
/*   else */
/*     BFD_ASSERT (gpinfop->g.relocs == 0); */
/*   if (k1fdpic_gotrel_section (info)->size == 0) */
/*     k1fdpic_gotrel_section (info)->flags |= SEC_EXCLUDE; */
/*   else */
/*     { */
/*       k1fdpic_gotrel_section (info)->contents = */
/*         (bfd_byte *) bfd_zalloc (dynobj, */
/*                                  k1fdpic_gotrel_section (info)->size); */
/*       if (k1fdpic_gotrel_section (info)->contents == NULL) */
/*         return FALSE; */
/*     } */


/*   k1fdpic_gotfixup_section (info)->size = (gpinfop->g.fixups + 1 )* 4; */
/*   if (k1fdpic_gotfixup_section (info)->size == 0) */
/*     k1fdpic_gotfixup_section (info)->flags |= SEC_EXCLUDE; */
/*   else */
/*     { */
/*       k1fdpic_gotfixup_section (info)->contents = */
/*         (bfd_byte *) bfd_zalloc (dynobj, */
/*                                  k1fdpic_gotfixup_section (info)->size); */
/*       if (k1fdpic_gotfixup_section (info)->contents == NULL) */
/*         return FALSE; */
/*     } */

/*   if (elf_hash_table (info)->dynamic_sections_created) */
/*     k1fdpic_pltrel_section (info)->size = */
/*       gpinfop->g.lzplt / LZPLT_NORMAL_SIZE * get_elf_backend_data (output_bfd)->s->sizeof_rel; */
/*   if (k1fdpic_pltrel_section (info)->size == 0) */
/*     k1fdpic_pltrel_section (info)->flags |= SEC_EXCLUDE; */
/*   else */
/*     { */
/*       k1fdpic_pltrel_section (info)->contents = */
/*         (bfd_byte *) bfd_zalloc (dynobj, */
/*                                  k1fdpic_pltrel_section (info)->size); */
/*       if (k1fdpic_pltrel_section (info)->contents == NULL) */
/*         return FALSE; */
/*     } */

/*   /\* Add 4 bytes for every block of at most 65535 lazy PLT entries, */
/*      such that there's room for the additional instruction needed to */
/*      call the resolver.  Since _k1fdpic_assign_got_entries didn't */
/*      account for them, our block size is 4 bytes smaller than the real */
/*      block size.  *\/ */
/*   if (elf_hash_table (info)->dynamic_sections_created) */
/*     { */
/*       k1fdpic_plt_section (info)->size = gpinfop->g.lzplt */
/*         + ((gpinfop->g.lzplt + (K1FDPIC_LZPLT_BLOCK_SIZE - 4) - LZPLT_NORMAL_SIZE) */
/*            / (K1FDPIC_LZPLT_BLOCK_SIZE - 4) * LZPLT_RESOLVER_EXTRA); */
/*     } */

/*   /\* Reset it, such that k1_elf32_fdpic_assign_plt_entries() can use it to */
/*      actually assign lazy PLT entries addresses.  *\/ */
/*   gpinfop->g.lzplt = 0; */

/*   /\* Save information that we're going to need to generate GOT and PLT */
/*      entries.  *\/ */
/*   k1fdpic_got_initial_offset (info) = -gpinfop->gothilo.min; */

/*   if (get_elf_backend_data (output_bfd)->want_got_sym) */
/*     elf_hash_table (info)->hgot->root.u.def.value */
/*       = k1fdpic_got_initial_offset (info); */

/*   if (elf_hash_table (info)->dynamic_sections_created) */
/*     k1fdpic_plt_initial_offset (info) = */
/*       k1fdpic_plt_section (info)->size; */

/*   htab_traverse (k1fdpic_relocs_info (info), k1_elf32_fdpic_assign_plt_entries, */
/*                  gpinfop); */

/*   /\* Allocate the PLT section contents only after */
/*      k1_elf32_fdpic_assign_plt_entries has a chance to add the size of the */
/*      non-lazy PLT entries.  *\/ */
/*   if (k1fdpic_plt_section (info)->size == 0) */
/*     k1fdpic_plt_section (info)->flags |= SEC_EXCLUDE; */
/*   else */
/*     { */
/*       k1fdpic_plt_section (info)->contents = */
/*         (bfd_byte *) bfd_zalloc (dynobj, */
/*                                  k1fdpic_plt_section (info)->size); */
/*       if (k1fdpic_plt_section (info)->contents == NULL) */
/*         return FALSE; */
/*     } */

/*   return TRUE; */
/* } */

/* static bfd_vma */
/* k1_elf32_fdpic_plt_sym_val (bfd_vma i, const asection *plt, */
/* 	 const arelent *rel ATTRIBUTE_UNUSED) */
/* { */
/*   return (bfd_vma)-1; */
/* } */


/* Update the got entry reference counts for the section being removed.  */

/* static bfd_boolean */
/* k1_elf32_fdpic_gc_sweep_hook (bfd *abfd, */
/* 			 struct bfd_link_info *info, */
/* 			 asection *sec, */
/* 			 const Elf_Internal_Rela *relocs) */
/* { */
/*   Elf_Internal_Shdr *symtab_hdr; */
/*   struct elf_link_hash_entry **sym_hashes, **sym_hashes_end; */
/*   const Elf_Internal_Rela *rel; */
/*   const Elf_Internal_Rela *rel_end; */
/*   struct k1fdpic_relocs_info *picrel; */

/*   BFD_ASSERT (IS_FDPIC (abfd)); */

/*   symtab_hdr = &elf_tdata (abfd)->symtab_hdr; */
/*   sym_hashes = elf_sym_hashes (abfd); */
/*   sym_hashes_end = sym_hashes + symtab_hdr->sh_size/sizeof(Elf32_External_Sym); */
/*   if (!elf_bad_symtab (abfd)) */
/*     sym_hashes_end -= symtab_hdr->sh_info; */

/*   rel_end = relocs + sec->reloc_count; */
/*   for (rel = relocs; rel < rel_end; rel++) */
/*     { */
/*       struct elf_link_hash_entry *h; */
/*       unsigned long r_symndx; */

/*       r_symndx = ELF32_R_SYM (rel->r_info); */
/*       if (r_symndx < symtab_hdr->sh_info) */
/*         h = NULL; */
/*       else */
/*         h = sym_hashes[r_symndx - symtab_hdr->sh_info]; */

/*       if (h != NULL) */
/* 	picrel = k1fdpic_relocs_info_for_global (k1fdpic_relocs_info (info), */
/* 						   abfd, h, */
/* 						   rel->r_addend, NO_INSERT); */
/*       else */
/* 	picrel = k1fdpic_relocs_info_for_local (k1fdpic_relocs_info */
/* 						  (info), abfd, r_symndx, */
/* 						  rel->r_addend, NO_INSERT); */

/*       if (!picrel) */
/* 	return TRUE; */

/*       switch (ELF32_R_TYPE (rel->r_info)) */
/*         { */
/* 	case R_K1_27_PCREL:	 */
/* 	  picrel->call--; */
/* 	  break; */

/* 	case R_K1_GLOB_DAT: */
/* 	  picrel->sym--; */
/* 	  if (bfd_get_section_flags (abfd, sec) & SEC_ALLOC) */
/* 	    picrel->relocs32--; */
/* 	  break; */

/* 	case R_K1_GOT: */
/* 	case R_K1_GOT_UP27: */
/* 	case R_K1_GOT_LO10: */
/* 	  picrel->gothilo--; */
/* 	  break; */

/* 	case R_K1_GOTOFF: */
/* 	case R_K1_GOTOFF_UP27: */
/* 	case R_K1_GOTOFF_LO10: */
/* 	  picrel->gotoff--; */
/* 	  break; */

/* 	default: */
/* 	  break; */
/*         } */
/*     } */

/*   return TRUE; */
/* } */

/* Look through the relocs for a section during the first phase.  */

static bfd_boolean
k1_elf32_relocate_section
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
  bfd_vma *local_got_offsets;
  struct k1_elf_link_hash_table *htab;
  htab = k1_elf_hash_table (info);
  if (htab == NULL)
    return FALSE;

  symtab_hdr = & elf_tdata (input_bfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (input_bfd);
  relend     = relocs + input_section->reloc_count;
  local_got_offsets = elf_local_got_offsets (input_bfd);

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
      
      DPRINT("k1_elf32_relocate_section");
      
      r_type = ELF32_R_TYPE (rel->r_info);

      r_symndx = ELF32_R_SYM (rel->r_info);

      howto  = elf_k1_howto_table + ELF32_R_TYPE (rel->r_info);
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
              if (sec != NULL &&
		  sec->output_section != NULL)
                relocation = (h->root.u.def.value
                          + sec->output_section->vma
                          + sec->output_offset);
            }
          else if (h->root.type == bfd_link_hash_undefweak)
            ;
          else if (info->unresolved_syms_in_objects == RM_IGNORE
               && ELF_ST_VISIBILITY (h->other) == STV_DEFAULT)
            ;
          else if (!bfd_link_relocatable(info) && !gp_disp_p)
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

      if (bfd_link_relocatable(info))
	continue;

      /* Finally, the sole K1-specific part.  */
      switch (r_type)
        {
        case R_K1_32:
          if ((input_section->flags & SEC_ALLOC) == 0){
            break;
	  }

          if ((bfd_link_pic(info)
               && (h == NULL
                   || ELF_ST_VISIBILITY (h->other) == STV_DEFAULT
                   || h->root.type != bfd_link_hash_undefweak))
              || (!bfd_link_pic(info)
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
              asection *sreloc;

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
                       && (!bfd_link_pic(info)
                           || !SYMBOLIC_BIND (info, h)
                           || !h->def_regular)){
                outrel.r_info = ELF32_R_INFO (h->dynindx, r_type);
		outrel.r_addend = rel->r_addend;
	      }
              else
                {
                  /* This symbol is local, or marked to become local.  */
                  relocate = TRUE;
                  outrel.r_info = ELF32_R_INFO (0, R_K1_RELATIVE);
		  outrel.r_addend = relocation + rel->r_addend;
                }

              sreloc = elf_section_data (input_section)->sreloc;

              BFD_ASSERT (sreloc != NULL && sreloc->contents != NULL);

              loc = sreloc->contents;
              loc += sreloc->reloc_count++ * k1_elf_rela_bytes(htab);

              bfd_elf32_swap_reloca_out (output_bfd, &outrel, loc);

              /* If this reloc is against an external symbol, we do
                 not want to fiddle with the addend.  Otherwise, we
                 need to include the symbol value so that it becomes
                 an addend for the dynamic reloc.  */
              if (!relocate)
                continue;
              }
                break;
	    /* Handle K1 specific things here */
        case R_K1_S37_UP27:
        case R_K1_S37_LO10:
          if (gp_disp_p)
          {
            bfd_vma got_value;
            bfd_vma p;

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
            if (r_type == R_K1_S37_LO10)
              relocation = got_value - p + 4;
            else
              relocation = got_value - p + 8;
          }
          break;
	case R_K1_S37_TPREL_LO10:
	case R_K1_S37_TPREL_UP27:
	case R_K1_TPREL_32:
	  {
	    asection *tls_sec = elf_hash_table (info)->tls_sec;
	    if (tls_sec == NULL)
	      {
		(*_bfd_error_handler)
		  (_("%B: missing TLS section for relocation %s against `%s' at 0x%lx in section `%A'."),
		   input_bfd, input_section, howto->name, name,
		   rel->r_offset);
		return FALSE;
	      }
	    relocation -=  tls_sec->vma;
	  }
	  break;
    case R_K1_GOTOFF:
    case R_K1_S37_GOTOFF_UP27:
    case R_K1_S37_GOTOFF_LO10:
        BFD_ASSERT (htab->sgotplt != NULL);
        relocation -= htab->sgotplt->output_section->vma
            + htab->sgotplt->output_offset;
        break;
	case R_K1_GOT:
	case R_K1_S37_GOT_UP27:
	case R_K1_S37_GOT_LO10:
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
                  if (bfd_link_pic(info))
                    {
                      Elf_Internal_Rela outrel;
                      bfd_byte *loc;
                      BFD_ASSERT (htab->srelgot != NULL);

                      outrel.r_offset = (htab->sgot->output_section->vma
                                         + htab->sgot->output_offset + off);
                      outrel.r_info = ELF32_R_INFO (0, R_K1_RELATIVE);
                      outrel.r_addend = relocation;

                      loc = htab->srelgot->contents;
                      loc +=
                        htab->srelgot->reloc_count++ * k1_elf_rela_bytes(htab);
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

		if (!WILL_CALL_FINISH_DYNAMIC_SYMBOL (dyn, bfd_link_pic(info), h)
		    || (bfd_link_pic(info)
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
          
        case R_K1_S37_PLT_UP27:
        case R_K1_S37_PLT_LO10:
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


/* Compute the number of dynamic relocations and fixups that a symbol
   requires, and add (or subtract) from the grand and per-symbol
   totals.  */

/* static void */
/* k1_elf32_fdpic_count_relocs_fixups (struct k1fdpic_relocs_info *entry, */
/*                                 struct _k1fdpic_dynamic_got_info *dinfo, */
/*                                 bfd_boolean subtract) */
/* { */
/*   bfd_vma relocs = 0, fixups = 0; */

/*   if (!bfd_link_pde(dinfo->info) || bfd_link_pie(dinfo->info)) */
/*     relocs = entry->relocs32 + entry->relocsfd + entry->relocsfdv; */
/*   else */
/*     { */
/*       if (entry->symndx != -1 || K1FDPIC_SYM_LOCAL (dinfo->info, entry->d.h)) */
/*         { */
/*           if (entry->symndx != -1 */
/*               || entry->d.h->root.type != bfd_link_hash_undefweak) */
/*             fixups += entry->relocs32 + 2 * entry->relocsfdv; */
/*         } */
/*       else */
/*         relocs += entry->relocs32 + entry->relocsfdv; */

/*       if (entry->symndx != -1 */
/*           || K1FDPIC_FUNCDESC_LOCAL (dinfo->info, entry->d.h)) */
/*         { */
/*           if (entry->symndx != -1 */
/*               || entry->d.h->root.type != bfd_link_hash_undefweak) */
/*             fixups += entry->relocsfd; */
/*         } */
/*       else */
/*         relocs += entry->relocsfd; */
/*     } */

/*   if (subtract) */
/*     { */
/*       relocs = - relocs; */
/*       fixups = - fixups; */
/*     } */

/*   entry->dynrelocs += relocs; */
/*   entry->fixups += fixups; */
/*   dinfo->relocs += relocs; */
/*   dinfo->fixups += fixups; */
/* } */



/* Compute the total GOT and PLT size required by each symbol in each range. *
   Symbols may require up to 4 words in the GOT: an entry pointing to
   the symbol, an entry pointing to its function descriptor, and a
   private function descriptors taking two words.  */

/* static int */
/* k1_elf32_fdpic_count_got_plt_entries (void **entryp, void *dinfo_) */
/* { */
/*   struct k1fdpic_relocs_info *entry = *entryp; */
/*   struct _k1fdpic_dynamic_got_info *dinfo = dinfo_; */

/*   _k1fdpic_count_nontls_entries (entry, dinfo); */

/*   k1_elf32_fdpic_count_relocs_fixups (entry, dinfo, FALSE); */

/*   return 1; */
/* } */

/* static bfd_boolean */
/* k1_elf32_fdpic_size_dynamic_sections (bfd *output_bfd ATTRIBUTE_UNUSED, */
/*                                       struct bfd_link_info *info) */
/* { */
/*   struct elf_link_hash_table *htab; */
/*   bfd *dynobj; */
/*   asection *s; */
/*   struct _k1fdpic_dynamic_got_plt_info gpinfo; */

/*   htab = elf_hash_table (info); */
/*   dynobj = htab->dynobj; */
/*   BFD_ASSERT (dynobj != NULL); */

/*   if (htab->dynamic_sections_created) */
/*     { */
/*       /\* Set the contents of the .interp section to the interpreter.  *\/ */
/*       if (bfd_link_executable(info)) */
/*         { */
/*           s = bfd_get_section_by_name (dynobj, ".interp"); */
/*           BFD_ASSERT (s != NULL); */
/*           s->size = sizeof ELF_DYNAMIC_INTERPRETER; */
/*           s->contents = (bfd_byte *) ELF_DYNAMIC_INTERPRETER; */
/*         } */
/*     } */

/*   memset (&gpinfo, 0, sizeof (gpinfo)); */
/*   gpinfo.g.info = info; */

/*   for (;;) */
/*     { */
/*       htab_t relocs = k1fdpic_relocs_info (info); */
/*       htab_traverse (relocs, _k1fdpic_resolve_final_relocs_info, &relocs); */
/*       if (relocs == k1fdpic_relocs_info (info)) */
/*         break; */
/*     } */
/*   htab_traverse (k1fdpic_relocs_info (info), k1_elf32_fdpic_count_got_plt_entries, */
/*                  &gpinfo.g); */
/*   /\* Allocate space to save the summary information, we're going to */
/*      use it if we're doing relaxations.  *\/ */
/*   k1fdpic_dynamic_got_plt_info (info) = bfd_alloc (dynobj, sizeof (gpinfo.g)); */
/*   if (!k1_elf32_fdpic_size_got_plt (output_bfd, &gpinfo)) */
/*       return FALSE; */

/*   if (elf_hash_table (info)->dynamic_sections_created) */
/*     { */
/*       if (k1fdpic_got_section (info)->size) */
/*         if (!_bfd_elf_add_dynamic_entry (info, DT_PLTGOT, 0)) */
/*           return FALSE; */
/*       if (k1fdpic_pltrel_section (info)->size) */
/*         if (!_bfd_elf_add_dynamic_entry (info, DT_PLTRELSZ, 0) */
/*             || !_bfd_elf_add_dynamic_entry (info, DT_PLTREL, DT_RELA) */
/*             || !_bfd_elf_add_dynamic_entry (info, DT_JMPREL, 0)) */
/*           return FALSE; */
/*       if (k1fdpic_gotrel_section (info)->size) */
/*         if (!_bfd_elf_add_dynamic_entry (info, DT_RELA, 0) */
/*             || !_bfd_elf_add_dynamic_entry (info, DT_RELASZ, 0) */
/*             || !_bfd_elf_add_dynamic_entry (info, DT_RELAENT, */
/*                                             sizeof (Elf32_External_Rela))) */
/*           return FALSE; */

/*       if (bfd_link_executable (info)) */
/*         { */
/*           if (!_bfd_elf_add_dynamic_entry (info, DT_DEBUG, 0)) */
/*             return FALSE; */
/*         } */
/*     } */

/*   s = bfd_get_section_by_name (dynobj, ".dynbss"); */
/*   if (s && s->size == 0) */
/*     s->flags |= SEC_EXCLUDE; */

/*   s = bfd_get_section_by_name (dynobj, ".rela.bss"); */
/*   if (s && s->size == 0) */
/*     s->flags |= SEC_EXCLUDE; */
/*   return TRUE; */
/* } */

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

      /* plt_index = h->plt.offset / PLT_ENTRY_SIZE - 1; */
      /* got_offset = (plt_index + 3) * 4; */

      /* Fill in the entry in the procedure linkage table.  */
        {
          unsigned int i;
	  unsigned int loc_plt_size;
          const bfd_vma *template;
	  switch(output_bfd->arch_info->mach){
	  case bfd_mach_k1c_k1c:
	    template = plt_small_entry_k1c_32;
	    loc_plt_size = plt_entry_k1c_32_size;
	    break;
	  case bfd_mach_k1c_k1c_64:
	    (*_bfd_error_handler)
	      ("64bits PLT not yet ready for machine: %d", output_bfd->arch_info->mach);
	    return FALSE;

	  default:
	    (*_bfd_error_handler)
	      ("can't make a plt entry for unknown mach: %d", output_bfd->arch_info->mach);
	    return FALSE;
	  }
	  plt_index = h->plt.offset / loc_plt_size - 1;
	  got_offset = (plt_index + 3) * 4;

          bfd_vma pcgotoffset = got->output_section->vma + gotplt->output_offset +got_offset;

          pcgotoffset -= plt->output_section->vma + plt->output_offset + h->plt.offset;

          BFD_ASSERT(plt->contents != NULL);
          for (i = 0; i < (loc_plt_size / 4); ++i)
            bfd_put_32(output_bfd, template[i], plt->contents + h->plt.offset + (4*i));
          
          _bfd_final_link_relocate (elf_k1_howto_table + R_K1_S37_LO10,
                                output_bfd, plt,
                                plt->contents + h->plt.offset + 4,
                                0, pcgotoffset, 0);
          _bfd_final_link_relocate (elf_k1_howto_table + R_K1_S37_UP27,
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
      
      rel.r_info = ELF32_R_INFO (h->dynindx, R_K1_JMP_SLOT);

      rel.r_addend = 0;
      loc = relplt->contents + plt_index * k1_elf_rela_bytes(htab);
      bfd_elf32_swap_reloca_out (output_bfd, &rel, loc);

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
      asection *sgot;
      asection *srela;
      Elf_Internal_Rela rela;
      bfd_byte *loc;

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
      if (bfd_link_pic(info)
          && (info->symbolic
              || h->dynindx == -1 || h->forced_local) && h->def_regular)
        {
          asection *sec = h->root.u.def.section;
          rela.r_info = ELF32_R_INFO (0, R_K1_RELATIVE);
          rela.r_addend = (h->root.u.def.value
                           + sec->output_section->vma
                           + sec->output_offset);
        }
      else
        {
          rela.r_info = ELF32_R_INFO (h->dynindx, R_K1_GLOB_DAT);
          rela.r_addend = 0;
        }
      bfd_put_32 (output_bfd, (bfd_vma) 0,
                      sgot->contents + (h->got.offset & ~(bfd_vma) 1));
      loc = srela->contents;
      loc += srela->reloc_count++ * k1_elf_rela_bytes(htab);
      bfd_elf32_swap_reloca_out (output_bfd, &rela, loc);
    }

  if (h->needs_copy)
    {
      abort();
    }
  /* Mark some specially defined symbols as absolute.  */
  if (strcmp (h->root.root.string, "_DYNAMIC") == 0
      || strcmp (h->root.root.string, "_GLOBAL_OFFSET_TABLE_") == 0
      || strcmp (h->root.root.string, "_PROCEDURE_LINKAGE_TABLE_") == 0
      || strcmp (h->root.root.string, "_gp_disp") == 0)
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
      Elf32_External_Dyn * dyncon;
      Elf32_External_Dyn * dynconend;

      BFD_ASSERT (htab->splt != NULL && sdyn != NULL);

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
        
         /* Fill in the first entry in the procedure linkage table.  */
      if (htab->splt->size > 0)
        {
          DPRINT("We shall fill the first PLT entry here");
          
	  if (bfd_link_pic(info))
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
        elf_section_data (htab->splt->output_section)->this_hdr.sh_entsize = htab->plt_entry_size;
    }
        
  if (htab->sgotplt && htab->sgotplt->size > 0)
    {
      /* Fill in the first three entries in the global offset table.  */      
      bfd_put_32 (output_bfd,
                   (sdyn == NULL ? 0
                    : sdyn->output_section->vma + sdyn->output_offset),
                      htab->sgotplt->contents);
      bfd_put_32 (output_bfd, 0, htab->sgotplt->contents + htab->bytes_per_address);
      bfd_put_32 (output_bfd, 0, htab->sgotplt->contents + htab->bytes_per_address * 2);
      

      elf_section_data (htab->sgotplt->output_section)->this_hdr.sh_entsize = htab->bytes_per_address;
    }

  if (htab->sgot && htab->sgot->size > 0)
    elf_section_data (htab->sgot->output_section)->this_hdr.sh_entsize = htab->bytes_per_address;
    

  return TRUE;
}

/* static bfd_boolean */
/* k1_elf32_fdpic_finish_dynamic_sections (bfd *output_bfd, */
/*                                         struct bfd_link_info *info) */
/* { */
/*   bfd *dynobj; */
/*   asection *sdyn; */

/*   dynobj = elf_hash_table (info)->dynobj;     */

/*   if (k1fdpic_got_section (info)) */
/*     { */
/*       BFD_ASSERT (k1fdpic_gotrel_section (info)->size */
/*                   >= (k1fdpic_gotrel_section (info)->reloc_count */
/*                       * sizeof (Elf32_External_Rela))); */

/*       if (k1fdpic_gotfixup_section (info)) */
/*         { */
/*           struct elf_link_hash_entry *hgot = elf_hash_table (info)->hgot; */
/*           bfd_vma got_value = hgot->root.u.def.value */
/*             + hgot->root.u.def.section->output_section->vma */
/*             + hgot->root.u.def.section->output_offset; */

/*           _k1fdpic_add_rofixup (output_bfd, k1fdpic_gotfixup_section (info), */
/*                                  got_value, 0); */

/*           if (k1fdpic_gotfixup_section (info)->size */
/*               != (k1fdpic_gotfixup_section (info)->reloc_count * 4)) */
/*             { */
/*               (*_bfd_error_handler) */
/*                 ("LINKER BUG: .rofixup section size mismatch"); */
/*               return FALSE; */
/*             } */
/*         } */
/*     } */
/*   if (elf_hash_table (info)->dynamic_sections_created) */
/*     { */
/*       BFD_ASSERT (k1fdpic_pltrel_section (info)->size */
/*                   == (k1fdpic_pltrel_section (info)->reloc_count */
/*                       * sizeof (Elf32_External_Rela))); */
/*     } */

/*   sdyn = bfd_get_section_by_name (dynobj, ".dynamic"); */

/*   if (elf_hash_table (info)->dynamic_sections_created) */
/*     { */
/*       Elf32_External_Dyn * dyncon; */
/*       Elf32_External_Dyn * dynconend; */

/*       BFD_ASSERT (sdyn != NULL); */

/*       dyncon = (Elf32_External_Dyn *) sdyn->contents; */
/*       dynconend = (Elf32_External_Dyn *) (sdyn->contents + sdyn->size); */

/*       for (; dyncon < dynconend; dyncon++) */
/*         { */
/*           Elf_Internal_Dyn dyn; */

/*           bfd_elf32_swap_dyn_in (dynobj, dyncon, &dyn); */

/*           switch (dyn.d_tag) */
/*             { */
/*             default: */
/*               break; */

/*             case DT_PLTGOT: */
/*               dyn.d_un.d_ptr = k1fdpic_got_section (info)->output_section->vma */
/*                 + k1fdpic_got_section (info)->output_offset */
/*                 + k1fdpic_got_initial_offset (info); */
/*               bfd_elf32_swap_dyn_out (output_bfd, &dyn, dyncon); */
/*               break; */

/*             case DT_JMPREL: */
/*               dyn.d_un.d_ptr = k1fdpic_pltrel_section (info) */
/*                 ->output_section->vma */
/*                 + k1fdpic_pltrel_section (info)->output_offset; */
/*               bfd_elf32_swap_dyn_out (output_bfd, &dyn, dyncon); */
/*               break; */

/*             case DT_PLTRELSZ: */
/*               dyn.d_un.d_val = k1fdpic_pltrel_section (info)->size; */
/*               bfd_elf32_swap_dyn_out (output_bfd, &dyn, dyncon); */
/*               break; */
/*             } */
/*         } */
/*     } */

/*   return TRUE; */
/* } */

// #ifndef ELF_ARCH
#define TARGET_LITTLE_SYM                       k1_elf32_vec
#define TARGET_LITTLE_NAME                      "elf32-k1"
#define ELF_ARCH                                bfd_arch_k1
// #endif//ELF_ARCH

#define ELF_TARGET_ID                           K1_ELF_DATA
#define ELF_MACHINE_CODE                        EM_K1
#define ELF_MAXPAGESIZE                         0x4000
#define bfd_elf32_bfd_reloc_type_lookup         k1_elf32_reloc_type_lookup
#define bfd_elf32_bfd_reloc_name_lookup         k1_elf32_reloc_name_lookup
#define elf_info_to_howto                       k1_elf32_info_to_howto
#define bfd_elf32_bfd_is_target_special_symbol  elf32_k1_is_target_special_symbol

#define bfd_elf32_bfd_link_hash_table_create    k1_elf32_link_hash_table_create

#define elf_backend_can_gc_sections             1
#define elf_backend_relocate_section            k1_elf32_relocate_section
#define elf_backend_rela_normal                 1


#define elf_backend_want_got_plt                1
#define elf_backend_plt_readonly                1
#define elf_backend_can_refcount                1
#define elf_backend_want_plt_sym                0
#define elf_backend_got_header_size             (4*3)

#define elf_backend_link_output_symbol_hook     k1_elf_link_output_symbol_hook

#define elf_backend_gc_mark_hook                _bfd_elf_gc_mark_hook
#define elf_backend_gc_sweep_hook               k1_gc_sweep_hook

#define elf_backend_adjust_dynamic_symbol       k1_adjust_dynamic_symbol

#define elf_backend_create_dynamic_sections     k1_create_dynamic_sections
#define elf_backend_finish_dynamic_sections     k1_elf32_finish_dynamic_sections
#define elf_backend_finish_dynamic_symbol       k1_elf32_finish_dynamic_symbol
#define elf_backend_size_dynamic_sections       k1_size_dynamic_sections


#define elf_backend_check_relocs                k1_elfxx_check_relocs
#define elf_backend_action_discarded            k1_bfd_elf_action_discarded


#define bfd_elf32_bfd_merge_private_bfd_data    k1_merge_private_bfd_data
#define bfd_elf32_bfd_set_private_flags         elf_k1_set_private_flags
#define bfd_elf32_bfd_print_private_bfd_data    elf_k1_print_private_bfd_data
#define bfd_elf32_bfd_copy_private_bfd_data     elf_k1_copy_private_bfd_data

#define elf_backend_final_write_processing      elf_k1_final_write_processing
#define elf_backend_object_p                    elf_k1_object_p


#define elf_backend_may_use_rel_p       0
#define elf_backend_may_use_rela_p      1

#undef elf_backend_plt_sym_val
#define elf_backend_plt_sym_val	 k1_plt_sym_val

#define elf_backend_rela_plts_and_copies_p 1

#include "elf32-target.h"
