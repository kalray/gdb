/* BFD support for the k1 processor.
   Frederic Brault - Kalray 2009
*/

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"

/* This routine is provided two arch_infos and returns if machines
   are compatible.
*/

static const bfd_arch_info_type *
k1_compatible (const bfd_arch_info_type *a, const bfd_arch_info_type *b)
{
  long amach =  a->mach, bmach =  b->mach;
  /* If a & b are for different architecture we can do nothing.  */
  if (a->arch != b->arch)
    return NULL;

  /* We do not want to transmute some machine into another one */
  if (amach != bmach)
    return NULL;

  /* If a & b are for the same machine then all is well.  */
  if (amach == bmach)
    return a;

  return NULL;
}

static struct
{
  unsigned int mach;
  char *       name;
}
processors[] =
{
  { bfd_mach_k1c, "k1c" },
  { bfd_mach_k1c_64, "k1c_64" },
  { bfd_mach_k1c_usr, "k1c_usr"},
};

static bfd_boolean
scan (const struct bfd_arch_info *info, const char *string)
{
  int  i;

  /* First test for an exact match.  */
  if (strcasecmp (string, info->printable_name) == 0)
    return TRUE;

  /* Next check for a processor name instead of an Architecture name.  */
  for (i = sizeof (processors) / sizeof (processors[0]); i--;)
    {
      if (strcasecmp (string, processors [i].name) == 0)
	break;
    }

  if (i != -1 && info->mach == processors [i].mach)
    return TRUE;

  /* Finally check for the default architecture.  */
  if (strcasecmp (string, "k1") == 0)
    return info->the_default;

  return FALSE;
}

#define N(addr_bits, machine, print, default, next)            \
{                                                              \
  32,                          /* 32 bits in a word.  */       \
  addr_bits,                   /* bits in an address.  */      \
  8,                           /* 8 bits in a byte.  */        \
  bfd_arch_k1,                                                 \
  machine,                     /* Machine number.  */          \
  "k1",                        /* Architecture name.   */      \
  print,                       /* Printable name.  */          \
  4,                           /* Section align power.  */     \
  default,                     /* Is this the default ?  */    \
  k1_compatible,                                      \
  scan,                                                        \
  bfd_arch_default_fill,                                       \
  next                                                         \
}

const bfd_arch_info_type bfd_k1_usr_arch =
  N (32, bfd_mach_k1c_usr,  "k1:k1c:usr", FALSE, NULL);

const bfd_arch_info_type bfd_k1_64_arch =
  N (64, bfd_mach_k1c_64,   "k1:k1c:64",  FALSE, & bfd_k1_usr_arch);

const bfd_arch_info_type bfd_k1_arch =
  N (32, bfd_mach_k1c,      "k1:k1c",     TRUE, & bfd_k1_64_arch);
