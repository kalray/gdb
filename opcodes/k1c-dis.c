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
#include <stdint.h>
#include <assert.h>
#include "elf/k1c.h"
#include "opcode/k1c.h"
#include "dis-asm.h"

// Steering values for the k1c VLIW architecture.
typedef enum {
  Steering_BCU,
  Steering_LSU,
  Steering_MAU,
  Steering_ALU,
  Steering__
} enum_Steering;
typedef uint8_t Steering;

// Extension values a.k.a. sub-steering values.
typedef enum {
  Extension_ALU0,
  Extension_ALU1,
  Extension_MAU,
  Extension_LSU,
  Extension__
} enum_Extension;
typedef uint8_t Extension;

/*
 * BundleIssue enumeration.
 */
typedef enum {
  BundleIssue_BCU,
  BundleIssue_TCA,
  BundleIssue_ALU0,
  BundleIssue_ALU1,
  BundleIssue_MAU,
  BundleIssue_LSU,
  BundleIssue__,
} enum_BundleIssue;
typedef uint8_t BundleIssue;

/*
 * An IMMX syllable is associated with the BundleIssue Extension_BundleIssue[extension].
 */
static const BundleIssue
Extension_BundleIssue[] = { BundleIssue_ALU0, BundleIssue_ALU1, BundleIssue_MAU, BundleIssue_LSU };

static inline int
k1c_steering(uint32_t x)
{
  return (((x) & 0x60000000) >> 29);
}

static inline int
k1c_extension(uint32_t x)
{
  return  (((x) & 0x18000000) >> 27);
}

static inline int
k1c_has_parallel_bit(uint32_t x)
{
  return (((x) & 0x80000000) == 0x80000000);
}

static inline int
k1c_is_tca_opcode(uint32_t x)
{
  unsigned major = ((x)>>24) & 0x1F;
  return (major > 0) && (major < 8);
}

static inline int
k1c_is_nop_opcode(uint32_t x)
{
  return ((x)<<1) == 0xFFFFFFFE;
}

typedef struct {
   uint32_t syllables[K1MAXSYLLABLES];
   int len;
} insn_t;


static uint32_t bundle_words[K1MAXBUNDLEWORDS];

static insn_t bundle_insn[K1MAXBUNDLEISSUE];

struct instr_s {
  int valid;
  int opcode;
  int immx[2];
  int immx_valid[2];
  int immx_count;
  int nb_syllables;
};

