/* Kalray MPPA generic disassembler support code.
   Copyright (C) 2009-2016 Kalray SA.

   This file is part of libopcodes.

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   It is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.  */

#include "sysdep.h"
#define STATIC_TABLE
#define DEFINE_TABLE

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include "elf/k1c.h"
#include "opcode/k1c.h"
#include "dis-asm.h"

#define MAXBUNDLESIZE 8
#define MAXIMMX 4
#define NOIMMX -1
#define GETSTEERING(x) (((x) & 0x60000000) >> 29)
#define GETEXU(x) (((x) & 0x18000000) >> 27)
#define BCU_STEER 0
#define LSU_STEER 1
#define MAU_STEER 2
#define ALU_STEER 3
#define ALU0_EXU 0
#define ALU1_EXU 1
#define MAU_EXU 2
#define LSU_EXU 3

__attribute__((unused)) static char *get_steering_name(int steering) {
  switch(steering) {
  case BCU_STEER:
    return "BCU";
  case LSU_STEER:
    return "LSU";
  case MAU_STEER:
    return "MAU";
  case ALU_STEER:
    return "ALU";
  default:
    return "UNKNOWN STEERING";
  }
}

typedef struct {
  unsigned int bit0: 1;
  unsigned int bit1: 1;
  unsigned int bit2: 1;
  unsigned int bit3: 1;
  unsigned int bit4: 1;
  unsigned int bit5: 1;
  unsigned int bit6: 1;
  unsigned int bit7: 1;
  unsigned int bit8: 1;
  unsigned int bit9: 1;
  unsigned int bit10: 1;
  unsigned int bit11: 1;
  unsigned int bit12: 1;
  unsigned int bit13: 1;
  unsigned int bit14: 1;
  unsigned int bit15: 1;
  unsigned int bit16: 1;
  unsigned int bit17: 1;
  unsigned int bit18: 1;
  unsigned int bit19: 1;
  unsigned int bit20: 1;
  unsigned int bit21: 1;
  unsigned int bit22: 1;
  unsigned int bit23: 1;
  unsigned int bit24: 1;
  unsigned int bit25: 1;
  unsigned int bit26: 1;
  unsigned int bit27: 1;
  unsigned int bit28: 1;
  unsigned int bit29: 1;
  unsigned int bit30: 1;
  unsigned int bit31: 1;
} opcode_bits_t;

typedef union {
  opcode_bits_t bits;
  unsigned int  value;
} opcode_t;

struct objdump_disasm_info {
    bfd *abfd;
    asection *sec;
    bfd_boolean require_sec;
};

typedef struct {
    unsigned int insn[K1MAXCODEWORDS];
    unsigned int len;
} bundle_t;


/* static unsigned int reordered_bundle[MAXBUNDLESIZE]; */
static unsigned int bundle_ops[MAXBUNDLESIZE];

static bundle_t bundle_insn[MAXBUNDLESIZE];

typedef int (*reassemble_bundle_t)(unsigned int *opcnt);

static int k1c_steering(unsigned int x) {
  return (((x) & 0x60000000) >> 29);
}

static int k1c_substeering(unsigned int x) {
  return  (((x) & 0x18000000) >> 27);
}

static int k1c_has_parallel_bit(unsigned int x) {
  return (((x) & 0x80000000) == 0x80000000);
}

