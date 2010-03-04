/* BFD support for the k1 processor.
   Frederic Brault - Kalray 2009
*/

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"

const bfd_arch_info_type bfd_k1_arch = {
      32,               /* 32 bits in a word.  */
      32,               /* 32 bits in an address.  */
      8,                /*  8 bits in a byte.  */
      bfd_arch_k1,      /* enum bfd_architecture arch.  */
      bfd_mach_k1dp,
      "k1",             /* name.  */
      "k1",             /* printed name.  */
      3,                /* section alignment power.  */
      TRUE,            
      bfd_default_compatible, 
      bfd_default_scan ,
      0,
};