static int
k1c_reassemble_bundle(int wordcount, int *_insncount) {

  // Debugging flag
  int debug = 0;

  // available resources
  int bcu_taken = 0;
  int tca_taken = 0;
  int alu0_taken = 0;
  int alu1_taken = 0;
  int mau_taken = 0;
  int lsu_taken = 0;

  int i, j;

  struct instr_s instr[K1MAXBUNDLEISSUE];
  assert(K1MAXBUNDLEISSUE >= BundleIssue__);
  memset(instr, 0, sizeof(instr));

  if(debug) fprintf(stderr,"k1c_reassemble_bundle: wordcount = %d\n",wordcount);

  if(wordcount == 0) {
    if(debug) fprintf(stderr,"wordcount == 0\n");
    return 1;
  }

  for (i = 0; i < wordcount ; i++) {
    uint32_t syllable = bundle_words[i];
    switch (k1c_steering(syllable)) {
      
    case Steering_BCU:
      // BCU or TCA instruction
      if (i == 0) {
        if (k1c_is_tca_opcode(syllable)) {
          if (tca_taken) {
            if(debug) fprintf(stderr,"Too many TCA instructions");
            return 1;
          }
          if(debug) fprintf(stderr,"Syllable 0: Set valid on TCA for instr %d with 0x%x\n",BundleIssue_TCA,syllable);
          instr[BundleIssue_TCA].valid = 1;
          instr[BundleIssue_TCA].opcode = syllable;
          instr[BundleIssue_TCA].nb_syllables = 1;
          tca_taken = 1;
        } else {
          if(debug) fprintf(stderr,"Syllable 0: Set valid on BCU for instr %d with 0x%x\n",BundleIssue_BCU,syllable);
          instr[BundleIssue_BCU].valid = 1;
          instr[BundleIssue_BCU].opcode = syllable;
          instr[BundleIssue_BCU].nb_syllables = 1;
          bcu_taken = 1;
        }
      } else {
        if (i == 1 && bcu_taken && k1c_is_tca_opcode(syllable)) {
          if (tca_taken) {
            if(debug) fprintf(stderr,"Too many TCA instructions");
            return 1;
          }
          if(debug) fprintf(stderr,"Syllable 0: Set valid on TCA for instr %d with 0x%x\n",BundleIssue_TCA,syllable);
          instr[BundleIssue_TCA].valid = 1;
          instr[BundleIssue_TCA].opcode = syllable;
          instr[BundleIssue_TCA].nb_syllables = 1;
          tca_taken = 1;
        } else {
          // Not first syllable in bundle, IMMX
          struct instr_s *instr_p = &(instr[Extension_BundleIssue[k1c_extension(syllable)]]);
          int immx_count = instr_p->immx_count;
          if (immx_count > 1) {
            if(debug) fprintf(stderr,"Too many IMMX syllables");
            return 1;
          }
          instr_p->immx[immx_count] = syllable;
          instr_p->immx_valid[immx_count] = 1;
          instr_p->nb_syllables++;
          if(debug) fprintf(stderr,"Set IMMX[%d] on instr %d for extension %d @ %d\n",
                            immx_count, Extension_BundleIssue[k1c_extension(syllable)],k1c_extension(syllable),i);
          instr_p->immx_count = immx_count + 1;
        }
      }
      break;
      
    case Steering_ALU:
      if (alu0_taken == 0) {
        if(debug) fprintf(stderr,"Set valid on ALU0 for instr %d with 0x%x\n",BundleIssue_ALU0,syllable);
        instr[BundleIssue_ALU0].valid = 1;
        instr[BundleIssue_ALU0].opcode = syllable;
        instr[BundleIssue_ALU0].nb_syllables = 1;
        alu0_taken = 1;
      } else if (alu1_taken == 0) {
        if(debug) fprintf(stderr,"Set valid on ALU1 for instr %d with 0x%x\n",BundleIssue_ALU1,syllable);
        instr[BundleIssue_ALU1].valid = 1;
        instr[BundleIssue_ALU1].opcode = syllable;
        instr[BundleIssue_ALU1].nb_syllables = 1;
        alu1_taken = 1;
      } else if (mau_taken == 0) {
        if(debug) fprintf(stderr,"Set valid on MAU (ALU) for instr %d with 0x%x\n",BundleIssue_MAU,syllable);
        instr[BundleIssue_MAU].valid = 1;
        instr[BundleIssue_MAU].opcode = syllable;
        instr[BundleIssue_MAU].nb_syllables = 1;
        mau_taken = 1;
      } else if (lsu_taken == 0) {
        if(debug) fprintf(stderr,"Set valid on LSU (ALU) for instr %d with 0x%x\n",BundleIssue_LSU,syllable);
        instr[BundleIssue_LSU].valid = 1;
        instr[BundleIssue_LSU].opcode = syllable;
        instr[BundleIssue_LSU].nb_syllables = 1;
        lsu_taken = 1;
      } else if (k1c_is_nop_opcode(syllable)) {
        if(debug) fprintf(stderr,"Ignoring NOP (ALU) syllable\n");
      } else {
        if(debug) fprintf(stderr,"Too many ALU instructions");
        return 1;
      }
      break;
      
    case Steering_MAU:
      if (mau_taken == 1) {
        if(debug) fprintf(stderr,"Too many MAU instructions");
        return 1;
      } else {
        if(debug) fprintf(stderr,"Set valid on MAU for instr %d with 0x%x\n",BundleIssue_MAU,syllable);
        instr[BundleIssue_MAU].valid = 1;
        instr[BundleIssue_MAU].opcode = syllable;
        instr[BundleIssue_MAU].nb_syllables = 1;
        mau_taken = 1;
      }
      break;
      
    case Steering_LSU:
      if (lsu_taken == 1) {
        if(debug) fprintf(stderr,"Too many LSU instructions");
        return 1;
      } else {
        if(debug) fprintf(stderr,"Set valid on LSU for instr %d with 0x%x\n",BundleIssue_LSU,syllable);
        instr[BundleIssue_LSU].valid = 1;
        instr[BundleIssue_LSU].opcode = syllable;
        instr[BundleIssue_LSU].nb_syllables = 1;
        lsu_taken = 1;
      }
    }
    if (!(k1c_has_parallel_bit(syllable))) {
      if(debug) fprintf(stderr,"Stop! stop bit is set 0x%x\n",syllable);
      break;
    }
    if(debug) fprintf(stderr,"Continue %d < %d?\n",i,wordcount);
    
  }
  if (k1c_has_parallel_bit(bundle_words[i])) {
      if(debug) fprintf(stderr,"bundle exceeds maximum size");
      return 1;
  }

  // Fill bundle_insn and count read syllables
  int instr_idx = 0;
  for (i = 0; i < K1MAXBUNDLEISSUE; i++) {
    if (instr[i].valid == 1) {
      int syllable_idx = 0;

      // First copy opcode
      bundle_insn[instr_idx].syllables[syllable_idx++] = instr[i].opcode;
      bundle_insn[instr_idx].len = 1;

      for(j=0; j < 2; j++) {
        if(instr[i].immx_valid[j]) {
          if(debug) fprintf(stderr,"Instr %d valid immx[%d] is valid\n",i,j);	
          bundle_insn[instr_idx].syllables[syllable_idx++] = instr[i].immx[j];
          bundle_insn[instr_idx].len++;
        }
      }

      if(debug) fprintf(stderr,"Instr %d valid, copying in bundle_insn (%d syllables <-> %d)\n",i,bundle_insn[instr_idx].len,instr[i].nb_syllables);
      instr_idx++;
    }
  }

  if(debug) fprintf(stderr,"End => %d instructions\n",instr_idx);

  *_insncount = instr_idx;
  return 0;
}


