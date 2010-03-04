/**
*** (c) Copyright Hewlett-Packard Company 1999-2003 
***
*** This program is free software; you can redistribute it and/or
*** modify it under the terms of the GNU General Public License
*** as published by the Free Software Foundation; either version
*** 2 of the License, or (at your option) any later version.
*** 
*** This program is distributed in the hope that it will be useful,
*** but WITHOUT ANY WARRANTY; without even the implied warranty of
*** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
*** General Public License for more details.
***
*** You should have received a copy of the GNU General Public License 
*** along with this program; if not, write to the Free Software
*** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**/

/*
  THIS FILE HAS BEEN MODIFIED OR ADDED BY STMicroelectronics, Inc. 1999-2003
*/

/**
*** static char sccs_id[] = "@(#)lx.h	1.2 02/11/00 21:28:02";
**/

/* LX ELF support for BFD.

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

/* This file holds definitions specific to the PPC ELF ABI.  Note
   that most of this is not actually implemented by BFD.  */

#ifndef _ELF_LX_H
#define _ELF_LX_H

#include "bfd.h"
#include "elf-bfd.h"

#include "elf/reloc-macros.h"

/*
 * Typedef relating to bfd library
 *
 *   rta = 0 : catch all
 *   rta = 1 : multiflow register assignment
 *   rta = 2 : lxbe embedded
 *   rta = 3 : lxbe PIC
 */

typedef union {
  struct {
    unsigned rta : 4;  
  } obj_compat;
  unsigned int f;
} lx_bfd_flags;

/* (pp) core */
#define _ELF_LX_CORE_BIT	(8)                      /* 1st bit position in byte */
#define ELF_LX_CORE_MASK	(0xff<<_ELF_LX_CORE_BIT)           /* mask */
#define ELF_LX_CORE_ST210	(0x0<<_ELF_LX_CORE_BIT)
#define ELF_LX_CORE_ST220	(0x1<<_ELF_LX_CORE_BIT)
#define ELF_LX_CORE_ST230	(0x2<<_ELF_LX_CORE_BIT)
#define ELF_LX_CORE_ST231	(0x3<<_ELF_LX_CORE_BIT)
#define ELF_LX_CORE_ST235	(0x4<<_ELF_LX_CORE_BIT)
#define ELF_LX_CORE_ST239	(0x5<<_ELF_LX_CORE_BIT)
#define ELF_LX_CORE_ST240	(0x6<<_ELF_LX_CORE_BIT)
#define ELF_LX_CORE_UNDEF	(0x7<<_ELF_LX_CORE_BIT)
#define _ELF_LX_CHECK_CORE(m) ((m&ELF_LX_CORE_MASK)==m)

/* (pp) cut */
#define _ELF_LX_CUT_BIT	(16)                             /* 1st bit position in byte */
#define ELF_LX_CUT_MASK		(0xf<<_ELF_LX_CUT_BIT)           /* mask */
#define ELF_LX_CUT_0		(0x0<<_ELF_LX_CUT_BIT)
#define ELF_LX_CUT_1		(0x1<<_ELF_LX_CUT_BIT)
#define ELF_LX_CUT_2		(0x2<<_ELF_LX_CUT_BIT)
#define ELF_LX_CUT_3		(0x3<<_ELF_LX_CUT_BIT)
#define ELF_LX_CUT_4		(0x4<<_ELF_LX_CUT_BIT)
#define ELF_LX_CUT_5		(0x5<<_ELF_LX_CUT_BIT)
#define ELF_LX_CUT_UNDEF	(0x6<<_ELF_LX_CUT_BIT)
#define _ELF_LX_CHECK_CUT(m) ((m&ELF_LX_CUT_MASK)==m)

/* (pp) abi */
#define _ELF_LX_ABI_BIT	(0)                             /* 1st bit position in byte */
#define ELF_LX_ABI_MASK		(0x7f<<_ELF_LX_ABI_BIT)           /* mask */
#define ELF_LX_ABI_NO		(0x0<<_ELF_LX_ABI_BIT)
#define ELF_LX_ABI_MULTI	(0x1<<_ELF_LX_ABI_BIT)
#define ELF_LX_ABI_EMBED	(0x2<<_ELF_LX_ABI_BIT)
#define ELF_LX_ABI_PIC		(0x3<<_ELF_LX_ABI_BIT)
#define ELF_LX_ABI_GCC		(0x4<<_ELF_LX_ABI_BIT)
#define ELF_LX_ABI_UNDEF	(0x5<<_ELF_LX_ABI_BIT)
#define ELF_LX_ABI_RELOC_EMBED  (0x6<<_ELF_LX_ABI_BIT)
#define _ELF_LX_CHECK_ABI(m) ((m&ELF_LX_ABI_MASK)==m)

/* compatibility with rta directive on solaris : rta bits where writen in the bits 28:31 on Solaris */
#define _ELF_LX_RTA_BIT	(28)                             /* 1st bit position in byte */

