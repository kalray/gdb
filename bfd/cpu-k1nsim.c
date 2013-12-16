/* BFD support for the Intel 386 architecture.
   Copyright 1992, 1994, 1995, 1996, 1998, 2000, 2001, 2002, 2004, 2005,
   2007, 2009, 2010, 2011
   Free Software Foundation, Inc.

   This file is part of BFD, the Binary File Descriptor library.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"

static const bfd_arch_info_type *
bfd_k1nsim_compatible (const bfd_arch_info_type *a,
		     const bfd_arch_info_type *b)
{
  /* If a & b are for different architecture we can do nothing.  */
  if (a->arch != b->arch)
      return NULL;

  /* If a & b are for the same machine then all is well.  */
  if (a->mach == b->mach)
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
  { bfd_mach_k1dp, "k1dp"  },
  { bfd_mach_k1io, "k1io"  },
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

#define N(number, print, default, next)  \
{  32, 32, 8, bfd_arch_k1, number, "k1", print, 4, default, \
  bfd_k1nsim_compatible, scan, bfd_arch_default_fill, next }

static const bfd_arch_info_type arch_info_struct[] =
{
  N (bfd_mach_k1dp,      "k1dp",   FALSE, & arch_info_struct[1]),
  N (bfd_mach_k1io,      "k1io",   FALSE, NULL),
};

const bfd_arch_info_type bfd_k1nsim_arch =
  N (bfd_mach_k1dp, "k1nsim", TRUE, & arch_info_struct[0]);

