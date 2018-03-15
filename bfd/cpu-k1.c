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

  if ((amach == bfd_mach_k1c_k1c && bmach == bfd_mach_k1c_k1c_usr))
    return b;
  if ((bmach == bfd_mach_k1c_k1c && amach == bfd_mach_k1c_k1c_usr))
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
  { bfd_mach_k1c_k1c, "k1c" },
  { bfd_mach_k1c_k1c_64, "k1c_64" },
  { bfd_mach_k1c_k1c_usr, "k1c_usr"},
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
  N (32, bfd_mach_k1c_k1c,      "k1:k1c",     FALSE, & arch_info_struct[1]),
  N (64, bfd_mach_k1c_k1c_64,   "k1:k1c:64",  FALSE, & arch_info_struct[2]),
  N (32, bfd_mach_k1c_k1c_usr,  "k1:k1c:usr", FALSE, NULL),
};

/* default must be coherent with default in elfNN_k1_object_p() */
const bfd_arch_info_type bfd_k1_arch =
  N (32, bfd_mach_k1c_k1c, "k1c", TRUE, & arch_info_struct[0]);
