#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"
#include "elf-bfd.h"
#include <elf/k1.h>

static reloc_howto_type* k1_reloc_type_lookup (bfd *, bfd_reloc_code_real_type);
static reloc_howto_type* k1_reloc_name_lookup (bfd *, const char *);
static void k1_elf_info_to_howto (bfd *, arelent *, Elf_Internal_Rela *);

/*
static bfd_boolean
k1_elf_is_local_label_name (bfd *abfd ATTRIBUTE_UNUSED, const char *name)
{
   return ((name[0] == '.' && name[1] == 'L') ||
          (name[0] == 'L' && name[1] == '?') ||
          (name[0] == '_' && name[1] == '?') ||
          (name[0] == '?') ||
          (name[0] == '$'));
}
*/

#define K1DP_K1CP
#include "elf32-k1.def"
#undef K1DP_K1CP

struct k1_reloc_map
{
  bfd_reloc_code_real_type bfd_reloc_val;
  unsigned int k1_reloc_val;
};

static const struct k1_reloc_map k1_reloc_map[] =
{
  { BFD_RELOC_NONE, R_K1_NONE },
  { BFD_RELOC_16,   R_K1_16},
  { BFD_RELOC_32,   R_K1_32},
  {BFD_RELOC_K1_17_PCREL, R_K1_17_PCREL},
  {BFD_RELOC_K1_18_PCREL, R_K1_18_PCREL},
  {BFD_RELOC_K1_27_PCREL, R_K1_27_PCREL},
  {BFD_RELOC_K1_32_PCREL, R_K1_32_PCREL},
  {BFD_RELOC_K1_LO10, R_K1_LO10},
  {BFD_RELOC_K1_HI22, R_K1_HI22},
  {BFD_RELOC_K1_GPREL_LO10, R_K1_GPREL_LO10},
  {BFD_RELOC_K1_GPREL_HI22, R_K1_GPREL_HI22},
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
  return sym->name && sym->name[0] == 'L' && sym->name[1] == '?';
}

#define TARGET_LITTLE_SYM         bfd_elf32_k1_vec
#define TARGET_LITTLE_NAME        "elf32-k1"
#define ELF_ARCH                  bfd_arch_k1
#define ELF_MACHINE_CODE          EM_K1
#define ELF_MAXPAGESIZE           64
#define bfd_elf32_bfd_reloc_type_lookup k1_reloc_type_lookup
#define bfd_elf32_bfd_reloc_name_lookup k1_reloc_name_lookup
#define elf_info_to_howto               k1_elf_info_to_howto
#define bfd_elf32_bfd_is_target_special_symbol  elf32_k1_is_target_special_symbol

#define elf_backend_can_gc_sections       1

#include "elf32-target.h"





