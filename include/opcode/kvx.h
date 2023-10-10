/* KVX assembler/disassembler support.

   Copyright (C) 2009-2023 Free Software Foundation, Inc.
   Contributed by Kalray SA.

   This file is part of GNU Binutils.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the license, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING3. If not,
   see <http://www.gnu.org/licenses/>.  */


#ifndef OPCODE_KVX_H
#define OPCODE_KVX_H

#define KVXMAXSYLLABLES 3
#define KVXMAXOPERANDS 7
#define KVXMAXBUNDLEISSUE 6
#define KVXMAXBUNDLEWORDS 8
#define KVXNUMCORES 3
#define KVXNUMBUNDLINGS 19


/*
 * The following macros are provided for compatibility with old
 * code.  They should not be used in new code.
 */


/***********************************************/
/*       DATA TYPES                            */
/***********************************************/

/*  Operand definition -- used in building     */
/*  format table                               */

enum kvx_rel {
  /* Absolute relocation. */
  KVX_REL_ABS,
  /* PC relative relocation. */
  KVX_REL_PC,
  /* GP relative relocation. */
  KVX_REL_GP,
  /* TP relative relocation. */
  KVX_REL_TP,
  /* GOT relative relocation. */
  KVX_REL_GOT,
  /* BASE load address relative relocation. */
  KVX_REL_BASE,
};

struct kvx_reloc {
  /* Size in bits. */
  int bitsize;
  /* Type of relative relocation. */
  enum kvx_rel relative;
  /* Number of BFD relocations. */
  int reloc_nb;
  /* List of BFD relocations. */
  unsigned int relocs[];
};

struct kvx_bitfield {
  /* Number of bits.  */
  int size;
  /* Offset in abstract value.  */
  int from_offset;
  /* Offset in encoded value.  */
  int to_offset;
};

struct kvx_operand {
  /* Operand type name.  */
  const char *tname;
  /* Type of operand.  */
  int type;
  /* Width of the operand. */
  int width;
  /* Encoded value shift. */
  int shift;
  /* Encoded value bias.  */
  int bias;
  /* Can be SIGNED|CANEXTEND|BITMASK|WRAPPED.  */
  int flags;
  /* Number of registers.  */
  int reg_nb;
  /* Valid registers for this operand (if no register get null pointer).  */
  int *regs;
  /* Number of relocations.  */
  int reloc_nb;
  /* List of relocations that can be applied to this operand.  */
  struct kvx_reloc **relocs;
  /* Number of given bitfields.  */
  int bitfields;
  /* Bitfields in most to least significant order.  */
  struct kvx_bitfield bfield[];
};

struct kvx_pseudo_relocs
{
  enum
  {
    S32_LO5_UP27,
    S37_LO10_UP27,
    S43_LO10_UP27_EX6,
    S64_LO10_UP27_EX27,
    S16,
    S32,
    S64,
  } reloc_type;

  int bitsize;

  /* Used when pseudo func should expand to different relocations
     based on the 32/64 bits mode.
     Enum values should match the kvx_arch_size var set by -m32
   */
  enum
  {
    PSEUDO_ALL = 0,
    PSEUDO_32_ONLY = 32,
    PSEUDO_64_ONLY = 64,
  } avail_modes;

  /* set to 1 when pseudo func does not take an argument */
  int has_no_arg;

  bfd_reloc_code_real_type reloc_lo5, reloc_lo10, reloc_up27, reloc_ex;
  bfd_reloc_code_real_type single;
  struct kvx_reloc *kreloc;
};

typedef struct symbol symbolS;

struct pseudo_func
{
  const char *name;

  symbolS *sym;
  struct kvx_pseudo_relocs pseudo_relocs;
};

/* some flags for kvx_operand                                 */
/* kvxSIGNED    : is this operand treated as signed ?         */
/* kvxCANEXTEND : can this operand have an extension          */
/* kvxBITMASK   : this operand is a bit mask */
/* kvxWRAPPED   : this operand can accept signed and unsigned integer ranges */


#define kvxSIGNED    1
#define kvxCANEXTEND 2
#define kvxBITMASK   4
#define kvxWRAPPED   8

#define kvxOPCODE_FLAG_UNDEF 0

#define kvxOPCODE_FLAG_IMMX0 1
#define kvxOPCODE_FLAG_IMMX1 2
#define kvxOPCODE_FLAG_BCU 4
#define kvxOPCODE_FLAG_ALU 8
#define kvxOPCODE_FLAG_LSU 16
#define kvxOPCODE_FLAG_MAU 32
#define kvxOPCODE_FLAG_MODE64 64
#define kvxOPCODE_FLAG_MODE32 128
/* Opcode definition.  */

struct kvx_codeword {
  /* The opcode.  */
  unsigned opcode;
  /* Disassembly mask.  */
  unsigned mask;
  /* Target dependent flags.  */
  unsigned flags;
};

struct kvxopc {
  /* asm name */
  const char  *as_op;
  /* 32 bits code words. */
  struct kvx_codeword codewords[KVXMAXSYLLABLES];
  /* Number of words in codewords[].  */
  int wordcount;
  /* coding size in case of variable length.  */
  unsigned coding_size;
  /* Bundling class.  */
  int bundling;
  /* Reservation class.  */
  int reservation;
  /* 0 terminated.  */
  struct kvx_operand *format[KVXMAXOPERANDS + 1];
  /* Resource class.  */
  const char *rclass;
  /* Formating string.  */
  const char *fmtstring;
};

struct kvx_core_info {
  struct kvxopc *optab;
  const char *name;
  const int *resources;
  int elf_core;
  struct pseudo_func *pseudo_funcs;
  int nb_pseudo_funcs;
  int **reservation_table_table;
  int reservation_table_lines;
  int resource_max;
  char **resource_names;
};

struct kvx_Register {
  int id;
  const char *name;
};

extern const int kvx_kv3_v1_reservation_table_lines;
extern const int *kvx_kv3_v1_reservation_table_table[];
extern const char *kvx_kv3_v1_resource_names[];

extern const int kvx_kv3_v1_resources[];
extern struct kvxopc kvx_kv3_v1_optab[];
extern const struct kvx_core_info kvx_kv3_v1_core_info;
extern const int kvx_kv3_v2_reservation_table_lines;
extern const int *kvx_kv3_v2_reservation_table_table[];
extern const char *kvx_kv3_v2_resource_names[];

extern const int kvx_kv3_v2_resources[];
extern struct kvxopc kvx_kv3_v2_optab[];
extern const struct kvx_core_info kvx_kv3_v2_core_info;
extern const int kvx_kv4_v1_reservation_table_lines;
extern const int *kvx_kv4_v1_reservation_table_table[];
extern const char *kvx_kv4_v1_resource_names[];

extern const int kvx_kv4_v1_resources[];
extern struct kvxopc kvx_kv4_v1_optab[];
extern const struct kvx_core_info kvx_kv4_v1_core_info;
extern const struct kvx_core_info *kvx_core_info_table[];
extern const char ***kvx_modifiers_table[];
extern const struct kvx_Register *kvx_registers_table[];
extern const int *kvx_regfiles_table[];

#define KVX_REGFILE_FIRST_GPR 0
#define KVX_REGFILE_LAST_GPR 1
#define KVX_REGFILE_DEC_GPR 2
#define KVX_REGFILE_FIRST_PGR 3
#define KVX_REGFILE_LAST_PGR 4
#define KVX_REGFILE_DEC_PGR 5
#define KVX_REGFILE_FIRST_QGR 6
#define KVX_REGFILE_LAST_QGR 7
#define KVX_REGFILE_DEC_QGR 8
#define KVX_REGFILE_FIRST_SFR 9
#define KVX_REGFILE_LAST_SFR 10
#define KVX_REGFILE_DEC_SFR 11
#define KVX_REGFILE_FIRST_X16R 12
#define KVX_REGFILE_LAST_X16R 13
#define KVX_REGFILE_DEC_X16R 14
#define KVX_REGFILE_FIRST_X2R 15
#define KVX_REGFILE_LAST_X2R 16
#define KVX_REGFILE_DEC_X2R 17
#define KVX_REGFILE_FIRST_X32R 18
#define KVX_REGFILE_LAST_X32R 19
#define KVX_REGFILE_DEC_X32R 20
#define KVX_REGFILE_FIRST_X4R 21
#define KVX_REGFILE_LAST_X4R 22
#define KVX_REGFILE_DEC_X4R 23
#define KVX_REGFILE_FIRST_X64R 24
#define KVX_REGFILE_LAST_X64R 25
#define KVX_REGFILE_DEC_X64R 26
#define KVX_REGFILE_FIRST_X8R 27
#define KVX_REGFILE_LAST_X8R 28
#define KVX_REGFILE_DEC_X8R 29
#define KVX_REGFILE_FIRST_XBR 30
#define KVX_REGFILE_LAST_XBR 31
#define KVX_REGFILE_DEC_XBR 32
#define KVX_REGFILE_FIRST_XCR 33
#define KVX_REGFILE_LAST_XCR 34
#define KVX_REGFILE_DEC_XCR 35
#define KVX_REGFILE_FIRST_XMR 36
#define KVX_REGFILE_LAST_XMR 37
#define KVX_REGFILE_DEC_XMR 38
#define KVX_REGFILE_FIRST_XTR 39
#define KVX_REGFILE_LAST_XTR 40
#define KVX_REGFILE_DEC_XTR 41
#define KVX_REGFILE_FIRST_XVR 42
#define KVX_REGFILE_LAST_XVR 43
#define KVX_REGFILE_DEC_XVR 44
#define KVX_REGFILE_REGISTERS 45
#define KVX_REGFILE_DEC_REGISTERS 46


extern int kvx_kv3_v1_regfiles[];
extern const char **kvx_kv3_v1_modifiers[];
extern struct kvx_Register kvx_kv3_v1_registers[];

extern int kvx_kv3_v1_dec_registers[];