int print_insn_k1 (bfd_vma memaddr, struct disassemble_info *info){
  static int insnindex = 0;
  static int insncount = 0;
  k1opc_t *op = NULL;             /* operation table index */
  insn_t *insn;                   /* the instruction       */
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
  int i;

  /* check that tables are initialized */

  if (info->arch != bfd_arch_k1) {
      fprintf(stderr, "error: Unknown architecture\n");
      exit(-1);
  }

  switch (info->mach) {

    case bfd_mach_k1c_k1c_64:
      k1_arch_size = 64;
    case bfd_mach_k1c_k1c_usr:
    case bfd_mach_k1c_k1c:
      opc_table = k1c_k1optab;
      k1_regfiles = k1_k1c_regfiles;
      k1_registers = k1_k1c_registers;
      k1_dec_registers = k1_k1c_dec_registers;
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
  if(insnindex == 0){
      int wordcount = 0;
      do{
          int status;
          assert(wordcount < K1MAXBUNDLEWORDS);
          status = (*info->read_memory_func) (memaddr + 4*wordcount, (bfd_byte*)(bundle_words + wordcount), 4, info);
          if (status != 0){
              (*info->memory_error_func) (status, memaddr + 4*wordcount, info);
              return -1;
          }
          wordcount++;
      } while (k1c_parallel_fld(bundle_words[wordcount-1]) && wordcount < K1MAXBUNDLEWORDS);
      invalid_bundle = k1c_reassemble_bundle(wordcount, &insncount);
  }
  assert(insnindex < K1MAXBUNDLEISSUE);
  insn = &(bundle_insn[insnindex]);
  readsofar = insn->len * 4;
  insnindex++;

  /* Check for extension to right iff this is not the end of bundle */
  for (op = opc_table; op->as_op && (((char)op->as_op[0]) != 0); op++){  /* find the format of this insn */
      int opcode_match = 1;
      int ch;

      if(invalid_bundle){
          break;
      }

      if(op->wordcount != insn->len){
          continue;
      }

      for(i=0; i < op->wordcount; i++) {
        if ((op->codewords[i].mask & insn->syllables[i]) != op->codewords[i].opcode) {
          opcode_match = 0;
        }
      }
      int encoding_space_flags = k1_arch_size == 32 ? k1OPCODE_FLAG_MODE32 : k1OPCODE_FLAG_MODE64;

      for(i=0; i < op->wordcount; i++) {
        if (! (op->codewords[i].flags & encoding_space_flags))
          opcode_match = 0;
      }

      if (opcode_match) {
          /* print the operands using the instructions format string. */
          fmtp = op->fmtstring;
          // If the user wants "pretty printing", ie, not the usual little endian objdump output
          if(opt_pretty){
              (*info->fprintf_func) (info->stream, "[ ");
              for(i = 0; i < insn->len; i++){
                  (*info->fprintf_func) (info->stream, "%08x ", insn->syllables[i]);
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
              int shift = op->format[i]->shift;
              int bias = op->format[i]->bias;
              unsigned long long value = 0;
              int bf_idx;


              /* Print characters in the format string up to the following % or nul. */
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
                  unsigned long long encoded_value = insn->syllables[insn_idx] >> to_offset;
                  encoded_value &= (1LL << bf[bf_idx].size) - 1;
                  value |= encoded_value << bf[bf_idx].from_offset;
              }
              if (flags & k1SIGNED){
                  unsigned long long signbit = 1LL << (width -1);
                  value = (value ^ signbit) - signbit;
              }
              value = (value << shift) + bias;

#define K1_PRINT_REG(regfile,value) \
    if(k1_regfiles[regfile]+value < k1_max_dec_registers) { \
        (*info->fprintf_func) (info->stream, "%s", k1_registers[k1_dec_registers[k1_regfiles[regfile]+value]].name); \
    } else { \
        (*info->fprintf_func) (info->stream, "$??"); \
    }

              switch (type) {

                  case RegClass_k1c_singleReg:
                      K1_PRINT_REG(K1_REGFILE_DEC_GRF,value)
                      break;
                  case RegClass_k1c_pairedReg:
                      K1_PRINT_REG(K1_REGFILE_DEC_PRF,value)
                      break;
                  case RegClass_k1c_quadReg:
                      K1_PRINT_REG(K1_REGFILE_DEC_QRF,value)
                      break;
                  case RegClass_k1c_systemReg:
                  case RegClass_k1c_nopcpsReg:
                  case RegClass_k1c_onlypsReg:
                  case RegClass_k1c_onlyraReg:
                  case RegClass_k1c_onlyfxReg:
                      K1_PRINT_REG(K1_REGFILE_DEC_SRF,value)
                      break;
                  case RegClass_k1c_coproReg:
                      K1_PRINT_REG(K1_REGFILE_DEC_CRF,value)
                      break;
                  case RegClass_k1c_blockReg:
                  case RegClass_k1c_blockRegE:
                  case RegClass_k1c_blockRegO:
                      K1_PRINT_REG(K1_REGFILE_DEC_BRF,value)
                      break;
                  case RegClass_k1c_accelReg:
                  case RegClass_k1c_accelRegE:
                  case RegClass_k1c_accelRegO:
                  case RegClass_k1c_vectorReg_0:
                  case RegClass_k1c_vectorReg_1:
                      K1_PRINT_REG(K1_REGFILE_DEC_ARF,value)
                      break;
                  case RegClass_k1c_vectorReg:
                      K1_PRINT_REG(K1_REGFILE_DEC_VRF,value)
                      break;

                  case Immediate_k1c_sysnumber:
                  case Immediate_k1c_signed10:
                  case Immediate_k1c_signed16:
                  case Immediate_k1c_signed27:
                  case Immediate_k1c_wrapped32:
                  case Immediate_k1c_signed37:
                  case Immediate_k1c_signed43:
                  case Immediate_k1c_signed54:
                  case Immediate_k1c_wrapped64:
                  case Immediate_k1c_unsigned5:
                  case Immediate_k1c_unsigned6:
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
                      (*info->print_address_func)(value + memaddr, info);
                      break;
                  default:
                      fprintf(stderr, "error: unexpected operand type (%s)\n", type_name);
                      exit(-1);
              };

#undef K1_PRINT_REG     
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

  if (found && (insnindex == insncount)){
      (*info->fprintf_func) (info->stream, ";;\n");
      insnindex = 0;
  }
  // couldn't find the opcode, skip this word
  if(!found){
      (*info->fprintf_func) (info->stream, "*** invalid opcode ***\n");
      insnindex = 0;
      readsofar = 4;
  }
  return readsofar;
}

void print_k1_disassembler_options(FILE *stream){
    fprintf(stream, "\nThe following K1 specific disassembler options are supported for use\nwith the -M switch (multiple options should be separated by commas):\n");
    fprintf(stream, "\npretty             Print 32-bit words in natural order corresponding to re-ordered instruction.\n\n");
}
