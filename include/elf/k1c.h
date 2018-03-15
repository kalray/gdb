/* K1 ELF support for BFD.
 *
 * Copyright (C) 2009-2016 Kalray SA.
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

//#include "bfd.h"
//#include "elf-bfd.h"

#include "elf/reloc-macros.h"


/* 	 16.15 	  8.7  4.3  0 */
/* +----------------------------+ */
/* |      CUT | CORE  |PIC |ABI | */
/* +----------------------------+ */


#define K1_CUT_MASK 0x00ff0000
#define K1_CORE_MASK 0x0000ff00
#define K1_ABI_MASK 0x000000ff
#define K1_MACH_MASK (K1_CUT_MASK | K1_CORE_MASK | K1_ABI_MASK)

/*
 * Machine private data :
 * - byte 0 = ABI specific (PIC, OS, ...)
 * - byte 1 = Core info :
 *   - bits 0..2 = V1/DP/IO
 *   - bit  3..6 = Arch:
 *          0000 : Invalid
 *          XX01 : Andey
 *          XX10 : Bostan
 *          0100 : Coolidge
 *   - bit  7    = 32/64 bits addressing
 * - byte 2 = Cut info
 */

/* (pp) core */
#define ELF_K1_CORE_BIT        (8)                      /* 1st bit position in
                                                            byte */
#define ELF_K1_CORE_MASK        (0x7f<<ELF_K1_CORE_BIT)           /* mask */

#define ELF_K1_CORE_C           (0x20<<ELF_K1_CORE_BIT)
#define ELF_K1_CORE_UNDEF       (0)

#define ELF_K1_CORE_C_C (ELF_K1_CORE_C)

/* Last bit in byte used for 64bits addressing */
#define ELF_K1_CORE_ADDR64_MASK (0x80<<ELF_K1_CORE_BIT)
#define ELF_K1_IS_K1C (((flags) & ELF_K1_CORE_C) == ELF_K1_CORE_C)

#define ELF_K1_CHECK_CORE(flags,m) (((flags) & ELF_K1_CORE_MASK)==(m))

#define ELF_K1_CHECK_ADDR64(flags) (((flags) & ELF_K1_CORE_ADDR64_MASK))

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
#define _ELF_K1_CHECK_CUT(flags,m) (((flags) & ELF_K1_CUT_MASK)==(m))

/* (pp) abi */
/* 4 bits bitfield */
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
#define _ELF_K1_CHECK_ABI(flags,m) (((flags) & ELF_K1_ABI_MASK)==(m))


/* These flags have to be in sync with Linux kernel */
/* FIXME: Have to clean it up with the flags above
   however we need to be able to set both PIC and FDPIC together
   depending on the options so the ABI option is not the best for that */

/* (FD)PIC specific */
#define _ELF_K1_PIC_BIT (4)

#define ELF_K1_PIC_MASK          (0x3<<_ELF_K1_PIC_BIT)
#define ELF_K1_NOPIC             (0x0<<_ELF_K1_PIC_BIT)
#define ELF_K1_PIC               (0x1<<_ELF_K1_PIC_BIT) /* -fpic   */

#define _ELF_K1_CHECK_PIC(flags,m) (((flags) & ELF_K1_PIC_MASK)==(m))


/* (pp) code generation mode */
#define _ELF_K1_MODE_BIT (7)                             /* 1st bit position in
                                                            byte */
#define ELF_K1_MODE_MASK        (0x1<<_ELF_K1_MODE_BIT)           /* mask */
#define ELF_K1_MODE_USER        (0x0<<_ELF_K1_MODE_BIT)
#define ELF_K1_MODE_KERNEL      (0x1<<_ELF_K1_MODE_BIT)
#define _ELF_K1_CHECK_MODE(flags,m) (((flags) & ELF_K1_MODE_MASK)==(m))

#define ELF_STRING_k1_pltoff ".k1.pltoff"

/* Generated from MDS */
#include "bfd/reloc.h"

#endif