enum Method_kvx_kv3_v1_enum {
  Immediate_kv3_v1_pcrel17 = 1,
  Immediate_kv3_v1_pcrel27 = 2,
  Immediate_kv3_v1_signed10 = 3,
  Immediate_kv3_v1_signed16 = 4,
  Immediate_kv3_v1_signed27 = 5,
  Immediate_kv3_v1_signed37 = 6,
  Immediate_kv3_v1_signed43 = 7,
  Immediate_kv3_v1_signed54 = 8,
  Immediate_kv3_v1_sysnumber = 9,
  Immediate_kv3_v1_unsigned6 = 10,
  Immediate_kv3_v1_wrapped32 = 11,
  Immediate_kv3_v1_wrapped64 = 12,
  Modifier_kv3_v1_column = 13,
  Modifier_kv3_v1_comparison = 14,
  Modifier_kv3_v1_doscale = 15,
  Modifier_kv3_v1_exunum = 16,
  Modifier_kv3_v1_floatcomp = 17,
  Modifier_kv3_v1_qindex = 18,
  Modifier_kv3_v1_rectify = 19,
  Modifier_kv3_v1_rounding = 20,
  Modifier_kv3_v1_roundint = 21,
  Modifier_kv3_v1_saturate = 22,
  Modifier_kv3_v1_scalarcond = 23,
  Modifier_kv3_v1_silent = 24,
  Modifier_kv3_v1_simplecond = 25,
  Modifier_kv3_v1_speculate = 26,
  Modifier_kv3_v1_splat32 = 27,
  Modifier_kv3_v1_variant = 28,
  RegClass_kv3_v1_aloneReg = 29,
  RegClass_kv3_v1_blockReg = 30,
  RegClass_kv3_v1_blockReg0M4 = 31,
  RegClass_kv3_v1_blockReg1M4 = 32,
  RegClass_kv3_v1_blockReg2M4 = 33,
  RegClass_kv3_v1_blockReg3M4 = 34,
  RegClass_kv3_v1_blockRegE = 35,
  RegClass_kv3_v1_blockRegO = 36,
  RegClass_kv3_v1_blockReg_0 = 37,
  RegClass_kv3_v1_blockReg_1 = 38,
  RegClass_kv3_v1_buffer16Reg = 39,
  RegClass_kv3_v1_buffer2Reg = 40,
  RegClass_kv3_v1_buffer32Reg = 41,
  RegClass_kv3_v1_buffer4Reg = 42,
  RegClass_kv3_v1_buffer64Reg = 43,
  RegClass_kv3_v1_buffer8Reg = 44,
  RegClass_kv3_v1_coproReg = 45,
  RegClass_kv3_v1_coproReg0M4 = 46,
  RegClass_kv3_v1_coproReg1M4 = 47,
  RegClass_kv3_v1_coproReg2M4 = 48,
  RegClass_kv3_v1_coproReg3M4 = 49,
  RegClass_kv3_v1_matrixReg = 50,
  RegClass_kv3_v1_matrixReg_0 = 51,
  RegClass_kv3_v1_matrixReg_1 = 52,
  RegClass_kv3_v1_matrixReg_2 = 53,
  RegClass_kv3_v1_matrixReg_3 = 54,
  RegClass_kv3_v1_onlyfxReg = 55,
  RegClass_kv3_v1_onlygetReg = 56,
  RegClass_kv3_v1_onlyraReg = 57,
  RegClass_kv3_v1_onlysetReg = 58,
  RegClass_kv3_v1_onlyswapReg = 59,
  RegClass_kv3_v1_pairedReg = 60,
  RegClass_kv3_v1_pairedReg_0 = 61,
  RegClass_kv3_v1_pairedReg_1 = 62,
  RegClass_kv3_v1_quadReg = 63,
  RegClass_kv3_v1_quadReg_0 = 64,
  RegClass_kv3_v1_quadReg_1 = 65,
  RegClass_kv3_v1_quadReg_2 = 66,
  RegClass_kv3_v1_quadReg_3 = 67,
  RegClass_kv3_v1_singleReg = 68,
  RegClass_kv3_v1_systemReg = 69,
  RegClass_kv3_v1_tileReg = 70,
  RegClass_kv3_v1_tileReg_0 = 71,
  RegClass_kv3_v1_tileReg_1 = 72,
  RegClass_kv3_v1_vectorReg = 73,
  RegClass_kv3_v1_vectorRegE = 74,
  RegClass_kv3_v1_vectorRegO = 75,
  RegClass_kv3_v1_vectorReg_0 = 76,
  RegClass_kv3_v1_vectorReg_1 = 77,
  RegClass_kv3_v1_vectorReg_2 = 78,
  RegClass_kv3_v1_vectorReg_3 = 79,
  Instruction_kv3_v1_abdd = 80,
  Instruction_kv3_v1_abdhq = 81,
  Instruction_kv3_v1_abdw = 82,
  Instruction_kv3_v1_abdwp = 83,
  Instruction_kv3_v1_absd = 84,
  Instruction_kv3_v1_abshq = 85,
  Instruction_kv3_v1_absw = 86,
  Instruction_kv3_v1_abswp = 87,
  Instruction_kv3_v1_acswapd = 88,
  Instruction_kv3_v1_acswapw = 89,
  Instruction_kv3_v1_addcd = 90,
  Instruction_kv3_v1_addcd_i = 91,
  Instruction_kv3_v1_addd = 92,
  Instruction_kv3_v1_addhcp_c = 93,
  Instruction_kv3_v1_addhq = 94,
  Instruction_kv3_v1_addsd = 95,
  Instruction_kv3_v1_addshq = 96,
  Instruction_kv3_v1_addsw = 97,
  Instruction_kv3_v1_addswp = 98,
  Instruction_kv3_v1_adduwd = 99,
  Instruction_kv3_v1_addw = 100,
  Instruction_kv3_v1_addwc_c = 101,
  Instruction_kv3_v1_addwd = 102,
  Instruction_kv3_v1_addwp = 103,
  Instruction_kv3_v1_addx16d = 104,
  Instruction_kv3_v1_addx16hq = 105,
  Instruction_kv3_v1_addx16uwd = 106,
  Instruction_kv3_v1_addx16w = 107,
  Instruction_kv3_v1_addx16wd = 108,
  Instruction_kv3_v1_addx16wp = 109,
  Instruction_kv3_v1_addx2d = 110,
  Instruction_kv3_v1_addx2hq = 111,
  Instruction_kv3_v1_addx2uwd = 112,
  Instruction_kv3_v1_addx2w = 113,
  Instruction_kv3_v1_addx2wd = 114,
  Instruction_kv3_v1_addx2wp = 115,
  Instruction_kv3_v1_addx4d = 116,
  Instruction_kv3_v1_addx4hq = 117,
  Instruction_kv3_v1_addx4uwd = 118,
  Instruction_kv3_v1_addx4w = 119,
  Instruction_kv3_v1_addx4wd = 120,
  Instruction_kv3_v1_addx4wp = 121,
  Instruction_kv3_v1_addx8d = 122,
  Instruction_kv3_v1_addx8hq = 123,
  Instruction_kv3_v1_addx8uwd = 124,
  Instruction_kv3_v1_addx8w = 125,
  Instruction_kv3_v1_addx8wd = 126,
  Instruction_kv3_v1_addx8wp = 127,
  Instruction_kv3_v1_aladdd = 128,
  Instruction_kv3_v1_aladdw = 129,
  Instruction_kv3_v1_alclrd = 130,
  Instruction_kv3_v1_alclrw = 131,
  Instruction_kv3_v1_aligno = 132,
  Instruction_kv3_v1_alignv = 133,
  Instruction_kv3_v1_andd = 134,
  Instruction_kv3_v1_andnd = 135,
  Instruction_kv3_v1_andnw = 136,
  Instruction_kv3_v1_andw = 137,
  Instruction_kv3_v1_avghq = 138,
  Instruction_kv3_v1_avgrhq = 139,
  Instruction_kv3_v1_avgruhq = 140,
  Instruction_kv3_v1_avgruw = 141,
  Instruction_kv3_v1_avgruwp = 142,
  Instruction_kv3_v1_avgrw = 143,
  Instruction_kv3_v1_avgrwp = 144,
  Instruction_kv3_v1_avguhq = 145,
  Instruction_kv3_v1_avguw = 146,
  Instruction_kv3_v1_avguwp = 147,
  Instruction_kv3_v1_avgw = 148,
  Instruction_kv3_v1_avgwp = 149,
  Instruction_kv3_v1_await = 150,
  Instruction_kv3_v1_barrier = 151,
  Instruction_kv3_v1_call = 152,
  Instruction_kv3_v1_cb = 153,
  Instruction_kv3_v1_cbsd = 154,
  Instruction_kv3_v1_cbsw = 155,
  Instruction_kv3_v1_cbswp = 156,
  Instruction_kv3_v1_clrf = 157,
  Instruction_kv3_v1_clsd = 158,
  Instruction_kv3_v1_clsw = 159,
  Instruction_kv3_v1_clswp = 160,
  Instruction_kv3_v1_clzd = 161,
  Instruction_kv3_v1_clzw = 162,
  Instruction_kv3_v1_clzwp = 163,
  Instruction_kv3_v1_cmoved = 164,
  Instruction_kv3_v1_cmovehq = 165,
  Instruction_kv3_v1_cmovewp = 166,
  Instruction_kv3_v1_cmuldt = 167,
  Instruction_kv3_v1_cmulghxdt = 168,
  Instruction_kv3_v1_cmulglxdt = 169,
  Instruction_kv3_v1_cmulgmxdt = 170,
  Instruction_kv3_v1_cmulxdt = 171,
  Instruction_kv3_v1_compd = 172,
  Instruction_kv3_v1_compnhq = 173,
  Instruction_kv3_v1_compnwp = 174,
  Instruction_kv3_v1_compuwd = 175,
  Instruction_kv3_v1_compw = 176,
  Instruction_kv3_v1_compwd = 177,
  Instruction_kv3_v1_convdhv0 = 178,
  Instruction_kv3_v1_convdhv1 = 179,
  Instruction_kv3_v1_convwbv0 = 180,
  Instruction_kv3_v1_convwbv1 = 181,
  Instruction_kv3_v1_convwbv2 = 182,
  Instruction_kv3_v1_convwbv3 = 183,
  Instruction_kv3_v1_copyd = 184,
  Instruction_kv3_v1_copyo = 185,
  Instruction_kv3_v1_copyq = 186,
  Instruction_kv3_v1_copyw = 187,
  Instruction_kv3_v1_crcbellw = 188,
  Instruction_kv3_v1_crcbelmw = 189,
  Instruction_kv3_v1_crclellw = 190,
  Instruction_kv3_v1_crclelmw = 191,
  Instruction_kv3_v1_ctzd = 192,
  Instruction_kv3_v1_ctzw = 193,
  Instruction_kv3_v1_ctzwp = 194,
  Instruction_kv3_v1_d1inval = 195,
  Instruction_kv3_v1_dinvall = 196,
  Instruction_kv3_v1_dot2suwd = 197,
  Instruction_kv3_v1_dot2suwdp = 198,
  Instruction_kv3_v1_dot2uwd = 199,
  Instruction_kv3_v1_dot2uwdp = 200,
  Instruction_kv3_v1_dot2w = 201,
  Instruction_kv3_v1_dot2wd = 202,
  Instruction_kv3_v1_dot2wdp = 203,
  Instruction_kv3_v1_dot2wzp = 204,
  Instruction_kv3_v1_dtouchl = 205,
  Instruction_kv3_v1_dzerol = 206,
  Instruction_kv3_v1_errop = 207,
  Instruction_kv3_v1_extfs = 208,
  Instruction_kv3_v1_extfz = 209,
  Instruction_kv3_v1_fabsd = 210,
  Instruction_kv3_v1_fabshq = 211,
  Instruction_kv3_v1_fabsw = 212,
  Instruction_kv3_v1_fabswp = 213,
  Instruction_kv3_v1_faddd = 214,
  Instruction_kv3_v1_fadddc = 215,
  Instruction_kv3_v1_fadddc_c = 216,
  Instruction_kv3_v1_fadddp = 217,
  Instruction_kv3_v1_faddhq = 218,
  Instruction_kv3_v1_faddw = 219,
  Instruction_kv3_v1_faddwc = 220,
  Instruction_kv3_v1_faddwc_c = 221,
  Instruction_kv3_v1_faddwcp = 222,
  Instruction_kv3_v1_faddwcp_c = 223,
  Instruction_kv3_v1_faddwp = 224,
  Instruction_kv3_v1_faddwq = 225,
  Instruction_kv3_v1_fcdivd = 226,
  Instruction_kv3_v1_fcdivw = 227,
  Instruction_kv3_v1_fcdivwp = 228,
  Instruction_kv3_v1_fcompd = 229,
  Instruction_kv3_v1_fcompnhq = 230,
  Instruction_kv3_v1_fcompnwp = 231,
  Instruction_kv3_v1_fcompw = 232,
  Instruction_kv3_v1_fdot2w = 233,
  Instruction_kv3_v1_fdot2wd = 234,
  Instruction_kv3_v1_fdot2wdp = 235,
  Instruction_kv3_v1_fdot2wzp = 236,
  Instruction_kv3_v1_fence = 237,
  Instruction_kv3_v1_ffmad = 238,
  Instruction_kv3_v1_ffmahq = 239,
  Instruction_kv3_v1_ffmahw = 240,
  Instruction_kv3_v1_ffmahwq = 241,
  Instruction_kv3_v1_ffmaw = 242,
  Instruction_kv3_v1_ffmawd = 243,
  Instruction_kv3_v1_ffmawdp = 244,
  Instruction_kv3_v1_ffmawp = 245,
  Instruction_kv3_v1_ffmsd = 246,
  Instruction_kv3_v1_ffmshq = 247,
  Instruction_kv3_v1_ffmshw = 248,
  Instruction_kv3_v1_ffmshwq = 249,
  Instruction_kv3_v1_ffmsw = 250,
  Instruction_kv3_v1_ffmswd = 251,
  Instruction_kv3_v1_ffmswdp = 252,
  Instruction_kv3_v1_ffmswp = 253,
  Instruction_kv3_v1_fixedd = 254,
  Instruction_kv3_v1_fixedud = 255,
  Instruction_kv3_v1_fixeduw = 256,
  Instruction_kv3_v1_fixeduwp = 257,
  Instruction_kv3_v1_fixedw = 258,
  Instruction_kv3_v1_fixedwp = 259,
  Instruction_kv3_v1_floatd = 260,
  Instruction_kv3_v1_floatud = 261,
  Instruction_kv3_v1_floatuw = 262,
  Instruction_kv3_v1_floatuwp = 263,
  Instruction_kv3_v1_floatw = 264,
  Instruction_kv3_v1_floatwp = 265,
  Instruction_kv3_v1_fmaxd = 266,
  Instruction_kv3_v1_fmaxhq = 267,
  Instruction_kv3_v1_fmaxw = 268,
  Instruction_kv3_v1_fmaxwp = 269,
  Instruction_kv3_v1_fmind = 270,
  Instruction_kv3_v1_fminhq = 271,
  Instruction_kv3_v1_fminw = 272,
  Instruction_kv3_v1_fminwp = 273,
  Instruction_kv3_v1_fmm212w = 274,
  Instruction_kv3_v1_fmma212w = 275,
  Instruction_kv3_v1_fmma242hw0 = 276,
  Instruction_kv3_v1_fmma242hw1 = 277,
  Instruction_kv3_v1_fmma242hw2 = 278,
  Instruction_kv3_v1_fmma242hw3 = 279,
  Instruction_kv3_v1_fmms212w = 280,
  Instruction_kv3_v1_fmuld = 281,
  Instruction_kv3_v1_fmulhq = 282,
  Instruction_kv3_v1_fmulhw = 283,
  Instruction_kv3_v1_fmulhwq = 284,
  Instruction_kv3_v1_fmulw = 285,
  Instruction_kv3_v1_fmulwc = 286,
  Instruction_kv3_v1_fmulwc_c = 287,
  Instruction_kv3_v1_fmulwd = 288,
  Instruction_kv3_v1_fmulwdc = 289,
  Instruction_kv3_v1_fmulwdc_c = 290,
  Instruction_kv3_v1_fmulwdp = 291,
  Instruction_kv3_v1_fmulwp = 292,
  Instruction_kv3_v1_fmulwq = 293,
  Instruction_kv3_v1_fnarrow44wh = 294,
  Instruction_kv3_v1_fnarrowdw = 295,
  Instruction_kv3_v1_fnarrowdwp = 296,
  Instruction_kv3_v1_fnarrowwh = 297,
  Instruction_kv3_v1_fnarrowwhq = 298,
  Instruction_kv3_v1_fnegd = 299,
  Instruction_kv3_v1_fneghq = 300,
  Instruction_kv3_v1_fnegw = 301,
  Instruction_kv3_v1_fnegwp = 302,
  Instruction_kv3_v1_frecw = 303,
  Instruction_kv3_v1_frsrw = 304,
  Instruction_kv3_v1_fsbfd = 305,
  Instruction_kv3_v1_fsbfdc = 306,
  Instruction_kv3_v1_fsbfdc_c = 307,
  Instruction_kv3_v1_fsbfdp = 308,
  Instruction_kv3_v1_fsbfhq = 309,
  Instruction_kv3_v1_fsbfw = 310,
  Instruction_kv3_v1_fsbfwc = 311,
  Instruction_kv3_v1_fsbfwc_c = 312,
  Instruction_kv3_v1_fsbfwcp = 313,
  Instruction_kv3_v1_fsbfwcp_c = 314,
  Instruction_kv3_v1_fsbfwp = 315,
  Instruction_kv3_v1_fsbfwq = 316,
  Instruction_kv3_v1_fscalewv = 317,
  Instruction_kv3_v1_fsdivd = 318,
  Instruction_kv3_v1_fsdivw = 319,
  Instruction_kv3_v1_fsdivwp = 320,
  Instruction_kv3_v1_fsrecd = 321,
  Instruction_kv3_v1_fsrecw = 322,
  Instruction_kv3_v1_fsrecwp = 323,
  Instruction_kv3_v1_fsrsrd = 324,
  Instruction_kv3_v1_fsrsrw = 325,
  Instruction_kv3_v1_fsrsrwp = 326,
  Instruction_kv3_v1_fwidenlhw = 327,
  Instruction_kv3_v1_fwidenlhwp = 328,
  Instruction_kv3_v1_fwidenlwd = 329,
  Instruction_kv3_v1_fwidenmhw = 330,
  Instruction_kv3_v1_fwidenmhwp = 331,
  Instruction_kv3_v1_fwidenmwd = 332,
  Instruction_kv3_v1_get = 333,
  Instruction_kv3_v1_goto = 334,
  Instruction_kv3_v1_i1inval = 335,
  Instruction_kv3_v1_i1invals = 336,
  Instruction_kv3_v1_icall = 337,
  Instruction_kv3_v1_iget = 338,
  Instruction_kv3_v1_igoto = 339,
  Instruction_kv3_v1_insf = 340,
  Instruction_kv3_v1_iord = 341,
  Instruction_kv3_v1_iornd = 342,
  Instruction_kv3_v1_iornw = 343,
  Instruction_kv3_v1_iorw = 344,
  Instruction_kv3_v1_landd = 345,
  Instruction_kv3_v1_landhq = 346,
  Instruction_kv3_v1_landw = 347,
  Instruction_kv3_v1_landwp = 348,
  Instruction_kv3_v1_lbs = 349,
  Instruction_kv3_v1_lbz = 350,
  Instruction_kv3_v1_ld = 351,
  Instruction_kv3_v1_lhs = 352,
  Instruction_kv3_v1_lhz = 353,
  Instruction_kv3_v1_lnandd = 354,
  Instruction_kv3_v1_lnandhq = 355,
  Instruction_kv3_v1_lnandw = 356,
  Instruction_kv3_v1_lnandwp = 357,
  Instruction_kv3_v1_lnord = 358,
  Instruction_kv3_v1_lnorhq = 359,
  Instruction_kv3_v1_lnorw = 360,
  Instruction_kv3_v1_lnorwp = 361,
  Instruction_kv3_v1_lo = 362,
  Instruction_kv3_v1_loopdo = 363,
  Instruction_kv3_v1_lord = 364,
  Instruction_kv3_v1_lorhq = 365,
  Instruction_kv3_v1_lorw = 366,
  Instruction_kv3_v1_lorwp = 367,
  Instruction_kv3_v1_lq = 368,
  Instruction_kv3_v1_lws = 369,
  Instruction_kv3_v1_lwz = 370,
  Instruction_kv3_v1_maddd = 371,
  Instruction_kv3_v1_madddt = 372,
  Instruction_kv3_v1_maddhq = 373,
  Instruction_kv3_v1_maddhwq = 374,
  Instruction_kv3_v1_maddsudt = 375,
  Instruction_kv3_v1_maddsuhwq = 376,
  Instruction_kv3_v1_maddsuwd = 377,
  Instruction_kv3_v1_maddsuwdp = 378,
  Instruction_kv3_v1_maddudt = 379,
  Instruction_kv3_v1_madduhwq = 380,
  Instruction_kv3_v1_madduwd = 381,
  Instruction_kv3_v1_madduwdp = 382,
  Instruction_kv3_v1_madduzdt = 383,
  Instruction_kv3_v1_maddw = 384,
  Instruction_kv3_v1_maddwd = 385,
  Instruction_kv3_v1_maddwdp = 386,
  Instruction_kv3_v1_maddwp = 387,
  Instruction_kv3_v1_make = 388,
  Instruction_kv3_v1_maxd = 389,
  Instruction_kv3_v1_maxhq = 390,
  Instruction_kv3_v1_maxud = 391,
  Instruction_kv3_v1_maxuhq = 392,
  Instruction_kv3_v1_maxuw = 393,
  Instruction_kv3_v1_maxuwp = 394,
  Instruction_kv3_v1_maxw = 395,
  Instruction_kv3_v1_maxwp = 396,
  Instruction_kv3_v1_mind = 397,
  Instruction_kv3_v1_minhq = 398,
  Instruction_kv3_v1_minud = 399,
  Instruction_kv3_v1_minuhq = 400,
  Instruction_kv3_v1_minuw = 401,
  Instruction_kv3_v1_minuwp = 402,
  Instruction_kv3_v1_minw = 403,
  Instruction_kv3_v1_minwp = 404,
  Instruction_kv3_v1_mm212w = 405,
  Instruction_kv3_v1_mma212w = 406,
  Instruction_kv3_v1_mma444hbd0 = 407,
  Instruction_kv3_v1_mma444hbd1 = 408,
  Instruction_kv3_v1_mma444hd = 409,
  Instruction_kv3_v1_mma444suhbd0 = 410,
  Instruction_kv3_v1_mma444suhbd1 = 411,
  Instruction_kv3_v1_mma444suhd = 412,
  Instruction_kv3_v1_mma444uhbd0 = 413,
  Instruction_kv3_v1_mma444uhbd1 = 414,
  Instruction_kv3_v1_mma444uhd = 415,
  Instruction_kv3_v1_mma444ushbd0 = 416,
  Instruction_kv3_v1_mma444ushbd1 = 417,
  Instruction_kv3_v1_mma444ushd = 418,
  Instruction_kv3_v1_mms212w = 419,
  Instruction_kv3_v1_movetq = 420,
  Instruction_kv3_v1_msbfd = 421,
  Instruction_kv3_v1_msbfdt = 422,
  Instruction_kv3_v1_msbfhq = 423,
  Instruction_kv3_v1_msbfhwq = 424,
  Instruction_kv3_v1_msbfsudt = 425,
  Instruction_kv3_v1_msbfsuhwq = 426,
  Instruction_kv3_v1_msbfsuwd = 427,
  Instruction_kv3_v1_msbfsuwdp = 428,
  Instruction_kv3_v1_msbfudt = 429,
  Instruction_kv3_v1_msbfuhwq = 430,
  Instruction_kv3_v1_msbfuwd = 431,
  Instruction_kv3_v1_msbfuwdp = 432,
  Instruction_kv3_v1_msbfuzdt = 433,
  Instruction_kv3_v1_msbfw = 434,
  Instruction_kv3_v1_msbfwd = 435,
  Instruction_kv3_v1_msbfwdp = 436,
  Instruction_kv3_v1_msbfwp = 437,
  Instruction_kv3_v1_muld = 438,
  Instruction_kv3_v1_muldt = 439,
  Instruction_kv3_v1_mulhq = 440,
  Instruction_kv3_v1_mulhwq = 441,
  Instruction_kv3_v1_mulsudt = 442,
  Instruction_kv3_v1_mulsuhwq = 443,
  Instruction_kv3_v1_mulsuwd = 444,
  Instruction_kv3_v1_mulsuwdp = 445,
  Instruction_kv3_v1_muludt = 446,
  Instruction_kv3_v1_muluhwq = 447,
  Instruction_kv3_v1_muluwd = 448,
  Instruction_kv3_v1_muluwdp = 449,
  Instruction_kv3_v1_mulw = 450,
  Instruction_kv3_v1_mulwc = 451,
  Instruction_kv3_v1_mulwc_c = 452,
  Instruction_kv3_v1_mulwd = 453,
  Instruction_kv3_v1_mulwdc = 454,
  Instruction_kv3_v1_mulwdc_c = 455,
  Instruction_kv3_v1_mulwdp = 456,
  Instruction_kv3_v1_mulwp = 457,
  Instruction_kv3_v1_mulwq = 458,
  Instruction_kv3_v1_nandd = 459,
  Instruction_kv3_v1_nandw = 460,
  Instruction_kv3_v1_negd = 461,
  Instruction_kv3_v1_neghq = 462,
  Instruction_kv3_v1_negw = 463,
  Instruction_kv3_v1_negwp = 464,
  Instruction_kv3_v1_niord = 465,
  Instruction_kv3_v1_niorw = 466,
  Instruction_kv3_v1_nop = 467,
  Instruction_kv3_v1_nord = 468,
  Instruction_kv3_v1_norw = 469,
  Instruction_kv3_v1_notd = 470,
  Instruction_kv3_v1_notw = 471,
  Instruction_kv3_v1_nxord = 472,
  Instruction_kv3_v1_nxorw = 473,
  Instruction_kv3_v1_ord = 474,
  Instruction_kv3_v1_ornd = 475,
  Instruction_kv3_v1_ornw = 476,
  Instruction_kv3_v1_orw = 477,
  Instruction_kv3_v1_pcrel = 478,
  Instruction_kv3_v1_ret = 479,
  Instruction_kv3_v1_rfe = 480,
  Instruction_kv3_v1_rolw = 481,
  Instruction_kv3_v1_rolwps = 482,
  Instruction_kv3_v1_rorw = 483,
  Instruction_kv3_v1_rorwps = 484,
  Instruction_kv3_v1_rswap = 485,
  Instruction_kv3_v1_satd = 486,
  Instruction_kv3_v1_satdh = 487,
  Instruction_kv3_v1_satdw = 488,
  Instruction_kv3_v1_sb = 489,
  Instruction_kv3_v1_sbfcd = 490,
  Instruction_kv3_v1_sbfcd_i = 491,
  Instruction_kv3_v1_sbfd = 492,
  Instruction_kv3_v1_sbfhcp_c = 493,
  Instruction_kv3_v1_sbfhq = 494,
  Instruction_kv3_v1_sbfsd = 495,
  Instruction_kv3_v1_sbfshq = 496,
  Instruction_kv3_v1_sbfsw = 497,
  Instruction_kv3_v1_sbfswp = 498,
  Instruction_kv3_v1_sbfuwd = 499,
  Instruction_kv3_v1_sbfw = 500,
  Instruction_kv3_v1_sbfwc_c = 501,
  Instruction_kv3_v1_sbfwd = 502,
  Instruction_kv3_v1_sbfwp = 503,
  Instruction_kv3_v1_sbfx16d = 504,
  Instruction_kv3_v1_sbfx16hq = 505,
  Instruction_kv3_v1_sbfx16uwd = 506,
  Instruction_kv3_v1_sbfx16w = 507,
  Instruction_kv3_v1_sbfx16wd = 508,
  Instruction_kv3_v1_sbfx16wp = 509,
  Instruction_kv3_v1_sbfx2d = 510,
  Instruction_kv3_v1_sbfx2hq = 511,
  Instruction_kv3_v1_sbfx2uwd = 512,
  Instruction_kv3_v1_sbfx2w = 513,
  Instruction_kv3_v1_sbfx2wd = 514,
  Instruction_kv3_v1_sbfx2wp = 515,
  Instruction_kv3_v1_sbfx4d = 516,
  Instruction_kv3_v1_sbfx4hq = 517,
  Instruction_kv3_v1_sbfx4uwd = 518,
  Instruction_kv3_v1_sbfx4w = 519,
  Instruction_kv3_v1_sbfx4wd = 520,
  Instruction_kv3_v1_sbfx4wp = 521,
  Instruction_kv3_v1_sbfx8d = 522,
  Instruction_kv3_v1_sbfx8hq = 523,
  Instruction_kv3_v1_sbfx8uwd = 524,
  Instruction_kv3_v1_sbfx8w = 525,
  Instruction_kv3_v1_sbfx8wd = 526,
  Instruction_kv3_v1_sbfx8wp = 527,
  Instruction_kv3_v1_sbmm8 = 528,
  Instruction_kv3_v1_sbmmt8 = 529,
  Instruction_kv3_v1_scall = 530,
  Instruction_kv3_v1_sd = 531,
  Instruction_kv3_v1_set = 532,
  Instruction_kv3_v1_sh = 533,
  Instruction_kv3_v1_sleep = 534,
  Instruction_kv3_v1_slld = 535,
  Instruction_kv3_v1_sllhqs = 536,
  Instruction_kv3_v1_sllw = 537,
  Instruction_kv3_v1_sllwps = 538,
  Instruction_kv3_v1_slsd = 539,
  Instruction_kv3_v1_slshqs = 540,
  Instruction_kv3_v1_slsw = 541,
  Instruction_kv3_v1_slswps = 542,
  Instruction_kv3_v1_so = 543,
  Instruction_kv3_v1_sq = 544,
  Instruction_kv3_v1_srad = 545,
  Instruction_kv3_v1_srahqs = 546,
  Instruction_kv3_v1_sraw = 547,
  Instruction_kv3_v1_srawps = 548,
  Instruction_kv3_v1_srld = 549,
  Instruction_kv3_v1_srlhqs = 550,
  Instruction_kv3_v1_srlw = 551,
  Instruction_kv3_v1_srlwps = 552,
  Instruction_kv3_v1_srsd = 553,
  Instruction_kv3_v1_srshqs = 554,
  Instruction_kv3_v1_srsw = 555,
  Instruction_kv3_v1_srswps = 556,
  Instruction_kv3_v1_stop = 557,
  Instruction_kv3_v1_stsud = 558,
  Instruction_kv3_v1_stsuw = 559,
  Instruction_kv3_v1_sw = 560,
  Instruction_kv3_v1_sxbd = 561,
  Instruction_kv3_v1_sxhd = 562,
  Instruction_kv3_v1_sxlbhq = 563,
  Instruction_kv3_v1_sxlhwp = 564,
  Instruction_kv3_v1_sxmbhq = 565,
  Instruction_kv3_v1_sxmhwp = 566,
  Instruction_kv3_v1_sxwd = 567,
  Instruction_kv3_v1_syncgroup = 568,
  Instruction_kv3_v1_tlbdinval = 569,
  Instruction_kv3_v1_tlbiinval = 570,
  Instruction_kv3_v1_tlbprobe = 571,
  Instruction_kv3_v1_tlbread = 572,
  Instruction_kv3_v1_tlbwrite = 573,
  Instruction_kv3_v1_waitit = 574,
  Instruction_kv3_v1_wfxl = 575,
  Instruction_kv3_v1_wfxm = 576,
  Instruction_kv3_v1_xcopyo = 577,
  Instruction_kv3_v1_xlo = 578,
  Instruction_kv3_v1_xmma484bw = 579,
  Instruction_kv3_v1_xmma484subw = 580,
  Instruction_kv3_v1_xmma484ubw = 581,
  Instruction_kv3_v1_xmma484usbw = 582,
  Instruction_kv3_v1_xmovefo = 583,
  Instruction_kv3_v1_xmovetq = 584,
  Instruction_kv3_v1_xmt44d = 585,
  Instruction_kv3_v1_xord = 586,
  Instruction_kv3_v1_xorw = 587,
  Instruction_kv3_v1_xso = 588,
  Instruction_kv3_v1_zxbd = 589,
  Instruction_kv3_v1_zxhd = 590,
  Instruction_kv3_v1_zxwd = 591,
  Separator_kv3_v1_comma = 592,
  Separator_kv3_v1_equal = 593,
  Separator_kv3_v1_qmark = 594,
  Separator_kv3_v1_rsbracket = 595,
  Separator_kv3_v1_lsbracket = 596
};