static int k1c_reassemble_bundle(unsigned int *_opcnt) {
  // units indexes in instr array
  int bcu_idx=0;
  int alu0_idx=1;
  int alu1_idx=2;
  int mau_idx=3;
  int lsu_idx=4;

  // Debugging flag
  int debug = 0;

  // units corresponding to the four substeering bits values : an immx whose substeering bits value is i goes to unit number immx_main_unit[i]
  int immx_main_unit[4] = {alu0_idx, alu1_idx, mau_idx, lsu_idx};

  struct instr_s {
    int valid;
    int opcode;
    int immx[2];
    int immx_valid[2];
    int immx_count;
    int nb_syllables;
  };

  unsigned int MAX_INSTRS=5;
  struct instr_s instr[MAX_INSTRS];

  // available resources
  int alu0_taken = 0;
  int alu1_taken = 0;
  int alu2_taken = 0;
  int alu3_taken = 0;
  int mau_taken = 0;
  int lsu_taken = 0;

  int i, j;
  int opcnt = *_opcnt;

  int instr_idx = 0;

  for(i=0; i < (int) MAX_INSTRS; i++) {
    instr[i].valid = 0;
    instr[i].immx_valid[0] = 0;
    instr[i].immx_valid[1] = 0;
    instr[i].immx_count = 0;
    instr[i].nb_syllables = 0;
  }

  if(debug) fprintf(stderr,"k1c_reassemble_bundle: opcnt = %d\n",opcnt);

  if(opcnt == 0) {
    if(debug) fprintf(stderr,"opcnt == 0\n");
    return 1;
  }

  for (i = 0; i < opcnt ; i ++) {
    switch (k1c_steering(bundle_ops[i])) {
      
      // immx syllable
    case BCU_STEER:
      // BCU instruction
      if(i == 0) {
	if(debug) fprintf(stderr,"Syllable 0: Set valid on BCU for instr %d with 0x%x\n",bcu_idx,bundle_ops[0]);
	instr[bcu_idx].valid = 1;
	instr[bcu_idx].opcode = bundle_ops[0];
	instr[bcu_idx].nb_syllables = 1;
      }
      else {
	// Not first syllable in bundle, IMMX
	struct instr_s *instr_p = &(instr[immx_main_unit[k1c_substeering(bundle_ops[i])]]);
	int immx_count = instr_p->immx_count;
	instr_p->immx[immx_count] = bundle_ops[i];
	instr_p->immx_valid[immx_count] = 1;
	instr_p->nb_syllables++;
	if(debug) fprintf(stderr,"Set IMMX[%d] on instr %d for substeering %d @ %d\n",
			  immx_count, immx_main_unit[k1c_substeering(bundle_ops[i])],k1c_substeering(bundle_ops[i]),i);
	instr_p->immx_count = immx_count + 1;
      }
      break;
      
    case ALU_STEER:
      if (alu0_taken == 0) {
	if(debug) fprintf(stderr,"Set valid on ALU0 for instr %d with 0x%x\n",alu0_idx,bundle_ops[i]);
	instr[alu0_idx].valid = 1;
	  instr[alu0_idx].opcode = bundle_ops[i];
	  instr[alu0_idx].nb_syllables = 1;
	  alu0_taken = 1;
      } else if (alu1_taken == 0) {
	if(debug) fprintf(stderr,"Set valid on ALU1 for instr %d with 0x%x\n",alu1_idx,bundle_ops[i]);
	instr[alu1_idx].valid = 1;
	instr[alu1_idx].opcode = bundle_ops[i];
	instr[alu1_idx].nb_syllables = 1;
	alu1_taken = 1;
      } else if (alu2_taken == 0) {
	if(debug) fprintf(stderr,"Set valid on MAU (ALU) for instr %d with 0x%x\n",mau_idx,bundle_ops[i]);
	instr[mau_idx].valid = 1;
	instr[mau_idx].opcode = bundle_ops[i];
	instr[mau_idx].nb_syllables = 1;
	alu2_taken = 1;
      } else if (alu3_taken == 0) {
	if(debug) fprintf(stderr,"Set valid on LSU (ALU) for instr %d with 0x%x\n",lsu_idx,bundle_ops[i]);
	instr[lsu_idx].valid = 1;
	instr[lsu_idx].opcode = bundle_ops[i];
	instr[lsu_idx].nb_syllables = 1;
	alu3_taken = 1;
      } else {
	if(debug) fprintf(stderr,"Too many ALU instructions");
	return 1;
      }
      break;
      
    case MAU_STEER:
      if (mau_taken == 1) {
	if(debug) fprintf(stderr,"Too many MAU instructions");
	return 1;
      } else {
	mau_taken = 1;
	if(debug) fprintf(stderr,"Set valid on MAU for instr %d with 0x%x\n",mau_idx,bundle_ops[i]);
	instr[mau_idx].valid = 1;
	instr[mau_idx].opcode = bundle_ops[i];
	instr[mau_idx].nb_syllables = 1;
	alu2_taken = 1;
      }
      break;
      
    case LSU_STEER:
      if (lsu_taken == 1) {
	if(debug) fprintf(stderr,"Too many LSU instructions");
	return 1;
      } else {
	lsu_taken = 1;
	if(debug) fprintf(stderr,"Set valid on LSU for instr %d with 0x%x\n",lsu_idx,bundle_ops[i]);
	instr[lsu_idx].valid = 1;
	instr[lsu_idx].opcode = bundle_ops[i];
	instr[lsu_idx].nb_syllables = 1;
	alu3_taken = 1;
      }
    }
    if (!(k1c_has_parallel_bit(bundle_ops[i]))) {
      if(debug) fprintf(stderr,"Stop! stop bit is set 0x%x\n",bundle_ops[i]);
      break;
    }
    if(debug) fprintf(stderr,"Continue %d < %d?\n",i,opcnt);
    
  }
  if (k1c_has_parallel_bit(bundle_ops[i])) {
      if(debug) fprintf(stderr,"bundle exceeds maximum size");
      return 1;
  }

  // Fill bundle_insn and count read syllables
  instr_idx = 0;
  for (i = 0; i < (int) MAX_INSTRS; i++) {
    if (instr[i].valid == 1) {
      int syllable_idx = 0;

      // First copy opcode
      bundle_insn[instr_idx].insn[syllable_idx++] = instr[i].opcode;
      bundle_insn[instr_idx].len = 1;

      for(j=0; j < 2; j++) {
	if(instr[i].immx_valid[j]) {
	  if(debug) fprintf(stderr,"Instr %d valid immx[%d] is valid\n",i,j);	
	  bundle_insn[instr_idx].insn[syllable_idx++] = instr[i].immx[j];
	  bundle_insn[instr_idx].len++;
	}
      }

      if(debug) fprintf(stderr,"Instr %d valid, copying in bundle_insn (%d syllables <-> %d)\n",i,bundle_insn[instr_idx].len,instr[i].nb_syllables);
      instr_idx++;
    }
  }

  if(debug) fprintf(stderr,"End => %d instructions\n",instr_idx);

  *_opcnt = instr_idx;
  return 0;
}


