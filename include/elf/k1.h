/**
 * *** (c) Copyright Hewlett-Packard Company 1999-2003 
 * ***
 * *** This program is free software; you can redistribute it and/or
 * *** modify it under the terms of the GNU General Public License
 * *** as published by the Free Software Foundation; either version
 * *** 2 of the License, or (at your option) any later version.
 * *** 
 * *** This program is distributed in the hope that it will be useful,
 * *** but WITHOUT ANY WARRANTY; without even the implied warranty of
 * *** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * *** General Public License for more details.
 * ***
 * *** You should have received a copy of the GNU General Public License 
 * *** along with this program; if not, write to the Free Software
 * *** Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 * **/

/*
 *   THIS FILE HAS BEEN MODIFIED OR ADDED BY STMicroelectronics, Inc. 1999-2003
 *   */

/* K1 ELF support for BFD.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

/* This file holds definitions specific to the PPC ELF ABI.  Note
 *    that most of this is not actually implemented by BFD.  */

#ifndef _ELF_K1_H
#define _ELF_K1_H

#include "bfd.h"
#include "elf-bfd.h"

#include "elf/reloc-macros.h"


typedef union {
    struct {
          unsigned rta : 4;  
            } obj_compat;
      unsigned int f;
} k1_bfd_flags;

/* (pp) core */
#define _ELF_K1_CORE_BIT        (8)                      /* 1st bit position in 
                                                            byte */
#define ELF_K1_CORE_MASK        (0xff<<_ELF_K1_CORE_BIT)           /* mask */
#define ELF_K1_CORE_V1          (0x0<<_ELF_K1_CORE_BIT)
#define ELF_K1_CORE_DP          (0x1<<_ELF_K1_CORE_BIT)
#define ELF_K1_CORE_IO          (0x2<<_ELF_K1_CORE_BIT)
#define ELF_K1_CORE_UNDEF       (0x7<<_ELF_K1_CORE_BIT)
#define _ELF_K1_CHECK_CORE(m) ((m&ELF_K1_CORE_MASK)==m)

/* (pp) cut */
#define _ELF_K1_CUT_BIT (16)                             /* 1st bit position in 
                                                            byte */
#define ELF_K1_CUT_MASK         (0xf<<_ELF_K1_CUT_BIT)           /* mask */
#define ELF_K1_CUT_0            (0x0<<_ELF_K1_CUT_BIT)
#define ELF_K1_CUT_1            (0x1<<_ELF_K1_CUT_BIT)
#define ELF_K1_CUT_2            (0x2<<_ELF_K1_CUT_BIT)
#define ELF_K1_CUT_3            (0x3<<_ELF_K1_CUT_BIT)
#define ELF_K1_CUT_4            (0x4<<_ELF_K1_CUT_BIT)
#define ELF_K1_CUT_5            (0x5<<_ELF_K1_CUT_BIT)
#define ELF_K1_CUT_UNDEF        (0x6<<_ELF_K1_CUT_BIT)
#define _ELF_K1_CHECK_CUT(m) ((m&ELF_K1_CUT_MASK)==m)

/* (pp) abi */
#define _ELF_K1_ABI_BIT (0)                             /* 1st bit position in b
                                                           yte */
#define ELF_K1_ABI_MASK         (0xf<<_ELF_K1_ABI_BIT)           /* mask */
#define ELF_K1_ABI_NO           (0x0<<_ELF_K1_ABI_BIT)
#define ELF_K1_ABI_MULTI        (0x1<<_ELF_K1_ABI_BIT)
#define ELF_K1_ABI_EMBED        (0x2<<_ELF_K1_ABI_BIT)
#define ELF_K1_ABI_PIC          (0x3<<_ELF_K1_ABI_BIT)
#define ELF_K1_ABI_GCC          (0x4<<_ELF_K1_ABI_BIT)
#define ELF_K1_ABI_UNDEF        (0x5<<_ELF_K1_ABI_BIT)
#define ELF_K1_ABI_RELOC_EMBED  (0x6<<_ELF_K1_ABI_BIT)
#define _ELF_K1_CHECK_ABI(m) ((m&ELF_K1_ABI_MASK)==m)


/* These flags have to be in sync with Linux kernel */
/* FIXME: Have to clean it up with the flags above
   however we need to be able to set both PIC and FDPIC together
   depending on the options so the ABI option is not the best for that */

/* (FD)PIC specific */
#define _ELF_K1_PIC_BIT (4)

#define ELF_K1_PIC_MASK          (0x3)
#define ELF_K1_NOPIC             (0x0<<_ELF_K1_PIC_BIT)
#define ELF_K1_PIC               (0x1<<_ELF_K1_PIC_BIT) /* -fpic   */
#define ELF_K1_FDPIC             (0x2<<_ELF_K1_PIC_BIT) /* -mfdpic */
#define _ELF_K1_CHECK_PIC(m) ((m&ELF_K1_PIC_MASK)==m)