enum Modifier_kv3_v1_exunum_enum {
  Modifier_kv3_v1_exunum_ALU0=0,
  Modifier_kv3_v1_exunum_ALU1=1,
  Modifier_kv3_v1_exunum_MAU=2,
  Modifier_kv3_v1_exunum_LSU=3,
};

extern const char *mod_kv3_v1_exunum[];
extern const char *mod_kv3_v1_scalarcond[];
extern const char *mod_kv3_v1_simplecond[];
extern const char *mod_kv3_v1_comparison[];
extern const char *mod_kv3_v1_floatcomp[];
extern const char *mod_kv3_v1_rounding[];
extern const char *mod_kv3_v1_silent[];
extern const char *mod_kv3_v1_roundint[];
extern const char *mod_kv3_v1_saturate[];
extern const char *mod_kv3_v1_rectify[];
extern const char *mod_kv3_v1_variant[];
extern const char *mod_kv3_v1_speculate[];
extern const char *mod_kv3_v1_column[];
extern const char *mod_kv3_v1_doscale[];
extern const char *mod_kv3_v1_qindex[];
extern const char *mod_kv3_v1_splat32[];
typedef enum {
  Bundling_kv3_v1_ALL,
  Bundling_kv3_v1_BCU,
  Bundling_kv3_v1_TCA,
  Bundling_kv3_v1_FULL,
  Bundling_kv3_v1_FULL_X,
  Bundling_kv3_v1_FULL_Y,
  Bundling_kv3_v1_LITE,
  Bundling_kv3_v1_LITE_X,
  Bundling_kv3_v1_LITE_Y,
  Bundling_kv3_v1_MAU,
  Bundling_kv3_v1_MAU_X,
  Bundling_kv3_v1_MAU_Y,
  Bundling_kv3_v1_LSU,
  Bundling_kv3_v1_LSU_X,
  Bundling_kv3_v1_LSU_Y,
  Bundling_kv3_v1_TINY,
  Bundling_kv3_v1_TINY_X,
  Bundling_kv3_v1_TINY_Y,
  Bundling_kv3_v1_NOP,
} Bundling_kv3_v1;


static const char *bundling_kv3_v1_names(Bundling_kv3_v1 bundling) __attribute__((unused));
static const char *bundling_kv3_v1_names(Bundling_kv3_v1 bundling) {
  switch(bundling) {
  case Bundling_kv3_v1_ALL: return "Bundling_kv3_v1_ALL";
  case Bundling_kv3_v1_BCU: return "Bundling_kv3_v1_BCU";
  case Bundling_kv3_v1_TCA: return "Bundling_kv3_v1_TCA";
  case Bundling_kv3_v1_FULL: return "Bundling_kv3_v1_FULL";
  case Bundling_kv3_v1_FULL_X: return "Bundling_kv3_v1_FULL_X";
  case Bundling_kv3_v1_FULL_Y: return "Bundling_kv3_v1_FULL_Y";
  case Bundling_kv3_v1_LITE: return "Bundling_kv3_v1_LITE";
  case Bundling_kv3_v1_LITE_X: return "Bundling_kv3_v1_LITE_X";
  case Bundling_kv3_v1_LITE_Y: return "Bundling_kv3_v1_LITE_Y";
  case Bundling_kv3_v1_MAU: return "Bundling_kv3_v1_MAU";
  case Bundling_kv3_v1_MAU_X: return "Bundling_kv3_v1_MAU_X";
  case Bundling_kv3_v1_MAU_Y: return "Bundling_kv3_v1_MAU_Y";
  case Bundling_kv3_v1_LSU: return "Bundling_kv3_v1_LSU";
  case Bundling_kv3_v1_LSU_X: return "Bundling_kv3_v1_LSU_X";
  case Bundling_kv3_v1_LSU_Y: return "Bundling_kv3_v1_LSU_Y";
  case Bundling_kv3_v1_TINY: return "Bundling_kv3_v1_TINY";
  case Bundling_kv3_v1_TINY_X: return "Bundling_kv3_v1_TINY_X";
  case Bundling_kv3_v1_TINY_Y: return "Bundling_kv3_v1_TINY_Y";
  case Bundling_kv3_v1_NOP: return "Bundling_kv3_v1_NOP";
  };
  return "unknown bundling";
};

/* Resources list */
#define Resource_kv3_v1_ISSUE 0
#define Resource_kv3_v1_TINY 1
#define Resource_kv3_v1_LITE 2
#define Resource_kv3_v1_FULL 3
#define Resource_kv3_v1_LSU 4
#define Resource_kv3_v1_MAU 5
#define Resource_kv3_v1_BCU 6
#define Resource_kv3_v1_TCA 7
#define Resource_kv3_v1_AUXR 8
#define Resource_kv3_v1_AUXW 9
#define Resource_kv3_v1_CRRP 10
#define Resource_kv3_v1_CRWL 11
#define Resource_kv3_v1_CRWH 12
#define Resource_kv3_v1_NOP 13
#define kvx_kv3_v1_RESOURCE_MAX 14


/* Reservations list */
#define Reservation_kv3_v1_ALL 0
#define Reservation_kv3_v1_ALU_NOP 1
#define Reservation_kv3_v1_ALU_TINY 2
#define Reservation_kv3_v1_ALU_TINY_X 3
#define Reservation_kv3_v1_ALU_TINY_Y 4
#define Reservation_kv3_v1_ALU_LITE 5
#define Reservation_kv3_v1_ALU_LITE_X 6
#define Reservation_kv3_v1_ALU_LITE_Y 7
#define Reservation_kv3_v1_ALU_LITE_CRWL 8
#define Reservation_kv3_v1_ALU_LITE_CRWH 9
#define Reservation_kv3_v1_ALU_FULL 10
#define Reservation_kv3_v1_ALU_FULL_X 11
#define Reservation_kv3_v1_ALU_FULL_Y 12
#define Reservation_kv3_v1_BCU 13
#define Reservation_kv3_v1_BCU_CRRP_CRWL_CRWH 14
#define Reservation_kv3_v1_BCU_TINY_AUXW_CRRP 15
#define Reservation_kv3_v1_BCU_TINY_TINY_MAU_XNOP 16
#define Reservation_kv3_v1_TCA 17
#define Reservation_kv3_v1_LSU 18
#define Reservation_kv3_v1_LSU_X 19
#define Reservation_kv3_v1_LSU_Y 20
#define Reservation_kv3_v1_LSU_CRRP 21
#define Reservation_kv3_v1_LSU_CRRP_X 22
#define Reservation_kv3_v1_LSU_CRRP_Y 23
#define Reservation_kv3_v1_LSU_AUXR 24
#define Reservation_kv3_v1_LSU_AUXR_X 25
#define Reservation_kv3_v1_LSU_AUXR_Y 26
#define Reservation_kv3_v1_LSU_AUXW 27
#define Reservation_kv3_v1_LSU_AUXW_X 28
#define Reservation_kv3_v1_LSU_AUXW_Y 29
#define Reservation_kv3_v1_LSU_AUXR_AUXW 30
#define Reservation_kv3_v1_LSU_AUXR_AUXW_X 31
#define Reservation_kv3_v1_LSU_AUXR_AUXW_Y 32
#define Reservation_kv3_v1_MAU 33
#define Reservation_kv3_v1_MAU_X 34
#define Reservation_kv3_v1_MAU_Y 35
#define Reservation_kv3_v1_MAU_AUXR 36
#define Reservation_kv3_v1_MAU_AUXR_X 37
#define Reservation_kv3_v1_MAU_AUXR_Y 38


extern struct kvx_reloc kv3_v1_rel16_reloc;
extern struct kvx_reloc kv3_v1_rel32_reloc;
extern struct kvx_reloc kv3_v1_rel64_reloc;
extern struct kvx_reloc kv3_v1_pcrel_signed16_reloc;
extern struct kvx_reloc kv3_v1_pcrel17_reloc;
extern struct kvx_reloc kv3_v1_pcrel27_reloc;
extern struct kvx_reloc kv3_v1_pcrel32_reloc;
extern struct kvx_reloc kv3_v1_pcrel_signed37_reloc;
extern struct kvx_reloc kv3_v1_pcrel_signed43_reloc;
extern struct kvx_reloc kv3_v1_pcrel_signed64_reloc;
extern struct kvx_reloc kv3_v1_pcrel64_reloc;
extern struct kvx_reloc kv3_v1_signed16_reloc;
extern struct kvx_reloc kv3_v1_signed32_reloc;
extern struct kvx_reloc kv3_v1_signed37_reloc;
extern struct kvx_reloc kv3_v1_gotoff_signed37_reloc;
extern struct kvx_reloc kv3_v1_gotoff_signed43_reloc;
extern struct kvx_reloc kv3_v1_gotoff_32_reloc;
extern struct kvx_reloc kv3_v1_gotoff_64_reloc;
extern struct kvx_reloc kv3_v1_got_32_reloc;
extern struct kvx_reloc kv3_v1_got_signed37_reloc;
extern struct kvx_reloc kv3_v1_got_signed43_reloc;
extern struct kvx_reloc kv3_v1_got_64_reloc;
extern struct kvx_reloc kv3_v1_glob_dat_reloc;
extern struct kvx_reloc kv3_v1_copy_reloc;
extern struct kvx_reloc kv3_v1_jump_slot_reloc;
extern struct kvx_reloc kv3_v1_relative_reloc;
extern struct kvx_reloc kv3_v1_signed43_reloc;
extern struct kvx_reloc kv3_v1_signed64_reloc;
extern struct kvx_reloc kv3_v1_gotaddr_signed37_reloc;
extern struct kvx_reloc kv3_v1_gotaddr_signed43_reloc;
extern struct kvx_reloc kv3_v1_gotaddr_signed64_reloc;
extern struct kvx_reloc kv3_v1_dtpmod64_reloc;
extern struct kvx_reloc kv3_v1_dtpoff64_reloc;
extern struct kvx_reloc kv3_v1_dtpoff_signed37_reloc;
extern struct kvx_reloc kv3_v1_dtpoff_signed43_reloc;
extern struct kvx_reloc kv3_v1_tlsgd_signed37_reloc;
extern struct kvx_reloc kv3_v1_tlsgd_signed43_reloc;
extern struct kvx_reloc kv3_v1_tlsld_signed37_reloc;
extern struct kvx_reloc kv3_v1_tlsld_signed43_reloc;
extern struct kvx_reloc kv3_v1_tpoff64_reloc;
extern struct kvx_reloc kv3_v1_tlsie_signed37_reloc;
extern struct kvx_reloc kv3_v1_tlsie_signed43_reloc;
extern struct kvx_reloc kv3_v1_tlsle_signed37_reloc;
extern struct kvx_reloc kv3_v1_tlsle_signed43_reloc;
extern struct kvx_reloc kv3_v1_rel8_reloc;

#define KVX_REGFILE_FIRST_GPR 0
#define KVX_REGFILE_LAST_GPR 1
#define KVX_REGFILE_DEC_GPR 2
#define KVX_REGFILE_FIRST_PGR 3
#define KVX_REGFILE_LAST_PGR 4
#define KVX_REGFILE_DEC_PGR 5
#define KVX_REGFILE_FIRST_QGR 6
#define KVX_REGFILE_LAST_QGR 7
#define KVX_REGFILE_DEC_QGR 8
#define KVX_REGFILE_FIRST_SFR 9
#define KVX_REGFILE_LAST_SFR 10
#define KVX_REGFILE_DEC_SFR 11
#define KVX_REGFILE_FIRST_X16R 12
#define KVX_REGFILE_LAST_X16R 13
#define KVX_REGFILE_DEC_X16R 14
#define KVX_REGFILE_FIRST_X2R 15
#define KVX_REGFILE_LAST_X2R 16
#define KVX_REGFILE_DEC_X2R 17
#define KVX_REGFILE_FIRST_X32R 18
#define KVX_REGFILE_LAST_X32R 19
#define KVX_REGFILE_DEC_X32R 20
#define KVX_REGFILE_FIRST_X4R 21
#define KVX_REGFILE_LAST_X4R 22
#define KVX_REGFILE_DEC_X4R 23
#define KVX_REGFILE_FIRST_X64R 24
#define KVX_REGFILE_LAST_X64R 25
#define KVX_REGFILE_DEC_X64R 26
#define KVX_REGFILE_FIRST_X8R 27
#define KVX_REGFILE_LAST_X8R 28
#define KVX_REGFILE_DEC_X8R 29
#define KVX_REGFILE_FIRST_XBR 30
#define KVX_REGFILE_LAST_XBR 31
#define KVX_REGFILE_DEC_XBR 32
#define KVX_REGFILE_FIRST_XCR 33
#define KVX_REGFILE_LAST_XCR 34
#define KVX_REGFILE_DEC_XCR 35
#define KVX_REGFILE_FIRST_XMR 36
#define KVX_REGFILE_LAST_XMR 37
#define KVX_REGFILE_DEC_XMR 38
#define KVX_REGFILE_FIRST_XTR 39
#define KVX_REGFILE_LAST_XTR 40
#define KVX_REGFILE_DEC_XTR 41
#define KVX_REGFILE_FIRST_XVR 42
#define KVX_REGFILE_LAST_XVR 43
#define KVX_REGFILE_DEC_XVR 44
#define KVX_REGFILE_REGISTERS 45
#define KVX_REGFILE_DEC_REGISTERS 46


extern int kvx_kv3_v2_regfiles[];
extern const char **kvx_kv3_v2_modifiers[];
extern struct kvx_Register kvx_kv3_v2_registers[];

extern int kvx_kv3_v2_dec_registers[];

