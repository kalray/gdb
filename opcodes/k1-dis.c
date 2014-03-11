#include "sysdep.h"
#define STATIC_TABLE
#define DEFINE_TABLE

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include "elf/k1.h"
#include "opcode/k1.h"
#include "dis-asm.h"

#define MAXBUNDLESIZE 8
#define ISSUES 5
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

/* FIXME: Why was it declared with ISSUES size when indexed
   by a variable that can count up to MAXBUNDLESIZE?
static bundle_t bundle_insn[ISSUES];
*/
static bundle_t bundle_insn[MAXBUNDLESIZE];

static int reassemble_bundle(unsigned int *opcnt) {
    unsigned int i;
    unsigned int insncnt = 0;
    unsigned int immxcnt = 0;
    int seen[4];
    int real_alu = 0;
    unsigned int bundle_immx[MAXIMMX];
    unsigned int opxd_mask   = 0x7803e000;
    unsigned int opxd_opcode = 0x7803e000;
    int has_opxd = 0;

    for(i=0; i < 4; i++){
        seen[i] = 0;
    }

    // First separate instructions and immx
    for(i=0; i < *opcnt; i++){
        if((GETSTEERING(bundle_ops[i]) == BCU_STEER) && (i > 0) && (immxcnt < MAXIMMX)){ // op has BCU steering and is not in slot 0 => IMMX
            bundle_immx[immxcnt] = bundle_ops[i];
            immxcnt++;
        } else {
            if((bundle_ops[i] & opxd_mask) == opxd_opcode){
                if (insncnt == 0) /*something is wrong, probably we're in data section */
                  return 1;
                has_opxd = 1;
                bundle_insn[insncnt - 1].insn[1] = bundle_ops[i];
                bundle_insn[insncnt - 1].len = 2;
            } else {
                bundle_insn[insncnt].insn[0] = bundle_ops[i];
                bundle_insn[insncnt].len = 1;
                insncnt++;
            }
            seen[GETSTEERING(bundle_ops[i])]++;
        }
        if((bundle_ops[i] & 0x80000000) == 0){
            break; // No more // bit, reached the end.
        }
    }

    // Do some check. Useful if trying to disass data section, where bundles have no meaning
    if(  (seen[BCU_STEER] > (MAXIMMX + 1))  // One BCU and MAXIMMX is the maximum BCU_STEER authorized
       ||(seen[ALU_STEER] > 4)              // 2 ALU + 2 LITE
       || (seen[MAU_STEER] > 1)             // 1 MAU
       || (seen[LSU_STEER] > 1)){          // 1 LSU
        return 1;
    }

    real_alu = (seen[ALU_STEER] > 2) ? 2 : seen[ALU_STEER]; // See how many real ALU are used
    real_alu -= has_opxd;

    // Get immx with their insn
    for(i=0; i < immxcnt; i++){
        switch(GETEXU(bundle_immx[i])){ // Get steering bit
            case ALU0_EXU:
                bundle_insn[seen[BCU_STEER]].insn[1 + has_opxd] = bundle_immx[i]; // If opxd, first slot is already taken by OPXD opcode
                bundle_insn[seen[BCU_STEER]].len = 2 + has_opxd;
                break;
            case ALU1_EXU:
                if(has_opxd){ // 128 insn for 64 bit immx
                    bundle_insn[seen[BCU_STEER]].insn[3] = bundle_immx[i];
                    bundle_insn[seen[BCU_STEER]].len = 4;
                } else {
                    bundle_insn[seen[BCU_STEER] + 1].insn[1] = bundle_immx[i];
                    bundle_insn[seen[BCU_STEER] + 1].len = 2;
                }
                break;
            case LSU_EXU:
                if(seen[LSU_STEER]){
                    // We know there is a LSU insn : find it
                    bundle_insn[seen[BCU_STEER] + real_alu + seen[MAU_STEER]].insn[1] = bundle_immx[i];
                    bundle_insn[seen[BCU_STEER] + real_alu + seen[MAU_STEER]].len = 2;
                } else { // LITE insn on LSU has to be the last ALU insn
                    bundle_insn[seen[BCU_STEER] + seen[ALU_STEER] - 1 - has_opxd + seen[MAU_STEER]].insn[1] = bundle_immx[i];
                    bundle_insn[seen[BCU_STEER] + seen[ALU_STEER] - 1 - has_opxd + seen[MAU_STEER]].len = 2;
                }
                break;
            case MAU_EXU:
                if(seen[MAU_STEER]){
                    bundle_insn[seen[BCU_STEER] + real_alu].insn[1] = bundle_immx[i];
                    bundle_insn[seen[BCU_STEER] + real_alu].len = 2;
                } else { // LITE insn on MAU : 2 ALU are full, and insn is right after LSU (if present)
                    bundle_insn[seen[BCU_STEER] + real_alu + seen[LSU_STEER]].insn[1] = bundle_immx[i];
                    bundle_insn[seen[BCU_STEER] + real_alu + seen[LSU_STEER]].len = 2;
                }
                break;
        }
    }

    *opcnt = insncnt;
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
  unsigned int  k1_max_registers = 0;
  unsigned int  k1_max_dec_registers = 0;
  int readsofar = 0;
  int opt_pretty = 0;
  int found = 0;
  int invalid_bundle = 0;
  
  /* check that tables are initialized */

  if (info->arch != bfd_arch_k1) {
      fprintf(stderr, "error: Unknown architecture\n");
      exit(-1);
  }

  switch (info->mach) {
    case bfd_mach_k1dp:
      opc_table = k1dp_k1optab;
      k1_regfiles = k1_k1dp_regfiles;
      k1_registers = k1_k1dp_registers;
      k1_dec_registers = k1_k1dp_dec_registers;
      break;
    case bfd_mach_k1io:
      opc_table = k1io_k1optab;
      k1_regfiles = k1_k1io_regfiles;
      k1_registers = k1_k1io_registers;
      k1_dec_registers = k1_k1io_dec_registers;
      break;
	case bfd_mach_k1bdp:
	  opc_table = k1bdp_k1optab;
	  k1_regfiles = k1_k1bdp_regfiles;
	  k1_registers = k1_k1bdp_registers;
	  k1_dec_registers = k1_k1bdp_dec_registers;
	  break;
	case bfd_mach_k1bio:
	  opc_table = k1bio_k1optab;
	  k1_regfiles = k1_k1bio_regfiles;
	  k1_registers = k1_k1bio_registers;
	  k1_dec_registers = k1_k1bio_dec_registers;
	  break;
    default:
      /* Core not supported */
      (*info->fprintf_func)(info->stream, "disassembling not supported for this K1 core! (core:%d)",
			    info->mach);
      return -1;
  }

  k1_max_registers = k1_regfiles[K1_REGFILE_REGISTERS];
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
      } while (k1_parallel_fld(bundle_ops[opcnt-1]) && opcnt < MAXBUNDLESIZE);
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

      if (opcode_match){
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

#define SRF_REGCLASSES(core)                           \
			case RegClass_ ## core ## _systemReg:      \
			case RegClass_ ## core ##_nopcpsReg:       \
			case RegClass_ ## core ## _onlypsReg:      \
			case RegClass_ ## core ## _onlyraReg:      \
			case RegClass_ ## core ## _onlyfxReg:

              switch (type) {
                  case RegClass_k1_singleReg:
 		      K1_PRINT_REG(K1_REGFILE_DEC_GRF,value)
                      break;
                  case RegClass_k1_pairedReg:
 		      K1_PRINT_REG(K1_REGFILE_DEC_PRF,value)
                      break;
			  SRF_REGCLASSES(k1)
#ifdef K1B_UNMERGED
				SRF_REGCLASSES(k1bdp)
				SRF_REGCLASSES(k1bio)
#endif//K1B_UNMERGED
 		      K1_PRINT_REG(K1_REGFILE_DEC_SRF,value)
                      break;
                  case RegClass_k1_remoteReg:
 		      K1_PRINT_REG(K1_REGFILE_DEC_NRF,value)
                      break;

                  case Immediate_k1_signed32:
                  case Immediate_k1_signed32M:
                  case Immediate_k1_signed5M:
                  case Immediate_k1_unsigned32L:
                  case Immediate_k1_unsigned32:
                  case Immediate_k1_extension22:
                  case Immediate_k1_eventmask2:
                  case Immediate_k1_flagmask2:
                  case Immediate_k1_signed10:
                  case Immediate_k1_signed16:
                  case Immediate_k1_signed27:
                  case Immediate_k1_brknumber:
                  case Immediate_k1_sysnumber:
                  case Immediate_k1_unsigned5:
                  case Immediate_k1_unsigned6:
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
                  case Immediate_k1_pcrel18:
                  case Immediate_k1_pcoff17:
                  case Immediate_k1_pcrel27:
                      (*info->print_address_func)((value << rightshift) + memaddr, info);
                      break;
                  default:
                      fprintf(stderr, "error: unexpected operand type (%s)\n", type_name);
                      exit(-1);
              };

#undef K1_PRINT_REG     
#undef SRF_REGCLASSES
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