int print_insn_k1 (bfd_vma memaddr, struct disassemble_info *info){
  static unsigned int opindex = 0;
  static unsigned int opcnt = 0;
  k1opc_t *op = NULL;             /* operation table index */
  bundle_t *insn;           /* the instruction       */
  int status;              /* temp                  */
  char *fmtp;
  k1opc_t *opc_table = NULL;
  int          *k1_regfiles = NULL;
  k1_Register  *k1_registers = NULL;
  int          *k1_dec_registers = NULL;
  // unsigned int  k1_max_registers = 0;
  unsigned int  k1_max_dec_registers = 0;
  int k1_arch_size = 32;
  int readsofar = 0;
  int opt_pretty = 0;
  int found = 0;
  int invalid_bundle = 0;
  reassemble_bundle_t reassemble_bundle = NULL;

  /* check that tables are initialized */

  if (info->arch != bfd_arch_k1) {
      fprintf(stderr, "error: Unknown architecture\n");
      exit(-1);
  }

  switch (info->mach) {

    case bfd_mach_k1c_k1pe_64:
      k1_arch_size = 64;
    case bfd_mach_k1c_k1pe_usr:
    case bfd_mach_k1c_k1pe:
      opc_table = k1pe_k1optab;
      k1_regfiles = k1_k1pe_regfiles;
      k1_registers = k1_k1pe_registers;
      k1_dec_registers = k1_k1pe_dec_registers;
      reassemble_bundle = k1c_reassemble_bundle;
      break;

    case bfd_mach_k1c_k1rm_64:
      k1_arch_size = 64;
    case bfd_mach_k1c_k1rm_usr:
    case bfd_mach_k1c_k1rm:
      opc_table = k1rm_k1optab;
      k1_regfiles = k1_k1rm_regfiles;
      k1_registers = k1_k1rm_registers;
      k1_dec_registers = k1_k1rm_dec_registers;
      reassemble_bundle = k1c_reassemble_bundle;
      break;

    default:
      /* Core not supported */
      (*info->fprintf_func)(info->stream, "disassembling not supported for this K1 core! (core:%d)",
			    (int) info->mach);
      return -1;
  }

  // k1_max_registers = k1_regfiles[K1_REGFILE_REGISTERS];
  k1_max_dec_registers = k1_regfiles[K1_REGFILE_DEC_REGISTERS];

  if (opc_table == NULL) {
      fprintf(stderr, "error: uninitialized opcode table\n");
      exit(-1);
  }

  // Set line length
  info->bytes_per_line = 16;

  // Use -Mpretty when calling objdump
  if(info->disassembler_options && strstr(info->disassembler_options, "pretty")){
    opt_pretty = 1;
  }

  /* read the instruction */

  /* If this is the beginning of the bundle, read BUNDLESIZE words and apply decentrifugate function */
  if(opindex == 0){
      opcnt = 0;
      do{
          assert(opcnt < MAXBUNDLESIZE);
          status = (*info->read_memory_func) (memaddr + 4*opcnt, (bfd_byte*)(bundle_ops + opcnt), 4, info);
          if (status != 0){
              (*info->memory_error_func) (status, memaddr + 4*opcnt, info);
              return -1;
          }
          opcnt++;
      } while (k1c_parallel_fld(bundle_ops[opcnt-1]) && opcnt < MAXBUNDLESIZE);
      invalid_bundle = reassemble_bundle(&opcnt);
  }
  assert(opindex < MAXBUNDLESIZE);
  insn = &(bundle_insn[opindex]);
  readsofar = insn->len * 4;
  opindex++;

  /* check for extension to right      */
  /* iff this is not the end of bundle */

  for (op = opc_table; op->as_op && (((char)op->as_op[0]) != 0); op++){  /* find the format of this insn */
      int opcode_match = 1;
      unsigned int i;
      int ch;

      if(invalid_bundle){
          break;
      }

      if(op->codewords != insn->len){
          continue;
      }


      for(i=0; i < op->codewords; i++) {
	if ((op->codeword[i].mask & insn->insn[i]) != op->codeword[i].opcode) {
	  opcode_match = 0;
	}
      }
      int encoding_space_flags = k1_arch_size == 32 ? k1OPCODE_FLAG_MODE32 : k1OPCODE_FLAG_MODE64;

      for(i=0; i < op->codewords; i++) {
	if (! (op->codeword[i].flags & encoding_space_flags))
	  opcode_match = 0;
      }

      if (opcode_match) {
          /* print the operands using the instructions format string. */
          fmtp = op->fmtstring;
          // If the user wants "pretty printing", ie, not the usual little endian objdump output
          if(opt_pretty){
              (*info->fprintf_func) (info->stream, "[ ");
              for(i = 0; i < insn->len; i++){
                  (*info->fprintf_func) (info->stream, "%08x ", insn->insn[i]);
              }
              (*info->fprintf_func) (info->stream, "] ");
          }


          /* print the opcode   */
          (*info->fprintf_func) (info->stream, "%s ", op->as_op);

          for (i = 0; op->format[i]; i++){
              k1_bitfield_t *bf = op->format[i]->bfield;
              int bf_nb = op->format[i]->bitfields;
              int width = op->format[i]->width;
              int type  = op->format[i]->type;
              char *type_name  = op->format[i]->tname;
              int flags = op->format[i]->flags;
              int rightshift = op->format[i]->rightshift;
              unsigned long long value = 0;
              int bf_idx;


              /* Print characters in the format string up to the following
               * % or nul. */
              while((ch=*fmtp) && ch != '%'){
                  (*info->fprintf_func) (info->stream, "%c", ch);
                  fmtp++;
              }

              /* Skip past %s */
              if(ch == '%'){
                  ch=*fmtp++;
                  fmtp++;
              }

              for(bf_idx=0;bf_idx < bf_nb; bf_idx++) {
                  int insn_idx = (int)bf[bf_idx].to_offset / 32;
                  int to_offset = bf[bf_idx].to_offset % 32;
                  unsigned long long encoded_value = insn->insn[insn_idx] >> to_offset;
                  encoded_value &= (1LL << bf[bf_idx].size) - 1;
                  value |= encoded_value << bf[bf_idx].from_offset;
              }
              if (flags & k1SIGNED){
                  unsigned long long signbit = 1LL << (width -1);
                  value = (value ^ signbit) - signbit;
              }

#define K1_PRINT_REG(regfile,value)					\
 if(k1_regfiles[regfile]+value < k1_max_dec_registers) {	        \
   (*info->fprintf_func) (info->stream, "%s", k1_registers[k1_dec_registers[k1_regfiles[regfile]+value]].name); \
 }									\
 else {									\
   (*info->fprintf_func) (info->stream, "$??");				\
 }

#define CASE_SRF_REGCLASSES(core)                           \
        case RegClass_ ## core ## _systemReg:      \
        case RegClass_ ## core ##_nopcpsReg:       \
        case RegClass_ ## core ## _onlypsReg:      \
        case RegClass_ ## core ## _onlyraReg:      \
        case RegClass_ ## core ## _onlyfxReg

              switch (type) {
                  case RegClass_k1c_singleReg:
                      K1_PRINT_REG(K1_REGFILE_DEC_GRF,value)
                      break;
                  case RegClass_k1c_pairedReg:
                      K1_PRINT_REG(K1_REGFILE_DEC_PRF,value)
                      break;
                  CASE_SRF_REGCLASSES(k1c):
                      K1_PRINT_REG(K1_REGFILE_DEC_SRF,value)
                      break;
                  case RegClass_k1c_accelReg:
                      K1_PRINT_REG(K1_REGFILE_DEC_ARF,value)
                      break;

                  case Immediate_k1c_eventmask2:
                  case Immediate_k1c_flagmask2:
                  case Immediate_k1c_brknumber:
                  case Immediate_k1c_sysnumber:
                  case Immediate_k1c_signed10:
                  case Immediate_k1c_signed16:
                  case Immediate_k1c_signed27:
                  case Immediate_k1c_signed32:
                  case Immediate_k1c_signed37:
                  case Immediate_k1c_signed43:
                  case Immediate_k1c_signed64:
                  case Immediate_k1c_unsigned5:
                  case Immediate_k1c_unsigned6:
                      value <<= rightshift;
                      if(flags & k1SIGNED){
                          if(width <= 32) {
                              (*info->fprintf_func) (info->stream, "%d (0x%x)", (int)value, (int)value);
                          }
                          else {
                              (*info->fprintf_func) (info->stream, "%lld (0x%llx)", value, value);
                          }
                      } else {
                          if(width <= 32) {
                              (*info->fprintf_func) (info->stream, "%u (0x%x)", (unsigned int) value, (unsigned int) value);
                          }
                          else {
                              (*info->fprintf_func) (info->stream, "%llu (0x%llx)", (unsigned long long) value, (unsigned long long) value);
                          }
                      }
                      break;
                  case Immediate_k1c_pcrel17:
                  case Immediate_k1c_pcrel27:
                      (*info->print_address_func)((value << rightshift) + memaddr, info);
                      break;
                  default:
                      fprintf(stderr, "error: unexpected operand type (%s)\n", type_name);
                      exit(-1);
              };

#undef K1_PRINT_REG     
#undef CASE_SRF_REGCLASSES
          }

          /* Print trailing characters in the format string, if any */
          while((ch=*fmtp)){
              (*info->fprintf_func) (info->stream, "%c", ch);
              fmtp++;
          }

          found = 1;
          break;
      }
  }



  if (found && (opindex == opcnt)){
      (*info->fprintf_func) (info->stream, ";;\n");
      opindex = 0;
  }
  // couldn't find the opcode, skip this word
  if(!found){
      (*info->fprintf_func) (info->stream, "*** invalid opcode ***\n");
      opindex = 0;
      readsofar = 4;
  }
  return readsofar;
}

void print_k1_disassembler_options(FILE *stream){
    fprintf(stream, "\nThe following K1 specific disassembler options are supported for use\nwith the -M switch (multiple options should be separated by commas):\n");
    fprintf(stream, "\npretty             Print 32-bit words in natural order corresponding to re-ordered instruction.\n\n");
}