enum Method_kvx_kv3_v2_enum {
  Immediate_kv3_v2_brknumber = 1,
  Immediate_kv3_v2_pcrel17 = 2,
  Immediate_kv3_v2_pcrel27 = 3,
  Immediate_kv3_v2_signed10 = 4,
  Immediate_kv3_v2_signed16 = 5,
  Immediate_kv3_v2_signed27 = 6,
  Immediate_kv3_v2_signed37 = 7,
  Immediate_kv3_v2_signed43 = 8,
  Immediate_kv3_v2_signed54 = 9,
  Immediate_kv3_v2_sysnumber = 10,
  Immediate_kv3_v2_unsigned6 = 11,
  Immediate_kv3_v2_wrapped32 = 12,
  Immediate_kv3_v2_wrapped64 = 13,
  Immediate_kv3_v2_wrapped8 = 14,
  Modifier_kv3_v2_accesses = 15,
  Modifier_kv3_v2_boolcas = 16,
  Modifier_kv3_v2_cachelev = 17,
  Modifier_kv3_v2_channel = 18,
  Modifier_kv3_v2_coherency = 19,
  Modifier_kv3_v2_comparison = 20,
  Modifier_kv3_v2_conjugate = 21,
  Modifier_kv3_v2_doscale = 22,
  Modifier_kv3_v2_exunum = 23,
  Modifier_kv3_v2_floatcomp = 24,
  Modifier_kv3_v2_hindex = 25,
  Modifier_kv3_v2_lsomask = 26,
  Modifier_kv3_v2_lsumask = 27,
  Modifier_kv3_v2_lsupack = 28,
  Modifier_kv3_v2_qindex = 29,
  Modifier_kv3_v2_rounding = 30,
  Modifier_kv3_v2_scalarcond = 31,
  Modifier_kv3_v2_shuffleV = 32,
  Modifier_kv3_v2_shuffleX = 33,
  Modifier_kv3_v2_silent = 34,
  Modifier_kv3_v2_simplecond = 35,
  Modifier_kv3_v2_speculate = 36,
  Modifier_kv3_v2_splat32 = 37,
  Modifier_kv3_v2_transpose = 38,
  Modifier_kv3_v2_variant = 39,
  RegClass_kv3_v2_aloneReg = 40,
  RegClass_kv3_v2_blockReg = 41,
  RegClass_kv3_v2_blockRegE = 42,
  RegClass_kv3_v2_blockRegO = 43,
  RegClass_kv3_v2_blockReg_0 = 44,
  RegClass_kv3_v2_blockReg_1 = 45,
  RegClass_kv3_v2_buffer16Reg = 46,
  RegClass_kv3_v2_buffer2Reg = 47,
  RegClass_kv3_v2_buffer32Reg = 48,
  RegClass_kv3_v2_buffer4Reg = 49,
  RegClass_kv3_v2_buffer64Reg = 50,
  RegClass_kv3_v2_buffer8Reg = 51,
  RegClass_kv3_v2_coproReg = 52,
  RegClass_kv3_v2_coproReg0M4 = 53,
  RegClass_kv3_v2_coproReg1M4 = 54,
  RegClass_kv3_v2_coproReg2M4 = 55,
  RegClass_kv3_v2_coproReg3M4 = 56,
  RegClass_kv3_v2_matrixReg = 57,
  RegClass_kv3_v2_matrixReg_0 = 58,
  RegClass_kv3_v2_matrixReg_1 = 59,
  RegClass_kv3_v2_matrixReg_2 = 60,
  RegClass_kv3_v2_matrixReg_3 = 61,
  RegClass_kv3_v2_onlyfxReg = 62,
  RegClass_kv3_v2_onlygetReg = 63,
  RegClass_kv3_v2_onlyraReg = 64,
  RegClass_kv3_v2_onlysetReg = 65,
  RegClass_kv3_v2_onlyswapReg = 66,
  RegClass_kv3_v2_pairedReg = 67,
  RegClass_kv3_v2_pairedReg_0 = 68,
  RegClass_kv3_v2_pairedReg_1 = 69,
  RegClass_kv3_v2_quadReg = 70,
  RegClass_kv3_v2_quadReg_0 = 71,
  RegClass_kv3_v2_quadReg_1 = 72,
  RegClass_kv3_v2_quadReg_2 = 73,
  RegClass_kv3_v2_quadReg_3 = 74,
  RegClass_kv3_v2_singleReg = 75,
  RegClass_kv3_v2_systemReg = 76,
  RegClass_kv3_v2_tileReg = 77,
  RegClass_kv3_v2_tileReg_0 = 78,
  RegClass_kv3_v2_tileReg_1 = 79,
  RegClass_kv3_v2_vectorReg = 80,
  RegClass_kv3_v2_vectorReg_0 = 81,
  RegClass_kv3_v2_vectorReg_1 = 82,
  RegClass_kv3_v2_vectorReg_2 = 83,
  RegClass_kv3_v2_vectorReg_3 = 84,
  Instruction_kv3_v2_abdbo = 85,
  Instruction_kv3_v2_abdd = 86,
  Instruction_kv3_v2_abdhq = 87,
  Instruction_kv3_v2_abdsbo = 88,
  Instruction_kv3_v2_abdsd = 89,
  Instruction_kv3_v2_abdshq = 90,
  Instruction_kv3_v2_abdsw = 91,
  Instruction_kv3_v2_abdswp = 92,
  Instruction_kv3_v2_abdubo = 93,
  Instruction_kv3_v2_abdud = 94,
  Instruction_kv3_v2_abduhq = 95,
  Instruction_kv3_v2_abduw = 96,
  Instruction_kv3_v2_abduwp = 97,
  Instruction_kv3_v2_abdw = 98,
  Instruction_kv3_v2_abdwp = 99,
  Instruction_kv3_v2_absbo = 100,
  Instruction_kv3_v2_absd = 101,
  Instruction_kv3_v2_abshq = 102,
  Instruction_kv3_v2_abssbo = 103,
  Instruction_kv3_v2_abssd = 104,
  Instruction_kv3_v2_absshq = 105,
  Instruction_kv3_v2_abssw = 106,
  Instruction_kv3_v2_absswp = 107,
  Instruction_kv3_v2_absw = 108,
  Instruction_kv3_v2_abswp = 109,
  Instruction_kv3_v2_acswapd = 110,
  Instruction_kv3_v2_acswapq = 111,
  Instruction_kv3_v2_acswapw = 112,
  Instruction_kv3_v2_addbo = 113,
  Instruction_kv3_v2_addcd = 114,
  Instruction_kv3_v2_addcd_i = 115,
  Instruction_kv3_v2_addd = 116,
  Instruction_kv3_v2_addhq = 117,
  Instruction_kv3_v2_addrbod = 118,
  Instruction_kv3_v2_addrhqd = 119,
  Instruction_kv3_v2_addrwpd = 120,
  Instruction_kv3_v2_addsbo = 121,
  Instruction_kv3_v2_addsd = 122,
  Instruction_kv3_v2_addshq = 123,
  Instruction_kv3_v2_addsw = 124,
  Instruction_kv3_v2_addswp = 125,
  Instruction_kv3_v2_addurbod = 126,
  Instruction_kv3_v2_addurhqd = 127,
  Instruction_kv3_v2_addurwpd = 128,
  Instruction_kv3_v2_addusbo = 129,
  Instruction_kv3_v2_addusd = 130,
  Instruction_kv3_v2_addushq = 131,
  Instruction_kv3_v2_addusw = 132,
  Instruction_kv3_v2_adduswp = 133,
  Instruction_kv3_v2_adduwd = 134,
  Instruction_kv3_v2_addw = 135,
  Instruction_kv3_v2_addwd = 136,
  Instruction_kv3_v2_addwp = 137,
  Instruction_kv3_v2_addx16bo = 138,
  Instruction_kv3_v2_addx16d = 139,
  Instruction_kv3_v2_addx16hq = 140,
  Instruction_kv3_v2_addx16uwd = 141,
  Instruction_kv3_v2_addx16w = 142,
  Instruction_kv3_v2_addx16wd = 143,
  Instruction_kv3_v2_addx16wp = 144,
  Instruction_kv3_v2_addx2bo = 145,
  Instruction_kv3_v2_addx2d = 146,
  Instruction_kv3_v2_addx2hq = 147,
  Instruction_kv3_v2_addx2uwd = 148,
  Instruction_kv3_v2_addx2w = 149,
  Instruction_kv3_v2_addx2wd = 150,
  Instruction_kv3_v2_addx2wp = 151,
  Instruction_kv3_v2_addx32d = 152,
  Instruction_kv3_v2_addx32uwd = 153,
  Instruction_kv3_v2_addx32w = 154,
  Instruction_kv3_v2_addx32wd = 155,
  Instruction_kv3_v2_addx4bo = 156,
  Instruction_kv3_v2_addx4d = 157,
  Instruction_kv3_v2_addx4hq = 158,
  Instruction_kv3_v2_addx4uwd = 159,
  Instruction_kv3_v2_addx4w = 160,
  Instruction_kv3_v2_addx4wd = 161,
  Instruction_kv3_v2_addx4wp = 162,
  Instruction_kv3_v2_addx64d = 163,
  Instruction_kv3_v2_addx64uwd = 164,
  Instruction_kv3_v2_addx64w = 165,
  Instruction_kv3_v2_addx64wd = 166,
  Instruction_kv3_v2_addx8bo = 167,
  Instruction_kv3_v2_addx8d = 168,
  Instruction_kv3_v2_addx8hq = 169,
  Instruction_kv3_v2_addx8uwd = 170,
  Instruction_kv3_v2_addx8w = 171,
  Instruction_kv3_v2_addx8wd = 172,
  Instruction_kv3_v2_addx8wp = 173,
  Instruction_kv3_v2_aladdd = 174,
  Instruction_kv3_v2_aladdw = 175,
  Instruction_kv3_v2_alclrd = 176,
  Instruction_kv3_v2_alclrw = 177,
  Instruction_kv3_v2_ald = 178,
  Instruction_kv3_v2_alw = 179,
  Instruction_kv3_v2_andd = 180,
  Instruction_kv3_v2_andnd = 181,
  Instruction_kv3_v2_andnw = 182,
  Instruction_kv3_v2_andrbod = 183,
  Instruction_kv3_v2_andrhqd = 184,
  Instruction_kv3_v2_andrwpd = 185,
  Instruction_kv3_v2_andw = 186,
  Instruction_kv3_v2_asd = 187,
  Instruction_kv3_v2_asw = 188,
  Instruction_kv3_v2_avgbo = 189,
  Instruction_kv3_v2_avghq = 190,
  Instruction_kv3_v2_avgrbo = 191,
  Instruction_kv3_v2_avgrhq = 192,
  Instruction_kv3_v2_avgrubo = 193,
  Instruction_kv3_v2_avgruhq = 194,
  Instruction_kv3_v2_avgruw = 195,
  Instruction_kv3_v2_avgruwp = 196,
  Instruction_kv3_v2_avgrw = 197,
  Instruction_kv3_v2_avgrwp = 198,
  Instruction_kv3_v2_avgubo = 199,
  Instruction_kv3_v2_avguhq = 200,
  Instruction_kv3_v2_avguw = 201,
  Instruction_kv3_v2_avguwp = 202,
  Instruction_kv3_v2_avgw = 203,
  Instruction_kv3_v2_avgwp = 204,
  Instruction_kv3_v2_await = 205,
  Instruction_kv3_v2_barrier = 206,
  Instruction_kv3_v2_break = 207,
  Instruction_kv3_v2_call = 208,
  Instruction_kv3_v2_cb = 209,
  Instruction_kv3_v2_cbsd = 210,
  Instruction_kv3_v2_cbsw = 211,
  Instruction_kv3_v2_cbswp = 212,
  Instruction_kv3_v2_clrf = 213,
  Instruction_kv3_v2_clsd = 214,
  Instruction_kv3_v2_clsw = 215,
  Instruction_kv3_v2_clswp = 216,
  Instruction_kv3_v2_clzd = 217,
  Instruction_kv3_v2_clzw = 218,
  Instruction_kv3_v2_clzwp = 219,
  Instruction_kv3_v2_cmovebo = 220,
  Instruction_kv3_v2_cmoved = 221,
  Instruction_kv3_v2_cmovehq = 222,
  Instruction_kv3_v2_cmovewp = 223,
  Instruction_kv3_v2_cmuldt = 224,
  Instruction_kv3_v2_cmulghxdt = 225,
  Instruction_kv3_v2_cmulglxdt = 226,
  Instruction_kv3_v2_cmulgmxdt = 227,
  Instruction_kv3_v2_cmulxdt = 228,
  Instruction_kv3_v2_compd = 229,
  Instruction_kv3_v2_compnbo = 230,
  Instruction_kv3_v2_compnd = 231,
  Instruction_kv3_v2_compnhq = 232,
  Instruction_kv3_v2_compnw = 233,
  Instruction_kv3_v2_compnwp = 234,
  Instruction_kv3_v2_compuwd = 235,
  Instruction_kv3_v2_compw = 236,
  Instruction_kv3_v2_compwd = 237,
  Instruction_kv3_v2_copyd = 238,
  Instruction_kv3_v2_copyo = 239,
  Instruction_kv3_v2_copyq = 240,
  Instruction_kv3_v2_copyw = 241,
  Instruction_kv3_v2_crcbellw = 242,
  Instruction_kv3_v2_crcbelmw = 243,
  Instruction_kv3_v2_crclellw = 244,
  Instruction_kv3_v2_crclelmw = 245,
  Instruction_kv3_v2_ctzd = 246,
  Instruction_kv3_v2_ctzw = 247,
  Instruction_kv3_v2_ctzwp = 248,
  Instruction_kv3_v2_d1inval = 249,
  Instruction_kv3_v2_dflushl = 250,
  Instruction_kv3_v2_dflushsw = 251,
  Instruction_kv3_v2_dinvall = 252,
  Instruction_kv3_v2_dinvalsw = 253,
  Instruction_kv3_v2_dot2suwd = 254,
  Instruction_kv3_v2_dot2suwdp = 255,
  Instruction_kv3_v2_dot2uwd = 256,
  Instruction_kv3_v2_dot2uwdp = 257,
  Instruction_kv3_v2_dot2w = 258,
  Instruction_kv3_v2_dot2wd = 259,
  Instruction_kv3_v2_dot2wdp = 260,
  Instruction_kv3_v2_dot2wzp = 261,
  Instruction_kv3_v2_dpurgel = 262,
  Instruction_kv3_v2_dpurgesw = 263,
  Instruction_kv3_v2_dtouchl = 264,
  Instruction_kv3_v2_errop = 265,
  Instruction_kv3_v2_extfs = 266,
  Instruction_kv3_v2_extfz = 267,
  Instruction_kv3_v2_fabsd = 268,
  Instruction_kv3_v2_fabshq = 269,
  Instruction_kv3_v2_fabsw = 270,
  Instruction_kv3_v2_fabswp = 271,
  Instruction_kv3_v2_faddd = 272,
  Instruction_kv3_v2_fadddc = 273,
  Instruction_kv3_v2_fadddc_c = 274,
  Instruction_kv3_v2_fadddp = 275,
  Instruction_kv3_v2_faddho = 276,
  Instruction_kv3_v2_faddhq = 277,
  Instruction_kv3_v2_faddw = 278,
  Instruction_kv3_v2_faddwc = 279,
  Instruction_kv3_v2_faddwc_c = 280,
  Instruction_kv3_v2_faddwcp = 281,
  Instruction_kv3_v2_faddwcp_c = 282,
  Instruction_kv3_v2_faddwp = 283,
  Instruction_kv3_v2_faddwq = 284,
  Instruction_kv3_v2_fcdivd = 285,
  Instruction_kv3_v2_fcdivw = 286,
  Instruction_kv3_v2_fcdivwp = 287,
  Instruction_kv3_v2_fcompd = 288,
  Instruction_kv3_v2_fcompnd = 289,
  Instruction_kv3_v2_fcompnhq = 290,
  Instruction_kv3_v2_fcompnw = 291,
  Instruction_kv3_v2_fcompnwp = 292,
  Instruction_kv3_v2_fcompw = 293,
  Instruction_kv3_v2_fdot2w = 294,
  Instruction_kv3_v2_fdot2wd = 295,
  Instruction_kv3_v2_fdot2wdp = 296,
  Instruction_kv3_v2_fdot2wzp = 297,
  Instruction_kv3_v2_fence = 298,
  Instruction_kv3_v2_ffdmasw = 299,
  Instruction_kv3_v2_ffdmaswp = 300,
  Instruction_kv3_v2_ffdmaswq = 301,
  Instruction_kv3_v2_ffdmaw = 302,
  Instruction_kv3_v2_ffdmawp = 303,
  Instruction_kv3_v2_ffdmawq = 304,
  Instruction_kv3_v2_ffdmdaw = 305,
  Instruction_kv3_v2_ffdmdawp = 306,
  Instruction_kv3_v2_ffdmdawq = 307,
  Instruction_kv3_v2_ffdmdsw = 308,
  Instruction_kv3_v2_ffdmdswp = 309,
  Instruction_kv3_v2_ffdmdswq = 310,
  Instruction_kv3_v2_ffdmsaw = 311,
  Instruction_kv3_v2_ffdmsawp = 312,
  Instruction_kv3_v2_ffdmsawq = 313,
  Instruction_kv3_v2_ffdmsw = 314,
  Instruction_kv3_v2_ffdmswp = 315,
  Instruction_kv3_v2_ffdmswq = 316,
  Instruction_kv3_v2_ffmad = 317,
  Instruction_kv3_v2_ffmaho = 318,
  Instruction_kv3_v2_ffmahq = 319,
  Instruction_kv3_v2_ffmahw = 320,
  Instruction_kv3_v2_ffmahwq = 321,
  Instruction_kv3_v2_ffmaw = 322,
  Instruction_kv3_v2_ffmawc = 323,
  Instruction_kv3_v2_ffmawcp = 324,
  Instruction_kv3_v2_ffmawd = 325,
  Instruction_kv3_v2_ffmawdp = 326,
  Instruction_kv3_v2_ffmawp = 327,
  Instruction_kv3_v2_ffmawq = 328,
  Instruction_kv3_v2_ffmsd = 329,
  Instruction_kv3_v2_ffmsho = 330,
  Instruction_kv3_v2_ffmshq = 331,
  Instruction_kv3_v2_ffmshw = 332,
  Instruction_kv3_v2_ffmshwq = 333,
  Instruction_kv3_v2_ffmsw = 334,
  Instruction_kv3_v2_ffmswc = 335,
  Instruction_kv3_v2_ffmswcp = 336,
  Instruction_kv3_v2_ffmswd = 337,
  Instruction_kv3_v2_ffmswdp = 338,
  Instruction_kv3_v2_ffmswp = 339,
  Instruction_kv3_v2_ffmswq = 340,
  Instruction_kv3_v2_fixedd = 341,
  Instruction_kv3_v2_fixedud = 342,
  Instruction_kv3_v2_fixeduw = 343,
  Instruction_kv3_v2_fixeduwp = 344,
  Instruction_kv3_v2_fixedw = 345,
  Instruction_kv3_v2_fixedwp = 346,
  Instruction_kv3_v2_floatd = 347,
  Instruction_kv3_v2_floatud = 348,
  Instruction_kv3_v2_floatuw = 349,
  Instruction_kv3_v2_floatuwp = 350,
  Instruction_kv3_v2_floatw = 351,
  Instruction_kv3_v2_floatwp = 352,
  Instruction_kv3_v2_fmaxd = 353,
  Instruction_kv3_v2_fmaxhq = 354,
  Instruction_kv3_v2_fmaxw = 355,
  Instruction_kv3_v2_fmaxwp = 356,
  Instruction_kv3_v2_fmind = 357,
  Instruction_kv3_v2_fminhq = 358,
  Instruction_kv3_v2_fminw = 359,
  Instruction_kv3_v2_fminwp = 360,
  Instruction_kv3_v2_fmm212w = 361,
  Instruction_kv3_v2_fmm222w = 362,
  Instruction_kv3_v2_fmma212w = 363,
  Instruction_kv3_v2_fmma222w = 364,
  Instruction_kv3_v2_fmms212w = 365,
  Instruction_kv3_v2_fmms222w = 366,
  Instruction_kv3_v2_fmuld = 367,
  Instruction_kv3_v2_fmulho = 368,
  Instruction_kv3_v2_fmulhq = 369,
  Instruction_kv3_v2_fmulhw = 370,
  Instruction_kv3_v2_fmulhwq = 371,
  Instruction_kv3_v2_fmulw = 372,
  Instruction_kv3_v2_fmulwc = 373,
  Instruction_kv3_v2_fmulwcp = 374,
  Instruction_kv3_v2_fmulwd = 375,
  Instruction_kv3_v2_fmulwdp = 376,
  Instruction_kv3_v2_fmulwp = 377,
  Instruction_kv3_v2_fmulwq = 378,
  Instruction_kv3_v2_fnarrowdw = 379,
  Instruction_kv3_v2_fnarrowdwp = 380,
  Instruction_kv3_v2_fnarrowwh = 381,
  Instruction_kv3_v2_fnarrowwhq = 382,
  Instruction_kv3_v2_fnegd = 383,
  Instruction_kv3_v2_fneghq = 384,
  Instruction_kv3_v2_fnegw = 385,
  Instruction_kv3_v2_fnegwp = 386,
  Instruction_kv3_v2_frecw = 387,
  Instruction_kv3_v2_frsrw = 388,
  Instruction_kv3_v2_fsbfd = 389,
  Instruction_kv3_v2_fsbfdc = 390,
  Instruction_kv3_v2_fsbfdc_c = 391,
  Instruction_kv3_v2_fsbfdp = 392,
  Instruction_kv3_v2_fsbfho = 393,
  Instruction_kv3_v2_fsbfhq = 394,
  Instruction_kv3_v2_fsbfw = 395,
  Instruction_kv3_v2_fsbfwc = 396,
  Instruction_kv3_v2_fsbfwc_c = 397,
  Instruction_kv3_v2_fsbfwcp = 398,
  Instruction_kv3_v2_fsbfwcp_c = 399,
  Instruction_kv3_v2_fsbfwp = 400,
  Instruction_kv3_v2_fsbfwq = 401,
  Instruction_kv3_v2_fsdivd = 402,
  Instruction_kv3_v2_fsdivw = 403,
  Instruction_kv3_v2_fsdivwp = 404,
  Instruction_kv3_v2_fsrecd = 405,
  Instruction_kv3_v2_fsrecw = 406,
  Instruction_kv3_v2_fsrecwp = 407,
  Instruction_kv3_v2_fsrsrd = 408,
  Instruction_kv3_v2_fsrsrw = 409,
  Instruction_kv3_v2_fsrsrwp = 410,
  Instruction_kv3_v2_fwidenlhw = 411,
  Instruction_kv3_v2_fwidenlhwp = 412,
  Instruction_kv3_v2_fwidenlwd = 413,
  Instruction_kv3_v2_fwidenmhw = 414,
  Instruction_kv3_v2_fwidenmhwp = 415,
  Instruction_kv3_v2_fwidenmwd = 416,
  Instruction_kv3_v2_get = 417,
  Instruction_kv3_v2_goto = 418,
  Instruction_kv3_v2_i1inval = 419,
  Instruction_kv3_v2_i1invals = 420,
  Instruction_kv3_v2_icall = 421,
  Instruction_kv3_v2_iget = 422,
  Instruction_kv3_v2_igoto = 423,
  Instruction_kv3_v2_insf = 424,
  Instruction_kv3_v2_iord = 425,
  Instruction_kv3_v2_iornd = 426,
  Instruction_kv3_v2_iornw = 427,
  Instruction_kv3_v2_iorrbod = 428,
  Instruction_kv3_v2_iorrhqd = 429,
  Instruction_kv3_v2_iorrwpd = 430,
  Instruction_kv3_v2_iorw = 431,
  Instruction_kv3_v2_landd = 432,
  Instruction_kv3_v2_landw = 433,
  Instruction_kv3_v2_lbs = 434,
  Instruction_kv3_v2_lbz = 435,
  Instruction_kv3_v2_ld = 436,
  Instruction_kv3_v2_lhs = 437,
  Instruction_kv3_v2_lhz = 438,
  Instruction_kv3_v2_lnandd = 439,
  Instruction_kv3_v2_lnandw = 440,
  Instruction_kv3_v2_lnord = 441,
  Instruction_kv3_v2_lnorw = 442,
  Instruction_kv3_v2_lo = 443,
  Instruction_kv3_v2_loopdo = 444,
  Instruction_kv3_v2_lord = 445,
  Instruction_kv3_v2_lorw = 446,
  Instruction_kv3_v2_lq = 447,
  Instruction_kv3_v2_lws = 448,
  Instruction_kv3_v2_lwz = 449,
  Instruction_kv3_v2_maddd = 450,
  Instruction_kv3_v2_madddt = 451,
  Instruction_kv3_v2_maddhq = 452,
  Instruction_kv3_v2_maddhwq = 453,
  Instruction_kv3_v2_maddmwq = 454,
  Instruction_kv3_v2_maddsudt = 455,
  Instruction_kv3_v2_maddsuhwq = 456,
  Instruction_kv3_v2_maddsumwq = 457,
  Instruction_kv3_v2_maddsuwd = 458,
  Instruction_kv3_v2_maddsuwdp = 459,
  Instruction_kv3_v2_maddudt = 460,
  Instruction_kv3_v2_madduhwq = 461,
  Instruction_kv3_v2_maddumwq = 462,
  Instruction_kv3_v2_madduwd = 463,
  Instruction_kv3_v2_madduwdp = 464,
  Instruction_kv3_v2_madduzdt = 465,
  Instruction_kv3_v2_maddw = 466,
  Instruction_kv3_v2_maddwd = 467,
  Instruction_kv3_v2_maddwdp = 468,
  Instruction_kv3_v2_maddwp = 469,
  Instruction_kv3_v2_maddwq = 470,
  Instruction_kv3_v2_make = 471,
  Instruction_kv3_v2_maxbo = 472,
  Instruction_kv3_v2_maxd = 473,
  Instruction_kv3_v2_maxhq = 474,
  Instruction_kv3_v2_maxrbod = 475,
  Instruction_kv3_v2_maxrhqd = 476,
  Instruction_kv3_v2_maxrwpd = 477,
  Instruction_kv3_v2_maxubo = 478,
  Instruction_kv3_v2_maxud = 479,
  Instruction_kv3_v2_maxuhq = 480,
  Instruction_kv3_v2_maxurbod = 481,
  Instruction_kv3_v2_maxurhqd = 482,
  Instruction_kv3_v2_maxurwpd = 483,
  Instruction_kv3_v2_maxuw = 484,
  Instruction_kv3_v2_maxuwp = 485,
  Instruction_kv3_v2_maxw = 486,
  Instruction_kv3_v2_maxwp = 487,
  Instruction_kv3_v2_minbo = 488,
  Instruction_kv3_v2_mind = 489,
  Instruction_kv3_v2_minhq = 490,
  Instruction_kv3_v2_minrbod = 491,
  Instruction_kv3_v2_minrhqd = 492,
  Instruction_kv3_v2_minrwpd = 493,
  Instruction_kv3_v2_minubo = 494,
  Instruction_kv3_v2_minud = 495,
  Instruction_kv3_v2_minuhq = 496,
  Instruction_kv3_v2_minurbod = 497,
  Instruction_kv3_v2_minurhqd = 498,
  Instruction_kv3_v2_minurwpd = 499,
  Instruction_kv3_v2_minuw = 500,
  Instruction_kv3_v2_minuwp = 501,
  Instruction_kv3_v2_minw = 502,
  Instruction_kv3_v2_minwp = 503,
  Instruction_kv3_v2_mm212w = 504,
  Instruction_kv3_v2_mma212w = 505,
  Instruction_kv3_v2_mms212w = 506,
  Instruction_kv3_v2_msbfd = 507,
  Instruction_kv3_v2_msbfdt = 508,
  Instruction_kv3_v2_msbfhq = 509,
  Instruction_kv3_v2_msbfhwq = 510,
  Instruction_kv3_v2_msbfmwq = 511,
  Instruction_kv3_v2_msbfsudt = 512,
  Instruction_kv3_v2_msbfsuhwq = 513,
  Instruction_kv3_v2_msbfsumwq = 514,
  Instruction_kv3_v2_msbfsuwd = 515,
  Instruction_kv3_v2_msbfsuwdp = 516,
  Instruction_kv3_v2_msbfudt = 517,
  Instruction_kv3_v2_msbfuhwq = 518,
  Instruction_kv3_v2_msbfumwq = 519,
  Instruction_kv3_v2_msbfuwd = 520,
  Instruction_kv3_v2_msbfuwdp = 521,
  Instruction_kv3_v2_msbfuzdt = 522,
  Instruction_kv3_v2_msbfw = 523,
  Instruction_kv3_v2_msbfwd = 524,
  Instruction_kv3_v2_msbfwdp = 525,
  Instruction_kv3_v2_msbfwp = 526,
  Instruction_kv3_v2_msbfwq = 527,
  Instruction_kv3_v2_muld = 528,
  Instruction_kv3_v2_muldt = 529,
  Instruction_kv3_v2_mulhq = 530,
  Instruction_kv3_v2_mulhwq = 531,
  Instruction_kv3_v2_mulmwq = 532,
  Instruction_kv3_v2_mulsudt = 533,
  Instruction_kv3_v2_mulsuhwq = 534,
  Instruction_kv3_v2_mulsumwq = 535,
  Instruction_kv3_v2_mulsuwd = 536,
  Instruction_kv3_v2_mulsuwdp = 537,
  Instruction_kv3_v2_muludt = 538,
  Instruction_kv3_v2_muluhwq = 539,
  Instruction_kv3_v2_mulumwq = 540,
  Instruction_kv3_v2_muluwd = 541,
  Instruction_kv3_v2_muluwdp = 542,
  Instruction_kv3_v2_mulw = 543,
  Instruction_kv3_v2_mulwd = 544,
  Instruction_kv3_v2_mulwdp = 545,
  Instruction_kv3_v2_mulwp = 546,
  Instruction_kv3_v2_mulwq = 547,
  Instruction_kv3_v2_nandd = 548,
  Instruction_kv3_v2_nandw = 549,
  Instruction_kv3_v2_negbo = 550,
  Instruction_kv3_v2_negd = 551,
  Instruction_kv3_v2_neghq = 552,
  Instruction_kv3_v2_negsbo = 553,
  Instruction_kv3_v2_negsd = 554,
  Instruction_kv3_v2_negshq = 555,
  Instruction_kv3_v2_negsw = 556,
  Instruction_kv3_v2_negswp = 557,
  Instruction_kv3_v2_negw = 558,
  Instruction_kv3_v2_negwp = 559,
  Instruction_kv3_v2_niord = 560,
  Instruction_kv3_v2_niorw = 561,
  Instruction_kv3_v2_nop = 562,
  Instruction_kv3_v2_nord = 563,
  Instruction_kv3_v2_norw = 564,
  Instruction_kv3_v2_notd = 565,
  Instruction_kv3_v2_notw = 566,
  Instruction_kv3_v2_nxord = 567,
  Instruction_kv3_v2_nxorw = 568,
  Instruction_kv3_v2_ord = 569,
  Instruction_kv3_v2_ornd = 570,
  Instruction_kv3_v2_ornw = 571,
  Instruction_kv3_v2_orrbod = 572,
  Instruction_kv3_v2_orrhqd = 573,
  Instruction_kv3_v2_orrwpd = 574,
  Instruction_kv3_v2_orw = 575,
  Instruction_kv3_v2_pcrel = 576,
  Instruction_kv3_v2_ret = 577,
  Instruction_kv3_v2_rfe = 578,
  Instruction_kv3_v2_rolw = 579,
  Instruction_kv3_v2_rolwps = 580,
  Instruction_kv3_v2_rorw = 581,
  Instruction_kv3_v2_rorwps = 582,
  Instruction_kv3_v2_rswap = 583,
  Instruction_kv3_v2_sb = 584,
  Instruction_kv3_v2_sbfbo = 585,
  Instruction_kv3_v2_sbfcd = 586,
  Instruction_kv3_v2_sbfcd_i = 587,
  Instruction_kv3_v2_sbfd = 588,
  Instruction_kv3_v2_sbfhq = 589,
  Instruction_kv3_v2_sbfsbo = 590,
  Instruction_kv3_v2_sbfsd = 591,
  Instruction_kv3_v2_sbfshq = 592,
  Instruction_kv3_v2_sbfsw = 593,
  Instruction_kv3_v2_sbfswp = 594,
  Instruction_kv3_v2_sbfusbo = 595,
  Instruction_kv3_v2_sbfusd = 596,
  Instruction_kv3_v2_sbfushq = 597,
  Instruction_kv3_v2_sbfusw = 598,
  Instruction_kv3_v2_sbfuswp = 599,
  Instruction_kv3_v2_sbfuwd = 600,
  Instruction_kv3_v2_sbfw = 601,
  Instruction_kv3_v2_sbfwd = 602,
  Instruction_kv3_v2_sbfwp = 603,
  Instruction_kv3_v2_sbfx16bo = 604,
  Instruction_kv3_v2_sbfx16d = 605,
  Instruction_kv3_v2_sbfx16hq = 606,
  Instruction_kv3_v2_sbfx16uwd = 607,
  Instruction_kv3_v2_sbfx16w = 608,
  Instruction_kv3_v2_sbfx16wd = 609,
  Instruction_kv3_v2_sbfx16wp = 610,
  Instruction_kv3_v2_sbfx2bo = 611,
  Instruction_kv3_v2_sbfx2d = 612,
  Instruction_kv3_v2_sbfx2hq = 613,
  Instruction_kv3_v2_sbfx2uwd = 614,
  Instruction_kv3_v2_sbfx2w = 615,
  Instruction_kv3_v2_sbfx2wd = 616,
  Instruction_kv3_v2_sbfx2wp = 617,
  Instruction_kv3_v2_sbfx32d = 618,
  Instruction_kv3_v2_sbfx32uwd = 619,
  Instruction_kv3_v2_sbfx32w = 620,
  Instruction_kv3_v2_sbfx32wd = 621,
  Instruction_kv3_v2_sbfx4bo = 622,
  Instruction_kv3_v2_sbfx4d = 623,
  Instruction_kv3_v2_sbfx4hq = 624,
  Instruction_kv3_v2_sbfx4uwd = 625,
  Instruction_kv3_v2_sbfx4w = 626,
  Instruction_kv3_v2_sbfx4wd = 627,
  Instruction_kv3_v2_sbfx4wp = 628,
  Instruction_kv3_v2_sbfx64d = 629,
  Instruction_kv3_v2_sbfx64uwd = 630,
  Instruction_kv3_v2_sbfx64w = 631,
  Instruction_kv3_v2_sbfx64wd = 632,
  Instruction_kv3_v2_sbfx8bo = 633,
  Instruction_kv3_v2_sbfx8d = 634,
  Instruction_kv3_v2_sbfx8hq = 635,
  Instruction_kv3_v2_sbfx8uwd = 636,
  Instruction_kv3_v2_sbfx8w = 637,
  Instruction_kv3_v2_sbfx8wd = 638,
  Instruction_kv3_v2_sbfx8wp = 639,
  Instruction_kv3_v2_sbmm8 = 640,
  Instruction_kv3_v2_sbmmt8 = 641,
  Instruction_kv3_v2_scall = 642,
  Instruction_kv3_v2_sd = 643,
  Instruction_kv3_v2_set = 644,
  Instruction_kv3_v2_sh = 645,
  Instruction_kv3_v2_sleep = 646,
  Instruction_kv3_v2_sllbos = 647,
  Instruction_kv3_v2_slld = 648,
  Instruction_kv3_v2_sllhqs = 649,
  Instruction_kv3_v2_sllw = 650,
  Instruction_kv3_v2_sllwps = 651,
  Instruction_kv3_v2_slsbos = 652,
  Instruction_kv3_v2_slsd = 653,
  Instruction_kv3_v2_slshqs = 654,
  Instruction_kv3_v2_slsw = 655,
  Instruction_kv3_v2_slswps = 656,
  Instruction_kv3_v2_slusbos = 657,
  Instruction_kv3_v2_slusd = 658,
  Instruction_kv3_v2_slushqs = 659,
  Instruction_kv3_v2_slusw = 660,
  Instruction_kv3_v2_sluswps = 661,
  Instruction_kv3_v2_so = 662,
  Instruction_kv3_v2_sq = 663,
  Instruction_kv3_v2_srabos = 664,
  Instruction_kv3_v2_srad = 665,
  Instruction_kv3_v2_srahqs = 666,
  Instruction_kv3_v2_sraw = 667,
  Instruction_kv3_v2_srawps = 668,
  Instruction_kv3_v2_srlbos = 669,
  Instruction_kv3_v2_srld = 670,
  Instruction_kv3_v2_srlhqs = 671,
  Instruction_kv3_v2_srlw = 672,
  Instruction_kv3_v2_srlwps = 673,
  Instruction_kv3_v2_srsbos = 674,
  Instruction_kv3_v2_srsd = 675,
  Instruction_kv3_v2_srshqs = 676,
  Instruction_kv3_v2_srsw = 677,
  Instruction_kv3_v2_srswps = 678,
  Instruction_kv3_v2_stop = 679,
  Instruction_kv3_v2_stsud = 680,
  Instruction_kv3_v2_stsuhq = 681,
  Instruction_kv3_v2_stsuw = 682,
  Instruction_kv3_v2_stsuwp = 683,
  Instruction_kv3_v2_sw = 684,
  Instruction_kv3_v2_sxbd = 685,
  Instruction_kv3_v2_sxhd = 686,
  Instruction_kv3_v2_sxlbhq = 687,
  Instruction_kv3_v2_sxlhwp = 688,
  Instruction_kv3_v2_sxmbhq = 689,
  Instruction_kv3_v2_sxmhwp = 690,
  Instruction_kv3_v2_sxwd = 691,
  Instruction_kv3_v2_syncgroup = 692,
  Instruction_kv3_v2_tlbdinval = 693,
  Instruction_kv3_v2_tlbiinval = 694,
  Instruction_kv3_v2_tlbprobe = 695,
  Instruction_kv3_v2_tlbread = 696,
  Instruction_kv3_v2_tlbwrite = 697,
  Instruction_kv3_v2_waitit = 698,
  Instruction_kv3_v2_wfxl = 699,
  Instruction_kv3_v2_wfxm = 700,
  Instruction_kv3_v2_xaccesso = 701,
  Instruction_kv3_v2_xaligno = 702,
  Instruction_kv3_v2_xandno = 703,
  Instruction_kv3_v2_xando = 704,
  Instruction_kv3_v2_xclampwo = 705,
  Instruction_kv3_v2_xcopyo = 706,
  Instruction_kv3_v2_xcopyv = 707,
  Instruction_kv3_v2_xcopyx = 708,
  Instruction_kv3_v2_xffma44hw = 709,
  Instruction_kv3_v2_xfmaxhx = 710,
  Instruction_kv3_v2_xfminhx = 711,
  Instruction_kv3_v2_xfmma484hw = 712,
  Instruction_kv3_v2_xfnarrow44wh = 713,
  Instruction_kv3_v2_xfscalewo = 714,
  Instruction_kv3_v2_xioro = 715,
  Instruction_kv3_v2_xlo = 716,
  Instruction_kv3_v2_xmadd44bw0 = 717,
  Instruction_kv3_v2_xmadd44bw1 = 718,
  Instruction_kv3_v2_xmaddifwo = 719,
  Instruction_kv3_v2_xmaddsu44bw0 = 720,
  Instruction_kv3_v2_xmaddsu44bw1 = 721,
  Instruction_kv3_v2_xmaddu44bw0 = 722,
  Instruction_kv3_v2_xmaddu44bw1 = 723,
  Instruction_kv3_v2_xmma4164bw = 724,
  Instruction_kv3_v2_xmma484bw = 725,
  Instruction_kv3_v2_xmmasu4164bw = 726,
  Instruction_kv3_v2_xmmasu484bw = 727,
  Instruction_kv3_v2_xmmau4164bw = 728,
  Instruction_kv3_v2_xmmau484bw = 729,
  Instruction_kv3_v2_xmmaus4164bw = 730,
  Instruction_kv3_v2_xmmaus484bw = 731,
  Instruction_kv3_v2_xmovefd = 732,
  Instruction_kv3_v2_xmovefo = 733,
  Instruction_kv3_v2_xmovefq = 734,
  Instruction_kv3_v2_xmovetd = 735,
  Instruction_kv3_v2_xmovetq = 736,
  Instruction_kv3_v2_xmsbfifwo = 737,
  Instruction_kv3_v2_xmt44d = 738,
  Instruction_kv3_v2_xnando = 739,
  Instruction_kv3_v2_xnoro = 740,
  Instruction_kv3_v2_xnxoro = 741,
  Instruction_kv3_v2_xord = 742,
  Instruction_kv3_v2_xorno = 743,
  Instruction_kv3_v2_xoro = 744,
  Instruction_kv3_v2_xorrbod = 745,
  Instruction_kv3_v2_xorrhqd = 746,
  Instruction_kv3_v2_xorrwpd = 747,
  Instruction_kv3_v2_xorw = 748,
  Instruction_kv3_v2_xrecvo = 749,
  Instruction_kv3_v2_xsbmm8dq = 750,
  Instruction_kv3_v2_xsbmmt8dq = 751,
  Instruction_kv3_v2_xsendo = 752,
  Instruction_kv3_v2_xsendrecvo = 753,
  Instruction_kv3_v2_xso = 754,
  Instruction_kv3_v2_xsplatdo = 755,
  Instruction_kv3_v2_xsplatov = 756,
  Instruction_kv3_v2_xsplatox = 757,
  Instruction_kv3_v2_xsx48bw = 758,
  Instruction_kv3_v2_xtrunc48wb = 759,
  Instruction_kv3_v2_xxoro = 760,
  Instruction_kv3_v2_xzx48bw = 761,
  Instruction_kv3_v2_zxbd = 762,
  Instruction_kv3_v2_zxhd = 763,
  Instruction_kv3_v2_zxlbhq = 764,
  Instruction_kv3_v2_zxlhwp = 765,
  Instruction_kv3_v2_zxmbhq = 766,
  Instruction_kv3_v2_zxmhwp = 767,
  Instruction_kv3_v2_zxwd = 768,
  Separator_kv3_v2_comma = 769,
  Separator_kv3_v2_equal = 770,
  Separator_kv3_v2_qmark = 771,
  Separator_kv3_v2_rsbracket = 772,
  Separator_kv3_v2_lsbracket = 773
};

