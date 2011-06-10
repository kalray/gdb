#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"
#include "elf-bfd.h"
#include <elf/k1.h>

#define K1_MACH_MASK 0x0000000f

static reloc_howto_type* k1_reloc_type_lookup (bfd *, bfd_reloc_code_real_type);
static reloc_howto_type* k1_reloc_name_lookup (bfd *, const char *);
static void k1_elf_info_to_howto (bfd *, arelent *, Elf_Internal_Rela *);

#define K1DP_K1V1
#include "elf32-k1.def"
#undef K1DP_K1V1

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
  case bfd_mach_k1v1:
    fprintf (f, "\nMachine     = k1v1\n");
    break;
  case bfd_mach_k1dp:
    fprintf (f, "\nMachine     = k1dp\n");
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
    case bfd_mach_k1v1:
    case bfd_mach_k1dp:
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
    case bfd_mach_k1v1:
    case bfd_mach_k1dp:
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
  Elf_Internal_Shdr *           symtab_hdr;
  struct elf_link_hash_entry ** sym_hashes;
  Elf_Internal_Rela *           rel;
  Elf_Internal_Rela *           relend;

  symtab_hdr = & elf_tdata (input_bfd)->symtab_hdr;
  sym_hashes = elf_sym_hashes (input_bfd);
  relend     = relocs + input_section->reloc_count;

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

#define TARGET_LITTLE_SYM         bfd_elf32_k1_vec
#define TARGET_LITTLE_NAME        "elf32-k1"
#define ELF_ARCH                  bfd_arch_k1
#define ELF_TARGET_ID             K1_ELF_DATA
#define ELF_MACHINE_CODE          EM_K1
#define ELF_MAXPAGESIZE           64
#define bfd_elf32_bfd_reloc_type_lookup k1_reloc_type_lookup
#define bfd_elf32_bfd_reloc_name_lookup k1_reloc_name_lookup
#define elf_info_to_howto               k1_elf_info_to_howto
#define bfd_elf32_bfd_is_target_special_symbol  elf32_k1_is_target_special_symbol

#define elf_backend_can_gc_sections       1
#define elf_backend_relocate_section      k1_elf_relocate_section
#define elf_backend_rela_normal           1

#define bfd_elf32_bfd_merge_private_bfd_data elf_k1_merge_private_bfd_data
#define bfd_elf32_bfd_set_private_flags	     elf_k1_set_private_flags
#define bfd_elf32_bfd_print_private_bfd_data elf_k1_print_private_bfd_data
#define elf_backend_final_write_processing   elf_k1_final_write_processing
#define elf_backend_object_p                 elf_k1_object_p


#include "elf32-target.h"





