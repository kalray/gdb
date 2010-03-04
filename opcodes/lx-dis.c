/*
  THIS FILE HAS BEEN MODIFIED OR ADDED BY STMicroelectronics, Inc. 1999-2003
*/

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

/**
*** static char sccs_id[] = "@(#)lx-dis.c	1.6 12/13/99 09:13:48";
**/

#include <stdlib.h>
#include <stdio.h>
#include "ansidecl.h"
#include "sysdep.h"
#include "dis-asm.h"
#include "opcode/lx.h"
#include "elf/lx.h"

/* Things to do 
 *     -- immext is handled as a bit of a hack
 *     -- bit extraction and insertion should be handled in
 *        lx-opc.c
 *     -- handle bundles, cluster info
 *
 */

/* Extra info passed by objdump (DUPLICATED from objdump.c !!!) */
struct objdump_disasm_info {
  bfd *abfd;
  asection *sec;
  bfd_boolean require_sec;
};

static int
print_insn_lx (bfd_vma memaddr, struct disassemble_info *info, int bigendian)
{
  lxopc_t *op;         /* operation table index */
  bfd_byte buffer[4];  /* buffer for code       */
  unsigned insn;       /* the instruction       */
  unsigned long long extension = 0;  /* immediate extension   */
  int hasextension = 0;/* flag for imm. ext.    */
  int status;          /* temp                  */
  int extnstop = 0;
  int extnclusterstop = 0;
  static int cluster = 0;
  char *fmtp;
  int ch;
  lxopc_t *opc_table = NULL;
  struct objdump_disasm_info *aux;
  flagword elf_private_flags = 0;
  int lx_core = 0;
  
  /* check that tables are initialized */

  /*  lx_init_asm_table(); */
  /* Get the enconding table to use (st220/st231/...) */
  aux = (struct objdump_disasm_info *) info->application_data;
  elf_private_flags = lx_elf_get_private_flags(aux->abfd);
  lx_core = elf_private_flags & ELF_LX_CORE_MASK;
  switch (lx_core) {
  case ELF_LX_CORE_ST220:
      opc_table = st220_lxoptab;
      break;
  case ELF_LX_CORE_ST231:
      opc_table = st231_lxoptab;
      break;
  case ELF_LX_CORE_ST240:
      opc_table = st240_lxoptab;
      break;
  default:
      /* Core not supported */
      (*info->fprintf_func)(info->stream, "disassembling not supported for this ST2xx core!");
      return -1;
  }

  if (opc_table == NULL) {
      fprintf(stderr, "error: uninitialized ST2xx opcode table\n");
      exit(-1);
  }

  /* read the instruction */

  status = (*info->read_memory_func) (memaddr, buffer, 4, info);
  if (status != 0)
    {
      (*info->memory_error_func) (status, memaddr, info);
      return -1;
    }
  if (bigendian)
    insn = bfd_getb32 (buffer);
  else
    insn = bfd_getl32 (buffer);

  if ((LXISIMMEXTL(insn)) || (LXISIMMEXTR(insn)))
    {
      (*info->fprintf_func) (info->stream, "");
      return 4;
    }

  /* read ahead/back one word if possible   */
  /* check this for extension syllable */

  if ((memaddr & 4)) /* instruction is odd aligned */
    {

      /* check for extension to right       */
      /* iff this is not the end of bundle */

      if (!LXSTOP(insn))
	{
	  status = (*info->read_memory_func) (memaddr + 4, buffer, 4, info);

	  if (status == 0)
	    {
	      if (bigendian)
		extension = bfd_getb32 (buffer);
	      else
		extension = bfd_getl32 (buffer);
	      if (LXISIMMEXTL(extension))
		{
		  hasextension = 1;
		  extnstop = LXSTOP(extension);
		  extnclusterstop = LXCLUSTER(extension);
		  extension = LXIMMVAL(extension);
		};
	    }
	}

      /* check for extension to left */

      if (!hasextension) 
	{
	  status = (*info->read_memory_func) (memaddr - 4, buffer, 4, info);
	  
	  if (status == 0)
	    {
	      if (bigendian)
		extension = bfd_getb32 (buffer);
	      else
		extension = bfd_getl32 (buffer);
	      if (!LXSTOP(extension) && LXISIMMEXTR(extension))
		{
		  hasextension = 1;
		  extension = LXIMMVAL(extension);
		};
	    }
	}
    }

  for (op = opc_table; ((char)op->as_op[0]) != 0; op++)  /* find the format of this inst */
    {
      int codewords = op->codewords;
      int opcode_match = 1;
      int i;
      for(i=0;i<codewords;i++) {
	if ((op->codeword[i].mask & ((insn >> (32 * i)) & 0xffffffffLL)) != op->codeword[i].opcode) {
	  opcode_match = 0;
	}
      }

      if (opcode_match)
	{
	  int i;

	  /* print the opcode   */

	  (*info->fprintf_func) (info->stream, "%s ", op->as_op);

	  /* print the operands using the instructions format string. */

	  fmtp = op->fmtstring;

	  for (i = 0; op->format[i]; i++)
	    {
	      /* if (op->format[i]) */
		{
		  lx_bitfield_t *bf = op->format[i]->bfield;
		  int bf_nb = op->format[i]->bitfields;
		  int width = op->format[i]->width;
		  int type  = op->format[i]->type;
		  int flags = op->format[i]->flags;
		  unsigned long long value=0;
		  int ch;
		  int bf_idx;

		  /* Print characters in the format string up to the following
		     % or nul. */
		  while((ch=*fmtp) && ch != '%')
		    {
		      (*info->fprintf_func)(info->stream, "%c", ch);
		      fmtp++;
		    }

		  /* Skip past %s */
		  if(ch == '%')
		    {
		      ch=*fmtp++;
		      /* if(ch != 's')
			 give error message */
		      fmtp++;
		    }
		  
		  for(bf_idx=0;bf_idx < bf_nb; bf_idx++) {
		    unsigned long long encoded_value = (insn >> bf[bf_idx].to_offset);
		    encoded_value &= (1 << bf[bf_idx].size) - 1;
		    value |= encoded_value << bf[bf_idx].from_offset;
		  }
		  if (flags & lxSIGNED)
		    {
		      unsigned long long signbit = 1LL << (width -1);
		      value = (value ^ signbit) - signbit;
		    }
		  switch (type)
		    {
		    case RegClass_st200_general :
		    case RegClass_st200_nolink :
		      (*info->fprintf_func)(info->stream,"$r%d", value);
		      break;
		    case RegClass_st200_paired :
		    case RegClass_st200_nzpaired :
		      (*info->fprintf_func)(info->stream,"$p%d", value);
		      break;
		    case RegClass_st200_branch :
		    case RegClass_st200_predicate :
		      (*info->fprintf_func)(info->stream,"$b%d", value);
		      break;
		    case RegClass_st200_link :
		      (*info->fprintf_func)(info->stream,"$r63");
		      break;
		    case Immediate_st200_brknum:
		    case Immediate_st200_imm:
		    case Immediate_st200_isrc2:
		    case Immediate_st200_sbrknum:
		    case Immediate_st200_xsrc2: {
		      if (hasextension)
			value = (value & ~(-1LL << width)) | extension;
		      (*info->fprintf_func)(info->stream,"%d (0x%x)", (int)value, (int)value);
		    }
		      break;
		    case Immediate_st200_btarg:
		      (*info->print_address_func)((value * 4) + memaddr, info);
		      break;
		    case RegClass_st200_pairedfirst:
		    case RegClass_st200_pairedsecond:
		    default:
		      fprintf(stderr, "error: unexpected operand type (%d)\n", type);
		      exit(-1);
		    };
		}
		/*else
		break;*/
	    }

	  /* Print trailing characters in the format string, if any */
	  while((ch=*fmtp))
	    {
	      (*info->fprintf_func)(info->stream, "%c", ch);
	      fmtp++;
	    }

#if MULTI_CLUSTERS_SUPPORTED
	  if (LXCLUSTER(insn) || extnclusterstop)
	    cluster++;
#else
	  if (LXCLUSTER(insn) || extnclusterstop)
	       (*info->fprintf_func)(info->stream, 
				     "(*** warning: Cluster Bit set!)");
#endif	  
	  if (LXSTOP(insn) || extnstop)
	    {
	      (*info->fprintf_func)(info->stream, ";;");
	      cluster = 0;
	    }
	  return 4;
	};
    }
  
  /* couldn't find the opcode, skip this word */
  
  (*info->fprintf_func) (info->stream, "*** warning: invalid opcode");
  return 4;
}


int 
print_insn_big_lx (bfd_vma memaddr, struct disassemble_info *info)
{
  return print_insn_lx (memaddr, info, 1);
}

int
print_insn_little_lx (bfd_vma memaddr, struct disassemble_info *info)
{
  return print_insn_lx (memaddr, info, 0);
}