enum Modifier_kv3_v2_exunum_enum {
  Modifier_kv3_v2_exunum_ALU0=0,
  Modifier_kv3_v2_exunum_ALU1=1,
  Modifier_kv3_v2_exunum_MAU=2,
  Modifier_kv3_v2_exunum_LSU=3,
};

extern const char *mod_kv3_v2_exunum[];
extern const char *mod_kv3_v2_scalarcond[];
extern const char *mod_kv3_v2_lsomask[];
extern const char *mod_kv3_v2_lsumask[];
extern const char *mod_kv3_v2_lsupack[];
extern const char *mod_kv3_v2_simplecond[];
extern const char *mod_kv3_v2_comparison[];
extern const char *mod_kv3_v2_floatcomp[];
extern const char *mod_kv3_v2_rounding[];
extern const char *mod_kv3_v2_silent[];
extern const char *mod_kv3_v2_variant[];
extern const char *mod_kv3_v2_speculate[];
extern const char *mod_kv3_v2_doscale[];
extern const char *mod_kv3_v2_qindex[];
extern const char *mod_kv3_v2_hindex[];
extern const char *mod_kv3_v2_cachelev[];
extern const char *mod_kv3_v2_coherency[];
extern const char *mod_kv3_v2_boolcas[];
extern const char *mod_kv3_v2_accesses[];
extern const char *mod_kv3_v2_channel[];
extern const char *mod_kv3_v2_conjugate[];
extern const char *mod_kv3_v2_transpose[];
extern const char *mod_kv3_v2_shuffleV[];
extern const char *mod_kv3_v2_shuffleX[];
extern const char *mod_kv3_v2_splat32[];
typedef enum {
  Bundling_kv3_v2_ALL,
  Bundling_kv3_v2_BCU,
  Bundling_kv3_v2_TCA,
  Bundling_kv3_v2_FULL,
  Bundling_kv3_v2_FULL_X,
  Bundling_kv3_v2_FULL_Y,
  Bundling_kv3_v2_LITE,
  Bundling_kv3_v2_LITE_X,
  Bundling_kv3_v2_LITE_Y,
  Bundling_kv3_v2_MAU,
  Bundling_kv3_v2_MAU_X,
  Bundling_kv3_v2_MAU_Y,
  Bundling_kv3_v2_LSU,
  Bundling_kv3_v2_LSU_X,
  Bundling_kv3_v2_LSU_Y,
  Bundling_kv3_v2_TINY,
  Bundling_kv3_v2_TINY_X,
  Bundling_kv3_v2_TINY_Y,
  Bundling_kv3_v2_NOP,
} Bundling_kv3_v2;


static const char *bundling_kv3_v2_names(Bundling_kv3_v2 bundling) __attribute__((unused));
static const char *bundling_kv3_v2_names(Bundling_kv3_v2 bundling) {
  switch(bundling) {
  case Bundling_kv3_v2_ALL: return "Bundling_kv3_v2_ALL";
  case Bundling_kv3_v2_BCU: return "Bundling_kv3_v2_BCU";
  case Bundling_kv3_v2_TCA: return "Bundling_kv3_v2_TCA";
  case Bundling_kv3_v2_FULL: return "Bundling_kv3_v2_FULL";
  case Bundling_kv3_v2_FULL_X: return "Bundling_kv3_v2_FULL_X";
  case Bundling_kv3_v2_FULL_Y: return "Bundling_kv3_v2_FULL_Y";
  case Bundling_kv3_v2_LITE: return "Bundling_kv3_v2_LITE";
  case Bundling_kv3_v2_LITE_X: return "Bundling_kv3_v2_LITE_X";
  case Bundling_kv3_v2_LITE_Y: return "Bundling_kv3_v2_LITE_Y";
  case Bundling_kv3_v2_MAU: return "Bundling_kv3_v2_MAU";
  case Bundling_kv3_v2_MAU_X: return "Bundling_kv3_v2_MAU_X";
  case Bundling_kv3_v2_MAU_Y: return "Bundling_kv3_v2_MAU_Y";
  case Bundling_kv3_v2_LSU: return "Bundling_kv3_v2_LSU";
  case Bundling_kv3_v2_LSU_X: return "Bundling_kv3_v2_LSU_X";
  case Bundling_kv3_v2_LSU_Y: return "Bundling_kv3_v2_LSU_Y";
  case Bundling_kv3_v2_TINY: return "Bundling_kv3_v2_TINY";
  case Bundling_kv3_v2_TINY_X: return "Bundling_kv3_v2_TINY_X";
  case Bundling_kv3_v2_TINY_Y: return "Bundling_kv3_v2_TINY_Y";
  case Bundling_kv3_v2_NOP: return "Bundling_kv3_v2_NOP";
  };
  return "unknown bundling";
};

/* Resources list */
#define Resource_kv3_v2_ISSUE 0
#define Resource_kv3_v2_TINY 1
#define Resource_kv3_v2_LITE 2
#define Resource_kv3_v2_FULL 3
#define Resource_kv3_v2_LSU 4
#define Resource_kv3_v2_MAU 5
#define Resource_kv3_v2_BCU 6
#define Resource_kv3_v2_TCA 7
#define Resource_kv3_v2_AUXR 8
#define Resource_kv3_v2_AUXW 9
#define Resource_kv3_v2_CRRP 10
#define Resource_kv3_v2_CRWL 11
#define Resource_kv3_v2_CRWH 12
#define Resource_kv3_v2_NOP 13
#define kvx_kv3_v2_RESOURCE_MAX 14