/* compatibility with rta directive on solaris : rta bits where writen in the bi
 * ts 28:31 on Solaris */
#define _ELF_K1_RTA_BIT (28)                             /* 1st bit position in 
                                                            byte */

/* (pp) code generation mode */
#define _ELF_K1_MODE_BIT (7)                             /* 1st bit position in 
                                                            byte */
#define ELF_K1_MODE_MASK        (0x1<<_ELF_K1_MODE_BIT)           /* mask */
#define ELF_K1_MODE_USER        (0x0<<_ELF_K1_MODE_BIT)
#define ELF_K1_MODE_KERNEL      (0x1<<_ELF_K1_MODE_BIT)
#define _ELF_K1_CHECK_MODE(m) ((m&ELF_K1_MODE_MASK)==m)

const char * core_printable_name(flagword flags);
const char * cut_printable_name(flagword flags);
const char * abi_printable_name(flagword flags, Elf_Internal_Ehdr * i_ehdrp);
const char * osabi_printable_name(Elf_Internal_Ehdr * i_ehdrp);
const char * code_generation_mode_printable_name(Elf_Internal_Ehdr * i_ehdrp);
flagword k1_elf_get_private_flags (bfd* abfd);
void k1_elf_dump_target_info(bfd *abfd, FILE *writer);

#if 0
/* FIXME: Do we need that? */
/* (pp) say wether a function can be moved by icacheopt or not */
#define STO_FUNC_NATURE_BIT (4)
#define STO_FUNC_NATURE_MASK   (0x1 << STO_FUNC_NATURE_BIT)
#define STO_MOVEABLE  (0x1 << STO_FUNC_NATURE_BIT)
#define is_STO_MOVEABLE(o) (((o)&STO_FUNC_NATURE_MASK)==STO_MOVEABLE)

/* (tb) say wether a symbol has used attribute (mean not to be deleted by binopt
 *  tool */
#define STO_SYMB_USED_BIT (5)
#define STO_SYMB_USED_MASK   (0x1 << STO_SYMB_USED_BIT)
#define STO_USED  (0x1 << STO_SYMB_USED_BIT)
#define is_STO_USED(o) (((o)&STO_SYMB_USED_MASK)==STO_USED)
#endif//0

#define ELF_STRING_k1_pltoff ".k1.pltoff"

START_RELOC_NUMBERS (elf_k1_reloc_type)
       RELOC_NUMBER (R_K1_NONE,                 0)
       RELOC_NUMBER (R_K1_16,                   1)
       RELOC_NUMBER (R_K1_32,                   2)
        RELOC_NUMBER(R_K1_17_PCREL, 3)
        RELOC_NUMBER(R_K1_18_PCREL, 4)
        RELOC_NUMBER(R_K1_27_PCREL, 5)
        RELOC_NUMBER(R_K1_32_PCREL, 6)
        RELOC_NUMBER(R_K1_LO10, 7)
        RELOC_NUMBER(R_K1_HI22, 8)
        RELOC_NUMBER(R_K1_GPREL_LO10, 9)
        RELOC_NUMBER(R_K1_GPREL_HI22, 10)
        RELOC_NUMBER(R_K1_TPREL_LO10, 11)
        RELOC_NUMBER(R_K1_TPREL_HI22, 12)
        RELOC_NUMBER(R_K1_TPREL_32, 13)
        RELOC_NUMBER(R_K1_GOTOFF_LO10, 14)
        RELOC_NUMBER(R_K1_GOTOFF_HI22, 15)
        RELOC_NUMBER(R_K1_GOT_LO10, 16)
        RELOC_NUMBER(R_K1_GOT_HI22, 17)
        RELOC_NUMBER(R_K1_GLOB_DAT, 18)
        RELOC_NUMBER(R_K1_PLT_LO10, 19)
        RELOC_NUMBER(R_K1_PLT_HI22, 20)
        RELOC_NUMBER(R_K1_FUNCDESC, 21)
        RELOC_NUMBER(R_K1_FUNCDESC_GOT_LO10, 22)
        RELOC_NUMBER(R_K1_FUNCDESC_GOT_HI22, 23)
        RELOC_NUMBER(R_K1_FUNCDESC_GOTOFF_LO10, 24)
        RELOC_NUMBER(R_K1_FUNCDESC_GOTOFF_HI22, 25)
        RELOC_NUMBER(R_K1_FUNCDESC_VALUE, 26)
        RELOC_NUMBER(R_K1_GOTOFF, 27)
        RELOC_NUMBER(R_K1_GOT, 28)
        RELOC_NUMBER(R_K1_10_GPREL, 29)
        RELOC_NUMBER(R_K1_16_GPREL, 30)
  END_RELOC_NUMBERS (R_K1_max)

#endif
  
