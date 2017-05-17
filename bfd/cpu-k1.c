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
compatible (const bfd_arch_info_type *a, const bfd_arch_info_type *b)
{
  long amach =  a->mach, bmach =  b->mach;
  /* If a & b are for different architecture we can do nothing.  */
  if (a->arch != b->arch)
      return NULL;

  /* If a & b are for the same machine then all is well.  */
  if (amach == bmach)
    return a;

  if ((amach == bfd_mach_k1c_k1rm && bmach == bfd_mach_k1c_k1rm_usr) ||
      (amach == bfd_mach_k1c_k1pe && bmach == bfd_mach_k1c_k1pe_usr))
    return b;
  if ((bmach == bfd_mach_k1c_k1rm && amach == bfd_mach_k1c_k1rm_usr) ||
      (bmach == bfd_mach_k1c_k1pe && amach == bfd_mach_k1c_k1pe_usr))
    return a;

  /* Otherwise if either a or b is the 'default' machine
     then it can be polymorphed into the other.  */
  if (a->the_default)
    return b;

  if (b->the_default)
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
  { bfd_mach_k1c_k1pe, "k1pe" },
  { bfd_mach_k1c_k1rm, "k1rm" },
  { bfd_mach_k1c_k1pe_64, "k1pe_64" },
  { bfd_mach_k1c_k1rm_64, "k1rm_64" },
  { bfd_mach_k1c_k1pe_usr, "k1pe_usr"},
  { bfd_mach_k1c_k1rm_usr, "k1rm_usr"},
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
  compatible,                                                  \
  scan,                                                        \
  bfd_arch_default_fill,                                       \
  next                                                         \
}
 
static const bfd_arch_info_type arch_info_struct[] =
{
  N (32, bfd_mach_k1c_k1pe,      "k1:k1pe",     FALSE, & arch_info_struct[1]),
  N (32, bfd_mach_k1c_k1rm,      "k1:k1rm",     FALSE, & arch_info_struct[2]),
  N (64, bfd_mach_k1c_k1pe_64,   "k1:k1pe:64",  FALSE, & arch_info_struct[3]),
  N (64, bfd_mach_k1c_k1rm_64,   "k1:k1rm:64",  FALSE, & arch_info_struct[4]),
  N (32, bfd_mach_k1c_k1pe_usr,  "k1:k1pe:usr", FALSE, & arch_info_struct[5]),
  N (32, bfd_mach_k1c_k1rm_usr,  "k1:k1rm:usr", FALSE, NULL),
};

const bfd_arch_info_type bfd_k1_arch =
  N (32, bfd_mach_k1c_k1pe, "k1c", TRUE, & arch_info_struct[0]);