/* Reservations list */
#define Reservation_kv3_v2_ALL 0
#define Reservation_kv3_v2_ALU_NOP 1
#define Reservation_kv3_v2_ALU_TINY 2
#define Reservation_kv3_v2_ALU_TINY_X 3
#define Reservation_kv3_v2_ALU_TINY_Y 4
#define Reservation_kv3_v2_ALU_TINY_CRRP 5
#define Reservation_kv3_v2_ALU_TINY_CRWL_CRWH 6
#define Reservation_kv3_v2_ALU_TINY_CRWL_CRWH_X 7
#define Reservation_kv3_v2_ALU_TINY_CRWL_CRWH_Y 8
#define Reservation_kv3_v2_ALU_TINY_CRRP_CRWL_CRWH 9
#define Reservation_kv3_v2_ALU_TINY_CRWL 10
#define Reservation_kv3_v2_ALU_TINY_CRWH 11
#define Reservation_kv3_v2_ALU_LITE 12
#define Reservation_kv3_v2_ALU_LITE_X 13
#define Reservation_kv3_v2_ALU_LITE_Y 14
#define Reservation_kv3_v2_ALU_LITE_CRWL 15
#define Reservation_kv3_v2_ALU_LITE_CRWH 16
#define Reservation_kv3_v2_ALU_FULL 17
#define Reservation_kv3_v2_ALU_FULL_X 18
#define Reservation_kv3_v2_ALU_FULL_Y 19
#define Reservation_kv3_v2_BCU 20
#define Reservation_kv3_v2_BCU_CRRP_CRWL_CRWH 21
#define Reservation_kv3_v2_BCU_TINY_AUXW_CRRP 22
#define Reservation_kv3_v2_BCU_TINY_TINY_MAU_XNOP 23
#define Reservation_kv3_v2_TCA 24
#define Reservation_kv3_v2_LSU 25
#define Reservation_kv3_v2_LSU_X 26
#define Reservation_kv3_v2_LSU_Y 27
#define Reservation_kv3_v2_LSU_CRRP 28
#define Reservation_kv3_v2_LSU_CRRP_X 29
#define Reservation_kv3_v2_LSU_CRRP_Y 30
#define Reservation_kv3_v2_LSU_AUXR 31
#define Reservation_kv3_v2_LSU_AUXR_X 32
#define Reservation_kv3_v2_LSU_AUXR_Y 33
#define Reservation_kv3_v2_LSU_AUXW 34
#define Reservation_kv3_v2_LSU_AUXW_X 35
#define Reservation_kv3_v2_LSU_AUXW_Y 36
#define Reservation_kv3_v2_LSU_AUXR_AUXW 37
#define Reservation_kv3_v2_LSU_AUXR_AUXW_X 38
#define Reservation_kv3_v2_LSU_AUXR_AUXW_Y 39
#define Reservation_kv3_v2_MAU 40
#define Reservation_kv3_v2_MAU_X 41
#define Reservation_kv3_v2_MAU_Y 42
#define Reservation_kv3_v2_MAU_AUXR 43
#define Reservation_kv3_v2_MAU_AUXR_X 44
#define Reservation_kv3_v2_MAU_AUXR_Y 45


extern struct kvx_reloc kv3_v2_rel16_reloc;
extern struct kvx_reloc kv3_v2_rel32_reloc;
extern struct kvx_reloc kv3_v2_rel64_reloc;
extern struct kvx_reloc kv3_v2_pcrel_signed16_reloc;
extern struct kvx_reloc kv3_v2_pcrel17_reloc;
extern struct kvx_reloc kv3_v2_pcrel27_reloc;
extern struct kvx_reloc kv3_v2_pcrel32_reloc;
extern struct kvx_reloc kv3_v2_pcrel_signed37_reloc;
extern struct kvx_reloc kv3_v2_pcrel_signed43_reloc;
extern struct kvx_reloc kv3_v2_pcrel_signed64_reloc;
extern struct kvx_reloc kv3_v2_pcrel64_reloc;
extern struct kvx_reloc kv3_v2_signed16_reloc;
extern struct kvx_reloc kv3_v2_signed32_reloc;
extern struct kvx_reloc kv3_v2_signed37_reloc;
extern struct kvx_reloc kv3_v2_gotoff_signed37_reloc;
extern struct kvx_reloc kv3_v2_gotoff_signed43_reloc;
extern struct kvx_reloc kv3_v2_gotoff_32_reloc;
extern struct kvx_reloc kv3_v2_gotoff_64_reloc;
extern struct kvx_reloc kv3_v2_got_32_reloc;
extern struct kvx_reloc kv3_v2_got_signed37_reloc;
extern struct kvx_reloc kv3_v2_got_signed43_reloc;
extern struct kvx_reloc kv3_v2_got_64_reloc;
extern struct kvx_reloc kv3_v2_glob_dat_reloc;
extern struct kvx_reloc kv3_v2_copy_reloc;
extern struct kvx_reloc kv3_v2_jump_slot_reloc;
extern struct kvx_reloc kv3_v2_relative_reloc;
extern struct kvx_reloc kv3_v2_signed43_reloc;
extern struct kvx_reloc kv3_v2_signed64_reloc;
extern struct kvx_reloc kv3_v2_gotaddr_signed37_reloc;
extern struct kvx_reloc kv3_v2_gotaddr_signed43_reloc;
extern struct kvx_reloc kv3_v2_gotaddr_signed64_reloc;
extern struct kvx_reloc kv3_v2_dtpmod64_reloc;
extern struct kvx_reloc kv3_v2_dtpoff64_reloc;
extern struct kvx_reloc kv3_v2_dtpoff_signed37_reloc;
extern struct kvx_reloc kv3_v2_dtpoff_signed43_reloc;
extern struct kvx_reloc kv3_v2_tlsgd_signed37_reloc;
extern struct kvx_reloc kv3_v2_tlsgd_signed43_reloc;
extern struct kvx_reloc kv3_v2_tlsld_signed37_reloc;
extern struct kvx_reloc kv3_v2_tlsld_signed43_reloc;
extern struct kvx_reloc kv3_v2_tpoff64_reloc;
extern struct kvx_reloc kv3_v2_tlsie_signed37_reloc;
extern struct kvx_reloc kv3_v2_tlsie_signed43_reloc;
extern struct kvx_reloc kv3_v2_tlsle_signed37_reloc;
extern struct kvx_reloc kv3_v2_tlsle_signed43_reloc;
extern struct kvx_reloc kv3_v2_rel8_reloc;

#define KVX_REGFILE_FIRST_GPR 0
#define KVX_REGFILE_LAST_GPR 1
#define KVX_REGFILE_DEC_GPR 2
#define KVX_REGFILE_FIRST_PGR 3
#define KVX_REGFILE_LAST_PGR 4
#define KVX_REGFILE_DEC_PGR 5
#define KVX_REGFILE_FIRST_QGR 6
#define KVX_REGFILE_LAST_QGR 7
#define KVX_REGFILE_DEC_QGR 8
#define KVX_REGFILE_FIRST_SFR 9
#define KVX_REGFILE_LAST_SFR 10
#define KVX_REGFILE_DEC_SFR 11
#define KVX_REGFILE_FIRST_X16R 12
#define KVX_REGFILE_LAST_X16R 13
#define KVX_REGFILE_DEC_X16R 14
#define KVX_REGFILE_FIRST_X2R 15
#define KVX_REGFILE_LAST_X2R 16
#define KVX_REGFILE_DEC_X2R 17
#define KVX_REGFILE_FIRST_X32R 18
#define KVX_REGFILE_LAST_X32R 19
#define KVX_REGFILE_DEC_X32R 20
#define KVX_REGFILE_FIRST_X4R 21
#define KVX_REGFILE_LAST_X4R 22
#define KVX_REGFILE_DEC_X4R 23
#define KVX_REGFILE_FIRST_X64R 24
#define KVX_REGFILE_LAST_X64R 25
#define KVX_REGFILE_DEC_X64R 26
#define KVX_REGFILE_FIRST_X8R 27
#define KVX_REGFILE_LAST_X8R 28
#define KVX_REGFILE_DEC_X8R 29
#define KVX_REGFILE_FIRST_XBR 30
#define KVX_REGFILE_LAST_XBR 31
#define KVX_REGFILE_DEC_XBR 32
#define KVX_REGFILE_FIRST_XCR 33
#define KVX_REGFILE_LAST_XCR 34
#define KVX_REGFILE_DEC_XCR 35
#define KVX_REGFILE_FIRST_XMR 36
#define KVX_REGFILE_LAST_XMR 37
#define KVX_REGFILE_DEC_XMR 38
#define KVX_REGFILE_FIRST_XTR 39
#define KVX_REGFILE_LAST_XTR 40
#define KVX_REGFILE_DEC_XTR 41
#define KVX_REGFILE_FIRST_XVR 42
#define KVX_REGFILE_LAST_XVR 43
#define KVX_REGFILE_DEC_XVR 44
#define KVX_REGFILE_REGISTERS 45
#define KVX_REGFILE_DEC_REGISTERS 46


extern int kvx_kv4_v1_regfiles[];
extern const char **kvx_kv4_v1_modifiers[];
extern struct kvx_Register kvx_kv4_v1_registers[];

extern int kvx_kv4_v1_dec_registers[];

