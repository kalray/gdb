/**
*** (c) Copyright Hewlett-Packard Company 1999-2003
**/

/*
  THIS FILE HAS BEEN MODIFIED OR ADDED BY STMicroelectronics, Inc. 1999-2003
*/


/**
*** static char sccs_id[] = "@(#)cpu-lx.c	1.1 05/07/98 18:43:24";
**/

/* BFD support for the HP Lisard architecture.
   Copyright 1992 Free Software Foundation, Inc.

This file is part of BFD, the Binary File Descriptor library.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#include "bfd.h"
#include "sysdep.h"
#include "libbfd.h"
#include "symcat.h"


#define N(number, print, default, next)  \
{  32, 32, 8, bfd_arch_lx, number, "lx/st200", print, 3, default, bfd_default_compatible, bfd_default_scan, next }

#define ST220_NEXT  &arch_info_struct[0]
#define ST230_NEXT  &arch_info_struct[1]
#define ST231_NEXT  &arch_info_struct[2]
#define ST240_NEXT  &arch_info_struct[3]

static const bfd_arch_info_type arch_info_struct[] =
{
  N (bfd_mach_st220,      "st220",   FALSE, ST230_NEXT),
  N (bfd_mach_st230,      "st230",   FALSE, ST231_NEXT),
  N (bfd_mach_st231,      "st231",   FALSE, ST240_NEXT),
  N (bfd_mach_st240,      "st240",   FALSE, NULL)
};

const bfd_arch_info_type bfd_lx_arch =
      N (0, "st200", TRUE, ST220_NEXT);



