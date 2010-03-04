/* ELF/LX support

   This file contains ELF/LX relocation support as specified
   in the ST200 ELF Specification, 2004.

   Copyright 2004
   Free Software Foundation, Inc.

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

#ifndef _ELF32_LX_H
#define _ELF32_LX_H

#include "elf-bfd.h"
#include "elf/lx.h"

void lx_elf_init_stub_bfd
  PARAMS ((bfd *, struct bfd_link_info *));

void elf_lx_next_input_section
  PARAMS ((struct bfd_link_info *, asection *));

int elf_lx_setup_section_lists
  PARAMS ((bfd *, struct bfd_link_info *));

bfd_boolean elf_lx_size_stubs
  PARAMS ((bfd *, bfd *, struct bfd_link_info *, bfd_signed_vma,
	   asection * (*) PARAMS ((const char *, asection *)),
	   void (*) PARAMS ((void))));

bfd_boolean elf_lx_set_gp
  PARAMS ((bfd *, struct bfd_link_info *));

bfd_boolean elf_lx_build_stubs
  PARAMS ((struct bfd_link_info *));

bfd_boolean elf_lx_secinit_compress_or_decompress
  PARAMS ((bfd *, bfd_boolean, char *));

int elf_lx_get_compact_info 
  PARAMS ((bfd *, sec_ptr, bfd_vma *, bfd_vma *,
	   bfd_size_type *, bfd_size_type *));

void lx_elf_dump_target_info
PARAMS ((bfd *, FILE *));

void
bfd_elf32_lx_set_transform_to_absolute
  PARAMS((struct bfd_link_info *, bfd_boolean value));
#endif /* _ELF32_LX_H */