/* (pp) code generation mode */
#define _ELF_LX_MODE_BIT (7)                             /* 1st bit position in byte */
#define ELF_LX_MODE_MASK	(0x1<<_ELF_LX_MODE_BIT)           /* mask */
#define ELF_LX_MODE_USER	(0x0<<_ELF_LX_MODE_BIT)
#define ELF_LX_MODE_KERNEL	(0x1<<_ELF_LX_MODE_BIT)
#define _ELF_LX_CHECK_MODE(m) ((m&ELF_LX_MODE_MASK)==m)

const char * core_printable_name(flagword flags);
const char * cut_printable_name(flagword flags);
const char * abi_printable_name(flagword flags, Elf_Internal_Ehdr * i_ehdrp);
const char * osabi_printable_name(Elf_Internal_Ehdr * i_ehdrp);
const char * code_generation_mode_printable_name(Elf_Internal_Ehdr * i_ehdrp);
flagword lx_elf_get_private_flags (bfd* abfd);
void lx_elf_dump_target_info(bfd *abfd, FILE *writer);

/* (pp) say wether a function can be moved by icacheopt or not */
#define STO_FUNC_NATURE_BIT (4)
#define STO_FUNC_NATURE_MASK   (0x1 << STO_FUNC_NATURE_BIT)
#define STO_MOVEABLE  (0x1 << STO_FUNC_NATURE_BIT)
#define is_STO_MOVEABLE(o) (((o)&STO_FUNC_NATURE_MASK)==STO_MOVEABLE)

/* (tb) say wether a symbol has used attribute (mean not to be deleted by binopt tool */
#define STO_SYMB_USED_BIT (5)
#define STO_SYMB_USED_MASK   (0x1 << STO_SYMB_USED_BIT)
#define STO_USED  (0x1 << STO_SYMB_USED_BIT)
#define is_STO_USED(o) (((o)&STO_SYMB_USED_MASK)==STO_USED)

#define ELF_STRING_lx_pltoff ".lx.pltoff"

START_RELOC_NUMBERS (elf_lx_reloc_type)
     RELOC_NUMBER (R_LX_NONE,                 0)
     RELOC_NUMBER (R_LX_16,                   1)
     RELOC_NUMBER (R_LX_32,                   2)
     RELOC_NUMBER (R_LX_32_PCREL,             3)
     RELOC_NUMBER (R_LX_23_PCREL,             4)
     RELOC_NUMBER (R_LX_HI23,                 5)
     RELOC_NUMBER (R_LX_LO9,                  6)
     RELOC_NUMBER (R_LX_GPREL_HI23,           7)
     RELOC_NUMBER (R_LX_GPREL_LO9,            8)
     RELOC_NUMBER (R_LX_REL32,                9)
     RELOC_NUMBER (R_LX_GOTOFF_HI23,         10)
     RELOC_NUMBER (R_LX_GOTOFF_LO9,          11)
     RELOC_NUMBER (R_LX_GOTOFFX_HI23,        12)
     RELOC_NUMBER (R_LX_GOTOFFX_LO9,         13)
     RELOC_NUMBER (R_LX_LTV32,               14)
     RELOC_NUMBER (R_LX_SEGREL32,            15)
     RELOC_NUMBER (R_LX_FPTR32,              16)
     RELOC_NUMBER (R_LX_PLTOFF_HI23,         17)
     RELOC_NUMBER (R_LX_PLTOFF_LO9,          18)
     RELOC_NUMBER (R_LX_GOTOFF_FPTR_HI23,    19)
     RELOC_NUMBER (R_LX_GOTOFF_FPTR_LO9,     20)
     RELOC_NUMBER (R_LX_IPLT,                21)
     RELOC_NUMBER (R_LX_NEG_GPREL_HI23,      22)
     RELOC_NUMBER (R_LX_NEG_GPREL_LO9,       23)
     RELOC_NUMBER (R_LX_COPY,                24)
     RELOC_NUMBER (R_LX_JMP_SLOT,            25)
     RELOC_NUMBER (R_LX_TPREL_HI23,          26)
     RELOC_NUMBER (R_LX_TPREL_LO9,           27)
     RELOC_NUMBER (R_LX_TPREL32,             28)
     RELOC_NUMBER (R_LX_GOTOFF_TPREL_HI23,   29)
     RELOC_NUMBER (R_LX_GOTOFF_TPREL_LO9,    30)
     RELOC_NUMBER (R_LX_GOTOFF_DTPLDM_HI23,  31)
     RELOC_NUMBER (R_LX_GOTOFF_DTPLDM_LO9,   32)
     RELOC_NUMBER (R_LX_DTPREL_HI23,         33)
     RELOC_NUMBER (R_LX_DTPREL_LO9,          34)
     RELOC_NUMBER (R_LX_DTPMOD32,            35)
     RELOC_NUMBER (R_LX_DTPREL32,            36)
     RELOC_NUMBER (R_LX_GOTOFF_DTPNDX_HI23,  37)
     RELOC_NUMBER (R_LX_GOTOFF_DTPNDX_LO9,   38)
END_RELOC_NUMBERS (R_LX_max)

#endif