enum Method_kvx_kv4_v1_enum {
  Immediate_kv4_v1_brknumber = 1,
  Immediate_kv4_v1_pcrel17 = 2,
  Immediate_kv4_v1_pcrel27 = 3,
  Immediate_kv4_v1_signed10 = 4,
  Immediate_kv4_v1_signed16 = 5,
  Immediate_kv4_v1_signed27 = 6,
  Immediate_kv4_v1_signed37 = 7,
  Immediate_kv4_v1_signed43 = 8,
  Immediate_kv4_v1_signed54 = 9,
  Immediate_kv4_v1_sysnumber = 10,
  Immediate_kv4_v1_unsigned6 = 11,
  Immediate_kv4_v1_wrapped32 = 12,
  Immediate_kv4_v1_wrapped64 = 13,
  Immediate_kv4_v1_wrapped8 = 14,
  Modifier_kv4_v1_accesses = 15,
  Modifier_kv4_v1_boolcas = 16,
  Modifier_kv4_v1_cachelev = 17,
  Modifier_kv4_v1_channel = 18,
  Modifier_kv4_v1_coherency = 19,
  Modifier_kv4_v1_comparison = 20,
  Modifier_kv4_v1_conjugate = 21,
  Modifier_kv4_v1_doscale = 22,
  Modifier_kv4_v1_exunum = 23,
  Modifier_kv4_v1_floatcomp = 24,
  Modifier_kv4_v1_hindex = 25,
  Modifier_kv4_v1_lsomask = 26,
  Modifier_kv4_v1_lsumask = 27,
  Modifier_kv4_v1_lsupack = 28,
  Modifier_kv4_v1_qindex = 29,
  Modifier_kv4_v1_rounding = 30,
  Modifier_kv4_v1_scalarcond = 31,
  Modifier_kv4_v1_shuffleV = 32,
  Modifier_kv4_v1_shuffleX = 33,
  Modifier_kv4_v1_silent = 34,
  Modifier_kv4_v1_simplecond = 35,
  Modifier_kv4_v1_speculate = 36,
  Modifier_kv4_v1_splat32 = 37,
  Modifier_kv4_v1_transpose = 38,
  Modifier_kv4_v1_variant = 39,
  RegClass_kv4_v1_aloneReg = 40,
  RegClass_kv4_v1_blockReg = 41,
  RegClass_kv4_v1_blockRegE = 42,
  RegClass_kv4_v1_blockRegO = 43,
  RegClass_kv4_v1_blockReg_0 = 44,
  RegClass_kv4_v1_blockReg_1 = 45,
  RegClass_kv4_v1_buffer16Reg = 46,
  RegClass_kv4_v1_buffer2Reg = 47,
  RegClass_kv4_v1_buffer32Reg = 48,
  RegClass_kv4_v1_buffer4Reg = 49,
  RegClass_kv4_v1_buffer64Reg = 50,
  RegClass_kv4_v1_buffer8Reg = 51,
  RegClass_kv4_v1_coproReg = 52,
  RegClass_kv4_v1_coproReg0M4 = 53,
  RegClass_kv4_v1_coproReg1M4 = 54,
  RegClass_kv4_v1_coproReg2M4 = 55,
  RegClass_kv4_v1_coproReg3M4 = 56,
  RegClass_kv4_v1_matrixReg = 57,
  RegClass_kv4_v1_matrixReg_0 = 58,
  RegClass_kv4_v1_matrixReg_1 = 59,
  RegClass_kv4_v1_matrixReg_2 = 60,
  RegClass_kv4_v1_matrixReg_3 = 61,
  RegClass_kv4_v1_onlyfxReg = 62,
  RegClass_kv4_v1_onlygetReg = 63,
  RegClass_kv4_v1_onlyraReg = 64,
  RegClass_kv4_v1_onlysetReg = 65,
  RegClass_kv4_v1_onlyswapReg = 66,
  RegClass_kv4_v1_pairedReg = 67,
  RegClass_kv4_v1_pairedReg_0 = 68,
  RegClass_kv4_v1_pairedReg_1 = 69,
  RegClass_kv4_v1_quadReg = 70,
  RegClass_kv4_v1_quadReg_0 = 71,
  RegClass_kv4_v1_quadReg_1 = 72,
  RegClass_kv4_v1_quadReg_2 = 73,
  RegClass_kv4_v1_quadReg_3 = 74,
  RegClass_kv4_v1_singleReg = 75,
  RegClass_kv4_v1_systemReg = 76,
  RegClass_kv4_v1_tileReg = 77,
  RegClass_kv4_v1_tileReg_0 = 78,
  RegClass_kv4_v1_tileReg_1 = 79,
  RegClass_kv4_v1_vectorReg = 80,
  RegClass_kv4_v1_vectorReg_0 = 81,
  RegClass_kv4_v1_vectorReg_1 = 82,
  RegClass_kv4_v1_vectorReg_2 = 83,
  RegClass_kv4_v1_vectorReg_3 = 84,
  Instruction_kv4_v1_abdbo = 85,
  Instruction_kv4_v1_abdd = 86,
  Instruction_kv4_v1_abdhq = 87,
  Instruction_kv4_v1_abdsbo = 88,
  Instruction_kv4_v1_abdsd = 89,
  Instruction_kv4_v1_abdshq = 90,
  Instruction_kv4_v1_abdsw = 91,
  Instruction_kv4_v1_abdswp = 92,
  Instruction_kv4_v1_abdubo = 93,
  Instruction_kv4_v1_abdud = 94,
  Instruction_kv4_v1_abduhq = 95,
  Instruction_kv4_v1_abduw = 96,
  Instruction_kv4_v1_abduwp = 97,
  Instruction_kv4_v1_abdw = 98,
  Instruction_kv4_v1_abdwp = 99,
  Instruction_kv4_v1_absbo = 100,
  Instruction_kv4_v1_absd = 101,
  Instruction_kv4_v1_abshq = 102,
  Instruction_kv4_v1_abssbo = 103,
  Instruction_kv4_v1_abssd = 104,
  Instruction_kv4_v1_absshq = 105,
  Instruction_kv4_v1_abssw = 106,
  Instruction_kv4_v1_absswp = 107,
  Instruction_kv4_v1_absw = 108,
  Instruction_kv4_v1_abswp = 109,
  Instruction_kv4_v1_acswapd = 110,
  Instruction_kv4_v1_acswapq = 111,
  Instruction_kv4_v1_acswapw = 112,
  Instruction_kv4_v1_addbo = 113,
  Instruction_kv4_v1_addcd = 114,
  Instruction_kv4_v1_addcd_i = 115,
  Instruction_kv4_v1_addd = 116,
  Instruction_kv4_v1_addhq = 117,
  Instruction_kv4_v1_addrbod = 118,
  Instruction_kv4_v1_addrhqd = 119,
  Instruction_kv4_v1_addrwpd = 120,
  Instruction_kv4_v1_addsbo = 121,
  Instruction_kv4_v1_addsd = 122,
  Instruction_kv4_v1_addshq = 123,
  Instruction_kv4_v1_addsw = 124,
  Instruction_kv4_v1_addswp = 125,
  Instruction_kv4_v1_addurbod = 126,
  Instruction_kv4_v1_addurhqd = 127,
  Instruction_kv4_v1_addurwpd = 128,
  Instruction_kv4_v1_addusbo = 129,
  Instruction_kv4_v1_addusd = 130,
  Instruction_kv4_v1_addushq = 131,
  Instruction_kv4_v1_addusw = 132,
  Instruction_kv4_v1_adduswp = 133,
  Instruction_kv4_v1_adduwd = 134,
  Instruction_kv4_v1_addw = 135,
  Instruction_kv4_v1_addwd = 136,
  Instruction_kv4_v1_addwp = 137,
  Instruction_kv4_v1_addx16bo = 138,
  Instruction_kv4_v1_addx16d = 139,
  Instruction_kv4_v1_addx16hq = 140,
  Instruction_kv4_v1_addx16uwd = 141,
  Instruction_kv4_v1_addx16w = 142,
  Instruction_kv4_v1_addx16wd = 143,
  Instruction_kv4_v1_addx16wp = 144,
  Instruction_kv4_v1_addx2bo = 145,
  Instruction_kv4_v1_addx2d = 146,
  Instruction_kv4_v1_addx2hq = 147,
  Instruction_kv4_v1_addx2uwd = 148,
  Instruction_kv4_v1_addx2w = 149,
  Instruction_kv4_v1_addx2wd = 150,
  Instruction_kv4_v1_addx2wp = 151,
  Instruction_kv4_v1_addx32d = 152,
  Instruction_kv4_v1_addx32uwd = 153,
  Instruction_kv4_v1_addx32w = 154,
  Instruction_kv4_v1_addx32wd = 155,
  Instruction_kv4_v1_addx4bo = 156,
  Instruction_kv4_v1_addx4d = 157,
  Instruction_kv4_v1_addx4hq = 158,
  Instruction_kv4_v1_addx4uwd = 159,
  Instruction_kv4_v1_addx4w = 160,
  Instruction_kv4_v1_addx4wd = 161,
  Instruction_kv4_v1_addx4wp = 162,
  Instruction_kv4_v1_addx64d = 163,
  Instruction_kv4_v1_addx64uwd = 164,
  Instruction_kv4_v1_addx64w = 165,
  Instruction_kv4_v1_addx64wd = 166,
  Instruction_kv4_v1_addx8bo = 167,
  Instruction_kv4_v1_addx8d = 168,
  Instruction_kv4_v1_addx8hq = 169,
  Instruction_kv4_v1_addx8uwd = 170,
  Instruction_kv4_v1_addx8w = 171,
  Instruction_kv4_v1_addx8wd = 172,
  Instruction_kv4_v1_addx8wp = 173,
  Instruction_kv4_v1_aladdd = 174,
  Instruction_kv4_v1_aladdw = 175,
  Instruction_kv4_v1_alclrd = 176,
  Instruction_kv4_v1_alclrw = 177,
  Instruction_kv4_v1_ald = 178,
  Instruction_kv4_v1_alw = 179,
  Instruction_kv4_v1_andd = 180,
  Instruction_kv4_v1_andnd = 181,
  Instruction_kv4_v1_andnw = 182,
  Instruction_kv4_v1_andrbod = 183,
  Instruction_kv4_v1_andrhqd = 184,
  Instruction_kv4_v1_andrwpd = 185,
  Instruction_kv4_v1_andw = 186,
  Instruction_kv4_v1_asd = 187,
  Instruction_kv4_v1_asw = 188,
  Instruction_kv4_v1_avgbo = 189,
  Instruction_kv4_v1_avghq = 190,
  Instruction_kv4_v1_avgrbo = 191,
  Instruction_kv4_v1_avgrhq = 192,
  Instruction_kv4_v1_avgrubo = 193,
  Instruction_kv4_v1_avgruhq = 194,
  Instruction_kv4_v1_avgruw = 195,
  Instruction_kv4_v1_avgruwp = 196,
  Instruction_kv4_v1_avgrw = 197,
  Instruction_kv4_v1_avgrwp = 198,
  Instruction_kv4_v1_avgubo = 199,
  Instruction_kv4_v1_avguhq = 200,
  Instruction_kv4_v1_avguw = 201,
  Instruction_kv4_v1_avguwp = 202,
  Instruction_kv4_v1_avgw = 203,
  Instruction_kv4_v1_avgwp = 204,
  Instruction_kv4_v1_await = 205,
  Instruction_kv4_v1_barrier = 206,
  Instruction_kv4_v1_break = 207,
  Instruction_kv4_v1_call = 208,
  Instruction_kv4_v1_cb = 209,
  Instruction_kv4_v1_cbsd = 210,
  Instruction_kv4_v1_cbsw = 211,
  Instruction_kv4_v1_cbswp = 212,
  Instruction_kv4_v1_clrf = 213,
  Instruction_kv4_v1_clsd = 214,
  Instruction_kv4_v1_clsw = 215,
  Instruction_kv4_v1_clswp = 216,
  Instruction_kv4_v1_clzd = 217,
  Instruction_kv4_v1_clzw = 218,
  Instruction_kv4_v1_clzwp = 219,
  Instruction_kv4_v1_cmovebo = 220,
  Instruction_kv4_v1_cmoved = 221,
  Instruction_kv4_v1_cmovehq = 222,
  Instruction_kv4_v1_cmovewp = 223,
  Instruction_kv4_v1_cmuldt = 224,
  Instruction_kv4_v1_cmulghxdt = 225,
  Instruction_kv4_v1_cmulglxdt = 226,
  Instruction_kv4_v1_cmulgmxdt = 227,
  Instruction_kv4_v1_cmulxdt = 228,
  Instruction_kv4_v1_compd = 229,
  Instruction_kv4_v1_compnbo = 230,
  Instruction_kv4_v1_compnd = 231,
  Instruction_kv4_v1_compnhq = 232,
  Instruction_kv4_v1_compnw = 233,
  Instruction_kv4_v1_compnwp = 234,
  Instruction_kv4_v1_compuwd = 235,
  Instruction_kv4_v1_compw = 236,
  Instruction_kv4_v1_compwd = 237,
  Instruction_kv4_v1_copyd = 238,
  Instruction_kv4_v1_copyo = 239,
  Instruction_kv4_v1_copyq = 240,
  Instruction_kv4_v1_copyw = 241,
  Instruction_kv4_v1_crcbellw = 242,
  Instruction_kv4_v1_crcbelmw = 243,
  Instruction_kv4_v1_crclellw = 244,
  Instruction_kv4_v1_crclelmw = 245,
  Instruction_kv4_v1_ctzd = 246,
  Instruction_kv4_v1_ctzw = 247,
  Instruction_kv4_v1_ctzwp = 248,
  Instruction_kv4_v1_d1inval = 249,
  Instruction_kv4_v1_dflushl = 250,
  Instruction_kv4_v1_dflushsw = 251,
  Instruction_kv4_v1_dinvall = 252,
  Instruction_kv4_v1_dinvalsw = 253,
  Instruction_kv4_v1_dpurgel = 254,
  Instruction_kv4_v1_dpurgesw = 255,
  Instruction_kv4_v1_dtouchl = 256,
  Instruction_kv4_v1_errop = 257,
  Instruction_kv4_v1_extfs = 258,
  Instruction_kv4_v1_extfz = 259,
  Instruction_kv4_v1_fabsd = 260,
  Instruction_kv4_v1_fabshq = 261,
  Instruction_kv4_v1_fabsw = 262,
  Instruction_kv4_v1_fabswp = 263,
  Instruction_kv4_v1_faddd = 264,
  Instruction_kv4_v1_fadddc = 265,
  Instruction_kv4_v1_fadddc_c = 266,
  Instruction_kv4_v1_fadddp = 267,
  Instruction_kv4_v1_faddho = 268,
  Instruction_kv4_v1_faddhq = 269,
  Instruction_kv4_v1_faddw = 270,
  Instruction_kv4_v1_faddwc = 271,
  Instruction_kv4_v1_faddwc_c = 272,
  Instruction_kv4_v1_faddwcp = 273,
  Instruction_kv4_v1_faddwcp_c = 274,
  Instruction_kv4_v1_faddwp = 275,
  Instruction_kv4_v1_faddwq = 276,
  Instruction_kv4_v1_fcdivd = 277,
  Instruction_kv4_v1_fcdivw = 278,
  Instruction_kv4_v1_fcdivwp = 279,
  Instruction_kv4_v1_fcompd = 280,
  Instruction_kv4_v1_fcompnd = 281,
  Instruction_kv4_v1_fcompnhq = 282,
  Instruction_kv4_v1_fcompnw = 283,
  Instruction_kv4_v1_fcompnwp = 284,
  Instruction_kv4_v1_fcompw = 285,
  Instruction_kv4_v1_fdot2w = 286,
  Instruction_kv4_v1_fdot2wd = 287,
  Instruction_kv4_v1_fdot2wdp = 288,
  Instruction_kv4_v1_fdot2wzp = 289,
  Instruction_kv4_v1_fence = 290,
  Instruction_kv4_v1_ffdmasw = 291,
  Instruction_kv4_v1_ffdmaswp = 292,
  Instruction_kv4_v1_ffdmaswq = 293,
  Instruction_kv4_v1_ffdmaw = 294,
  Instruction_kv4_v1_ffdmawp = 295,
  Instruction_kv4_v1_ffdmawq = 296,
  Instruction_kv4_v1_ffdmdaw = 297,
  Instruction_kv4_v1_ffdmdawp = 298,
  Instruction_kv4_v1_ffdmdawq = 299,
  Instruction_kv4_v1_ffdmdsw = 300,
  Instruction_kv4_v1_ffdmdswp = 301,
  Instruction_kv4_v1_ffdmdswq = 302,
  Instruction_kv4_v1_ffdmsaw = 303,
  Instruction_kv4_v1_ffdmsawp = 304,
  Instruction_kv4_v1_ffdmsawq = 305,
  Instruction_kv4_v1_ffdmsw = 306,
  Instruction_kv4_v1_ffdmswp = 307,
  Instruction_kv4_v1_ffdmswq = 308,
  Instruction_kv4_v1_ffmad = 309,
  Instruction_kv4_v1_ffmaho = 310,
  Instruction_kv4_v1_ffmahq = 311,
  Instruction_kv4_v1_ffmahw = 312,
  Instruction_kv4_v1_ffmahwq = 313,
  Instruction_kv4_v1_ffmaw = 314,
  Instruction_kv4_v1_ffmawc = 315,
  Instruction_kv4_v1_ffmawcp = 316,
  Instruction_kv4_v1_ffmawd = 317,
  Instruction_kv4_v1_ffmawdp = 318,
  Instruction_kv4_v1_ffmawp = 319,
  Instruction_kv4_v1_ffmawq = 320,
  Instruction_kv4_v1_ffmsd = 321,
  Instruction_kv4_v1_ffmsho = 322,
  Instruction_kv4_v1_ffmshq = 323,
  Instruction_kv4_v1_ffmshw = 324,
  Instruction_kv4_v1_ffmshwq = 325,
  Instruction_kv4_v1_ffmsw = 326,
  Instruction_kv4_v1_ffmswc = 327,
  Instruction_kv4_v1_ffmswcp = 328,
  Instruction_kv4_v1_ffmswd = 329,
  Instruction_kv4_v1_ffmswdp = 330,
  Instruction_kv4_v1_ffmswp = 331,
  Instruction_kv4_v1_ffmswq = 332,
  Instruction_kv4_v1_fixedd = 333,
  Instruction_kv4_v1_fixedud = 334,
  Instruction_kv4_v1_fixeduw = 335,
  Instruction_kv4_v1_fixeduwp = 336,
  Instruction_kv4_v1_fixedw = 337,
  Instruction_kv4_v1_fixedwp = 338,
  Instruction_kv4_v1_floatd = 339,
  Instruction_kv4_v1_floatud = 340,
  Instruction_kv4_v1_floatuw = 341,
  Instruction_kv4_v1_floatuwp = 342,
  Instruction_kv4_v1_floatw = 343,
  Instruction_kv4_v1_floatwp = 344,
  Instruction_kv4_v1_fmaxd = 345,
  Instruction_kv4_v1_fmaxhq = 346,
  Instruction_kv4_v1_fmaxw = 347,
  Instruction_kv4_v1_fmaxwp = 348,
  Instruction_kv4_v1_fmind = 349,
  Instruction_kv4_v1_fminhq = 350,
  Instruction_kv4_v1_fminw = 351,
  Instruction_kv4_v1_fminwp = 352,
  Instruction_kv4_v1_fmm212w = 353,
  Instruction_kv4_v1_fmm222w = 354,
  Instruction_kv4_v1_fmma212w = 355,
  Instruction_kv4_v1_fmma222w = 356,
  Instruction_kv4_v1_fmms212w = 357,
  Instruction_kv4_v1_fmms222w = 358,
  Instruction_kv4_v1_fmuld = 359,
  Instruction_kv4_v1_fmulho = 360,
  Instruction_kv4_v1_fmulhq = 361,
  Instruction_kv4_v1_fmulhw = 362,
  Instruction_kv4_v1_fmulhwq = 363,
  Instruction_kv4_v1_fmulw = 364,
  Instruction_kv4_v1_fmulwc = 365,
  Instruction_kv4_v1_fmulwcp = 366,
  Instruction_kv4_v1_fmulwd = 367,
  Instruction_kv4_v1_fmulwdp = 368,
  Instruction_kv4_v1_fmulwp = 369,
  Instruction_kv4_v1_fmulwq = 370,
  Instruction_kv4_v1_fnarrowdw = 371,
  Instruction_kv4_v1_fnarrowdwp = 372,
  Instruction_kv4_v1_fnarrowwh = 373,
  Instruction_kv4_v1_fnarrowwhq = 374,
  Instruction_kv4_v1_fnegd = 375,
  Instruction_kv4_v1_fneghq = 376,
  Instruction_kv4_v1_fnegw = 377,
  Instruction_kv4_v1_fnegwp = 378,
  Instruction_kv4_v1_frecw = 379,
  Instruction_kv4_v1_frsrw = 380,
  Instruction_kv4_v1_fsbfd = 381,
  Instruction_kv4_v1_fsbfdc = 382,
  Instruction_kv4_v1_fsbfdc_c = 383,
  Instruction_kv4_v1_fsbfdp = 384,
  Instruction_kv4_v1_fsbfho = 385,
  Instruction_kv4_v1_fsbfhq = 386,
  Instruction_kv4_v1_fsbfw = 387,
  Instruction_kv4_v1_fsbfwc = 388,
  Instruction_kv4_v1_fsbfwc_c = 389,
  Instruction_kv4_v1_fsbfwcp = 390,
  Instruction_kv4_v1_fsbfwcp_c = 391,
  Instruction_kv4_v1_fsbfwp = 392,
  Instruction_kv4_v1_fsbfwq = 393,
  Instruction_kv4_v1_fsdivd = 394,
  Instruction_kv4_v1_fsdivw = 395,
  Instruction_kv4_v1_fsdivwp = 396,
  Instruction_kv4_v1_fsrecd = 397,
  Instruction_kv4_v1_fsrecw = 398,
  Instruction_kv4_v1_fsrecwp = 399,
  Instruction_kv4_v1_fsrsrd = 400,
  Instruction_kv4_v1_fsrsrw = 401,
  Instruction_kv4_v1_fsrsrwp = 402,
  Instruction_kv4_v1_fwidenlhw = 403,
  Instruction_kv4_v1_fwidenlhwp = 404,
  Instruction_kv4_v1_fwidenlwd = 405,
  Instruction_kv4_v1_fwidenmhw = 406,
  Instruction_kv4_v1_fwidenmhwp = 407,
  Instruction_kv4_v1_fwidenmwd = 408,
  Instruction_kv4_v1_get = 409,
  Instruction_kv4_v1_goto = 410,
  Instruction_kv4_v1_i1inval = 411,
  Instruction_kv4_v1_i1invals = 412,
  Instruction_kv4_v1_icall = 413,
  Instruction_kv4_v1_iget = 414,
  Instruction_kv4_v1_igoto = 415,
  Instruction_kv4_v1_insf = 416,
  Instruction_kv4_v1_iord = 417,
  Instruction_kv4_v1_iornd = 418,
  Instruction_kv4_v1_iornw = 419,
  Instruction_kv4_v1_iorrbod = 420,
  Instruction_kv4_v1_iorrhqd = 421,
  Instruction_kv4_v1_iorrwpd = 422,
  Instruction_kv4_v1_iorw = 423,
  Instruction_kv4_v1_landd = 424,
  Instruction_kv4_v1_landw = 425,
  Instruction_kv4_v1_lbs = 426,
  Instruction_kv4_v1_lbz = 427,
  Instruction_kv4_v1_ld = 428,
  Instruction_kv4_v1_lhs = 429,
  Instruction_kv4_v1_lhz = 430,
  Instruction_kv4_v1_lnandd = 431,
  Instruction_kv4_v1_lnandw = 432,
  Instruction_kv4_v1_lnord = 433,
  Instruction_kv4_v1_lnorw = 434,
  Instruction_kv4_v1_lo = 435,
  Instruction_kv4_v1_loopdo = 436,
  Instruction_kv4_v1_lord = 437,
  Instruction_kv4_v1_lorw = 438,
  Instruction_kv4_v1_lq = 439,
  Instruction_kv4_v1_lws = 440,
  Instruction_kv4_v1_lwz = 441,
  Instruction_kv4_v1_maddd = 442,
  Instruction_kv4_v1_madddt = 443,
  Instruction_kv4_v1_maddhq = 444,
  Instruction_kv4_v1_maddhwq = 445,
  Instruction_kv4_v1_maddmwq = 446,
  Instruction_kv4_v1_maddsudt = 447,
  Instruction_kv4_v1_maddsuhwq = 448,
  Instruction_kv4_v1_maddsumwq = 449,
  Instruction_kv4_v1_maddsuwd = 450,
  Instruction_kv4_v1_maddsuwdp = 451,
  Instruction_kv4_v1_maddudt = 452,
  Instruction_kv4_v1_madduhwq = 453,
  Instruction_kv4_v1_maddumwq = 454,
  Instruction_kv4_v1_madduwd = 455,
  Instruction_kv4_v1_madduwdp = 456,
  Instruction_kv4_v1_madduzdt = 457,
  Instruction_kv4_v1_maddw = 458,
  Instruction_kv4_v1_maddwd = 459,
  Instruction_kv4_v1_maddwdp = 460,
  Instruction_kv4_v1_maddwp = 461,
  Instruction_kv4_v1_maddwq = 462,
  Instruction_kv4_v1_make = 463,
  Instruction_kv4_v1_maxbo = 464,
  Instruction_kv4_v1_maxd = 465,
  Instruction_kv4_v1_maxhq = 466,
  Instruction_kv4_v1_maxrbod = 467,
  Instruction_kv4_v1_maxrhqd = 468,
  Instruction_kv4_v1_maxrwpd = 469,
  Instruction_kv4_v1_maxubo = 470,
  Instruction_kv4_v1_maxud = 471,
  Instruction_kv4_v1_maxuhq = 472,
  Instruction_kv4_v1_maxurbod = 473,
  Instruction_kv4_v1_maxurhqd = 474,
  Instruction_kv4_v1_maxurwpd = 475,
  Instruction_kv4_v1_maxuw = 476,
  Instruction_kv4_v1_maxuwp = 477,
  Instruction_kv4_v1_maxw = 478,
  Instruction_kv4_v1_maxwp = 479,
  Instruction_kv4_v1_minbo = 480,
  Instruction_kv4_v1_mind = 481,
  Instruction_kv4_v1_minhq = 482,
  Instruction_kv4_v1_minrbod = 483,
  Instruction_kv4_v1_minrhqd = 484,
  Instruction_kv4_v1_minrwpd = 485,
  Instruction_kv4_v1_minubo = 486,
  Instruction_kv4_v1_minud = 487,
  Instruction_kv4_v1_minuhq = 488,
  Instruction_kv4_v1_minurbod = 489,
  Instruction_kv4_v1_minurhqd = 490,
  Instruction_kv4_v1_minurwpd = 491,
  Instruction_kv4_v1_minuw = 492,
  Instruction_kv4_v1_minuwp = 493,
  Instruction_kv4_v1_minw = 494,
  Instruction_kv4_v1_minwp = 495,
  Instruction_kv4_v1_mm212w = 496,
  Instruction_kv4_v1_mma212w = 497,
  Instruction_kv4_v1_mms212w = 498,
  Instruction_kv4_v1_msbfd = 499,
  Instruction_kv4_v1_msbfdt = 500,
  Instruction_kv4_v1_msbfhq = 501,
  Instruction_kv4_v1_msbfhwq = 502,
  Instruction_kv4_v1_msbfmwq = 503,
  Instruction_kv4_v1_msbfsudt = 504,
  Instruction_kv4_v1_msbfsuhwq = 505,
  Instruction_kv4_v1_msbfsumwq = 506,
  Instruction_kv4_v1_msbfsuwd = 507,
  Instruction_kv4_v1_msbfsuwdp = 508,
  Instruction_kv4_v1_msbfudt = 509,
  Instruction_kv4_v1_msbfuhwq = 510,
  Instruction_kv4_v1_msbfumwq = 511,
  Instruction_kv4_v1_msbfuwd = 512,
  Instruction_kv4_v1_msbfuwdp = 513,
  Instruction_kv4_v1_msbfuzdt = 514,
  Instruction_kv4_v1_msbfw = 515,
  Instruction_kv4_v1_msbfwd = 516,
  Instruction_kv4_v1_msbfwdp = 517,
  Instruction_kv4_v1_msbfwp = 518,
  Instruction_kv4_v1_msbfwq = 519,
  Instruction_kv4_v1_muld = 520,
  Instruction_kv4_v1_muldt = 521,
  Instruction_kv4_v1_mulhq = 522,
  Instruction_kv4_v1_mulhwq = 523,
  Instruction_kv4_v1_mulmwq = 524,
  Instruction_kv4_v1_mulsudt = 525,
  Instruction_kv4_v1_mulsuhwq = 526,
  Instruction_kv4_v1_mulsumwq = 527,
  Instruction_kv4_v1_mulsuwd = 528,
  Instruction_kv4_v1_mulsuwdp = 529,
  Instruction_kv4_v1_muludt = 530,
  Instruction_kv4_v1_muluhwq = 531,
  Instruction_kv4_v1_mulumwq = 532,
  Instruction_kv4_v1_muluwd = 533,
  Instruction_kv4_v1_muluwdp = 534,
  Instruction_kv4_v1_mulw = 535,
  Instruction_kv4_v1_mulwd = 536,
  Instruction_kv4_v1_mulwdp = 537,
  Instruction_kv4_v1_mulwp = 538,
  Instruction_kv4_v1_mulwq = 539,
  Instruction_kv4_v1_nandd = 540,
  Instruction_kv4_v1_nandw = 541,
  Instruction_kv4_v1_negbo = 542,
  Instruction_kv4_v1_negd = 543,
  Instruction_kv4_v1_neghq = 544,
  Instruction_kv4_v1_negsbo = 545,
  Instruction_kv4_v1_negsd = 546,
  Instruction_kv4_v1_negshq = 547,
  Instruction_kv4_v1_negsw = 548,
  Instruction_kv4_v1_negswp = 549,
  Instruction_kv4_v1_negw = 550,
  Instruction_kv4_v1_negwp = 551,
  Instruction_kv4_v1_niord = 552,
  Instruction_kv4_v1_niorw = 553,
  Instruction_kv4_v1_nop = 554,
  Instruction_kv4_v1_nord = 555,
  Instruction_kv4_v1_norw = 556,
  Instruction_kv4_v1_notd = 557,
  Instruction_kv4_v1_notw = 558,
  Instruction_kv4_v1_nxord = 559,
  Instruction_kv4_v1_nxorw = 560,
  Instruction_kv4_v1_ord = 561,
  Instruction_kv4_v1_ornd = 562,
  Instruction_kv4_v1_ornw = 563,
  Instruction_kv4_v1_orrbod = 564,
  Instruction_kv4_v1_orrhqd = 565,
  Instruction_kv4_v1_orrwpd = 566,
  Instruction_kv4_v1_orw = 567,
  Instruction_kv4_v1_pcrel = 568,
  Instruction_kv4_v1_ret = 569,
  Instruction_kv4_v1_rfe = 570,
  Instruction_kv4_v1_rolw = 571,
  Instruction_kv4_v1_rolwps = 572,
  Instruction_kv4_v1_rorw = 573,
  Instruction_kv4_v1_rorwps = 574,
  Instruction_kv4_v1_rswap = 575,
  Instruction_kv4_v1_sb = 576,
  Instruction_kv4_v1_sbfbo = 577,
  Instruction_kv4_v1_sbfcd = 578,
  Instruction_kv4_v1_sbfcd_i = 579,
  Instruction_kv4_v1_sbfd = 580,
  Instruction_kv4_v1_sbfhq = 581,
  Instruction_kv4_v1_sbfsbo = 582,
  Instruction_kv4_v1_sbfsd = 583,
  Instruction_kv4_v1_sbfshq = 584,
  Instruction_kv4_v1_sbfsw = 585,
  Instruction_kv4_v1_sbfswp = 586,
  Instruction_kv4_v1_sbfusbo = 587,
  Instruction_kv4_v1_sbfusd = 588,
  Instruction_kv4_v1_sbfushq = 589,
  Instruction_kv4_v1_sbfusw = 590,
  Instruction_kv4_v1_sbfuswp = 591,
  Instruction_kv4_v1_sbfuwd = 592,
  Instruction_kv4_v1_sbfw = 593,
  Instruction_kv4_v1_sbfwd = 594,
  Instruction_kv4_v1_sbfwp = 595,
  Instruction_kv4_v1_sbfx16bo = 596,
  Instruction_kv4_v1_sbfx16d = 597,
  Instruction_kv4_v1_sbfx16hq = 598,
  Instruction_kv4_v1_sbfx16uwd = 599,
  Instruction_kv4_v1_sbfx16w = 600,
  Instruction_kv4_v1_sbfx16wd = 601,
  Instruction_kv4_v1_sbfx16wp = 602,
  Instruction_kv4_v1_sbfx2bo = 603,
  Instruction_kv4_v1_sbfx2d = 604,
  Instruction_kv4_v1_sbfx2hq = 605,
  Instruction_kv4_v1_sbfx2uwd = 606,
  Instruction_kv4_v1_sbfx2w = 607,
  Instruction_kv4_v1_sbfx2wd = 608,
  Instruction_kv4_v1_sbfx2wp = 609,
  Instruction_kv4_v1_sbfx32d = 610,
  Instruction_kv4_v1_sbfx32uwd = 611,
  Instruction_kv4_v1_sbfx32w = 612,
  Instruction_kv4_v1_sbfx32wd = 613,
  Instruction_kv4_v1_sbfx4bo = 614,
  Instruction_kv4_v1_sbfx4d = 615,
  Instruction_kv4_v1_sbfx4hq = 616,
  Instruction_kv4_v1_sbfx4uwd = 617,
  Instruction_kv4_v1_sbfx4w = 618,
  Instruction_kv4_v1_sbfx4wd = 619,
  Instruction_kv4_v1_sbfx4wp = 620,
  Instruction_kv4_v1_sbfx64d = 621,
  Instruction_kv4_v1_sbfx64uwd = 622,
  Instruction_kv4_v1_sbfx64w = 623,
  Instruction_kv4_v1_sbfx64wd = 624,
  Instruction_kv4_v1_sbfx8bo = 625,
  Instruction_kv4_v1_sbfx8d = 626,
  Instruction_kv4_v1_sbfx8hq = 627,
  Instruction_kv4_v1_sbfx8uwd = 628,
  Instruction_kv4_v1_sbfx8w = 629,
  Instruction_kv4_v1_sbfx8wd = 630,
  Instruction_kv4_v1_sbfx8wp = 631,
  Instruction_kv4_v1_sbmm8 = 632,
  Instruction_kv4_v1_sbmmt8 = 633,
  Instruction_kv4_v1_scall = 634,
  Instruction_kv4_v1_sd = 635,
  Instruction_kv4_v1_set = 636,
  Instruction_kv4_v1_sh = 637,
  Instruction_kv4_v1_sleep = 638,
  Instruction_kv4_v1_sllbos = 639,
  Instruction_kv4_v1_slld = 640,
  Instruction_kv4_v1_sllhqs = 641,
  Instruction_kv4_v1_sllw = 642,
  Instruction_kv4_v1_sllwps = 643,
  Instruction_kv4_v1_slsbos = 644,
  Instruction_kv4_v1_slsd = 645,
  Instruction_kv4_v1_slshqs = 646,
  Instruction_kv4_v1_slsw = 647,
  Instruction_kv4_v1_slswps = 648,
  Instruction_kv4_v1_slusbos = 649,
  Instruction_kv4_v1_slusd = 650,
  Instruction_kv4_v1_slushqs = 651,
  Instruction_kv4_v1_slusw = 652,
  Instruction_kv4_v1_sluswps = 653,
  Instruction_kv4_v1_so = 654,
  Instruction_kv4_v1_sq = 655,
  Instruction_kv4_v1_srabos = 656,
  Instruction_kv4_v1_srad = 657,
  Instruction_kv4_v1_srahqs = 658,
  Instruction_kv4_v1_sraw = 659,
  Instruction_kv4_v1_srawps = 660,
  Instruction_kv4_v1_srlbos = 661,
  Instruction_kv4_v1_srld = 662,
  Instruction_kv4_v1_srlhqs = 663,
  Instruction_kv4_v1_srlw = 664,
  Instruction_kv4_v1_srlwps = 665,
  Instruction_kv4_v1_srsbos = 666,
  Instruction_kv4_v1_srsd = 667,
  Instruction_kv4_v1_srshqs = 668,
  Instruction_kv4_v1_srsw = 669,
  Instruction_kv4_v1_srswps = 670,
  Instruction_kv4_v1_stop = 671,
  Instruction_kv4_v1_stsud = 672,
  Instruction_kv4_v1_stsuhq = 673,
  Instruction_kv4_v1_stsuw = 674,
  Instruction_kv4_v1_stsuwp = 675,
  Instruction_kv4_v1_sw = 676,
  Instruction_kv4_v1_sxbd = 677,
  Instruction_kv4_v1_sxhd = 678,
  Instruction_kv4_v1_sxlbhq = 679,
  Instruction_kv4_v1_sxlhwp = 680,
  Instruction_kv4_v1_sxmbhq = 681,
  Instruction_kv4_v1_sxmhwp = 682,
  Instruction_kv4_v1_sxwd = 683,
  Instruction_kv4_v1_syncgroup = 684,
  Instruction_kv4_v1_tlbdinval = 685,
  Instruction_kv4_v1_tlbiinval = 686,
  Instruction_kv4_v1_tlbprobe = 687,
  Instruction_kv4_v1_tlbread = 688,
  Instruction_kv4_v1_tlbwrite = 689,
  Instruction_kv4_v1_waitit = 690,
  Instruction_kv4_v1_wfxl = 691,
  Instruction_kv4_v1_wfxm = 692,
  Instruction_kv4_v1_xaccesso = 693,
  Instruction_kv4_v1_xaligno = 694,
  Instruction_kv4_v1_xandno = 695,
  Instruction_kv4_v1_xando = 696,
  Instruction_kv4_v1_xclampwo = 697,
  Instruction_kv4_v1_xcopyo = 698,
  Instruction_kv4_v1_xcopyv = 699,
  Instruction_kv4_v1_xcopyx = 700,
  Instruction_kv4_v1_xffma44hw = 701,
  Instruction_kv4_v1_xfmaxhx = 702,
  Instruction_kv4_v1_xfminhx = 703,
  Instruction_kv4_v1_xfmma484hw = 704,
  Instruction_kv4_v1_xfnarrow44wh = 705,
  Instruction_kv4_v1_xfscalewo = 706,
  Instruction_kv4_v1_xioro = 707,
  Instruction_kv4_v1_xlo = 708,
  Instruction_kv4_v1_xmadd44bw0 = 709,
  Instruction_kv4_v1_xmadd44bw1 = 710,
  Instruction_kv4_v1_xmaddifwo = 711,
  Instruction_kv4_v1_xmaddsu44bw0 = 712,
  Instruction_kv4_v1_xmaddsu44bw1 = 713,
  Instruction_kv4_v1_xmaddu44bw0 = 714,
  Instruction_kv4_v1_xmaddu44bw1 = 715,
  Instruction_kv4_v1_xmma4164bw = 716,
  Instruction_kv4_v1_xmma484bw = 717,
  Instruction_kv4_v1_xmmasu4164bw = 718,
  Instruction_kv4_v1_xmmasu484bw = 719,
  Instruction_kv4_v1_xmmau4164bw = 720,
  Instruction_kv4_v1_xmmau484bw = 721,
  Instruction_kv4_v1_xmmaus4164bw = 722,
  Instruction_kv4_v1_xmmaus484bw = 723,
  Instruction_kv4_v1_xmovefd = 724,
  Instruction_kv4_v1_xmovefo = 725,
  Instruction_kv4_v1_xmovefq = 726,
  Instruction_kv4_v1_xmovetd = 727,
  Instruction_kv4_v1_xmovetq = 728,
  Instruction_kv4_v1_xmsbfifwo = 729,
  Instruction_kv4_v1_xmt44d = 730,
  Instruction_kv4_v1_xnando = 731,
  Instruction_kv4_v1_xnoro = 732,
  Instruction_kv4_v1_xnxoro = 733,
  Instruction_kv4_v1_xord = 734,
  Instruction_kv4_v1_xorno = 735,
  Instruction_kv4_v1_xoro = 736,
  Instruction_kv4_v1_xorrbod = 737,
  Instruction_kv4_v1_xorrhqd = 738,
  Instruction_kv4_v1_xorrwpd = 739,
  Instruction_kv4_v1_xorw = 740,
  Instruction_kv4_v1_xrecvo = 741,
  Instruction_kv4_v1_xsbmm8dq = 742,
  Instruction_kv4_v1_xsbmmt8dq = 743,
  Instruction_kv4_v1_xsendo = 744,
  Instruction_kv4_v1_xsendrecvo = 745,
  Instruction_kv4_v1_xso = 746,
  Instruction_kv4_v1_xsplatdo = 747,
  Instruction_kv4_v1_xsplatov = 748,
  Instruction_kv4_v1_xsplatox = 749,
  Instruction_kv4_v1_xsx48bw = 750,
  Instruction_kv4_v1_xtrunc48wb = 751,
  Instruction_kv4_v1_xxoro = 752,
  Instruction_kv4_v1_xzx48bw = 753,
  Instruction_kv4_v1_zxbd = 754,
  Instruction_kv4_v1_zxhd = 755,
  Instruction_kv4_v1_zxlbhq = 756,
  Instruction_kv4_v1_zxlhwp = 757,
  Instruction_kv4_v1_zxmbhq = 758,
  Instruction_kv4_v1_zxmhwp = 759,
  Instruction_kv4_v1_zxwd = 760,
  Separator_kv4_v1_comma = 761,
  Separator_kv4_v1_equal = 762,
  Separator_kv4_v1_qmark = 763,
  Separator_kv4_v1_rsbracket = 764,
  Separator_kv4_v1_lsbracket = 765
};

enum Modifier_kv4_v1_exunum_enum {
  Modifier_kv4_v1_exunum_ALU0=0,
  Modifier_kv4_v1_exunum_ALU1=1,
  Modifier_kv4_v1_exunum_MAU=2,
  Modifier_kv4_v1_exunum_LSU=3,
};

extern const char *mod_kv4_v1_exunum[];
extern const char *mod_kv4_v1_scalarcond[];
extern const char *mod_kv4_v1_lsomask[];
extern const char *mod_kv4_v1_lsumask[];
extern const char *mod_kv4_v1_lsupack[];
extern const char *mod_kv4_v1_simplecond[];
extern const char *mod_kv4_v1_comparison[];
extern const char *mod_kv4_v1_floatcomp[];
extern const char *mod_kv4_v1_rounding[];
extern const char *mod_kv4_v1_silent[];
extern const char *mod_kv4_v1_variant[];
extern const char *mod_kv4_v1_speculate[];
extern const char *mod_kv4_v1_doscale[];
extern const char *mod_kv4_v1_qindex[];
extern const char *mod_kv4_v1_hindex[];
extern const char *mod_kv4_v1_cachelev[];
extern const char *mod_kv4_v1_coherency[];
extern const char *mod_kv4_v1_boolcas[];
extern const char *mod_kv4_v1_accesses[];
extern const char *mod_kv4_v1_channel[];
extern const char *mod_kv4_v1_conjugate[];
extern const char *mod_kv4_v1_transpose[];
extern const char *mod_kv4_v1_shuffleV[];
extern const char *mod_kv4_v1_shuffleX[];
extern const char *mod_kv4_v1_splat32[];
typedef enum {
  Bundling_kv4_v1_ALL,
  Bundling_kv4_v1_BCU,
  Bundling_kv4_v1_TCA,
  Bundling_kv4_v1_FULL,
  Bundling_kv4_v1_FULL_X,
  Bundling_kv4_v1_FULL_Y,
  Bundling_kv4_v1_LITE,
  Bundling_kv4_v1_LITE_X,
  Bundling_kv4_v1_LITE_Y,
  Bundling_kv4_v1_MAU,
  Bundling_kv4_v1_MAU_X,
  Bundling_kv4_v1_MAU_Y,
  Bundling_kv4_v1_LSU,
  Bundling_kv4_v1_LSU_X,
  Bundling_kv4_v1_LSU_Y,
  Bundling_kv4_v1_TINY,
  Bundling_kv4_v1_TINY_X,
  Bundling_kv4_v1_TINY_Y,
  Bundling_kv4_v1_NOP,
} Bundling_kv4_v1;


static const char *bundling_kv4_v1_names(Bundling_kv4_v1 bundling) __attribute__((unused));
static const char *bundling_kv4_v1_names(Bundling_kv4_v1 bundling) {
  switch(bundling) {
  case Bundling_kv4_v1_ALL: return "Bundling_kv4_v1_ALL";
  case Bundling_kv4_v1_BCU: return "Bundling_kv4_v1_BCU";
  case Bundling_kv4_v1_TCA: return "Bundling_kv4_v1_TCA";
  case Bundling_kv4_v1_FULL: return "Bundling_kv4_v1_FULL";
  case Bundling_kv4_v1_FULL_X: return "Bundling_kv4_v1_FULL_X";
  case Bundling_kv4_v1_FULL_Y: return "Bundling_kv4_v1_FULL_Y";
  case Bundling_kv4_v1_LITE: return "Bundling_kv4_v1_LITE";
  case Bundling_kv4_v1_LITE_X: return "Bundling_kv4_v1_LITE_X";
  case Bundling_kv4_v1_LITE_Y: return "Bundling_kv4_v1_LITE_Y";
  case Bundling_kv4_v1_MAU: return "Bundling_kv4_v1_MAU";
  case Bundling_kv4_v1_MAU_X: return "Bundling_kv4_v1_MAU_X";
  case Bundling_kv4_v1_MAU_Y: return "Bundling_kv4_v1_MAU_Y";
  case Bundling_kv4_v1_LSU: return "Bundling_kv4_v1_LSU";
  case Bundling_kv4_v1_LSU_X: return "Bundling_kv4_v1_LSU_X";
  case Bundling_kv4_v1_LSU_Y: return "Bundling_kv4_v1_LSU_Y";
  case Bundling_kv4_v1_TINY: return "Bundling_kv4_v1_TINY";
  case Bundling_kv4_v1_TINY_X: return "Bundling_kv4_v1_TINY_X";
  case Bundling_kv4_v1_TINY_Y: return "Bundling_kv4_v1_TINY_Y";
  case Bundling_kv4_v1_NOP: return "Bundling_kv4_v1_NOP";
  };
  return "unknown bundling";
};

/* Resources list */
#define Resource_kv4_v1_ISSUE 0
#define Resource_kv4_v1_TINY 1
#define Resource_kv4_v1_LITE 2
#define Resource_kv4_v1_FULL 3
#define Resource_kv4_v1_LSU 4
#define Resource_kv4_v1_MAU 5
#define Resource_kv4_v1_BCU 6
#define Resource_kv4_v1_TCA 7
#define Resource_kv4_v1_AUXR 8
#define Resource_kv4_v1_AUXW 9
#define Resource_kv4_v1_CRRP 10
#define Resource_kv4_v1_CRWL 11
#define Resource_kv4_v1_CRWH 12
#define Resource_kv4_v1_NOP 13
#define kvx_kv4_v1_RESOURCE_MAX 14


/* Reservations list */
#define Reservation_kv4_v1_ALL 0
#define Reservation_kv4_v1_ALU_NOP 1
#define Reservation_kv4_v1_ALU_TINY 2
#define Reservation_kv4_v1_ALU_TINY_X 3
#define Reservation_kv4_v1_ALU_TINY_Y 4
#define Reservation_kv4_v1_ALU_TINY_CRRP 5
#define Reservation_kv4_v1_ALU_TINY_CRWL_CRWH 6
#define Reservation_kv4_v1_ALU_TINY_CRWL_CRWH_X 7
#define Reservation_kv4_v1_ALU_TINY_CRWL_CRWH_Y 8
#define Reservation_kv4_v1_ALU_TINY_CRRP_CRWL_CRWH 9
#define Reservation_kv4_v1_ALU_TINY_CRWL 10
#define Reservation_kv4_v1_ALU_TINY_CRWH 11
#define Reservation_kv4_v1_ALU_LITE 12
#define Reservation_kv4_v1_ALU_LITE_X 13
#define Reservation_kv4_v1_ALU_LITE_Y 14
#define Reservation_kv4_v1_ALU_LITE_CRWL 15
#define Reservation_kv4_v1_ALU_LITE_CRWH 16
#define Reservation_kv4_v1_ALU_FULL 17
#define Reservation_kv4_v1_ALU_FULL_X 18
#define Reservation_kv4_v1_ALU_FULL_Y 19
#define Reservation_kv4_v1_BCU 20
#define Reservation_kv4_v1_BCU_CRRP_CRWL_CRWH 21
#define Reservation_kv4_v1_BCU_TINY_AUXW_CRRP 22
#define Reservation_kv4_v1_BCU_TINY_TINY_MAU_XNOP 23
#define Reservation_kv4_v1_TCA 24
#define Reservation_kv4_v1_LSU 25
#define Reservation_kv4_v1_LSU_X 26
#define Reservation_kv4_v1_LSU_Y 27
#define Reservation_kv4_v1_LSU_CRRP 28
#define Reservation_kv4_v1_LSU_CRRP_X 29
#define Reservation_kv4_v1_LSU_CRRP_Y 30
#define Reservation_kv4_v1_LSU_AUXR 31
#define Reservation_kv4_v1_LSU_AUXR_X 32
#define Reservation_kv4_v1_LSU_AUXR_Y 33
#define Reservation_kv4_v1_LSU_AUXW 34
#define Reservation_kv4_v1_LSU_AUXW_X 35
#define Reservation_kv4_v1_LSU_AUXW_Y 36
#define Reservation_kv4_v1_LSU_AUXR_AUXW 37
#define Reservation_kv4_v1_LSU_AUXR_AUXW_X 38
#define Reservation_kv4_v1_LSU_AUXR_AUXW_Y 39
#define Reservation_kv4_v1_MAU 40
#define Reservation_kv4_v1_MAU_X 41
#define Reservation_kv4_v1_MAU_Y 42
#define Reservation_kv4_v1_MAU_AUXR 43
#define Reservation_kv4_v1_MAU_AUXR_X 44
#define Reservation_kv4_v1_MAU_AUXR_Y 45


extern struct kvx_reloc kv4_v1_rel16_reloc;
extern struct kvx_reloc kv4_v1_rel32_reloc;
extern struct kvx_reloc kv4_v1_rel64_reloc;
extern struct kvx_reloc kv4_v1_pcrel_signed16_reloc;
extern struct kvx_reloc kv4_v1_pcrel17_reloc;
extern struct kvx_reloc kv4_v1_pcrel27_reloc;
extern struct kvx_reloc kv4_v1_pcrel32_reloc;
extern struct kvx_reloc kv4_v1_pcrel_signed37_reloc;
extern struct kvx_reloc kv4_v1_pcrel_signed43_reloc;
extern struct kvx_reloc kv4_v1_pcrel_signed64_reloc;
extern struct kvx_reloc kv4_v1_pcrel64_reloc;
extern struct kvx_reloc kv4_v1_signed16_reloc;
extern struct kvx_reloc kv4_v1_signed32_reloc;
extern struct kvx_reloc kv4_v1_signed37_reloc;
extern struct kvx_reloc kv4_v1_gotoff_signed37_reloc;
extern struct kvx_reloc kv4_v1_gotoff_signed43_reloc;
extern struct kvx_reloc kv4_v1_gotoff_32_reloc;
extern struct kvx_reloc kv4_v1_gotoff_64_reloc;
extern struct kvx_reloc kv4_v1_got_32_reloc;
extern struct kvx_reloc kv4_v1_got_signed37_reloc;
extern struct kvx_reloc kv4_v1_got_signed43_reloc;
extern struct kvx_reloc kv4_v1_got_64_reloc;
extern struct kvx_reloc kv4_v1_glob_dat_reloc;
extern struct kvx_reloc kv4_v1_copy_reloc;
extern struct kvx_reloc kv4_v1_jump_slot_reloc;
extern struct kvx_reloc kv4_v1_relative_reloc;
extern struct kvx_reloc kv4_v1_signed43_reloc;
extern struct kvx_reloc kv4_v1_signed64_reloc;
extern struct kvx_reloc kv4_v1_gotaddr_signed37_reloc;
extern struct kvx_reloc kv4_v1_gotaddr_signed43_reloc;
extern struct kvx_reloc kv4_v1_gotaddr_signed64_reloc;
extern struct kvx_reloc kv4_v1_dtpmod64_reloc;
extern struct kvx_reloc kv4_v1_dtpoff64_reloc;
extern struct kvx_reloc kv4_v1_dtpoff_signed37_reloc;
extern struct kvx_reloc kv4_v1_dtpoff_signed43_reloc;
extern struct kvx_reloc kv4_v1_tlsgd_signed37_reloc;
extern struct kvx_reloc kv4_v1_tlsgd_signed43_reloc;
extern struct kvx_reloc kv4_v1_tlsld_signed37_reloc;
extern struct kvx_reloc kv4_v1_tlsld_signed43_reloc;
extern struct kvx_reloc kv4_v1_tpoff64_reloc;
extern struct kvx_reloc kv4_v1_tlsie_signed37_reloc;
extern struct kvx_reloc kv4_v1_tlsie_signed43_reloc;
extern struct kvx_reloc kv4_v1_tlsle_signed37_reloc;
extern struct kvx_reloc kv4_v1_tlsle_signed43_reloc;
extern struct kvx_reloc kv4_v1_rel8_reloc;


#endif /* OPCODE_KVX_H */
