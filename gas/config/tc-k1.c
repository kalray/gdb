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
 * THIS FILE HAS BEEN MODIFIED OR ADDED BY STMicroelectronics, Inc. 1999-2003
 */


/* Author Geoffrey Brown                */
/* Modified by Giuseppe Desoli Jul 1999 */

/**
 *** static char sccs_id[] = "@(#)tc-k1.c	1.20 07/18/00 13:19:21";
 **/

#include "as.h"
#include "obstack.h"
#include "subsegs.h"
#include "tc-k1.h"
#include "opcode/k1.h"
#include "libiberty.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#ifdef OBJ_ELF
#include "elf/k1.h"
#include "dwarf2dbg.h"
#include "dw2gencfi.h"
#endif

#define D(args...) do { if(debug) fprintf(args); }while(0)

static void supported_cores(char buf[], size_t buflen);

#define O_pseudo_fixup (O_max + 1)

#define NELEMS(a)	((int) (sizeof (a)/sizeof ((a)[0])))

#define STREQ(x,y) !strcmp(((x) ? (x) : ""), ((y) ? (y) : ""))
#define STRNEQ(x,y,n) !strncmp(((x) ? (x) : ""), ((y) ? (y) : ""),(n))

/* Global flag to activate debuging */
static int debug = 0;

int emit_all_relocs = 0;
/*TB begin*/
int size_type_function = 1;
/*TB end */
static int nop_insertion_allowed = 1;
/* Resource usage checking is disabled by default, because it
 * can produce false positives. */
static int check_resource_usage = 1;
/* Generate illegal code : only use for debugging !*/
static int generate_illegal_code = 0;
/* Dump asm tables : for debugging */
static int dump_table = 0;
/* Core string passed as argument with -mcore option */
char *mcore= NULL;

/* This string should contains position in string where error occured. */
char *error_str=NULL;

/* Default values used if no assume directive is given */
static const K1_Core_Info *k1_core_info = NULL;
static int subcore_id = 0;

/* Default k1_registers array. */
static const k1_Register *k1_registers = NULL;

/* Default k1_regfiles array. */
static const int *k1_regfiles = NULL;

int k1_cut = ELF_K1_CUT_0;
int k1_abi = ELF_K1_ABI_NO;
int k1_osabi = ELFOSABI_NONE;
int k1_mode = ELF_K1_MODE_USER;
int k1_core = -1;
int k1_core_set = 0;
int k1_cut_set = 0;
int k1_abi_set = 0;
int k1_osabi_set = 0;
int k1_mode_set = 0;

/* ST2xx NOP instruction encodings for alignment in code sections */
static char const k1_noop[4] = { 0x00, 0x00, 0x00, 0x80 };
static char const k1_noop_stop_bit[4] = { 0x00, 0x00, 0x00, 0x00 };

#ifdef OBJ_FDPIC_ELF
# define DEFAULT_FDPIC ELF_K1_FDPIC
#else
# define DEFAULT_FDPIC 0
#endif

static flagword k1_pic_flags = DEFAULT_FDPIC;
// static const char *k1_pic_flag = EF_K1_FDPIC ? "-mfdpic" : (const char *)0;


/***********************************************/
/*    Generic Globals for GAS                  */
/***********************************************/

const char comment_chars[] = "#";
const char line_comment_chars[] = "#";
const char line_separator_chars[] = ";";
const char EXP_CHARS[] = "eE";
const char FLT_CHARS[] = "dD";
const int md_short_jump_size = 0;
const int md_long_jump_size = 0;

/***********************************************/
/*           Local Types                       */
/***********************************************/

/* a fix up record                       */

struct k1_fixup_s
 {
    expressionS exp;		/* the expression used            */
    int where;			/* where (byte) in frag this goes */
    bfd_reloc_code_real_type reloc;
};

typedef struct k1_fixup_s k1_fixup_t;

typedef enum {
  K1_UNDEF = 0,
  K1_BCU = 1,
  K1_ALU0= 2,
  K1_ALU1= 4,
  K1_MAU = 8,
  K1_LSU = 16,
} k1_slots_t;

typedef enum {
  K1_OTHER,  /* Not defined type */
  K1_LITE,   /* LITE */
  K1_TINY,   /* TINY */
  K1_LMD,    /* LITE MONO DOUBLE */
  K1_TMD,    /* TINY MONO DOUBLE */
} k1_insn_type_t;

/* a single assembled instruction record */
/* may include immediate extension word  */

struct k1insn_s {
  int written;		                /* written out ?                           */
  const k1opc_t *opdef;	                /* Opcode table entry for this insn        */
  unsigned len;		                /* length of instruction in words (1 or 2) */
  int immx;                             /* insn is extended */
  int immx64;                           /* only used for 64 immx */
  unsigned int insn[K1MAXCODEWORDS];	/* instruction data                        */
  int nfixups;		                /* the number of fixups 0, 1               */
  k1_fixup_t fixup[1];	                /* the actual fixups                       */
  Bundling bundling;                    /* the bundling type                       */
  k1_slots_t slots;                     /* Used slots (one slot per bit).          */
  k1_insn_type_t type;                  /* Type of instruction.                    */
};

typedef struct k1insn_s k1insn_t;

typedef void (*reorder_bundle_t)(k1insn_t *bundle_insn[], int *bundle_insncnt_p);
static reorder_bundle_t reorder_bundle = NULL;

typedef enum match_operands_code_ {MATCH_NOT_FOUND=0, MATCH_FOUND=1} match_operands_code;

/* We leave an extra slot in K1MAXINSN in case we need to emit a nop
 * bundle to gain even alignment before emitting a cluster.
 */
#define K1MAXINSN 5
#define K1MAXBUNDLEWORDS 8

/* Constant to tell if branches are required to be
 * emitted as the first syllable in a cluster */
#define K1BRANCHFIRST 1

static k1insn_t insbuf[K1MAXINSN];
static int insncnt = 0;
static k1insn_t immxbuf[K1MAXIMMX];
static int immxcnt = 0;

#define K1LANEALIGNMENT 3 /* Minimal section alignment required to handle
correctly odd/even constraints at link time.
2**3 = 8 */
/*
 * Track the current text segment alginment so that syllable
 * alignments are correct (e.g. multiplies must be
 * in odd lanes).
 */
/* static int text_alignment = 0; */
/* There may be several text segments, so we use now_seg->target_index to keep
 * track of the byte counter, because this is a target dependant field only
 * used in the linker  */

static void set_byte_counter(asection *sec, int value);
void set_byte_counter(asection *sec, int value)
 {
    sec->target_index = value;
}

static int get_byte_counter(asection *sec);
int get_byte_counter(asection *sec)
 {
    return sec->target_index;
}

static int is_code_section(asection *sec);
int is_code_section(asection *sec)
 {
    return ((bfd_get_section_flags(NULL, sec) & (SEC_CODE))) ;
}

static char *k1_slots_name(const k1insn_t *insn) {
  switch(insn->slots) {
  case K1_BCU: return "BCU";
  case K1_ALU0: return "ALU0";
  case K1_ALU1: return "ALU1";
  case K1_ALU0 | K1_ALU1: return "ALU0 + ALU1";
  case K1_MAU: return "MAU";
  case K1_LSU: return "LSU";
  default: return "UNKNOWN";
  }
}

static char *k1_type_name(const k1insn_t *insn) {
  switch(insn->type) {
  case K1_OTHER: return "OTHER";
  case K1_LITE:  return "LITE";
  case K1_TINY:  return "TINY";
  case K1_TMD:   return "TINY MONO DOUBLE";
  case K1_LMD:   return "LITE MONO DOUBLE";
  default: return "UNKNOWN";
  }
}

/****************************************************/
/*             Local Variables                      */
/****************************************************/

static struct hash_control *k1_opcode_hash;

/****************************************************/
/*  ASSEMBLER Pseudo-ops.  Some of this just        */
/*  extends the default definitions                 */
/*  others are K1 specific                          */
/****************************************************/

enum unwrecord
 {
    UNW_HEADER,
    UNW_PROLOGUE,
    UNW_BODY,
    UNW_MEM_STACK_F,
    UNW_MEM_STACK_V,
    UNW_PSP_GR,
    UNW_PSP_SPREL,
    UNW_RP_WHEN,
    UNW_RP_GR,
    UNW_RP_PSPREL,
    UNW_RP_SPREL,
    UNW_GR_MEM_S,
    UNW_GR_MEM_L,
    UNW_SPILL_BASE,
    UNW_SPILL_MASK,
    UNW_EPILOGUE,
    UNW_LABEL_STATE,
    UNW_COPY_STATE,
    UNW_SPILL_PSREL,
    UNW_SPILL_SPREL
};
static int get_regnum_by_name(char *name);
static void k1_align(int bytes, int is_byte);
static void k1_align_bytes(int bytes);
static void k1_align_ptwo(int pow);
static void k1_skip(int mult);
static void k1_comm(int ignore);
static void k1_cons(int size);
static void k1_set_rta_flags(int);
static void k1_set_assume_flags(int);
static void k1_nop_insertion(int);
static void k1_check_resources(int);
static void k1_float_cons(int type);
static void k1_stringer(int append_zero);
static void k1_ignore(int size);
static void k1_proc(int start);
static void k1_endp(int start);
static void k1_type(int start);
static void k1_unwind(int r);
static void k1_pic_ptr (int);
#if 0
static void md_after_pass(void);
#endif

const pseudo_typeS md_pseudo_table[] =
 {
    /* override ones defined in read.c */

     {"ascii", k1_stringer, 8},
     {"asciz", k1_stringer, 9},
     {"byte", k1_cons, 1},
     {"double", k1_float_cons, 'd'},
     {"float", k1_float_cons, 'f'},
     {"hword", k1_cons, 2},
     {"int", k1_cons, 4},
     {"long", k1_cons, 4},
     {"octa", k1_cons, 16},
     {"quad", k1_cons, 8},
     {"short", k1_cons, 2},
     {"single", k1_float_cons, 'f'},
     {"string", k1_stringer, 9},
     {"word", k1_cons, 4},

     /* override ones defined in obj-elf.c */

     {"2byte", k1_cons, 2},
     {"4byte", k1_cons, 4},
     {"8byte", k1_cons, 8},

     /* k1-specific */

     {"picptr", k1_pic_ptr, 4},
     {"assume", k1_set_assume_flags, 0},
     {"rta", k1_set_rta_flags, 0},
     {"align", k1_align_bytes, 4},
     {"balign", k1_align_bytes, 4},
     {"balignw", k1_align_bytes, -2},
     {"balignl", k1_align_bytes, -4},
     {"comm", k1_comm, 0},
     {"data1", k1_cons, 1},
     {"data2", k1_cons, 2},
     {"data4", k1_cons, 4},
     {"real4", k1_cons, 4},
     {"data8", k1_cons, 8},	/* uncertain syntax */
     {"real8", k1_cons, 8},	/* uncertain syntax */
     {"skip", k1_skip, 0},		/* equiv to GNU .space */
     {"space", k1_skip, 0},	/* equiv to GNU .space */
     {"nopinsertion", k1_nop_insertion, 1},
     {"nonopinsertion", k1_nop_insertion, 0},
     {"checkresources", k1_check_resources, 1},
     {"nocheckresources", k1_check_resources, 0},

     /* unwind descriptor directives */

     {"header", k1_unwind, (int) UNW_HEADER},
     {"prologue", k1_unwind, (int) UNW_PROLOGUE},
     {"body", k1_unwind, (int) UNW_BODY},
     {"mem_stack_f", k1_unwind, (int) UNW_MEM_STACK_F},
     {"mem_stack_v", k1_unwind, (int) UNW_MEM_STACK_V},
     {"psp_gr", k1_unwind, (int) UNW_PSP_GR},
     {"psp_sprel", k1_unwind, (int) UNW_PSP_SPREL},
     {"rp_when", k1_unwind, (int) UNW_RP_WHEN},
     {"rp_gr", k1_unwind, (int) UNW_RP_GR},
     {"rp_psrel", k1_unwind, (int) UNW_RP_PSPREL},
     {"rp_sprel", k1_unwind, (int) UNW_RP_SPREL},
     {"gr_mem_s", k1_unwind, (int) UNW_GR_MEM_S},
     {"gr_mem_l", k1_unwind, (int) UNW_GR_MEM_L},
     {"spill_base", k1_unwind, (int) UNW_SPILL_BASE},
     {"spill_mask", k1_unwind, (int) UNW_SPILL_MASK},
     {"epilogue", k1_unwind, (int) UNW_EPILOGUE},
     {"label_state", k1_unwind, (int) UNW_LABEL_STATE},
     {"copy_state", k1_unwind, UNW_COPY_STATE},
     {"spill_psrel", k1_unwind, (int) UNW_SPILL_PSREL},
     {"spill_sprel", k1_unwind, (int) UNW_SPILL_SPREL},

     /* ignore cs directives */

     {"comment", k1_ignore, 0},
     {"endp", k1_endp, 0},
     {"entry", k1_ignore, 0},
     {"import", k1_ignore, 0},
     {"proc", k1_proc, 1},
     {"return", k1_ignore, 0},
     {"sversion", k1_ignore, 0},
     {"trace", k1_ignore, 0},
     {"type", k1_type, 0},
     {"call", k1_ignore, 0},
     {"longjmp", k1_ignore, 0},
     {"_longjmp", k1_ignore, 0},
     {"__longjmp", k1_ignore, 0},
     {"siglongjmp", k1_ignore, 0},
     {"_siglongjmp", k1_ignore, 0},
     {"setjmp", k1_ignore, 0},
     {"_setjmp", k1_ignore, 0},
     {"__setjmp", k1_ignore, 0},
     {"sigsetjmp", k1_ignore, 0},
     {"_sigsetjmp", k1_ignore, 0},

     /* ignore some standard ones */

     {"dc", s_ignore, 0},
     {"dc.b", s_ignore, 0},
     {"dc.d", s_ignore, 0},
     {"dc.l", s_ignore, 0},
     {"dc.s", s_ignore, 0},
     {"dc.w", s_ignore, 0},
     {"dc.x", s_ignore, 0},
     {"dcb", s_ignore, 0},
     {"dcb.b", s_ignore, 0},
     {"dcb.d", s_ignore, 0},
     {"dcb.l", s_ignore, 0},
     {"dcb.s", s_ignore, 0},
     {"dcb.w", s_ignore, 0},
     {"dcb.x", s_ignore, 0},
     {"ds", s_ignore, 0},
     {"ds.b", s_ignore, 0},
     {"ds.d", s_ignore, 0},
     {"ds.l", s_ignore, 0},
     {"ds.p", s_ignore, 0},
     {"ds.s", s_ignore, 0},
     {"ds.w", s_ignore, 0},
     {"ds.x", s_ignore, 0},
     {"lflags", s_ignore, 0},
     {"mri", s_ignore, 0},
     {".mri", s_ignore, 0},
     {"org", s_ignore, 0},
     {"p2align", k1_align_ptwo, 2},
     {"p2alignw", k1_align_ptwo, -2},
     {"p2alignl", k1_align_ptwo, -4},
#ifdef OBJ_ELF
     { "file", (void (*) PARAMS((int))) dwarf2_directive_file, 0},
     { "loc", dwarf2_directive_loc, 0},
#endif
     {NULL, 0, 0}
};

enum reloc_func 
  {
    /* FUNC_FPTR_RELATIVE, */
      FUNC_GP_RELATIVE,
      FUNC_GP_10_RELATIVE,
      FUNC_GP_16_RELATIVE,
      FUNC_GOTOFF_RELATIVE,
      FUNC_GOT_RELATIVE,
      FUNC_PLT_RELATIVE,
    /* FUNC_GOTX_RELATIVE, */
      FUNC_GOT_FDESC_RELATIVE,
      FUNC_GOTOFF_FDESC_RELATIVE,
      FUNC_FDESC_RELATIVE,
    /* FUNC_SEG_RELATIVE, */
    /* FUNC_LTV_RELATIVE, */
    /* FUNC_GOT_FPTR_RELATIVE, */
    /* FUNC_IPLT_RELOC, */
    /* FUNC_NEG_GP_RELATIVE, */
      FUNC_TP_RELATIVE,
      FUNC_PC_RELATIVE,
    /* FUNC_DTP_RELATIVE, */
    /* FUNC_DTP_MODULE, */
    /* FUNC_DTP_INDEX, */
    /* FUNC_DTP_LOAD_MODULE, */
    /* FUNC_GOT_TP_RELATIVE, */
    /* FUNC_GOT_DTP_INDEX_RELATIVE, */
    /* FUNC_GOT_DTP_LOAD_MODULE_RELATIVE */
  };
/* Pseudo functions used to indicate relocation types (these functions
 * start with an at sign (@).  */
static struct
 {
    const char *name;
    enum pseudo_type
 {
        PSEUDO_FUNC_NONE,
        PSEUDO_FUNC_RELOC
    }
    type;
    union
 {
        unsigned long ival;
        symbolS *sym;
    }
    u;
    bfd_reloc_code_real_type reloc_lo, reloc_hi, reloc_32;
}
pseudo_func[] =
 {
    // reloc pseudo functions:
    /* { "fptr",	PSEUDO_FUNC_RELOC, { 0 }, BFD_RELOC_UNUSED, */
    /* BFD_RELOC_UNUSED,              BFD_RELOC_K1_FPTR32 }, */
     { "gprel",	PSEUDO_FUNC_RELOC, { 0 }, BFD_RELOC_K1_GPREL_LO10,
       BFD_RELOC_K1_GPREL_HI22,       BFD_RELOC_UNUSED },
     { "gprel10",PSEUDO_FUNC_RELOC, { 0 }, BFD_RELOC_K1_10_GPREL,
       BFD_RELOC_UNUSED, BFD_RELOC_UNUSED },
     { "gprel16",PSEUDO_FUNC_RELOC, { 0 }, BFD_RELOC_K1_16_GPREL,
       BFD_RELOC_UNUSED,  BFD_RELOC_UNUSED },
     { "gotoff",	PSEUDO_FUNC_RELOC, { 0 }, BFD_RELOC_K1_GOTOFF_LO10,
       BFD_RELOC_K1_GOTOFF_HI22,      BFD_RELOC_K1_GOTOFF },
     { "got",      PSEUDO_FUNC_RELOC, { 0 }, BFD_RELOC_K1_GOT_LO10,
       BFD_RELOC_K1_GOT_HI22,      BFD_RELOC_K1_GOT },
    /* { "gotoffx",PSEUDO_FUNC_RELOC, { 0 }, BFD_RELOC_K1_GOTOFFX_LO9, */
    /* BFD_RELOC_K1_GOTOFFX_HI23,     BFD_RELOC_UNUSED }, */
     { "plt",	PSEUDO_FUNC_RELOC, { 0 }, BFD_RELOC_K1_PLT_LO10,
     BFD_RELOC_K1_PLT_HI22,      BFD_RELOC_UNUSED },
     { "got_funcdesc",    PSEUDO_FUNC_RELOC, { 0 }, BFD_RELOC_K1_FUNCDESC_GOT_LO10,
       BFD_RELOC_K1_FUNCDESC_GOT_HI22,      BFD_RELOC_UNUSED },
     { "gotoff_funcdesc",    PSEUDO_FUNC_RELOC, { 0 }, BFD_RELOC_K1_FUNCDESC_GOTOFF_LO10,
       BFD_RELOC_K1_FUNCDESC_GOTOFF_HI22,      BFD_RELOC_UNUSED },
     { "funcdesc",    PSEUDO_FUNC_RELOC, { 0 },BFD_RELOC_UNUSED,
       BFD_RELOC_UNUSED,      BFD_RELOC_K1_FUNCDESC },
    /*{ "segrel",	PSEUDO_FUNC_RELOC, { 0 }, BFD_RELOC_UNUSED,*/
    /* BFD_RELOC_UNUSED,              BFD_RELOC_K1_SEGREL32 }, */
    /* { "ltv",	PSEUDO_FUNC_RELOC, { 0 }, BFD_RELOC_UNUSED, */
    /* BFD_RELOC_UNUSED,              BFD_RELOC_K1_LTV32}, */
    /* // placeholder for FUNC_GOT_FPTR_RELATIVE */
    /* { "", 0, { 0 }, BFD_RELOC_K1_GOTOFF_FPTR_LO9, */
    /* BFD_RELOC_K1_GOTOFF_FPTR_HI23, BFD_RELOC_UNUSED }, */
    /* { "iplt",	PSEUDO_FUNC_RELOC, { 0 }, BFD_RELOC_UNUSED, */
    /* BFD_RELOC_UNUSED,              BFD_RELOC_K1_IPLT }, */
    /* { "neggprel",PSEUDO_FUNC_RELOC,{ 0 }, BFD_RELOC_K1_NEG_GPREL_LO9, */
    /* BFD_RELOC_K1_NEG_GPREL_HI23,   BFD_RELOC_UNUSED }, */
    { "tprel",   PSEUDO_FUNC_RELOC,{ 0 }, BFD_RELOC_K1_TPREL_LO10,
    BFD_RELOC_K1_TPREL_HI22,       BFD_RELOC_K1_TPREL_32 },
//     { "pcrel",   PSEUDO_FUNC_RELOC,{ 0 }, BFD_RELOC_K1_PCREL_LO10,
//     BFD_RELOC_K1_PCREL_HI22,       BFD_RELOC_UNUSED },
    /* { "dtprel",  PSEUDO_FUNC_RELOC,{ 0 }, BFD_RELOC_K1_DTPREL_LO9, */
    /* BFD_RELOC_K1_DTPREL_HI23,      BFD_RELOC_K1_DTPREL32 }, */
    /* { "dtpmod",  PSEUDO_FUNC_RELOC,{ 0 }, BFD_RELOC_UNUSED, */
    /* BFD_RELOC_UNUSED,              BFD_RELOC_K1_DTPMOD32 }, */
    /* { "dtpndx",  PSEUDO_FUNC_RELOC,{ 0 }, BFD_RELOC_UNUSED, */
    /* BFD_RELOC_UNUSED,              BFD_RELOC_UNUSED }, */
    /* { "dtpldm",  PSEUDO_FUNC_RELOC,{ 0 }, BFD_RELOC_UNUSED, */
    /* BFD_RELOC_UNUSED,              BFD_RELOC_UNUSED }, */
    /* // placeholder for GOTOFF_TPREL */
    /* { "",        0,                { 0 }, BFD_RELOC_K1_GOTOFF_TPREL_LO9, */
    /* BFD_RELOC_K1_GOTOFF_TPREL_HI23,BFD_RELOC_UNUSED }, */
    /* // placeholder for GOTOFF_DTPNDX */
    /* { "" ,       0,                { 0 }, BFD_RELOC_K1_GOTOFF_DTPNDX_LO9, */
    /* BFD_RELOC_K1_GOTOFF_DTPNDX_HI23, BFD_RELOC_UNUSED }, */
    /* // placeholder for GOTOFF_DTPLDM */
    /* { "",        0,                { 0 }, BFD_RELOC_K1_GOTOFF_DTPLDM_LO9, */
    /* BFD_RELOC_K1_GOTOFF_DTPLDM_HI23, BFD_RELOC_UNUSED } */
};

/*****************************************************/
/*          Unwind information                       */
/*                                                   */
/*     unwind_proc_symbol -- frag address            */
/*                           following proc          */
/*     unwind_info_symbol -- frag address of         */
/*                           unwind info block       */
/*     unwind_bundle_count -- bundles seen since proc */
/*                                                   */
/*****************************************************/

#define UNWIND_HEADER_SECTION_NAME ".unwindh"
#define UNWIND_INFO_SECTION_NAME   ".unwindi"
#define UNWIND_VERSION             1

static int unwind_bundle_count = 0;

static int inside_bundle = 0;

/* Stores the labels inside bundles (typically debug labels) that need
   to be postponed to the next bundle. */
struct label_fix {
    struct label_fix *next;
    symbolS *sym;
} * label_fixes = 0;

/*****************************************************/
/*   OPTIONS PROCESSING                              */
/*****************************************************/

const char *md_shortopts = "hV";	/* catted to std short options */

/* added to std long options */

#define OPTION_HEXFILE	(OPTION_MD_BASE + 0)
#define OPTION_DUMPTABLES (OPTION_MD_BASE + 1)
#define OPTION_DUMPVERILOG (OPTION_MD_BASE + 2)
#define OPTION_LITTLE_ENDIAN (OPTION_MD_BASE + 3)
#define OPTION_BIG_ENDIAN (OPTION_MD_BASE + 4)
#define OPTION_EMITALLRELOCS (OPTION_MD_BASE + 6)
#define OPTION_MCORE (OPTION_MD_BASE + 7)
#define OPTION_CHECK_RESOURCES (OPTION_MD_BASE + 8)
#define OPTION_NO_CHECK_RESOURCES (OPTION_MD_BASE + 9)
#define OPTION_GENERATE_ILLEGAL_CODE (OPTION_MD_BASE + 10)
#define OPTION_DUMP_TABLE (OPTION_MD_BASE + 11)
#define OPTION_PIC	(OPTION_MD_BASE + 12)
#define OPTION_BIGPIC	(OPTION_MD_BASE + 13)
#define OPTION_FDPIC	(OPTION_MD_BASE + 14)
#define OPTION_NOPIC    (OPTION_MD_BASE + 15)

struct option md_longopts[] =
 {
     {"hex-output", required_argument, NULL, OPTION_HEXFILE},
     {"EL", no_argument, NULL, OPTION_LITTLE_ENDIAN},
     {"EB", no_argument, NULL, OPTION_BIG_ENDIAN},
     {"emit-all-relocs", no_argument, NULL, OPTION_EMITALLRELOCS},
     {"mcore", required_argument, NULL, OPTION_MCORE},
     {"check-resources", no_argument, NULL, OPTION_CHECK_RESOURCES},
     {"no-check-resources", no_argument, NULL, OPTION_NO_CHECK_RESOURCES},
     {"generate-illegal-code", no_argument, NULL, OPTION_GENERATE_ILLEGAL_CODE},
     {"dump-table", no_argument, NULL, OPTION_DUMP_TABLE},
     {"mpic", no_argument, NULL, OPTION_PIC},
     {"mPIC", no_argument, NULL, OPTION_BIGPIC},
     {"mfdpic",	no_argument,	NULL, OPTION_FDPIC},
     {"mnopic", no_argument,    NULL, OPTION_NOPIC},
     {"mno-fdpic", no_argument,    NULL, OPTION_NOPIC},
     {NULL, no_argument, NULL, 0}
};

size_t md_longopts_size = sizeof (md_longopts);

int md_parse_option(int c, char *arg ATTRIBUTE_UNUSED) {
  int i;
  int find_core = 0;

  switch (c) {
  case 'h':
    md_show_usage(stdout);
    exit(EXIT_SUCCESS);
    break;
    
    /* -V: SVR4 argument to print version ID.  */
  case 'V':
    print_version_id();
    break;
  case OPTION_HEXFILE:
    /*        hex_file_name = strdup (arg);
     * if (!hex_file_name)
     * as_fatal ("virtual memory exhausted"); */
    break;
  case OPTION_EMITALLRELOCS:
    emit_all_relocs = 1;
    break;
  case OPTION_MCORE:
    mcore = strdup(arg);
    i = 0;
    while(i < K1_NCORES && ! find_core) {
      subcore_id = 0;
      while(k1_core_info_table[i]->elf_cores[subcore_id] != -1 && ! find_core) {
	if (strcasecmp(mcore, k1_core_info_table[i]->names[subcore_id]) == 0
	    && k1_core_info_table[i]->supported){

	  k1_core_info = k1_core_info_table[i];
	  k1_registers = k1_registers_table[i];
	  k1_regfiles = k1_regfiles_table[i];
	
	  find_core = 1;
	}
	else {
	  subcore_id++;
	}
      }
      if(find_core) { break; }
      i++;
    }
    if (i == K1_NCORES){
      char buf[100];
      supported_cores(buf, sizeof(buf));
      as_fatal("Core specified not supported [%s]", buf);
    }
    break;
  case OPTION_CHECK_RESOURCES:
    check_resource_usage = 1;
    break;
  case OPTION_NO_CHECK_RESOURCES:
    check_resource_usage = 0;
    break;
  case OPTION_GENERATE_ILLEGAL_CODE:
    generate_illegal_code = 1;
    break;
  case OPTION_DUMP_TABLE:
    dump_table = 1;
    break;
  case OPTION_PIC:
  /* fallthrough, for now the same on K1 */
  case OPTION_BIGPIC:
    k1_pic_flags |= ELF_K1_PIC;
    break;
  case OPTION_FDPIC:
    k1_pic_flags |= ELF_K1_FDPIC;
//     k1_pic_flag = "-mfdpic";
    break;
  case OPTION_NOPIC:
    k1_pic_flags &= ~(ELF_K1_FDPIC);
    break;
  default:
    return 0;
  }
  return 1;
}

void md_show_usage(FILE * stream){
    char buf[100];
    supported_cores(buf, sizeof(buf));

    fprintf(stream, "\nThe options -M, --mri and -f");
    fprintf(stream, " are not supported in this assembler.\n");
    fprintf(stream, "--emit-all-relocs \t emit all relocs\n");
    fprintf(stream, "--check-resources \t perform minimal resource checking\n");
    fprintf(stream, "--mcore [%s] \t select encoding table and ELF flags\n", buf);
    fprintf(stream, "-V \t\t\t print assembler version number\n");
}

/**************************************************/
/*              UTILITIES                         */
/**************************************************/

/*
 *  Write a value to the object file
 */

void md_number_to_chars(char *buf, valueT val, int n){
    number_to_chars_littleendian(buf, val, n);
}

/*
 * Read a value from to the object file
 */

static valueT md_chars_to_number(char *buf, int n);
valueT md_chars_to_number(char *buf, int n){
    valueT val = 0;

    if (n > (int)sizeof (val) || n <= 0)
        abort();

    while (n--){
        val <<= 8;
        val |= (buf[n] & 0xff);
    }

    return val;
}

static void
real_k1_reloc_type(symbolS *sym, bfd_reloc_code_real_type *reloc_lo,
        bfd_reloc_code_real_type *reloc_hi,
        bfd_reloc_code_real_type *reloc_32)
 {
    int i;

    for (i = 0; i < NELEMS(pseudo_func); i++)
        if (sym == pseudo_func[i].u.sym)
 {
            if (reloc_lo)
                *reloc_lo = pseudo_func[i].reloc_lo;
            if (reloc_hi)
                *reloc_hi = pseudo_func[i].reloc_hi;
            if (reloc_32)
                *reloc_32 = pseudo_func[i].reloc_32;
            break;
        }
    if (i == NELEMS(pseudo_func))
        abort();
}

static void supported_cores(char buf[], size_t buflen) {
  int i, j;
  buf[0] = '\0';
  for (i = 0; i < K1_NCORES; i++) {
    j = 0;
    while(k1_core_info_table[i]->elf_cores[j] != -1) {
      if (k1_core_info_table[i]->supported) {
	if (buf[0] == '\0') {
	  strcpy(buf, k1_core_info_table[i]->names[j]);
	}
	else {
	  int l = strlen(buf);
	  if ((l + 1 + strlen(k1_core_info_table[i]->names[j]) + 1) < buflen) {
	    strcat(buf, "|");
	    strcat(buf, k1_core_info_table[i]->names[j]);
	  }
	}
      }
    j++;
    }
  }
}

int get_regnum_by_name(char *name){
    int i;
    for(i=0; i < k1_regfiles[K1_REGFILE_REGISTERS]; i++){
        if(STREQ(k1_registers[i].name, name)){
            return k1_registers[i].id;
        }
    }
    return -1;
}

/***************************************************/
/*   ASSEMBLE AN INSTRUCTION                       */
/***************************************************/

/* Parse the arguments to an opcode. STR is a C string containing the
 * arguments. TOK is the output array of tokens, which are assembly
 * expressions. NTOK is the size of the array into which the tokens
 * will be placed. The return value is the number of tokens found if
 * no errors were encountered, otherwise -1 is returned.  */

static int
tokenize_arguments(char *str, expressionS tok[], char *tok_begins[], int ntok) {
    expressionS *end_tok = tok + ntok;
    char *old_input_line_pointer;
    int saw_comma = 0, saw_arg = 0;
    int tokcnt = 0;
    
    memset(tok, 0, sizeof (*tok) * ntok);

    /* Save and restore input_line_pointer around this function */

    old_input_line_pointer = input_line_pointer;

    input_line_pointer = str;

    while ((tok < end_tok) && *input_line_pointer) {
        SKIP_WHITESPACE();
        if (tok_begins)
            tok_begins[tokcnt] = input_line_pointer;

        switch (*input_line_pointer) {
            case '\n':
            case '\0':
                goto fini;
            case ',':
            case '=':
            case ':':
	    case '?':
                ++input_line_pointer;
                if (saw_comma || !saw_arg)
                    goto err;
                saw_comma = 1;
                break;
            case '[': {
	      /* clarkes: for ldwl/stwl, we allow comma separator
	       * before the [, so I have removed this test.
	       * if (saw_comma)
	       * goto err; */
	      expression(tok);
	      if (tok->X_op == O_register) {
		saw_comma = 0;
		saw_arg = 1;
		++tok;
		++tokcnt;
		break;
	      }
	      else {
		D(stderr, "tok type == %d\n", tok->X_op);
		as_warn("expected a register");
		break;
	      }
            }
	    default: {
	      if (saw_arg && !saw_comma) {
		goto err;
	      }
	      expression(tok);
	      if (tok->X_op == O_illegal || tok->X_op == O_absent) {
		goto err;
	      }
	      
	      saw_comma = 0;
	      saw_arg = 1;
	      ++tok;
	      ++tokcnt;
	      break;
	    }
        }
    }

    fini:
    if (saw_comma) {
      goto err;
    }
    if (tok_begins) {
      tok_begins[tokcnt] = input_line_pointer;
    }
    input_line_pointer = old_input_line_pointer;
    return ntok - (end_tok - tok);

    err:
    error_str = input_line_pointer;
    input_line_pointer = old_input_line_pointer;

    return -1;
}

/*
 * Check input expressions against required operands
 */

static match_operands_code
match_operands(const k1opc_t * op, const expressionS * tok,
        int ntok)
 {
    int i;
    int nop;
    k1bfield *opdef;
    long long min, max;
    unsigned long long mask;

    /* First check that number of operands are the same. */
    for (nop = 0; op && op->format[nop]; nop++);
    if (ntok != nop) {
      return MATCH_NOT_FOUND;
    }

#define IS_K1_REGFILE_GRF(tok) ((((tok).X_add_number) >= k1_regfiles[K1_REGFILE_FIRST_GRF]) \
				&& (((tok).X_add_number) <= k1_regfiles[K1_REGFILE_LAST_GRF]))
#define IS_K1_REGFILE_PRF(tok) ((((tok).X_add_number) >= k1_regfiles[K1_REGFILE_FIRST_PRF]) \
				&& (((tok).X_add_number) <= k1_regfiles[K1_REGFILE_LAST_PRF]))
#define IS_K1_REGFILE_SRF(tok) ((((tok).X_add_number) >= k1_regfiles[K1_REGFILE_FIRST_SRF]) \
				 && (((tok).X_add_number) <= k1_regfiles[K1_REGFILE_LAST_SRF]))
#define IS_K1_REGFILE_NRF(tok) ((((tok).X_add_number) >= k1_regfiles[K1_REGFILE_FIRST_NRF]) \
				&& (((tok).X_add_number) <= k1_regfiles[K1_REGFILE_LAST_NRF]))

#define MATCH_K1_REGFILE(tok,is_regfile)				\
    if (((tok).X_op == O_register) && (is_regfile(tok))) {		\
      break;								\
    }									\
    else {								\
      return MATCH_NOT_FOUND;						\
    }

    /* Now check for compatiblility of each operand. */
    for (i = 0; i < ntok; i++) {
        int operand_type = op->format[i]->type;
        char *operand_type_name = op->format[i]->tname;
        opdef = op->format[i];
        int *valid_regs = op->format[i]->regs;

	/* When operand is a register, check if it is valid. */
	if ((tok[i].X_op == O_register) &&
	    (valid_regs == NULL || !valid_regs[k1_registers[tok[i].X_add_number].id])) {
	  return MATCH_NOT_FOUND;
	}

#define SRF_REGCLASSES(core)                           \
	  case RegClass_ ## core ## _systemReg:                        \
	  case RegClass_ ## core ##_nopcpsReg:                 \
	  case RegClass_ ## core ## _onlypsReg:                        \
	  case RegClass_ ## core ## _onlyraReg:                        \
	  case RegClass_ ## core ## _onlyfxReg:

        switch (operand_type) {
            case RegClass_k1_singleReg:
	      MATCH_K1_REGFILE(tok[i],IS_K1_REGFILE_GRF)
            case RegClass_k1_pairedReg:
	      MATCH_K1_REGFILE(tok[i],IS_K1_REGFILE_PRF)
			SRF_REGCLASSES(k1)
			SRF_REGCLASSES(k1b)
	      MATCH_K1_REGFILE(tok[i],IS_K1_REGFILE_SRF)
            case RegClass_k1_remoteReg:
	      MATCH_K1_REGFILE(tok[i],IS_K1_REGFILE_NRF)

            case Immediate_k1_flagmask2:
            case Immediate_k1_brknumber:
            case Immediate_k1_sysnumber:
            case Immediate_k1_signed5:
            case Immediate_k1_unsigned5:
            case Immediate_k1_unsigned6:
            case Immediate_k1_eventmask2:
            case Immediate_k1_unsigned32:
            case Immediate_k1_unsigned32L:
            case Immediate_k1_signed32M:
            case Immediate_k1_signed37:
            case Immediate_k1_signed43:

                if(tok[i].X_op == O_symbol) {
		  return MATCH_NOT_FOUND;
                }
            case Immediate_k1_signed10:
            case Immediate_k1_signed11:
            case Immediate_k1_signed16:
            case Immediate_k1_signed27:
            case Immediate_k1_pcrel18:
            case Immediate_k1_pcrel17:
            case Immediate_k1_pcrel27:
            case Immediate_k1_signed32:
                if(tok[i].X_op == O_symbol || tok[i].X_op == O_pseudo_fixup){
                    break;
                }
                if (tok[i].X_op == O_constant){
  		    long long signed_value = tok[i].X_add_number;
  		    unsigned long long unsigned_value = tok[i].X_add_number;
  		    int match_signed = 0;
		    int match_unsigned = 0;

                    // Operand is not signed, but the token is.
                    if( !(opdef->flags & k1SIGNED) && (tok[i].X_unsigned == 0)){
		      return MATCH_NOT_FOUND;
                    }

		    // [JV] Special case of both signed and unsigned 
		    if(opdef->width == 32) {
		      signed long long high_mask = 0x8000000000000000LL;
		      int shift = (sizeof(signed long long) * 8) - opdef->width - 1;

		      high_mask = high_mask >> shift;

		      // If high bits set to zero, can perform sign extension.
		      if((signed_value & high_mask) == 0) {
			signed_value = (signed_value << (64 - opdef->width)) >> (64 - opdef->width);
		      }
		    }

                    max = (1LL << (opdef->width - 1)) - 1;
                    min = (-1LL << (opdef->width - 1));
                    mask = ~(-1LL << opdef->width);

		    match_signed = (((signed_value >> opdef->rightshift) >= min) && ((signed_value >> opdef->rightshift) <= max));
		    match_unsigned = (((unsigned_value >> opdef->rightshift) & mask) == (unsigned_value >> opdef->rightshift));

                    if ( ( (!(opdef->flags & k1SIGNED)) && !match_unsigned ) ||
			 ( (opdef->flags & k1SIGNED)    && !match_signed ) ) {
		      return MATCH_NOT_FOUND;
                    }
                    break;
                }

                return MATCH_NOT_FOUND;
            default:
                as_bad("[match_operands] : couldn't find operand type %s \n", operand_type_name);
		return MATCH_NOT_FOUND;
        }
    }
    return MATCH_FOUND;

#undef IS_K1_REGFILE_GRF
#undef IS_K1_REGFILE_PRF
#undef IS_K1_REGFILE_SRF
#undef IS_K1_REGFILE_NRF
#undef MATCH_K1_REGFILE
#undef SRF_REGCLASSES
}

/*
 * Given an initial pointer into the opcode table, OPCODE,
 * find the format that matches the given set of operands. NTOK tells
 * the number of operands in the operand array pointed to by TOK.
 * If a matching format is found, a pointer to it is returned,
 * otherwise a null pointer is returned.
 */

static const k1opc_t *
find_format(const k1opc_t * opcode,
        const expressionS * tok,
        int ntok)
 {
    char *name = opcode->as_op;
    const k1opc_t *t = opcode;

    while (STREQ(name, t->as_op)){
        if (match_operands(t, tok, ntok) == MATCH_FOUND){
            return t;
        }
        t++;
    }

    return NULL;
}

/*
 * Insert an operand into the instruction.
 */

static void
insert_operand(k1insn_t * insn,
        k1bfield * opdef,
        const expressionS * arg)
 {
    unsigned long long op = 0;
    k1_bitfield_t *bfields = opdef->bfield;
    int bf_nb = opdef->bitfields;
    int bf_idx;

    //    max = (1LL << (opdef->width - 1)) - 1;
    //    min = (-1LL << (opdef->width - 1));

    if (opdef->width == 0)
        return;			/* syntactic sugar ? */

    /* try to resolve the value */

    switch (arg->X_op)
 {
        case O_register:
            op = k1_registers[arg->X_add_number].id;
            break;
        case O_pseudo_fixup:
	    if (insn->nfixups == 0)
		{
		    bfd_reloc_code_real_type reloc_lo, reloc_hi;
		    expressionS reloc_arg;
		    
		    real_k1_reloc_type(arg->X_op_symbol, &reloc_lo, &reloc_hi, 0);
		    reloc_arg = *arg;
		    reloc_arg.X_op = O_symbol;

		    if (reloc_hi != BFD_RELOC_UNUSED 
			&& reloc_lo == BFD_RELOC_UNUSED) {
			insn->fixup[0].reloc = reloc_hi;
			insn->fixup[0].exp = reloc_arg;
			insn->fixup[0].where = 0;
			insn->nfixups = 1;
		    } else if (reloc_hi == BFD_RELOC_UNUSED 
			       && reloc_lo != BFD_RELOC_UNUSED) {
			insn->fixup[0].reloc = reloc_lo;
			insn->fixup[0].exp = reloc_arg;
			insn->fixup[0].where = 0;
			insn->nfixups = 1;
		    } else if (reloc_hi != BFD_RELOC_UNUSED 
			       && reloc_lo != BFD_RELOC_UNUSED) {
			if (insn->len > 1)
			    as_fatal("only one immediate extension allowed !");
			insn->fixup[0].reloc = reloc_lo;
			insn->fixup[0].exp = reloc_arg;
			insn->fixup[0].where = 0;
			insn->nfixups = 1;
			insn->immx = immxcnt;
			immxbuf[immxcnt].insn[0] = 0;
			immxbuf[immxcnt].fixup[0].reloc = reloc_hi;
			immxbuf[immxcnt].fixup[0].exp = reloc_arg;
			immxbuf[immxcnt].fixup[0].where = 0;
			immxbuf[immxcnt].nfixups = 1;
			immxbuf[immxcnt].len = 1;
			immxcnt++;
		    }
		}
	    else
		{
		    as_fatal ("No room for fixup ");
		}
      break;
        case O_constant:		/* we had better generate a fixup if > max */
            if (!(arg->X_add_symbol))
            {
                if(opdef->flags & k1SIGNED){
                  op = (long long)arg->X_add_number >> opdef->rightshift;
                }else{
                  op = (unsigned long long)arg->X_add_number >> opdef->rightshift;
                }
                break;
            }
            /* else falls through to fixup */
        default:
            /*      fprintf(stdout, "generate a fixup\n"); */
        {
            if (insn->nfixups == 0)
            {
                switch (opdef->type)
                {
                  case Immediate_k1_pcrel17:
                        insn->fixup[0].reloc = BFD_RELOC_K1_17_PCREL;
                        insn->fixup[0].exp = *arg;
                        insn->fixup[0].where = 0;
                        insn->nfixups = 1;
                        break;
                  case Immediate_k1_pcrel18:
                        insn->fixup[0].reloc = BFD_RELOC_K1_18_PCREL;
                        insn->fixup[0].exp = *arg;
                        insn->fixup[0].where = 0;
                        insn->nfixups = 1;
                        break;
                    case Immediate_k1_pcrel27:
                        insn->fixup[0].reloc = BFD_RELOC_K1_27_PCREL;
                        insn->fixup[0].exp = *arg;
                        insn->fixup[0].where = 0;
                        insn->nfixups = 1;
                        break;
                    case Immediate_k1_signed27:
                        insn->fixup[0].reloc = BFD_RELOC_K1_27_PCREL; /* FIXME K1B */
                        insn->fixup[0].exp = *arg;
                        insn->fixup[0].where = 0;
                        insn->nfixups = 1;
                        break;

                    case Immediate_k1_signed10:
                    case Immediate_k1_signed16:
			insn->fixup[0].reloc = BFD_RELOC_K1_LO10;
			insn->fixup[0].exp = *arg;
			insn->fixup[0].where = 0;
			insn->nfixups = 1;
			insn->immx = immxcnt;
			immxbuf[immxcnt].insn[0] = 0;
			immxbuf[immxcnt].fixup[0].reloc = BFD_RELOC_K1_HI22;
			immxbuf[immxcnt].fixup[0].exp = *arg;
			immxbuf[immxcnt].fixup[0].where = 0;
			immxbuf[immxcnt].nfixups = 1;
			immxbuf[immxcnt].len = 1;
			immxcnt++;
			break;

                    default:
                        as_fatal("don't know how to generate a fixup record");
                }
                return;
            }
            else
 {
                as_fatal("No room for fixup ");
            }
        }
    }

    for(bf_idx=0;bf_idx < bf_nb; bf_idx++) {
      unsigned long long value = ((unsigned long long)op >> bfields[bf_idx].from_offset);
      int j = 0;
      int to_offset = bfields[bf_idx].to_offset;
      value &= (1LL << bfields[bf_idx].size) - 1;
      j = to_offset / 32;
      to_offset = to_offset % 32;
      insn->insn[j] |= value << to_offset;
    }
    return;
}

/*
 * Given a set of operands and a matching instruction,
 * assemble it
 *
 */

static void
assemble_insn( const k1opc_t * opcode,
        const expressionS * tok,
        int ntok,
        k1insn_t * insn)
 {
    int argidx;
    unsigned int i;
    memset(insn, 0, sizeof (*insn));
    insn->opdef = opcode;
    for(i=0; i < opcode->codewords; i++) {
        insn->insn[i] = (unsigned int)opcode->codeword[i].opcode;
        insn->len += 1;
    }
    insn->immx = NOIMMX;
    insn->immx64 = NOIMMX;
    for (argidx = 0; argidx < ntok; argidx++){
        insert_operand(insn, opcode->format[argidx], &tok[argidx]);
    }
    for(i=0; i < opcode->codewords; i++){
        if(opcode->codeword[i].flags & k1OPCODE_FLAG_IMMX0){
            immxbuf[immxcnt].insn[0] = insn->insn[i];
            immxbuf[immxcnt].nfixups = 0;
            immxbuf[immxcnt].len = 1;
            insn->len -= 1; 
            insn->immx = immxcnt;
            immxcnt++;
        }
        if(opcode->codeword[i].flags & k1OPCODE_FLAG_IMMX1){
            immxbuf[immxcnt].insn[0] = insn->insn[i];
            immxbuf[immxcnt].nfixups = 0;
            immxbuf[immxcnt].len = 1;
            insn->len -= 1;
            insn->immx64 = immxcnt;
            immxcnt++;
        }
    }
//    printf("Insert instruction: len = %d, insn[0] = 0x%x, insn[1] = 0x%x, immx : %d (%#x), immx64 = %d\n",insn->len,insn->insn[0],insn->insn[1], insn->immx, immxbuf[insn->immx].insn[0], insn->immx64);
    return;
}


/* Emit an instruction from the instruction array into the object
 * file. INSN points to an element of the instruction array. STOPFLAG
 * is true if this is the last instruction in the bundle.
 */
static void
emit_insn(k1insn_t * insn, int stopflag){
    char *f;
    int i;
    unsigned image;

    /* if we are listing, attach frag to previous line */

    if (listing)
 {
        listing_prev_line();
    }

    /* Update text size for lane parity checking */

    set_byte_counter(now_seg, (get_byte_counter(now_seg) + (insn->len * 4)) );

    /* allocate space in the fragment      */

    f = frag_more(insn->len * 4);

    /* spit out bits          */

    for (i = 0; i < (int)insn->len; i++) {
      image = (unsigned)insn->insn[i];
      /* Handle bundle parallel bit. */ ;
      if ((i == (int)insn->len - 1) && stopflag){
        image &= 0x7FFFFFFF;
      }else{
        image |= 0x80000000;
      }
      //printf("Emmiting %#x\n", image);
      /* Emit the instruction image. */
      md_number_to_chars(f + (i * 4), image, 4);
    }

    /* generate fixup records */

    for (i = 0; i < insn->nfixups; i++) {
      int size, pcrel;
      fixS *fixP;
      reloc_howto_type *reloc_howto = bfd_reloc_type_lookup(stdoutput, insn->fixup[i].reloc);
      assert(reloc_howto);
      size = bfd_get_reloc_size(reloc_howto);
      pcrel = reloc_howto->pc_relative;
      fixP = fix_new_exp(frag_now, f - frag_now->fr_literal + insn->fixup[i].where, size, &(insn->fixup[i].exp), pcrel, insn->fixup[i].reloc);
    }
}


/* Determines if the expression is constant absolute */
int is_constant_expression(expressionS* exp)
 {
    int retval=FALSE;

    if ( exp!=NULL )
 {
        if ( exp->X_op==O_constant )
 {
            retval=TRUE;
        }
        else if ( !(exp->X_add_symbol) )
 {
            retval=TRUE;
        }
        else if ( ! symbol_resolved_p(exp->X_add_symbol) )
 {
            retval=FALSE;
        }
        else
 {
            if ( exp->X_add_symbol )
 {
                if ( bfd_is_abs_section(S_GET_SEGMENT(exp->X_add_symbol)) )
 {
                    retval=TRUE;
                }
                /* FIXME (lc) rather bad to test if we need another symbol */
                if ( exp->X_op>=O_logical_not )
 {
                    if ( bfd_is_abs_section(S_GET_SEGMENT(exp->X_op_symbol)) )
 {
                        retval=TRUE;
                    }
                    else
 {
                        retval=FALSE;
                    }
                } /* if (exp->X_op_symbol) */
            } /* if ( exp->X_add_symbol && ...sy_resolved) */
        } /* else if ( !(exp->X_add_symbol) && !(exp->X_op_symbol) ) */
    }

    return retval;
}

/* Called for any expression that can not be recognized.  When the
 * function is called, `input_line_pointer' will point to the start of
 * the expression.  */

void
md_operand(expressionS *e) {
   enum pseudo_type pseudo_type;
   const char *name;
   size_t len;
   int ch, i;

 switch (*input_line_pointer)
   {
   case '@':
     /* Find what relocation pseudo-function we're dealing with. */
     pseudo_type = 0;
     ch = *++input_line_pointer;
     for (i = 0; i < NELEMS (pseudo_func); ++i)
	if (pseudo_func[i].name && pseudo_func[i].name[0] == ch)
	  {
	    len = strlen (pseudo_func[i].name);
	    if (strncmp (pseudo_func[i].name + 1,
			 input_line_pointer + 1, len - 1) == 0
		&& !is_part_of_name (input_line_pointer[len]))
	      {
		input_line_pointer += len;
		pseudo_type = pseudo_func[i].type;
		break;
	      }
	  }
     switch (pseudo_type)
	{
	case PSEUDO_FUNC_RELOC:
	  SKIP_WHITESPACE ();
	  if (*input_line_pointer != '(')
	    {
	      as_bad ("Expected '('");
	      goto err;
	    }
	  /* Skip '('.  */
	  ++input_line_pointer;
	  expression (e);
	  if (*input_line_pointer++ != ')')
	    {
	      as_bad ("Missing ')'");
	      goto err;
	    }
	  if (e->X_op != O_symbol)
	    {
		  as_bad ("Illegal combination of relocation functions");
		  /* if (e->X_op != O_pseudo_fixup) */
		  /*     { */
		  /* 	  as_bad ("Not a symbolic expression"); */
		  /* 	  goto err; */
		  /*     } */
		   if (i != FUNC_GOT_RELATIVE)
                   {
                     as_bad ("Illegal combination of relocation functions");
                     goto err;
                   }
		  /* switch (S_GET_VALUE (e->X_op_symbol)) */
		  /*     { */
		  /*     case FUNC_FPTR_RELATIVE: */
		  /* 	  i = FUNC_GOT_FPTR_RELATIVE; break; */
		  /*     case FUNC_TP_RELATIVE: */
		  /* 	  i = FUNC_GOT_TP_RELATIVE; break; */
		  /*     case FUNC_DTP_INDEX: */
		  /* 	  i = FUNC_GOT_DTP_INDEX_RELATIVE; break; */
		  /*     case FUNC_DTP_LOAD_MODULE: */
		  /* 	  i = FUNC_GOT_DTP_LOAD_MODULE_RELATIVE; break; */
		  /*     default: */
		  /* 	  as_bad ("Illegal combination of relocation functions"); */
		  /* 	  goto err; */
		  /*     } */
	    }
	  /* Make sure gas doesn't get rid of local symbols that are used
	     in relocs.  */
	  e->X_op = O_pseudo_fixup;
	  e->X_op_symbol = pseudo_func[i].u.sym;
	  break;

	default:
	  name = input_line_pointer - 1;
	  get_symbol_end ();
	  as_bad ("Unknown pseudo function `%s'", name);
	  goto err;
	}
     break;
   default:
     break;
   }
    return;

   err:
                       ignore_rest_of_line();
 }

/* Return 1 if it's OK to adjust a reloc by replacing the symbol with
 * a section symbol plus some offset.  For relocs involving @fptr(),
 * directives we don't want such adjustments since we need to have the
 * original symbol's name in the reloc.  */
int k1_fix_adjustable(fix)
fixS *fix;
 {
    if (emit_all_relocs)
        return 0;

#if defined (OBJ_ELF) || defined (OBJ_MAYBE_ELF)
    /* Prevent all adjustments to global symbols, or else ELF dynamic
     * linking will not work correctly.  */
    if (S_IS_EXTERNAL(fix->fx_addsy)
            || S_IS_WEAK(fix->fx_addsy))
        return 0;
#endif
    switch (fix->fx_r_type)
 {
        default:
            break;
            /*
             * case BFD_RELOC_K1_FPTR32:
             * case BFD_RELOC_K1_GOTOFF_FPTR_HI23:
             * case BFD_RELOC_K1_GOTOFF_FPTR_LO9:
             */
	case BFD_RELOC_K1_GOTOFF:
        case BFD_RELOC_K1_GOTOFF_HI22:
        case BFD_RELOC_K1_GOTOFF_LO10:
	case BFD_RELOC_K1_GOT:
        case BFD_RELOC_K1_GOT_HI22:
        case BFD_RELOC_K1_GOT_LO10:
        case BFD_RELOC_K1_PLT_HI22:
        case BFD_RELOC_K1_PLT_LO10:
        case BFD_RELOC_K1_FUNCDESC_GOT_LO10:
        case BFD_RELOC_K1_FUNCDESC_GOT_HI22:
        case BFD_RELOC_K1_FUNCDESC_GOTOFF_LO10:
        case BFD_RELOC_K1_FUNCDESC_GOTOFF_HI22:
	case BFD_RELOC_K1_FUNCDESC:
	case BFD_RELOC_K1_GLOB_DAT:
             /* case BFD_RELOC_K1_GOTOFFX_HI23:
             * case BFD_RELOC_K1_GOTOFFX_LO9:             
             * case BFD_RELOC_K1_IPLT:
             * case BFD_RELOC_K1_JMP_SLOT:
             */
             return 0;
    }

    return 1;
 }

static void
assemble_tokens(const char *opname,
        const expressionS * tok,
        int ntok);

/*
 * Return the Bundling type for an insn.
 */

static Bundling find_bundling(const k1insn_t *insn)
 {
    int insn_bundlings = insn->opdef->bundlings;
    return insn_bundlings;
}

static int find_reservation(const k1insn_t *insn)
 {
    int insn_reservation = insn->opdef->reservation;
    return insn_reservation;
}

static int cmp_bundling(const void *a, const void *b)
 {
    const Bundling *ba = (const Bundling *)a;
    const Bundling *bb = (const Bundling *)b;
    return (*bb < *ba) - (*ba < *bb);
}

/*
 * Find a bundle type that matches the operations in
 * bundle_insn.
 * If found, return the index in bundlematch_table.
 * If not found, return:
 *  -2 if operations cannot be bundled on this alignment
 *  -1 if operations cannot be bundled on any alignment
 * As a side-effect, reorder the operations in
 * bundle_insn and add nops to match the required ordering
 * for the bundle type.
 * If no bundle type found, the operations
 * are in the original order.
 *
 */

static int find_bundle_type(k1insn_t *bundle_insn[], int *bundle_insn_cnt){
  int hash = 0;
  int i;
  int canonical_ix;
  const BundleMatchType *match;
  Bundling canonical_order[K1MAXBUNDLESIZE];
  
  
  if (*bundle_insn_cnt > K1MAXBUNDLESIZE) {
    return -1;
  }
  
  for (i = 0; i < *bundle_insn_cnt; i++) {
    canonical_order[i] = bundle_insn[i]->bundling;
  }
  qsort(canonical_order, *bundle_insn_cnt, sizeof(Bundling), cmp_bundling);
  
  for (i = 0; i < *bundle_insn_cnt; i++) {
    hash = (hash * K1NUMBUNDLINGS) + canonical_order[i];
  }
  
  if (hash > bundlematch_table_size) {
    return -1;
  }
  
  canonical_ix = bundlematch_table[hash];
  
  if (canonical_ix == -1) {
    /* No match at all for canonical, on any alignment. */
    return -1;
  }
  
  match = &canonical_table[canonical_ix];
  
  /* Try each bundle type for this canonical. */
  for (i = 0; i < match->entries; i++) {
    int bt = match->entry[i];
    const BundleType *btype = &bundle_types[bt];
    int sec_align = 1 << bfd_get_section_alignment(stdoutput, now_seg);
    /* Our known alignment is current pc modulo section align.
     * That must satisfy the bundle requirements. */
    int cur_align = get_byte_counter(now_seg) % sec_align;

    if ((btype->nnops == 0 || nop_insertion_allowed)
	&& sec_align >= btype->base
	&& (cur_align % btype->base) == btype->bias) {
      /* We have a match. Reorder bundle_insn to match it. */
      int entry, insn;
      int next_nop = 0;
      
      for (entry = 0; entry < *bundle_insn_cnt; entry++) {
	/* Put correct insn in bundle_insn[entry] */
	if (entry == btype->nops[next_nop]) {
	  bundle_insn[(*bundle_insn_cnt)++] = bundle_insn[entry];
	  assemble_tokens("nop", 0, 0);
	  insbuf[insncnt-1].bundling = find_bundling(&insbuf[insncnt-1]);
	  bundle_insn[entry] = &insbuf[insncnt-1];
	  next_nop++;
	}
	else {
	  for (insn = entry; insn < *bundle_insn_cnt; insn++) {
	    if (bundle_insn[insn]->bundling == btype->bundling[entry]) {
	      k1insn_t *t = bundle_insn[entry];
	      bundle_insn[entry] = bundle_insn[insn];
	      bundle_insn[insn] = t;
	      break;
	    }
	  }
	}
      }
      return canonical_ix;
    }
  }

  /* Here if we matched a canonical, but the alignment was not good for
   * any of the bundle types. */
  return -2;
}

/*
 * Given an opcode name and a pre-tokenized
 * set of arguments, take the
 * opcode all the way through emission
 */

static void
assemble_tokens(const char *opname,
        const expressionS * tok,
        int ntok) {
  const k1opc_t *opcode;
  k1insn_t *insn;
  
  /* make sure there is room in instruction buffer */
  
  if (insncnt >= K1MAXINSN) {
    as_fatal("too many instructions in bundle ");
  }

  insn = insbuf + insncnt;

  /* find the instruction in the opcode table */
  
  opcode = (k1opc_t *) hash_find(k1_opcode_hash, opname);
  if (opcode) {
    if (!(opcode = find_format(opcode, tok, ntok))) {
      as_bad("[assemble_tokens] : couldn't find format %s \n",
	     opname);
    }
    else {
      assemble_insn(opcode, tok, ntok, insn);
      insncnt++;
    }
  }
  else {
    as_bad("[assemble_tokens] : couldn't find op %s\n", opname);
  }
}


static int
k1a_is_equivalent_bundle(Bundling b1, Bundling b2){
    switch(b1){
        case Bundling_k1_BCU:
            if(b2 == Bundling_k1_BCU){
                return 1;
            } else {
                return 0;
            }
        case Bundling_k1_ALU:
            if(b2 == Bundling_k1_ALU || b2 == Bundling_k1_ALU_X){
                return 1;
            } else {
                return 0;
            }
        case Bundling_k1_ALUD:
            if(b2 == Bundling_k1_ALUD || b2 == Bundling_k1_ALUD_Z || b2 == Bundling_k1_ALUD_Y){
                return 1;
            } else {
                return 0;
            }
        case Bundling_k1_MAU:
            if(b2 == Bundling_k1_MAU || b2 == Bundling_k1_MAU_X){
                return 1;
            } else {
                return 0;
            }
        case Bundling_k1_LSU:
            if(b2 == Bundling_k1_LSU || b2 == Bundling_k1_LSU_X){
                return 1;
            } else {
                return 0;
            }
        default:
            return 0;
    }
    return 0;
}

/* Reorder a bundle according to BCU, ALU0, ALU1, MAU, LSU, Tiny0, Tiny1  (7 slots)*/
static void
k1a_reorder_bundle(k1insn_t *bundle_insn[], int *bundle_insncnt_p){
    k1insn_t *shadow_bundle[7];
    int bundle_insncnt = *bundle_insncnt_p;
    int i, j;
    int num_ALU = 0;
    int num_BCU = 0;
    int num_MAU = 0;
    int num_LSU = 0;
    int num_TINY = 0;
    int tag = 0;
    int priority[] = {Bundling_k1_BCU, Bundling_k1_ALUD, Bundling_k1_ALU, Bundling_k1_MAU, Bundling_k1_LSU};
    int bundle_type;

    for(i=0; i<bundle_insncnt; i++){
        if(find_bundling(bundle_insn[i]) == Bundling_k1_ALL){
            if(bundle_insncnt == 1){
                return;
            } else {
                as_fatal("Too many ops in a single op bundle\n");
            }
        }
        if(find_bundling(bundle_insn[i]) == Bundling_k1_TINY){
            num_TINY++;
        }
    }

    for(i=0; i < 7; i++){
        shadow_bundle[i] = NULL;
    }


    for(i=0; i < 5 ; i++){
        bundle_type = priority[i];
        for(j=0; j < bundle_insncnt; j++){
            if(k1a_is_equivalent_bundle(bundle_type, find_bundling(bundle_insn[j]))){
                switch(bundle_type){
                    case Bundling_k1_ALU:
                    case Bundling_k1_ALU_X:
                        if(num_ALU > 1){
                            as_fatal("Too many ALU op\n");
                        }
                        if(shadow_bundle[num_ALU + num_BCU] != NULL){
                            as_fatal("Wrong bundle\n");
                        }
                        shadow_bundle[num_ALU + num_BCU] =  bundle_insn[j]; // Put in first available ALU
                        tag = Modifier_k1_exunum_ALU0 + num_ALU;
                        num_ALU++;
                        break;
                    case Bundling_k1_ALUD:
                    case Bundling_k1_ALUD_Y:
                    case Bundling_k1_ALUD_Z:
                        if(num_ALU > 0){
                            as_fatal("Too many ALU op\n");
                        }
                        if(shadow_bundle[i] != NULL || shadow_bundle[i+1] != NULL){
                            as_fatal("Wrong bundle\n");
                        }
                        shadow_bundle[i] = bundle_insn[j];
                        num_ALU = 2;
                        tag = Modifier_k1_exunum_ALU0;
                        break;
                    case Bundling_k1_BCU:
                        if(shadow_bundle[i] != NULL){
                            as_fatal("Wrong bundle\n");
                        }
                        shadow_bundle[i] = bundle_insn[j];
                        num_BCU++;
                        break;
                    case Bundling_k1_MAU:
                    case Bundling_k1_MAU_X:
                        if(shadow_bundle[i] != NULL){
                            as_fatal("Wrong bundle\n");
                        }
                        shadow_bundle[i] = bundle_insn[j];
                        tag = Modifier_k1_exunum_MAU;
                        num_MAU++;
                        break;
                    case Bundling_k1_LSU:
                    case Bundling_k1_LSU_X:
                        if(shadow_bundle[i] != NULL){
                            as_fatal("Wrong bundle\n");
                        }
                        shadow_bundle[i] = bundle_insn[j];
                        tag = Modifier_k1_exunum_LSU;
                        num_LSU++;
                        break;
                    default:
                        as_fatal("Wrong Bundling\n");
                }

                // Tag EXU on IMMX
                if(bundle_insn[j]->immx != NOIMMX){
                    immxbuf[bundle_insn[j]->immx].insn[0] |= (tag << 27);
                    D(stderr, "insn : %#llx (%s), immx : %#llx (%d), tag %d\n", bundle_insn[j]->insn[0], bundle_insn[j]->opdef->as_op,immxbuf[bundle_insn[j]->immx].insn[0], bundle_insn[j]->immx, tag);
                }
                if(bundle_insn[j]->immx64 != NOIMMX){
                    immxbuf[bundle_insn[j]->immx64].insn[0] |= (Modifier_k1_exunum_ALU1 << 27); // immx64 only exist on ALU1 slots
                }
            }
        }
    }

    // Now handle the "TINY" problem : quite easy : put them in ALUs, or append at the end !
    num_TINY = 0;
    for(j=0; j < bundle_insncnt; j++){
        if(find_bundling(bundle_insn[j]) == Bundling_k1_TINY || find_bundling(bundle_insn[j]) == Bundling_k1_TINY_X){
            if(num_ALU < 2){
                shadow_bundle[num_BCU + num_ALU] = bundle_insn[j]; // put in an ALU
		tag = Modifier_k1_exunum_ALU0 + num_ALU;
		num_ALU++;
            } else {
                if(num_TINY + num_MAU + num_LSU > 2){
                    as_fatal("Too many TINY ops\n");
                }
                shadow_bundle[5 + num_TINY] = bundle_insn[j];
                if(num_MAU == 0){
                    tag = Modifier_k1_exunum_MAU;
                    num_MAU++;
                } else {
                    tag = Modifier_k1_exunum_LSU;
                    num_LSU++;
                }
                num_TINY++;
            }
            // Tag EXU on IMMX
            if(bundle_insn[j]->immx != NOIMMX){
                immxbuf[bundle_insn[j]->immx].insn[0] |= (tag << 27);
                D(stderr, "TINY : insn : %#llx (%s), immx : %#llx (%d), tag %d\n", bundle_insn[j]->insn[0],bundle_insn[j]->opdef->as_op, immxbuf[bundle_insn[j]->immx].insn[0], bundle_insn[j]->immx, tag);
            }
        }
    }

    j = 0;
    for(i=0; i < 7; i++){
        if(shadow_bundle[i] != NULL){
            bundle_insn[j] = shadow_bundle[i];
            j++;
        }
    }
    *bundle_insncnt_p = j;
}

static int
k1b_is_equivalent_bundle(Bundling b1, const k1insn_t *insn){
  Bundling b2 = find_bundling(insn);

    switch(b1){
        case Bundling_k1_BCU:
            if(b2 == Bundling_k1_BCU){
                return 1;
            } else {
                return 0;
            }
        case Bundling_k1_ALU:
            if(b2 == Bundling_k1_ALU || b2 == Bundling_k1_ALU_X){
                return 1;
            } else {
                return 0;
            }
        case Bundling_k1_ALUD:
	    if(b2 == Bundling_k1_ALUD || b2 == Bundling_k1_ALUD_Z || b2 == Bundling_k1_ALUD_Y) {
                return 1;
            } else {
                return 0;
            }
        case Bundling_k1_MAU:
            if(b2 == Bundling_k1_MAU || b2 == Bundling_k1_MAU_X){
                return 1;
            } else {
                return 0;
            }
        case Bundling_k1_LSU:
            if(b2 == Bundling_k1_LSU || b2 == Bundling_k1_LSU_X){
                return 1;
            } else {
                return 0;
            }
        default:
            return 0;
    }
    return 0;
}

static int is_mono_double(const k1insn_t *insn) {
  int reservation = find_reservation(insn);
  return (reservation == Reservation_k1_ALUD_LITE   ||
	  reservation == Reservation_k1_ALUD_LITE_X ||
	  reservation == Reservation_k1_ALUD_TINY   ||
	  reservation == Reservation_k1_ALUD_TINY_X);
}


static int used_resources(const k1insn_t *insn, int resource) {
  int insn_reservations = insn->opdef->reservation;
  int reservation = insn_reservations  & 0xff;
  const int *reservation_table = k1_reservation_table_table[reservation];

  if(resource < 0 || resource > RESOURCE_MAX) {
    as_fatal("Unknown resource ID");
  }
  return reservation_table[resource];
}

static int is_tiny(const k1insn_t *insn) {
  return used_resources(insn,Resource_k1_TINY);
}

static int is_lite(const k1insn_t *insn) {
  return used_resources(insn,Resource_k1_LITE);
}

static int is_alud(const k1insn_t *insn) {
  return used_resources(insn,Resource_k1_ALUD);
}

static void bundle_resources(char string_buffer[], int buffer_size, int resource, const k1insn_t *shadow_bundle[], int bundle_size) {
  int i;

  if(buffer_size == 0) { return; }

  string_buffer[0] = '\0';

  for(i=0; i < bundle_size; i++){
    if(shadow_bundle[i] != NULL){
      char tmp_str[256];
      int used_resources_val = used_resources(shadow_bundle[i],resource);
      if(resource == Resource_k1_TINY || resource == Resource_k1_LITE) {
	if(shadow_bundle[i]->slots == (K1_ALU0 | K1_ALU1)) {
	  used_resources_val++;
	}
      }
      snprintf(tmp_str,256,"\t%s: %s slot(s) %d %s(s)\n",
	       shadow_bundle[i]->opdef->as_op,
	       k1_slots_name(shadow_bundle[i]),
	       used_resources_val,
	       k1_resource_names[resource]);
      strncat(string_buffer,tmp_str,buffer_size);
    }
  }
}

static char *k1b_insn_slot_type(const k1insn_t *insn) {
  char *slot_type=k1_slots_name(insn);
  
  if(is_tiny(insn)) {
    slot_type = "TINY";
  }
  if(is_lite(insn)) {
    slot_type = "LITE";
  }
  if(is_mono_double(insn)) {
    slot_type = "MONODOUBLE";
    if(is_tiny(insn)) {
      slot_type = "TINY MONODOUBLE";
    }
    if(is_lite(insn)) {
      slot_type = "LITE MONODOUBLE";
    }
  }
  
  return slot_type;
}

static int get_needed_LITE(const k1insn_t *insn, int k1b_free_resources[RESOURCE_MAX], int total_LITE_MONODOUBLE, int total_TINY_MONODOUBLE, int total_single_LITE, int total_single_TINY) {

  /* If more than one LMD: 1 goes on MAU and the other on ALU0/ALU1 */
  int needed_LITE = total_LITE_MONODOUBLE + total_single_LITE;
  if((total_LITE_MONODOUBLE + total_single_LITE) > 0) {

    D(stderr,"%s:%d OP %s: total_single_LITE + total_single_TINY) = %d, k1b_free_resources[Resource_k1_ALU] = %d\n",
      __FUNCTION__,__LINE__,insn->opdef->as_op,(total_single_LITE + total_single_TINY), k1b_free_resources[Resource_k1_ALU]);

    if((total_single_LITE + total_single_TINY) <= 1 && k1b_free_resources[Resource_k1_ALU] == 2) {
      /* LITE MONO DOUBLE may go on ALU0/ALU1 */
      needed_LITE++;
    }

    D(stderr,"%s:%d OP %s: total_LITE_MONODOUBLE = %d, free LITEs = %d, free ALUs = %d\n",
      __FUNCTION__,__LINE__,insn->opdef->as_op,total_LITE_MONODOUBLE,
      k1b_free_resources[Resource_k1_LITE],k1b_free_resources[Resource_k1_ALU]);

    if(total_LITE_MONODOUBLE > 0 && k1b_free_resources[Resource_k1_LITE] == 2 && k1b_free_resources[Resource_k1_ALU] == 2) {
      /* LITE MONO DOUBLE and no more MAU to place this instr. It must go on ALU0/ALU1 and so use one more LITE */
      needed_LITE++;
    }
  }

  return needed_LITE;
}

static int can_go_on_MAU(const k1insn_t *insn, int num_ALU, int num_MAU, int num_LSU, int total_LITE_MONODOUBLE,
			 int total_TINY_MONODOUBLE, int total_single_LITE, int total_single_TINY) {

  /* We have to decide for LITE instruction. It should be placed on MAU before LSU because 
     LSU cannot execute LITE. */

  int mono_double = is_mono_double(insn);
  int free_ALU = k1b_resources[Resource_k1_ALU] - num_ALU;

  D(stderr,"%s:%d OP %s (%s), num_ALU = %d, free_ALU: %d\n",__FUNCTION__,__LINE__,
    insn->opdef->as_op,k1_type_name(insn), num_ALU, free_ALU );
  
  if((insn->type == K1_TINY || insn->type == K1_TMD) &&
     (total_single_LITE + total_LITE_MONODOUBLE) > 0 &&
     (k1b_resources[Resource_k1_LSU] - num_LSU) > 0 &&
     !(free_ALU == 2 && (total_single_LITE + total_single_TINY) == 1 && (total_TINY_MONODOUBLE + total_LITE_MONODOUBLE) == 1) ) {
    D(stderr,"%s:%d OP %s: is TINY and need LITE slot if LSU slot is available -> say no\n",__FUNCTION__,__LINE__,insn->opdef->as_op);
    return 0;
  }

  D(stderr,"%s:%d OP %s -> say yes\n",__FUNCTION__,__LINE__,insn->opdef->as_op);
  return 1;
}

static int can_go_on_ALU0_ALU1(const k1insn_t *insn, int num_ALU,
			       int num_MAU, int num_LSU,
			       int total_single_TINY,
			       int total_single_LITE,
			       int total_TINY_MONODOUBLE,
			       int total_LITE_MONODOUBLE) {
  int mono_double = is_mono_double(insn);

  int k1b_free_resources[RESOURCE_MAX];
  int needed_LITE = 0;

  /* How to fill ALU0/ALU1:
     ALU0  | ALU1  | MAU  | LSU
     ==========================
      T0   |       |      |
      T0   | T1    |      |
      ALU0 | T0    | LMD0 |
      T0   | T1    | TMD0 |
      T0   | L0    | LMD0 | TMD1
      T0   | T1    | TMD0 | TMD1
      L0   | L1    | TMD0 | TMD1
      L0   | L1    | LMD0 | TMD1
      LMD0 | LMD0  | LMD1 | TMD1
      LMD0 | TMD0  |      |
      LMD0 | LMD1  | T0   |      -> Need always 3 Tiny slots (if T0 goes on ALU0, MD0 cannot use ALU1)
      ...

      Tx: TINY x
      Lx: LITE x
      TMDx: TINY MONO DOUBLE x
      LMDx: LITE MONO DOUBLE x
   */

  D(stderr,"%s:%d %s (%s)\n",
    __FUNCTION__, __LINE__, insn->opdef->as_op,k1b_insn_slot_type(insn));

  D(stderr,"\ttotal_single_TINY: %d\n\ttotal_single_LITE: %d\n\ttotal_TINY_MONODOUBLE: %d\n\ttotal_LITE_MONODOUBLE: %d\n\n",
    total_single_TINY,total_single_LITE, total_TINY_MONODOUBLE, total_LITE_MONODOUBLE);

  /* Constraint 1: still have free ALU slot */
  if(num_ALU > k1b_resources[Resource_k1_ALU]) {
    D(stderr,"%s:%d OP %s no free ALU slot -> say no\n",__FUNCTION__,__LINE__,insn->opdef->as_op);
    return 0;
  }

  k1b_free_resources[Resource_k1_ALU]  = k1b_resources[Resource_k1_ALU] - num_ALU; 

  /* Constraint 2: if mono double still have 2 free ALU slots */
  if(mono_double && k1b_free_resources[Resource_k1_ALU] < 2) {
    D(stderr,"%s:%d OP %s Need ALU0/ALU1 for mono double: not engouh rooms -> say no\n",__FUNCTION__,__LINE__,insn->opdef->as_op);
    return 0;
  }

  /* Constraint 3: Do not use ALU0/ALU1 for mono double if there is enough single TINY/LITE to fill them: avoid lack of TINY/LITE slots */
  if(mono_double && (total_single_LITE + total_single_TINY) >= 2) {
    D(stderr,"%s:%d OP %s is mono double and there is more than 2 LITE/TINY, place TINY/LITE first -> say no\n",__FUNCTION__,__LINE__,insn->opdef->as_op);
    return 0;
  }


  k1b_free_resources[Resource_k1_TINY] = k1b_resources[Resource_k1_TINY] - (num_ALU + num_MAU + num_LSU);
  k1b_free_resources[Resource_k1_LITE] = k1b_resources[Resource_k1_LITE] - (num_ALU + num_MAU);

  /* Constraint 4: If there is 2 free ALU slots and at least 1 mono double and
     1 single TINY or LITE and
     there is room for LITE if current instruction is a LITE:
     -> mono double can go on ALU0/ALU1 */

  if(!mono_double &&
     k1b_free_resources[Resource_k1_ALU] == 2 &&
     (total_LITE_MONODOUBLE + total_TINY_MONODOUBLE) >= 1 &&
     (total_single_TINY + total_single_LITE) == 1 &&
     /* Special case where MAU and so LITE is taken */
     ! (insn->type == K1_LITE && k1b_free_resources[Resource_k1_LITE] == 2 && total_TINY_MONODOUBLE > 0)) {
    D(stderr,"%s:%d OP %s is a TINY or LITE and there is at least one mono double to schedule and there is 2 free ALU slots -> say no\n",__FUNCTION__,__LINE__,insn->opdef->as_op);
    return 0;
  }

  needed_LITE = get_needed_LITE(insn, k1b_free_resources, total_LITE_MONODOUBLE, total_TINY_MONODOUBLE, total_single_LITE, total_single_TINY);


  D(stderr,"%s:%d OP %s needed_LITE: %d, free LITE: %d\n",__FUNCTION__,__LINE__,insn->opdef->as_op,needed_LITE, k1b_free_resources[Resource_k1_LITE]);

  /* Constraint 5: TINY (Single or Mono double) should be placed on LSU if all LITE slots are taken. */
  if((insn->type == K1_TINY || insn->type == K1_TMD) && ((k1b_free_resources[Resource_k1_LITE] - needed_LITE) <= 0)) {
    D(stderr,"%s:%d OP %s is tiny and must go on LSU as there is no more LITE available (need %d LITE(s)) -> say no\n",__FUNCTION__,__LINE__,insn->opdef->as_op,needed_LITE);
    return 0;
  }

  D(stderr,"%s:%d OP %s default -> say yes\n",__FUNCTION__,__LINE__,insn->opdef->as_op);
  return 1;
}

/* Reorder a bundle according to BCU, ALU0, ALU1, MAU, LSU, Tiny0, Tiny1  (7 slots)*/
static void
k1b_reorder_bundle(k1insn_t *bundle_insn[], int *bundle_insncnt_p){

  int shadow_bundle_size = 8;
  k1insn_t *shadow_bundle[shadow_bundle_size];
  int bundle_insncnt = *bundle_insncnt_p;
  int i, j;
  int num_ALU = 0;
  int num_BCU = 0;
  int num_MAU = 0;
  int num_LSU = 0;
  int num_TINY = 0;
  int num_LITE = 0;
  int shadow_idx = 0;

  /* Number of true TINYs (no mono double) */
  int total_single_TINY = 0;
  int total_single_LITE = 0;

  int total_LITE_MONODOUBLE = 0;
  int total_TINY_MONODOUBLE = 0;

  int tag = 0;
  int priority[] = {Bundling_k1_BCU, Bundling_k1_ALUD, Bundling_k1_ALU, Bundling_k1_MAU, Bundling_k1_LSU};
  int bundle_type;

  for(i=0; i<bundle_insncnt; i++){
    if(find_bundling(bundle_insn[i]) == Bundling_k1_ALL){
      if(bundle_insncnt == 1){
	return;
      } else {
	as_fatal("Too many ops in a single op bundle\n");
      }
    }
  }

  for(i=0; i < shadow_bundle_size; i++){
    shadow_bundle[i] = NULL;
  }


  for(i=0; i < 5 ; i++){
    bundle_type = priority[i];
    for(j=0; j < bundle_insncnt; j++){
      if(k1b_is_equivalent_bundle(bundle_type, bundle_insn[j]) ){
	switch(bundle_type){
	case Bundling_k1_ALU:
	case Bundling_k1_ALU_X:
	  if(num_ALU > 1){
	    as_fatal("Too many ALU op\n");
	  }
	  if(shadow_bundle[num_ALU + num_BCU] != NULL){
	    as_fatal("Wrong bundle\n");
	  }

	  bundle_insn[j]->slots = ((num_ALU == 0) ? K1_ALU0 : K1_ALU1);
	  D(stderr,"%s:%d \t%s:\t%s\n", __FUNCTION__, __LINE__, bundle_insn[j]->opdef->as_op, k1_slots_name(bundle_insn[j]));

	  shadow_bundle[num_ALU + num_BCU] =  bundle_insn[j]; // Put in first available ALU
	  tag = Modifier_k1_exunum_ALU0 + num_ALU;
	  num_ALU++;
	  
	  break;
	case Bundling_k1_ALUD:
	case Bundling_k1_ALUD_Y:
	case Bundling_k1_ALUD_Z:
	  if(num_ALU > 0){
	    as_fatal("Too many ALU op\n");
	  }
	  if(shadow_bundle[i] != NULL || shadow_bundle[i+1] != NULL){
	    as_fatal("Wrong bundle\n");
	  }

	  bundle_insn[j]->slots = (K1_ALU0 | K1_ALU1);
	  D(stderr,"%s: %d \t%s:\t%s\n", __FUNCTION__, __LINE__, bundle_insn[j]->opdef->as_op, k1_slots_name(bundle_insn[j]));

	  shadow_bundle[i] = bundle_insn[j];
	  num_ALU = 2;
	  tag = Modifier_k1_exunum_ALU0;
	  break;
	case Bundling_k1_BCU:
	  if(shadow_bundle[i] != NULL){
	    as_fatal("Wrong bundle\n");
	  }
	  bundle_insn[j]->slots = K1_BCU;
	  D(stderr,"%s: %d \t%s:\t%s\n", __FUNCTION__, __LINE__, bundle_insn[j]->opdef->as_op, k1_slots_name(bundle_insn[j]));

	  shadow_bundle[i] = bundle_insn[j];
	  num_BCU++;
	  break;
	case Bundling_k1_MAU:
	case Bundling_k1_MAU_X:
	  if(shadow_bundle[i] != NULL){
	    as_fatal("Wrong bundle\n");
	  }
	  bundle_insn[j]->slots = K1_MAU;
	  D(stderr,"%s: %d \t%s:\t%s\n", __FUNCTION__, __LINE__, bundle_insn[j]->opdef->as_op, k1_slots_name(bundle_insn[j]));

	  shadow_bundle[i] = bundle_insn[j];
	  tag = Modifier_k1_exunum_MAU;
	  num_MAU++;
	  break;
	case Bundling_k1_LSU:
	case Bundling_k1_LSU_X:
	  if(shadow_bundle[i] != NULL){
	    as_fatal("Wrong bundle\n");
	  }
	  bundle_insn[j]->slots = K1_LSU;
	  D(stderr,"%s: %d \t%s:\t%s\n", __FUNCTION__, __LINE__, bundle_insn[j]->opdef->as_op, k1_slots_name(bundle_insn[j]));

	  shadow_bundle[i] = bundle_insn[j];
	  tag = Modifier_k1_exunum_LSU;
	  num_LSU++;
	  break;
	default:
	  as_fatal("Wrong Bundling\n");
	}

	// Tag EXU on IMMX
	if(bundle_insn[j]->immx != NOIMMX){
	  immxbuf[bundle_insn[j]->immx].insn[0] |= (tag << 27);
	  D(stderr, "insn : %#llx (%s), immx : %#llx (%d), tag %d\n", bundle_insn[j]->insn[0], bundle_insn[j]->opdef->as_op,immxbuf[bundle_insn[j]->immx].insn[0], bundle_insn[j]->immx, tag);
	}
	if(bundle_insn[j]->immx64 != NOIMMX){
	  immxbuf[bundle_insn[j]->immx64].insn[0] |= (Modifier_k1_exunum_ALU1 << 27); // immx64 only exist on ALU1 slots
	}
      }
    }
  }

  // Count TINYs en LITEs...
  for(j=0; j < bundle_insncnt; j++){
    int bundling = find_bundling(bundle_insn[j]);
    if(bundling == Bundling_k1_TINY || bundling == Bundling_k1_TINY_X){
      // [JV] Counter total number of TINYs. It includes mono double!
      // It is used to decide where to insert mono double and LITE: On ALU0 + ALU1 or MAU/LSU.
      if(is_mono_double(bundle_insn[j]) ) {
	if(is_lite(bundle_insn[j])) {
	  bundle_insn[j]->type = K1_LMD;
	  total_LITE_MONODOUBLE++;
	}
	else if(is_tiny(bundle_insn[j])) {
	  bundle_insn[j]->type = K1_TMD;
	  total_TINY_MONODOUBLE++;
	}
	else {
	  as_fatal(_("%s is mono double but not LITE nor TINY"),bundle_insn[j]->opdef->as_op);
	}
      }
      else {
	if(is_lite(bundle_insn[j])) {
	  if(!is_tiny(bundle_insn[j])) {
	    as_fatal(_("%s: is LITE and not TINY!"),bundle_insn[j]->opdef->as_op);
	  }
	  bundle_insn[j]->type = K1_LITE;
	  total_single_LITE++;
	}
	else if(is_tiny(bundle_insn[j])) {
	  bundle_insn[j]->type = K1_TINY;
	  total_single_TINY++;
	}
      }
    }
  }

  // Now handle the "TINY" problem : quite easy : put them in ALUs, or append at the end !
  num_TINY = num_ALU;
  num_LITE = num_ALU;
  shadow_idx = 0;
  for(j=0; j < bundle_insncnt; j++){
    if(find_bundling(bundle_insn[j]) == Bundling_k1_TINY || find_bundling(bundle_insn[j]) == Bundling_k1_TINY_X){
      if(num_ALU < 2 && can_go_on_ALU0_ALU1(bundle_insn[j],num_ALU,num_MAU,num_LSU,
					    total_single_TINY,total_single_LITE,total_TINY_MONODOUBLE,total_LITE_MONODOUBLE) ) {

	shadow_bundle[num_BCU + num_ALU] = bundle_insn[j]; // put in an ALU
	tag = Modifier_k1_exunum_ALU0 + num_ALU;

	// Mono double reserves ALU0 and ALU1
	// if is ALUD => mono double because all FULL ALUD have already a slot (previous for loop).
	if(is_alud(bundle_insn[j])) {
	  num_ALU++;
  	  bundle_insn[j]->slots = (K1_ALU0 | K1_ALU1);
	  if(is_lite(bundle_insn[j])) {
	    total_LITE_MONODOUBLE--;
	  }
	  else if(is_tiny(bundle_insn[j])) {
	    total_TINY_MONODOUBLE--;
	  }
	  else {
	    as_fatal(_("%s is mono double but not TINY nor LITE"),bundle_insn[j]->opdef->as_op);
	  }
	}
	else {
  	  bundle_insn[j]->slots = ((num_ALU == 0) ? K1_ALU0 : K1_ALU1);
	  total_single_TINY--;
	}

	num_ALU++;

	if(is_tiny(bundle_insn[j])) {
	  num_TINY++;
	  D(stderr,"%s:%d %s: num_TINY: %d\n", __FUNCTION__, __LINE__,bundle_insn[j]->opdef->as_op, num_TINY);
	  if(is_alud(bundle_insn[j])) {
	    num_TINY++;
	    D(stderr,"%s:%d %s: num_TINY: %d\n", __FUNCTION__, __LINE__,bundle_insn[j]->opdef->as_op, num_TINY);
	  }
	}
	if(is_lite(bundle_insn[j])) {
	  num_LITE++;
	  if(is_alud(bundle_insn[j])) {
	    num_LITE++;
	  }
	  else {
	    total_single_LITE--;
	  }
	}
      } else {
	if(num_MAU == 0 && can_go_on_MAU(bundle_insn[j], num_ALU, num_MAU, num_LSU,
					 total_LITE_MONODOUBLE, total_TINY_MONODOUBLE, total_single_LITE, total_single_TINY)){
	  // 5 is reserved for MAU. LSU and all others goes after.
	  shadow_bundle[5] = bundle_insn[j];
	  tag = Modifier_k1_exunum_MAU;
	  num_MAU++;
  	  bundle_insn[j]->slots = K1_MAU;
	} else {
	  shadow_bundle[6 + shadow_idx++] = bundle_insn[j];
	  tag = Modifier_k1_exunum_LSU;
	  num_LSU++;
  	  bundle_insn[j]->slots = K1_LSU;
	}

	// Updating number of TINY/LITE used.
	if(is_alud(bundle_insn[j])) {
	  if(is_lite(bundle_insn[j])) {
	    total_LITE_MONODOUBLE--;
	  }
	  else if(is_tiny(bundle_insn[j])) {
	    total_TINY_MONODOUBLE--;
	  }
	  else {
	    as_fatal(_("%s is mono double but not TINY nor LITE"),bundle_insn[j]->opdef->as_op);
	  }
	}

	if(!is_alud(bundle_insn[j])) {
	  total_single_TINY--;
	}

	if(is_tiny(bundle_insn[j])) {
	  num_TINY++;
	  D(stderr,"%s:%d %s: num_TINY: %d\n", __FUNCTION__, __LINE__,bundle_insn[j]->opdef->as_op, num_TINY);
	}
	if(is_lite(bundle_insn[j])) {
	  num_LITE++;
	  if(!is_alud(bundle_insn[j])) {
	    total_single_LITE--;
	  }
	}
      }

      D(stderr,"%s:%d \t%s:\t%s\tTag: 0x%x\t(%s)\n", __FUNCTION__, __LINE__,
	bundle_insn[j]->opdef->as_op, k1_slots_name(bundle_insn[j]), tag,
	k1b_insn_slot_type(bundle_insn[j]));
      
      // Tag EXU on IMMX
      if(bundle_insn[j]->immx != NOIMMX){
	immxbuf[bundle_insn[j]->immx].insn[0] |= (tag << 27);
	D(stderr, "TINY : insn : %#llx (%s), immx : %#llx (%d), tag %d\n", bundle_insn[j]->insn[0],bundle_insn[j]->opdef->as_op, immxbuf[bundle_insn[j]->immx].insn[0], bundle_insn[j]->immx, tag);
      }
    }
  }

  if(total_single_TINY > 0) {
    as_fatal(_("total_single_TINY should be equal to zero, is equal to %d"),total_single_TINY);
  }

  if(total_single_LITE > 0) {
    as_fatal(_("total_single_LITE should be equal to zero, is equal to %d"),total_single_LITE);
  }

  /* Doing some checks on TINY/LITE (always enabled) */
  {
    if(num_TINY > k1_core_info->resources[Resource_k1_TINY]) {
      char string_buffer[1024];
      
      bundle_resources(string_buffer,1024,Resource_k1_TINY,shadow_bundle,shadow_bundle_size);
      as_fatal(_("Too many TINY ops (used %d, available: %d):\n%s"),
	       num_TINY,
	       k1_core_info->resources[Resource_k1_TINY],
	       string_buffer);
    }
    
    if(num_LITE > k1_core_info->resources[Resource_k1_LITE]) {
      char string_buffer[1024];
      
      bundle_resources(string_buffer,1024,Resource_k1_LITE,shadow_bundle,shadow_bundle_size);
      as_fatal(_("Too many LITE ops (used %d, available: %d):\n%s"),
	       num_LITE,
	       k1_core_info->resources[Resource_k1_LITE],
	       string_buffer);
    }
  }
  
  D(stderr,"\nFinal bundle:\n");
  j = 0;
  for(i=0; i < 7; i++){
    if(shadow_bundle[i] != NULL){
      char *slot_type = k1b_insn_slot_type(shadow_bundle[i]);
      bundle_insn[j] = shadow_bundle[i];
      
      D(stderr,"%s:%d \t%s:\t%s\tTag: 0x%x\t(%s: %s)\n", __FUNCTION__, __LINE__,
	bundle_insn[j]->opdef->as_op, k1_slots_name(bundle_insn[j]), tag,
	bundling_names(find_bundling(bundle_insn[j])), slot_type);

      if(bundle_insn[j]->slots == K1_LSU && (bundle_insn[j]->type == K1_LITE || bundle_insn[j]->type == K1_LMD)) {
	char string_buffer[1024];
      
	bundle_resources(string_buffer,1024,Resource_k1_LITE,shadow_bundle,shadow_bundle_size);
	as_fatal(_("LITE instruction '%s' on LSU slot:\n%s\n"),
		 bundle_insn[j]->opdef->as_op,
		 string_buffer);
      }
      
      j++;
    }
  }

  D(stderr,";;\n\n");
  
  *bundle_insncnt_p = j;
}


/* called by core to assemble a single line */

void
md_assemble(char *s)
 {
    char *t;
    int i;
    int tlen;
    char opname[32];
    expressionS tok[K1MAXOPERANDS];
    char *tok_begins[2*K1MAXOPERANDS];
    int ntok;
    int start_bundle;

    if (get_byte_counter(now_seg) & 3)
        as_fatal("code segment not word aligned in md_assemble\n");

    t = s;

    while (t && t[0] && (t[0] == ' '))
        t++;

    /* ;; was converted to "be" by line hook          */
    /* here we look for the bundle end                */
    /* and actually output any instructions in bundle */
    /* also we need to implement the stop bit         */
    /* check for bundle end */

    if (strncmp(t, "be", 2) == 0) {
        int j;
        int sec_align;

        inside_bundle = 0;

        unwind_bundle_count++;	/* count of bundles in current proc */

        sec_align = bfd_get_section_alignment(stdoutput, now_seg);

        {

            k1insn_t *bundle_insn[K1MAXINSN];
            int bundle_insn_cnt = 0;
            int syllables = 0;
            int entry;
            int bundle_err_done = 0;
            int align_warn_done = 0; /* Alignment contraint warning already
             * raised for this bundle or not */

            /* retain bundle start adress for error messages */
            start_bundle = get_byte_counter(now_seg);

#ifdef OBJ_ELF
            /* Emit Dwarf debug line information */
            dwarf2_emit_insn(0);
#endif
            for (j = 0; j < insncnt; j++) {
                Bundling bundling = find_bundling(&insbuf[j]);
                insbuf[j].bundling = bundling;
                bundle_insn[bundle_insn_cnt++] = &insbuf[j];
                syllables += insbuf[j].len;
                if (sec_align < K1LANEALIGNMENT && !align_warn_done) {
                    if (insbuf[j].len == 2) {
                        align_warn_done = 1;
                        as_bad("Minimum section alignment of %d required due to instruction (%s) requiring extended immediate (current alignment is %d)", (1 << K1LANEALIGNMENT), insbuf[j].opdef->as_op, (1 << sec_align));
                    }
                }
            }

            if(syllables + immxcnt > K1MAXBUNDLEWORDS){
                as_bad("Bundle has too many syllables : %d instead of %d\n", syllables + immxcnt, K1MAXBUNDLEWORDS);
            }

            /* Check that resources are not oversubscribed.
             * We check only for a single bundle, so resources that are used
             * in multiple cycles will not be fully checked. */
            if (check_resource_usage) {
                const int reservation_table_len = (k1_reservation_table_lines * k1_resource_max);
                const int *resources = k1_core_info->resources;
                int *resources_used;

                resources_used = (int *)alloca(reservation_table_len * sizeof (int));
                memset(resources_used, 0, reservation_table_len * sizeof (int));

                for (i = 0; i < bundle_insn_cnt; i++) {
                    int insn_reservations = bundle_insn[i]->opdef->reservation;
                    int reservation = insn_reservations  & 0xff;
                    const int *reservation_table = k1_reservation_table_table[reservation];
                    for (j = 0; j < reservation_table_len; j++)
                        resources_used[j] += reservation_table[j];
                }
                for (i = 0; i < k1_reservation_table_lines; i++) {
                    for (j = 0; j < k1_resource_max; j++)
                        if (resources_used[(i * k1_resource_max) + j] > resources[j]) {
                            as_bad("Resource %s over-used in bundle: %d used, %d available", k1_resource_names[j], resources_used[(i * k1_resource_max) + j], resources[j]);
                            bundle_err_done = TRUE;
                        }
                }
            }

            if(!generate_illegal_code){
              // reorder and check the bundle
              reorder_bundle(bundle_insn, &bundle_insn_cnt);
            }

            /* The ordering of the insns has been set correctly
             * in bundle_insn. */
            for (entry = 0; entry < bundle_insn_cnt; entry++) {
                emit_insn(bundle_insn[entry], (entry == (bundle_insn_cnt + immxcnt - 1)));
                bundle_insn[entry]->written = 1;
            }
            // Emit immx, ordering them by EXU tags, 0 to 3
            entry = 0;
            for(i=0; i < 4; i++){
              for (j = 0; j < immxcnt; j++) {
                  if(k1_exunum2_fld(immxbuf[j].insn[0]) == i){
                      emit_insn(&(immxbuf[j]), (entry == (immxcnt - 1)));
                      immxbuf[j].written = 1;
                      entry++;
                  }
              }
            }

            // fprintf(stderr, "Emit %d + %d syllables\n", bundle_insn_cnt, immxcnt);

	}

	{
	    /* The debug label that appear in the middle of bundles
	       had better appear to be attached to the next
	       bundle. This is because usually these labels point to
	       the first instruction where some condition is met. If
	       the label isn't handled this way it will be attached to
	       the current bundle which is wrong as the conresponding
	       instruction isn't executed yet. */
	    while (label_fixes) {
		struct label_fix *fix = label_fixes;

		label_fixes = fix->next;
		symbol_set_value_now (fix->sym);
		free (fix);
	    }
	}

        insncnt = 0;
        immxcnt = 0;
        return;
    }


    /* get opcode info    */

    while (t && t[0] && (t[0] == ' '))
        t++;
    i = strspn(t, "abcdefghijklmnopqrstuvwxyz.,_0123456789");
    tlen = (i < 31) ? i : 31;
    memcpy(opname, t, tlen);
    opname[tlen] = '\0';

    t += i;

    /* parse arguments             */

    if ((ntok = tokenize_arguments(t, tok, tok_begins, K1MAXOPERANDS)) < 0) {
      if(error_str != NULL) {
          as_bad("syntax error at: %s", error_str);
          error_str = NULL;
      }
      else {
          as_bad("syntax error");
      }
    }

    inside_bundle = 1;
    /* build an instruction record */

    assemble_tokens(opname, tok, ntok);
}

static void
k1_set_cpu(void) {
  if (!k1_core_info) {
      k1_core_info = &k1a_core_info;
      bfd_set_arch_mach(stdoutput, TARGET_ARCH, bfd_mach_k1dp);
  }

  if(!k1_registers) {
    k1_registers = k1_k1a_registers;
  }

  if(!k1_regfiles) {
    k1_regfiles = k1_k1a_regfiles;
  }

  switch(k1_core_info->elf_cores[subcore_id]) {
  case ELF_K1_CORE_DP:
    bfd_set_arch_mach(stdoutput, TARGET_ARCH, bfd_mach_k1dp);
    reorder_bundle = k1a_reorder_bundle;
    break;
  case ELF_K1_CORE_IO:
    bfd_set_arch_mach(stdoutput, TARGET_ARCH, bfd_mach_k1io);
    reorder_bundle = k1a_reorder_bundle;
    break;
  case ELF_K1_CORE_B_DP:
    bfd_set_arch_mach(stdoutput, TARGET_ARCH, bfd_mach_k1bdp);
    reorder_bundle = k1b_reorder_bundle;
    break;
  case ELF_K1_CORE_B_IO:
    bfd_set_arch_mach(stdoutput, TARGET_ARCH, bfd_mach_k1bio);
    reorder_bundle = k1b_reorder_bundle;
    break;
  default:
    as_fatal("Unknown elf core: %d\n",k1_core_info->elf_cores[subcore_id]);
  }
}

static int k1op_compar(const void *a, const void *b)
 {
    const k1opc_t *opa = (const k1opc_t *)a;
    const k1opc_t *opb = (const k1opc_t *)b;
    return strcmp(opa->as_op, opb->as_op);
}

/***************************************************/
/*    INITIALIZE ASSEMBLER                          */
/***************************************************/


static void
print_hash(const char *key, PTR val){
  printf("%s\n", key);
}
 

void
md_begin()
 {
    int i;
    k1_set_cpu();


    /*
     * alias register names with symbols
     */
    for(i = 0; i < k1_regfiles[K1_REGFILE_REGISTERS]; i++) {
      symbol_table_insert(symbol_create(k1_registers[i].name, reg_section, i, &zero_address_frag));
    }

    /* Sort optab, so that identical mnemonics appear consecutively */
    {
        int nel;
        for (nel = 0; !STREQ("", k1_core_info->optab[nel].as_op); nel++) ;
        qsort(k1_core_info->optab, nel, sizeof(k1_core_info->optab[0]), k1op_compar);
    }

    /* The '?' is an operand separator */
    lex_type['?'] = 0;

    /* Create the opcode hash table      */
    /* Each name should appear only once */

    k1_opcode_hash = hash_new();
 {
        k1opc_t *op;
        const char *name = 0;
        const char *retval = 0;
        for (op = k1_core_info->optab; !(STREQ("", op->as_op)) ; op++)
 {

            /* enter in hash table if this is a new name */

            if (!(STREQ(name, op->as_op)))
 {
                name = op->as_op;
                retval = hash_insert(k1_opcode_hash, name, (PTR) op);
                if (retval)
                    as_fatal("internal error: can't hash opcode `%s': %s",
                            name, retval);
            }
        }
 }

    if(dump_table){
      hash_traverse(k1_opcode_hash, print_hash);
      exit(0);
    }

    /* Here we enforce the minimum section alignment.  Remember, in
     * the linker we can make the boudaries between the linked sections
     * on larger boundaries.  The text segment is aligned to long words
     * because of the odd/even constraint on immediate extensions
     */

    bfd_set_section_alignment(stdoutput, text_section, 3);	/* -- 8 bytes */
    bfd_set_section_alignment(stdoutput, data_section, 2);	/* -- 4 bytes */
    bfd_set_section_alignment(stdoutput, bss_section, 2);	/* -- 4 bytes */
    subseg_set(text_section, 0);

    /*
     * pseudo_func[FUNC_FPTR_RELATIVE].u.sym =
     * symbol_create (".<fptr>", undefined_section, FUNC_FPTR_RELATIVE,
     * &zero_address_frag);
     */
    pseudo_func[FUNC_GP_RELATIVE].u.sym =
        symbol_create (".<gprel>", undefined_section, FUNC_GP_RELATIVE,
                       &zero_address_frag);

    pseudo_func[FUNC_GP_10_RELATIVE].u.sym =
        symbol_create (".<gprel10>", undefined_section, FUNC_GP_10_RELATIVE,
                       &zero_address_frag);

    pseudo_func[FUNC_GP_16_RELATIVE].u.sym =
        symbol_create (".<gprel16>", undefined_section, FUNC_GP_16_RELATIVE,
                       &zero_address_frag);

    pseudo_func[FUNC_GOTOFF_RELATIVE].u.sym =
        symbol_create (".<gotoff>", undefined_section, FUNC_GOT_RELATIVE,
                       &zero_address_frag);

    pseudo_func[FUNC_GOT_RELATIVE].u.sym =
        symbol_create (".<got>", undefined_section, FUNC_GOT_RELATIVE,
        &zero_address_frag);
        
    pseudo_func[FUNC_PLT_RELATIVE].u.sym =
        symbol_create (".<plt>", undefined_section, FUNC_PLT_RELATIVE,
        &zero_address_frag);
        
    pseudo_func[FUNC_GOT_FDESC_RELATIVE].u.sym =
        symbol_create (".<got.fdesc>", undefined_section, FUNC_GOT_FDESC_RELATIVE,
        &zero_address_frag);
    pseudo_func[FUNC_GOTOFF_FDESC_RELATIVE].u.sym =
        symbol_create (".<gotoff.fdesc>", undefined_section, FUNC_GOTOFF_FDESC_RELATIVE,
        &zero_address_frag);
    pseudo_func[FUNC_FDESC_RELATIVE].u.sym =
        symbol_create (".<fdesc>", undefined_section, FUNC_FDESC_RELATIVE,
        &zero_address_frag);


    /* pseudo_func[FUNC_GOTX_RELATIVE].u.sym =
     * symbol_create (".<gotoffx>", undefined_section, FUNC_GOTX_RELATIVE,
     * &zero_address_frag);
     *
     *
     * pseudo_func[FUNC_SEG_RELATIVE].u.sym =
     * symbol_create (".<segrel>", undefined_section, FUNC_SEG_RELATIVE,
     * &zero_address_frag);
     *
     * pseudo_func[FUNC_LTV_RELATIVE].u.sym =
     * symbol_create (".<ltv>", undefined_section, FUNC_LTV_RELATIVE,
     * &zero_address_frag);
     *
     * pseudo_func[FUNC_GOT_FPTR_RELATIVE].u.sym =
     * symbol_create (".<gotoff.fptr>", undefined_section, FUNC_GOT_FPTR_RELATIVE,
     * &zero_address_frag);
     *
     * pseudo_func[FUNC_IPLT_RELOC].u.sym =
     * symbol_create (".<iplt>", undefined_section, FUNC_IPLT_RELOC,
     * &zero_address_frag);
     *
     * pseudo_func[FUNC_NEG_GP_RELATIVE].u.sym =
     * symbol_create (".<neggprel>", undefined_section, FUNC_NEG_GP_RELATIVE,
     * &zero_address_frag);
     */
    
    pseudo_func[FUNC_TP_RELATIVE].u.sym =
	symbol_create (".<tprel>", undefined_section, FUNC_TP_RELATIVE,
		       &zero_address_frag);
//     pseudo_func[FUNC_TP_RELATIVE].u.sym =
//         symbol_create (".<pcrel>", undefined_section, FUNC_PC_RELATIVE,
//                        &zero_address_frag);
    /*
     * pseudo_func[FUNC_DTP_RELATIVE].u.sym =
     * symbol_create (".<dtprel>", undefined_section, FUNC_DTP_RELATIVE,
     * &zero_address_frag);
     *
     * pseudo_func[FUNC_DTP_MODULE].u.sym =
     * symbol_create (".<dtpmod>", undefined_section, FUNC_DTP_MODULE,
     * &zero_address_frag);
     *
     * pseudo_func[FUNC_DTP_INDEX].u.sym =
     * symbol_create (".<dtpndx>", undefined_section, FUNC_DTP_INDEX,
     * &zero_address_frag);
     *
     * pseudo_func[FUNC_DTP_LOAD_MODULE].u.sym =
     * symbol_create (".<dtpldm>", undefined_section, FUNC_DTP_LOAD_MODULE,
     * &zero_address_frag);
     *
     * pseudo_func[FUNC_GOT_TP_RELATIVE].u.sym =
     * symbol_create (".<gotoff.tprel>", undefined_section, FUNC_GOT_TP_RELATIVE,
     * &zero_address_frag);
     *
     * pseudo_func[FUNC_GOT_DTP_INDEX_RELATIVE].u.sym =
     * symbol_create (".<gotoff.dtpndx>", undefined_section, FUNC_GOT_DTP_INDEX_RELATIVE,
     * &zero_address_frag);
     *
     * pseudo_func[FUNC_GOT_DTP_LOAD_MODULE_RELATIVE].u.sym =
     * symbol_create (".<gotoff.dtpldm>", undefined_section, FUNC_GOT_DTP_LOAD_MODULE_RELATIVE,
     * &zero_address_frag);
     */
    /*  bfd_set_private_flags(stdoutput, 0); *//* default flags */
}

/***************************************************/
/*          ASSEMBLER CLEANUP STUFF                */
/***************************************************/

#if 0
static void
md_after_pass(void)		/* called from md_end */
 {
}
#endif

/***************************************************/
/*          ASSEMBLER FIXUP STUFF                  */
/***************************************************/

void
md_apply_fix(fixS * fixP, valueT * valueP,
        segT segmentP ATTRIBUTE_UNUSED)
 {
    bfd_byte *buf;
    long insn;
    char *const fixpos = fixP->fx_frag->fr_literal + fixP->fx_where;
    //char *const fixpos2 = fixP->fx_frag->fr_literal + fixP->fx_where - 4;
    valueT value = *valueP;
    //valueT value2 = *valueP;
    unsigned image;
    arelent *rel;

    rel = (arelent *)xmalloc(sizeof(arelent));

    rel->howto = bfd_reloc_type_lookup(stdoutput, fixP->fx_r_type);
    if(rel->howto == NULL){
        as_fatal("[md_apply_fix] unsupported relocation type");
    }

    if (fixP->fx_addsy == NULL && fixP->fx_pcrel == 0)
        fixP->fx_done = 1;

    if (fixP->fx_size > 0)
        image = md_chars_to_number(fixpos, fixP->fx_size);
    else
        image = 0;
    if (fixP->fx_addsy != NULL)
 {
        switch (fixP->fx_r_type)
          {
            case BFD_RELOC_K1_TPREL_HI22:
            case BFD_RELOC_K1_TPREL_LO10:
            case BFD_RELOC_K1_TPREL_32:
//             case BFD_RELOC_K1_GOTOFF_HI22:
//             case BFD_RELOC_K1_GOTOFF_LO10:
                S_SET_THREAD_LOCAL (fixP->fx_addsy);
                break;
            default:
                break;
          }
    }
    /* Convert GPREL10 to GPREL16 for instruction 'make' */
    /* FIXME: [AP] is there a better way to do that using some
     * MDS mechanisms? */
    if (fixP->fx_r_type == BFD_RELOC_K1_10_GPREL)
    {
      buf = (bfd_byte *) (fixP->fx_frag->fr_literal + fixP->fx_where);
      insn = (buf[3] << 24) | (buf[2] << 16) | (buf[1] << 8) | buf[0];
      if ((insn & 0x7f030000) == 0x60000000)
        fixP->fx_r_type = BFD_RELOC_K1_16_GPREL;
    }
    switch (fixP->fx_r_type)
 {
//    case BFD_RELOC_8:
        case BFD_RELOC_16:
        case BFD_RELOC_32:
        case BFD_RELOC_K1_TPREL_32:
        case BFD_RELOC_K1_FUNCDESC:
        case BFD_RELOC_K1_GLOB_DAT:
        case BFD_RELOC_K1_GOT:
        case BFD_RELOC_K1_GOTOFF:
            //  case BFD_RELOC_32_PCREL:
            image = value;
            md_number_to_chars(fixpos, image, fixP->fx_size);
            break;
        case BFD_RELOC_K1_17_PCREL:
        case BFD_RELOC_K1_18_PCREL:
        case BFD_RELOC_K1_27_PCREL:
            if (fixP->fx_pcrel || fixP->fx_addsy)
                return;
            value = (((value >> rel->howto->rightshift) << rel->howto->bitpos ) & rel->howto->dst_mask);
            image = (image & ~(rel->howto->dst_mask)) | value;
            md_number_to_chars(fixpos, image, fixP->fx_size);
            break;
        case BFD_RELOC_K1_HI22:
        case BFD_RELOC_K1_TPREL_HI22:
//         case BFD_RELOC_K1_PCREL_HI22:
        case BFD_RELOC_K1_GPREL_HI22:
//    case BFD_RELOC_K1_NEG_GPREL_HI23:
        case BFD_RELOC_K1_GOTOFF_HI22:
        case BFD_RELOC_K1_GOT_HI22:
//    case BFD_RELOC_K1_GOTOFFX_HI23:
//    case BFD_RELOC_K1_GOTOFF_FPTR_HI23:
        case BFD_RELOC_K1_PLT_HI22:
//    case BFD_RELOC_K1_TPREL_HI23:
//    case BFD_RELOC_K1_GOTOFF_TPREL_HI23:
//    case BFD_RELOC_K1_GOTOFF_DTPLDM_HI23:
//    case BFD_RELOC_K1_DTPREL_HI23:
//    case BFD_RELOC_K1_GOTOFF_DTPNDX_HI23:
        case BFD_RELOC_K1_FUNCDESC_GOT_HI22:
	case BFD_RELOC_K1_FUNCDESC_GOTOFF_HI22:
//
            value = (((value >> rel->howto->rightshift)  << rel->howto->bitpos ) &  rel->howto->dst_mask);
            image = (image & ~(rel->howto->dst_mask)) | value;
/*
            rel->howto = bfd_reloc_type_lookup(stdoutput, BFD_RELOC_K1_LO10);
            if(rel->howto == NULL){
                as_fatal("[md_apply_fix] unsupported relocation type");
            }
*/
            /* Create a mask with the size of the (small) immediate
            mask = (1 << rel->howto->bitsize) - 1;
            value2 = (value2 & mask);
            image2 = md_chars_to_number(fixpos2, fixP->fx_size);

            image2 = image2 & (~((mask) << rel->howto->bitpos));
            image2 = image2 | (value2 << rel->howto->bitpos);
            image2 |= 0x80000000;
            */
            md_number_to_chars(fixpos, image, fixP->fx_size);

            //md_number_to_chars(fixpos2, image2, fixP->fx_size);
            break;
        case BFD_RELOC_K1_LO10:
        case BFD_RELOC_K1_TPREL_LO10:
//         case BFD_RELOC_K1_PCREL_LO10:
        case BFD_RELOC_K1_GPREL_LO10:
//    case BFD_RELOC_K1_NEG_GPREL_LO9:
        case BFD_RELOC_K1_GOTOFF_LO10:
        case BFD_RELOC_K1_GOT_LO10:
//    case BFD_RELOC_K1_GOTOFFX_LO9:
//    case BFD_RELOC_K1_GOTOFF_FPTR_LO9:
        case BFD_RELOC_K1_PLT_LO10:
        case BFD_RELOC_K1_FUNCDESC_GOT_LO10:
	case BFD_RELOC_K1_FUNCDESC_GOTOFF_LO10:
//    case BFD_RELOC_K1_TPREL_LO9:
//    case BFD_RELOC_K1_GOTOFF_TPREL_LO9:
//    case BFD_RELOC_K1_GOTOFF_DTPLDM_LO9:
//    case BFD_RELOC_K1_DTPREL_LO9:
//    case BFD_RELOC_K1_GOTOFF_DTPNDX_LO9:
        case BFD_RELOC_K1_10_GPREL:
        case BFD_RELOC_K1_16_GPREL:
            value = (((value >> rel->howto->rightshift) << rel->howto->bitpos ) & rel->howto->dst_mask);
            image = (image & ~(rel->howto->dst_mask)) | value;
            md_number_to_chars(fixpos, image, fixP->fx_size);

            break;
//    case BFD_RELOC_K1_REL32:
//    case BFD_RELOC_K1_FPTR32:
//    case BFD_RELOC_K1_IPLT:
//    case BFD_RELOC_K1_LTV32:
//    case BFD_RELOC_K1_SEGREL32:
//    case BFD_RELOC_K1_JMP_SLOT:
//    case BFD_RELOC_K1_DTPMOD32:
//    case BFD_RELOC_K1_DTPREL32:
//    case BFD_RELOC_K1_TPREL32:
//      break;
        default:
            as_fatal("[md_apply_fix] unsupported relocation type");
    }
}

void
k1_validate_fix(fixS *fix)
 {
    switch (fix->fx_r_type)
 {
        // case BFD_RELOC_K1_FPTR32:
        // case BFD_RELOC_K1_GOTOFF_FPTR_LO9:
        // case BFD_RELOC_K1_GOTOFF_FPTR_HI23:
        //   if (fix->fx_offset != 0)
        //     as_bad_where (fix->fx_file, fix->fx_line,
        //     	      "No addend allowed in @fptr() relocation");
        //   break;
        default:
            break;
    }

    return;
}

/*
 * Warning: Can be called only in fixup_segment() after fx_addsy field
 * has been updated by calling symbol_get_value_expression(...->X_add_symbol)
 */
int
k1_validate_sub_fix(fixS *fixP)
 {
    segT add_symbol_segment, sub_symbol_segment;

    switch (fixP->fx_r_type)
 {
        case BFD_RELOC_16:
        case BFD_RELOC_32:
            if (fixP->fx_addsy != NULL)
                add_symbol_segment = S_GET_SEGMENT(fixP->fx_addsy);
            else
                return 0;
            if (fixP->fx_subsy != NULL)
                sub_symbol_segment = S_GET_SEGMENT(fixP->fx_subsy);
            else
                return 0;

            if ((strcmp(S_GET_NAME(fixP->fx_addsy),
                    S_GET_NAME(fixP->fx_subsy)) == 0) &&
                    (add_symbol_segment == sub_symbol_segment)) {
                return 1;
            }

            break;
        default:
            break;
    }

    return 0;
}

/* This is called whenever some data item (not an instruction) needs a
 * fixup.  */
void
k1_cons_fix_new(fragS *f, int where, int nbytes, expressionS *exp)
 {
    bfd_reloc_code_real_type code;

    if (exp->X_op == O_pseudo_fixup)
 {
        exp->X_op = O_symbol;
        real_k1_reloc_type(exp->X_op_symbol, 0, 0, &code);
        if (code == BFD_RELOC_UNUSED)
            as_bad("Unsupported relocation");
    }
    else
        switch (nbytes)
 {
            /* [SC] We have no relocation for BFD_RELOC_8, but accept it
             * here in case we can later eliminate the fixup (in md_apply_fix).
             * This is required to pass the gas test forward.s.
             */
            case 1:
                code = BFD_RELOC_8;
                break;
            case 2:
                code = BFD_RELOC_16;
                break;
            case 4:
                code = BFD_RELOC_32;
                break;
            default:
                as_bad("unsupported BFD relocation size %u", nbytes);
                code = BFD_RELOC_32;
                break;
        }

        fix_new_exp(f, where, nbytes, exp, 0, code);
}

/*
 * generate a relocation record
 */

arelent *
        tc_gen_reloc(asection * sec ATTRIBUTE_UNUSED, fixS * fixp)
 {
    arelent *reloc;
    bfd_reloc_code_real_type code;

    reloc = (arelent *) xmalloc(sizeof (arelent));

    reloc->sym_ptr_ptr = (asymbol **) xmalloc(sizeof (asymbol *));
    *reloc->sym_ptr_ptr = symbol_get_bfdsym(fixp->fx_addsy);
    reloc->address = fixp->fx_frag->fr_address + fixp->fx_where;

    code = fixp->fx_r_type;
    if (code == BFD_RELOC_32 && fixp->fx_pcrel)
 {
        code = BFD_RELOC_32_PCREL;
    }
    reloc->howto = bfd_reloc_type_lookup(stdoutput, code);

    if (reloc->howto == NULL)
 {
        as_bad_where(fixp->fx_file, fixp->fx_line,
                "cannot represent `%s' relocation in object file",
                bfd_get_reloc_code_name(code));
        return NULL;
    }

    if (!fixp->fx_pcrel != !reloc->howto->pc_relative)
 {
        as_fatal("internal error? cannot generate `%s' relocation",
                bfd_get_reloc_code_name(code));
    }
    assert(!fixp->fx_pcrel == !reloc->howto->pc_relative);

    reloc->addend = fixp->fx_offset;

    /*
     * Ohhh, this is ugly.  The problem is that if this is a local global
     * symbol, the relocation will entirely be performed at link time, not
     * at assembly time.  bfd_perform_reloc doesn't know about this sort
     * of thing, and as a result we need to fake it out here.
     */

    /* GD I'm not sure what this is used for in the k1 case but it sure  */
    /* messes up the relocs when emit_all_relocs is used as they are not */
    /* resolved with respect to a global sysmbol (e.g. .text), and hence */
    /* they are ALWAYS resolved at link time                             */
    /* FIXME FIXME                                                       */

    /* clarkes: 030827:  This code (and the other half of the fix in write.c)
     * have caused problems with the PIC relocations.
     * The root problem is that bfd_install_relocation adds in to the reloc
     * addend the section offset of a symbol defined in the current object.
     * This causes problems on numerous other targets too, and there are
     * several different methods used to get around it:
     *   1.  In tc_gen_reloc, subtract off the value that bfd_install_relocation
     *       added.  That is what we do here, and it is also done the
     *       same way for alpha.
     *   2.  In md_apply_fix, subtract off the value that bfd_install_relocation
     *       will add.  This is done on SH (non-ELF) and sparc targets.
     *   3.  In the howto structure for the relocations, specify a
     *       special function that does not return bfd_reloc_continue.
     *       This causes bfd_install_relocaion to terminate before it
     *       adds in the symbol offset.  This is done on SH ELF targets.
     *       Note that on ST200 we specify bfd_elf_generic_reloc as
     *       the special function.  This will return bfd_reloc_continue
     *       only in some circumstances, but in particular if the reloc
     *       is marked as partial_inplace in the bfd howto structure, then
     *       bfd_elf_generic_reloc will return bfd_reloc_continue.
     *       Some ST200 relocations are marked as partial_inplace
     *       (this is an error in my opinion because ST200 always uses
     *       a separate addend), but some are not.  The PIC relocations
     *       are not marked as partial_inplace, so for them,
     *       bfd_elf_generic_reloc returns bfd_reloc_ok, and the addend
     *       is not modified by bfd_install_relocation.   The relocations
     *       R_K1_16 and R_K1_32 are marked partial_inplace, and so for
     *       these we need to correct the addend.
     * In the code below, the condition in the emit_all_relocs branch
     * (now moved to write.c) is the inverse of the condition that
     * bfd_elf_generic_reloc uses to short-circuit the code in
     * bfd_install_relocation that modifies the addend.  The condition
     * in the else branch match the condition used in the alpha version
     * of tc_gen_reloc (see tc-alpha.c).
     * I do not know why we need to use different conditions in these
     * two branches, it seems to me that the condition should be the same
     * whether or not emit_all_relocs is true.
     * I also do not understand why it was necessary to move the emit_all_relocs
     * condition to write.c.
     */
    if (emit_all_relocs) {
        /* Thierry Bidault: This is done later in write.c to fix a bug */
        /*       if (!((fixp->fx_addsy->bsym->flags & BSF_SECTION_SYM) == 0 */
        /*        && (!reloc->howto->partial_inplace */
        /*            || reloc->addend == 0))) */
        /*    reloc->addend -= fixp->fx_addsy->bsym->value; */
    }
    else {
        if (S_IS_EXTERNAL(fixp->fx_addsy)  &&
                !S_IS_COMMON(fixp->fx_addsy) &&
                reloc->howto->partial_inplace)
            reloc->addend -= symbol_get_bfdsym(fixp->fx_addsy)->value;
    }

    return reloc;
}

/* Round up segment to appropriate boundary */

valueT
md_section_align(asection * seg ATTRIBUTE_UNUSED, valueT size)
 {
#ifndef OBJ_ELF
    /* This is not right for ELF; a.out wants it, and COFF will force
     * the alignment anyways.  */
    int align = bfd_get_section_alignment(stdoutput, seg);
    valueT mask = ((valueT) 1 << align) - 1;
    return (size + mask) & ~mask;
#else
    return size;
#endif
}

int
md_estimate_size_before_relax(register fragS * fragP ATTRIBUTE_UNUSED,
        segT segtype ATTRIBUTE_UNUSED)
 {
    as_fatal("estimate_size_before_relax called\n");
}

void
md_convert_frag(bfd * abfd ATTRIBUTE_UNUSED,
        asection * sec ATTRIBUTE_UNUSED,
        fragS * fragp ATTRIBUTE_UNUSED)
 {
    as_fatal("k1 convert_frag\n");
}

symbolS *
        md_undefined_symbol(char *name ATTRIBUTE_UNUSED)
 {
    return 0;
}

char *
md_atof(int type ATTRIBUTE_UNUSED,
        char *litp ATTRIBUTE_UNUSED,
        int *sizep ATTRIBUTE_UNUSED)
 {
    /* we'll need this for reading floating point constants */
    return _("floating-point literals are not supported");
}

/*
 * calculate the base for a pcrel fixup
 * -- for relocation, we might need to add addend ?
 */

long
md_pcrel_from(fixS * fixP)
 {
    return (fixP->fx_where + fixP->fx_frag->fr_address);
}

/************************************************************/
/*   Hooks into standard processing -- we hook into label   */
/*   handling code to detect double ':' and we hook before  */
/*   a line of code is processed to do some simple sed style */
/*   edits.                                                 */
/************************************************************/

static symbolS *last_proc_sym = NULL;
static int update_last_proc_sym = 0;

void
k1_frob_label(symbolS * sym) {
    if (input_line_pointer[1] == ':')	/* second colon => global symbol */ {
        S_SET_EXTERNAL(sym);
        input_line_pointer++;
    }

    if (update_last_proc_sym) {
        last_proc_sym = sym;
        update_last_proc_sym = 0;
    }

    if (inside_bundle) {
	struct label_fix *fix;
	fix = malloc (sizeof (*fix));
	fix->next = label_fixes;
	fix->sym = sym;
	label_fixes = fix;
    }
}

/*  edit out some syntactic sugar that confuses GAS       */
/*  input_line_pointer is guaranteed to point to the      */
/*  the current line but may include text from following  */
/*  lines.  Thus, '\n' must be scanned for as well as '\0' */

void
k1_md_start_line_hook(void) {
    char *t;

    for (t = input_line_pointer; t && t[0] == ' '; t++);

    /* Detect illegal syntax patterns:
     * - two bundle ends on the same line: ;; ;;
     * - illegal token: ;;;
     */
    if (t && (t[0] == ';') && (t[1] == ';')) {
        char *tmp_t;
        bfd_boolean newline_seen = FALSE;

        if (t[2] == ';') {
            as_fatal("Syntax error: Illegal ;;; token");
        }

        tmp_t = t + 2;

        while (tmp_t && tmp_t[0]) {
            while (tmp_t && tmp_t[0] &&
                    ((tmp_t[0] == ' ') || (tmp_t[0] == '\n'))) {
                if (tmp_t[0] == '\n') {
                    newline_seen = TRUE;
                }
                tmp_t++;
            }
            if (tmp_t[0] == ';' && tmp_t[1] == ';') {
                /* if there's no newline between the two bundle stops
                 * then raise a syntax error now, otherwise a strange error
                 * message from read.c will be raised: "junk at end of line..."
                 */
                if (tmp_t[2] == ';') {
                    as_fatal("Syntax error: Illegal ;;; token");
                }

                if (!newline_seen) {
                    as_fatal("Syntax error: More than one bundle stop on a line");
                }
                newline_seen = FALSE; /* reset */

                /* For cores st231 and onward, empty bundles don't need to cause the
                 * generation of a NOP instruction. Just ignore them...
                 */
                if ((k1_core_info->elf_cores[subcore_id] & ELF_K1_CORE_MASK) !=
                        -6) {
                    /* this is an empty bundle, transform it into an
                     * empty statement */
                    tmp_t[0] = ';';
                    tmp_t[1] = ' ';
                }
                tmp_t += 2;
            } else {
                break;
            }
        }
    }

    /* check for bundle end                             */
    /* we transform these into a special opcode BE      */
    /* because gas has ';' hardwired as a statement end */
    if (t && (t[0] == ';') && (t[1] == ';'))
 {
        t[0] = 'B';
        t[1] = 'E';
        return;
    }
}

/*********************************************************/
/*     Hooks to handle assembler directives              */
/*     These override defaults by checking segment       */
/*     info.                                             */
/*********************************************************/

static void
k1_cons(int size)
 {
    if (is_code_section(now_seg))
        set_byte_counter(now_seg, (get_byte_counter(now_seg) + size) );
    cons(size);
}

static int is_assume_param(char** input, const char* param)
 {
    if ( (input!=NULL) && (strncmp(*input, param, strlen(param))==0) )
 {
        *input=*input+strlen(param);
        return TRUE;
    }
    else
        return FALSE;
}

static void set_assume_param(int* param, int param_value, int* param_set);
void
set_assume_param(int* param, int param_value, int* param_set)
 {
    if (!*param_set) {
        *param = param_value;
        *param_set = 1;
    } else {
        as_bad("Attempt to redefine .assume or .rta parameter");
        demand_empty_rest_of_line();
    }
}

/* Frv specific function to handle 4 byte initializations for pointers that are
   considered 'safe' for use with pic support.  Until k1_frob_file{,_section}
   is run, we encode it a BFD_RELOC_CTOR, and it is turned back into a normal
   BFD_RELOC_32 at that time.  */

void
k1_pic_ptr (int nbytes)
{
  expressionS exp;
  char *p;

  if (nbytes != 4)
    abort ();

#ifdef md_flush_pending_output
  md_flush_pending_output ();
#endif

  if (is_it_end_of_statement ())
    {
      demand_empty_rest_of_line ();
      return;
    }

#ifdef md_cons_align
  md_cons_align (nbytes);
#endif

  do
    {
      bfd_reloc_code_real_type reloc_type = BFD_RELOC_K1_GLOB_DAT;

      if (strncasecmp (input_line_pointer, "funcdesc(", 9) == 0)
	{
	  input_line_pointer += 9;
	  expression (&exp);
	  if (*input_line_pointer == ')')
	    input_line_pointer++;
	  else
	    as_bad (_("missing ')'"));
	  reloc_type = BFD_RELOC_K1_FUNCDESC;
	}
//       else if (strncasecmp (input_line_pointer, "tlsmoff(", 8) == 0)
// 	{
// 	  input_line_pointer += 8;
// 	  expression (&exp);
// 	  if (*input_line_pointer == ')')
// 	    input_line_pointer++;
// 	  else
// 	    as_bad (_("missing ')'"));
// 	  reloc_type = BFD_RELOC_FRV_TLSMOFF;
// 	}
      else
	expression (&exp);

      p = frag_more (4);
      memset (p, 0, 4);
      fix_new_exp (frag_now, p - frag_now->fr_literal, 4, &exp, 0,
		   reloc_type);
    }
  while (*input_line_pointer++ == ',');

  input_line_pointer--;			/* Put terminator back into stream. */
  demand_empty_rest_of_line ();
}


#define MAX_STR_LENGTH 20
static void
k1_set_assume_flags(int ignore ATTRIBUTE_UNUSED)
 {
    char* param;
    const char *target_name = k1_core_info->names[subcore_id];

    param=input_line_pointer;
    while ( (input_line_pointer!=NULL)
            && ! is_end_of_line [(unsigned char) *input_line_pointer])
 {
        int found = FALSE;
        int i, j;
        SKIP_WHITESPACE();

        /* core */
        for (i = 0; i < K1_NCORES; i++) {
	  j=0;
	  while(k1_core_info_table[i]->elf_cores[j] != -1) {
            if (is_assume_param(&input_line_pointer, k1_core_info_table[i]->names[j])) {
                set_assume_param(&k1_core, k1_core_info_table[i]->elf_cores[subcore_id], &k1_core_set);
                if (k1_core_info != k1_core_info_table[i])
                    as_fatal("assume machine '%s' is inconsistent with current machine '%s'",
                            k1_core_info_table[i]->names[j], target_name);
                found = TRUE;
                break;
            }
	    j++;
	  }
        }

        if (! found) {
            static const struct {
                const char *name;
                int value;
                int *variable;
                int *set_variable;
            } assume_params[] = {
	      { "cut0", ELF_K1_CUT_0, &k1_cut, &k1_cut_set },
	      { "cut1", ELF_K1_CUT_1, &k1_cut, &k1_cut_set },
	      { "cut2", ELF_K1_CUT_2, &k1_cut, &k1_cut_set },
	      { "cut3", ELF_K1_CUT_3, &k1_cut, &k1_cut_set },
	      { "cut4", ELF_K1_CUT_4, &k1_cut, &k1_cut_set },
	      { "cut5", ELF_K1_CUT_5, &k1_cut, &k1_cut_set },
	      { "no-abi", ELF_K1_ABI_NO, &k1_abi, &k1_abi_set },
	      { "old-multiflow-abi", ELF_K1_ABI_MULTI, &k1_abi, &k1_abi_set },
	      { "abi-k1dp-embedded", ELF_K1_ABI_EMBED, &k1_abi, &k1_abi_set },
	      { "abi-k1dp-pic", ELF_K1_ABI_PIC, &k1_abi, &k1_abi_set },
	      { "abi-k1io-embedded", ELF_K1_ABI_EMBED, &k1_abi, &k1_abi_set },
	      { "abi-k1io-pic", ELF_K1_ABI_PIC, &k1_abi, &k1_abi_set },
		  { "abi-k1bdp-embedded", ELF_K1_ABI_EMBED, &k1_abi, &k1_abi_set },
		  { "abi-k1bdp-pic", ELF_K1_ABI_PIC, &k1_abi, &k1_abi_set },
		  { "abi-k1bio-embedded", ELF_K1_ABI_EMBED, &k1_abi, &k1_abi_set },
		  { "abi-k1bio-pic", ELF_K1_ABI_PIC, &k1_abi, &k1_abi_set },
	      { "gcc-abi", ELF_K1_ABI_GCC, &k1_abi, &k1_abi_set },
	      { "bare-machine", ELFOSABI_NONE, &k1_osabi, &k1_osabi_set },
	      { "linux", ELFOSABI_LINUX, &k1_osabi, &k1_osabi_set },
	      { "user", ELF_K1_MODE_USER, &k1_mode, &k1_mode_set },
	      { "kernel", ELF_K1_MODE_KERNEL, &k1_mode, &k1_mode_set },
            };

            for (i = 0; i < ((int)(sizeof(assume_params)/sizeof(assume_params[0]))); i++) {
                if (is_assume_param(&input_line_pointer, assume_params[i].name)) {
		  set_assume_param(assume_params[i].variable,
				   assume_params[i].value,
				   assume_params[i].set_variable);
		  
		  found = TRUE;
		  break;
                }
            }
        }

        if (! found)
 {
            as_bad("Bad assume parameter");
            demand_empty_rest_of_line();
        }

        SKIP_WHITESPACE();
        if ( (*input_line_pointer!=',')
                && ! is_end_of_line[(unsigned char) *input_line_pointer])
 {
            as_bad("Bad assume parameter");
            demand_empty_rest_of_line();
        }
        if ( *input_line_pointer==',' )
 {
            input_line_pointer++;
            SKIP_WHITESPACE();
        }
    } /* end while */
}

static void
k1_nop_insertion(int f)
 {
    nop_insertion_allowed = f;
}

static void
k1_check_resources(int f)
 {
    check_resource_usage = f;
}

static void
k1_set_rta_flags(int f ATTRIBUTE_UNUSED)
 {
    int param_value;

    param_value = get_absolute_expression();
    /* Ignore .rta directive from know on. */
    /*  set_assume_param(&k1_abi, get_absolute_expression (), &k1_abi_set); */
}

/** called before write_object_file */
void
k1_end(void)
 {
    int newflags;
    Elf_Internal_Ehdr * i_ehdrp;

    if (! k1_core_set)
        k1_core = k1_core_info->elf_cores[subcore_id];

    /* (pp) the flags must be set at once */
    newflags= k1_core | k1_cut | k1_abi | k1_pic_flags;
    bfd_set_private_flags(stdoutput, newflags);

    i_ehdrp = elf_elfheader(stdoutput);
    i_ehdrp->e_ident[EI_ABIVERSION] = k1_abi | k1_mode;
    i_ehdrp->e_ident[EI_OSABI] = k1_osabi;
}


static void
k1_float_cons(int type)
 {
    if (is_code_section(now_seg))
 {
        if (type == 'd')
            set_byte_counter(now_seg, (get_byte_counter(now_seg) + 8) );
        if (type == 'f')
            set_byte_counter(now_seg, (get_byte_counter(now_seg) + 4) );
    }
    float_cons(type);
}

static void
k1_skip(int mult)
 {
    char * saved_input_line_pointer;
    int skip;

    if (is_code_section(now_seg)) {
        saved_input_line_pointer = input_line_pointer;
        /* Get argument of .skip/.space directive */
        if (is_end_of_line[(unsigned char) *input_line_pointer]) {
            skip = mult;
        } else {
            skip = get_absolute_expression();
        }
        set_byte_counter(now_seg, (get_byte_counter(now_seg) + skip) );

        /* Reset input_line_pointer to its original value in order to be able to run
         * through the standard s_space procedure */
        input_line_pointer = saved_input_line_pointer;
    }
    s_space(mult);
}


/* AP: It seems it is copied from ST200, but for K1 it's better
       if we simply fill with zeros */

#if 0
/* This is called from HANDLE_ALIGN in write.c.  Fill in the contents
 * of an rs_align_code fragment.  */
void
k1_handle_align(fragP)
fragS *fragP;
 {
    /* Use ST200 NOP instruction with Stop_Bit set */

    int bytes, fix, noop_size;
    char * p;
    const char * noop, * noop_stop_bit;
    /* the bundle size must be a power of 2 otherwise the way
     * modulo is computed below must be changed */
    int k1_bundle_size = K1MAXBUNDLESIZE;
    int pad_counter = 1;

    if (fragP->fr_type != rs_align_code)
        return;

    bytes = fragP->fr_next->fr_address - fragP->fr_address - fragP->fr_fix;
    p = fragP->fr_literal + fragP->fr_fix;
    fix = 0;

#if 0 /* No Max for ST200 */
    if (bytes > MAX_MEM_FOR_RS_ALIGN_CODE)
        bytes &= MAX_MEM_FOR_RS_ALIGN_CODE;
#endif

    noop = k1_noop;
    noop_stop_bit = k1_noop_stop_bit;

    noop_size = sizeof (k1_noop);

    if (bytes & (noop_size - 1))
 {
        fix = bytes & (noop_size - 1);
        memset(p, 0, fix);
        p += fix;
        bytes -= fix;
    }

    while (bytes >= noop_size)
 {
        if (((bytes - noop_size) == 0) || /* Put a stop bit for last nop */
                ((pad_counter & (k1_bundle_size - 1)) == 0)) /* or last inst. of bundle */
 {
            /* Insert NOP =with= bundle stop bit */
            memcpy(p, noop_stop_bit, noop_size);
        }
        else
 {
            /* Insert NOP =without= bundle stop bit */
            memcpy(p, noop, noop_size);
        }
        pad_counter++;
        p += noop_size;
        bytes -= noop_size;
        fix += noop_size;
    }

    fragP->fr_fix += fix;
    fragP->fr_var = noop_size;
 }

/* Called from md_do_align.  Used to create an alignment
 * frag in a code section.  */
void
        k1_frag_align_code(n, max)
int n;
int max;
 {
    char * p;
    int alignment = n; /* For MAX_MEM_FOR_RS_ALIGN_CODE macro */

#if 0 /* No Max for ST200 */
    /* We assume that there will never be a requirment
     * to support alignments greater than 32 bytes.  */
    if (max > MAX_MEM_FOR_RS_ALIGN_CODE)
        as_fatal(_("alignments greater than 32 bytes not supported in .text sections."));
#endif

    p = frag_var(rs_align_code,
            MAX_MEM_FOR_RS_ALIGN_CODE,
            1,
            (relax_substateT) max,
            (symbolS *) NULL,
            (offsetT) n,
            (char *) NULL);
    *p = 0;

 }
#endif /*0*/

static void
k1_align_ptwo(int pow)
 {
    k1_align(pow, 0);
    return;
}

static void
k1_align_bytes(int bytes)
 {
    k1_align(bytes, 1);
    return;
}

/*
 * arg is default alignment if none spec
 * is_bytes is 1 if arg is a number of bytes, 0 if it's a power of 2
 */
static void
k1_align(int arg, int is_bytes)
 {
    char * saved_input_line_pointer = input_line_pointer;
    int align;
    offsetT fill = 0;
    int max;

    if (is_code_section(now_seg))
 {
        /* Get argument of .align directive */
        if (is_end_of_line[(unsigned char) *input_line_pointer])
 {
            if (!is_bytes) {
                int i;

                align = 1;
                if (arg > 0)
                    for ( i = 0; i < arg; i++)
                        align *= 2;
            } else {
                align = arg;
            }
        }
        else
 {
            align = get_absolute_expression();
            if (!is_bytes) {
                int i, tmp = 1;

                if (align > 0) {
                    for ( i = 0; i < align; i ++)
                        tmp *= 2;
                } else {
                    /* Let later error handling do it's work */
                }
                align = tmp;
            }

            SKIP_WHITESPACE();
        }
        /* Check that there's no optional third operand (max bytes to skip).
         * Currently we cannot evaluate the "byte_counter" in this case,
         * as the decision of performing the align or not is postponed
         * in a later phase */
        if (*input_line_pointer == ',')
          {
            ++input_line_pointer;
            if (*input_line_pointer != ',')
              {
                fill = get_absolute_expression();
                SKIP_WHITESPACE();
              }
            if (*input_line_pointer == ',')
              {
                ++input_line_pointer;
                max = get_absolute_expression();
                if (max)
                  {
                    as_fatal(".align : Third operand (max bytes) not supported on text sections");
                  }
              }
          }
        /* Reset counter */
        set_byte_counter(now_seg, get_byte_counter(now_seg) & ~(align - 1));

        /* Reset input_line_pointer to its original value in order to be able to run
         * through the standard s_align_bytes procedure */
        input_line_pointer = saved_input_line_pointer;
    }
    if (!is_bytes)
        s_align_ptwo(arg);
    else
        s_align_bytes(arg);
}

/* Handle .comm directive */
static void
k1_comm(int param) {
    s_comm_internal (param, elf_common_parse);
}

/* .ascii, .asciz, .string -  in text segment, we lose track of */
/* of alignment, so we punt a little -- force to byte alignment */

static void
k1_stringer(int append_zero)
 {
    if (is_code_section(now_seg))
 {
        set_byte_counter(now_seg, 1);
    }
    stringer(append_zero);
}

static void
k1_ignore(int arg ATTRIBUTE_UNUSED)
 {
    /* the cs directives may have ';' in them.  These we must skip ! */

    while (input_line_pointer && (input_line_pointer[0] != '\n'))
        input_line_pointer++;
}

static void
k1_type(int start ATTRIBUTE_UNUSED)
 {
    char *name;
    char c;
    int type;
    const char *typename;
    symbolS *sym;
    elf_symbol_type *elfsym;

    name = input_line_pointer;
    c = get_symbol_end();
    sym = symbol_find_or_make(name);
    elfsym = (elf_symbol_type *) symbol_get_bfdsym(sym);
    *input_line_pointer = c;

    SKIP_WHITESPACE();
    if (*input_line_pointer == ',')
        ++input_line_pointer;

    SKIP_WHITESPACE();
    if (   *input_line_pointer == '#'
            || *input_line_pointer == '@'
            || *input_line_pointer == '"'
            || *input_line_pointer == '%')
        ++input_line_pointer;

    typename = input_line_pointer;
    c = get_symbol_end();

    type = 0;
    if (strcmp(typename, "function") == 0
            || strcmp(typename, "STT_FUNC") == 0)
        type = BSF_FUNCTION;
    else if (strcmp(typename, "object") == 0
            || strcmp(typename, "STT_OBJECT") == 0)
        type = BSF_OBJECT;
    else if (strcmp(typename, "tls_object") == 0
            || strcmp(typename, "STT_TLS") == 0)
        type = BSF_OBJECT | BSF_THREAD_LOCAL;
    else if (strcmp(typename, "notype") == 0
            || strcmp(typename, "STT_NOTYPE") == 0)
        ;
#ifdef md_elf_symbol_type
    else if ((type = md_elf_symbol_type(typename, sym, elfsym)) != -1)
        ;
#endif
    else
        as_bad(_("unrecognized symbol type \"%s\""), typename);

    *input_line_pointer = c;

    if (*input_line_pointer == '"')
        ++input_line_pointer;

    elfsym->symbol.flags |= type;
    symbol_get_bfdsym(sym)->flags |= type;

/* FIXME: do we need that? */
/*
    while (*input_line_pointer == ',') {
        ++input_line_pointer;

        SKIP_WHITESPACE();

        typename = input_line_pointer;
        c = get_symbol_end();
        if (strcmp(typename, "moveable") == 0) {
            if (emit_all_relocs) {
                S_SET_OTHER(sym, S_GET_OTHER(sym) | STO_MOVEABLE);
            }
        } else if (strcmp(typename, "used") == 0) {
            if (emit_all_relocs) {
                S_SET_OTHER(sym, S_GET_OTHER(sym) | STO_USED);
            }
        } else
            as_bad(_("unrecognized symbol type \"%s\""), typename);
        *input_line_pointer = c;
    }
*/
    demand_empty_rest_of_line();
}

#define ENDPROCEXTENSION	"$endproc"
#define MINUSEXPR		".-"

static int proc_endp_status = 0;

static void
k1_endp(int start ATTRIBUTE_UNUSED)
 {
    char * procname;
    char c;


    if(inside_bundle){
      as_warn(".endp directive inside a bundle.");
    }
    /* function name is optionnal and is ignored */
    /* there may be several names separated by commas... */
    while (1)
 {
        SKIP_WHITESPACE();
        procname = input_line_pointer;
        c = get_symbol_end();
        *input_line_pointer = c;
        SKIP_WHITESPACE();
        if (*input_line_pointer != ',')
            break;
        ++input_line_pointer;
    }
    demand_empty_rest_of_line();

    if (!proc_endp_status)
 {
        as_warn(".endp directive doesn't follow .proc -- ignoring ");
        return;
    }

    proc_endp_status = 0;

    /* TB begin : add BSF_FUNCTION attribute to last_proc_sym symbol */
    if (size_type_function) {
        if (!last_proc_sym) {
            as_bad("Cannot set function attributes (bad symbol)");
            return;
        }

        /*    last_proc_sym->symbol.flags |= BSF_FUNCTION; */
        symbol_get_bfdsym(last_proc_sym)->flags |= BSF_FUNCTION;
        /* Add .size funcname,.-funcname in order to add size
         * attribute to the current function */
        {
            char *newdirective = (char *)alloca(strlen(S_GET_NAME(last_proc_sym)) +
            strlen(MINUSEXPR) + 1);
            char *savep = input_line_pointer;
            expressionS exp;

            *newdirective = '\0';
            /* BUILD :".-funcname" expression */
            strcat(newdirective, MINUSEXPR);
            strcat(newdirective, S_GET_NAME(last_proc_sym));
            input_line_pointer = newdirective;
            expression(&exp);

            if (exp.X_op == O_constant)
 {
                S_SET_SIZE(last_proc_sym, exp.X_add_number);
                if (symbol_get_obj(last_proc_sym)->size)
 {
                    xfree(symbol_get_obj(last_proc_sym)->size);
                    symbol_get_obj(last_proc_sym)->size = NULL;
                }
            }
            else
 {
                symbol_get_obj(last_proc_sym)->size =
                        (expressionS *) xmalloc(sizeof (expressionS));
                *symbol_get_obj(last_proc_sym)->size = exp;
            }

            /* just restore the real input pointer */
            input_line_pointer = savep;
        }
    }
    /* TB end */

/* FIXME: Do we need that? */
#if 0    
    /* (pp) encode wether a function is moveable by icacheopt in the st_other field of the ELF symbol */
    if (emit_all_relocs) {
        S_SET_OTHER(last_proc_sym, S_GET_OTHER(last_proc_sym) | STO_MOVEABLE);
    }
#endif//0

#if 0
    /* this code emit a global symbol to mark the end of each function    */
    /* the symbol emitted has a name formed by the original function name */
    /* cocatenated with $endproc so if _foo is a function name the symbol */
    /* marking the end of it is _foo$endproc                              */


    if (emit_all_relocs)
 {
        char *newlab;
        char *savep = input_line_pointer;

        if (!last_proc_sym) {
            as_bad("Cannot set symbol at end of function (bad symbol)");
            return;
        }

        newlab = (char *) alloca(strlen(S_GET_NAME(last_proc_sym)) +
                strlen(ENDPROCEXTENSION) + 1
                + 1);	/* in case of "_" */

#ifdef STRIP_UNDERSCORE
        strcpy(newlab, "_");
#else
        *newlab = '\0';
#endif
        strcat(newlab, S_GET_NAME(last_proc_sym));
        strcat(newlab, ENDPROCEXTENSION);

        colon(newlab);

        /* just make sure nobody did anything to the real input pointer */
        input_line_pointer = savep;

    }

#endif

    last_proc_sym = NULL;


}

static void
k1_proc(int start ATTRIBUTE_UNUSED)
 {
    char * procname;
    char c;
    /* there may be several names separated by commas... */
    while (1)
 {
        SKIP_WHITESPACE();
        procname = input_line_pointer;
        c = get_symbol_end();
        *input_line_pointer = c;
        SKIP_WHITESPACE();
        if (*input_line_pointer != ',')
            break;
        ++input_line_pointer;
    }
    demand_empty_rest_of_line();

    if (proc_endp_status)
 {
        as_warn(".proc follows .proc -- ignoring");
        return;
    }

    proc_endp_status = 1;

    /* this code emit a global symbol to mark the end of each function    */
    /* the symbol emitted has a name formed by the original function name */
    /* cocatenated with $endproc so if _foo is a function name the symbol */
    /* marking the end of it is _foo$endproc                              */
    /* It is also required for generation of .size directive in k1_endp() */

    if ((emit_all_relocs) || (size_type_function))
 {
        update_last_proc_sym = 1;
    }
}

int
k1_force_reloc(fixS * fixP)
 {
    symbolS *sym;
    asection *symsec;

    if (generic_force_reloc(fixP))
        return 1;

    if (!emit_all_relocs)
      {
        switch (fixP->fx_r_type)
          {
	    case BFD_RELOC_K1_GOTOFF:
            case BFD_RELOC_K1_GOTOFF_HI22:
            case BFD_RELOC_K1_GOTOFF_LO10:
	    case BFD_RELOC_K1_GOT:
            case BFD_RELOC_K1_GOT_HI22:
            case BFD_RELOC_K1_GOT_LO10:
//             case BFD_RELOC_K1_PCREL_HI22:
//             case BFD_RELOC_K1_PCREL_LO10:
            case BFD_RELOC_K1_PLT_HI22:
            case BFD_RELOC_K1_PLT_LO10:
            case BFD_RELOC_K1_FUNCDESC_GOT_LO10:
            case BFD_RELOC_K1_FUNCDESC_GOT_HI22:
	    case BFD_RELOC_K1_FUNCDESC_GOTOFF_LO10:
            case BFD_RELOC_K1_FUNCDESC_GOTOFF_HI22:
	    case BFD_RELOC_K1_FUNCDESC:
	    case BFD_RELOC_K1_GLOB_DAT:
              
 /* case BFD_RELOC_K1_GOTOFFX_HI23:
  * case BFD_RELOC_K1_GOTOFFX_LO9:
  * case BFD_RELOC_K1_FPTR32:  
  * case BFD_RELOC_K1_GOTOFF_FPTR_HI23:
  * case BFD_RELOC_K1_GOTOFF_FPTR_LO9:
  */
              return 1;
            default:
                return 0;
        }
    }
    sym = fixP->fx_addsy;
    if (sym)
    {
        symsec = S_GET_SEGMENT(sym);
        /* if (bfd_is_abs_section (symsec)) return 0; */
        if (!SEG_NORMAL(symsec))
            return 0;
    }
    return 1;
}

int
k1_force_reloc_sub_same(fixS * fixP, segT sec)
 {
    symbolS *sym;
    asection *symsec;
    const char *sec_name = NULL;

    if (generic_force_reloc(fixP))
        return 1;

    if (!emit_all_relocs)
    {
        switch (fixP->fx_r_type)
        {
	  case BFD_RELOC_K1_GOTOFF:
          case BFD_RELOC_K1_GOTOFF_HI22:
          case BFD_RELOC_K1_GOTOFF_LO10:
	  case BFD_RELOC_K1_GOT:
          case BFD_RELOC_K1_GOT_HI22:
          case BFD_RELOC_K1_GOT_LO10:
//           case BFD_RELOC_K1_PCREL_HI22:
//           case BFD_RELOC_K1_PCREL_LO10:
          case BFD_RELOC_K1_PLT_HI22:
          case BFD_RELOC_K1_PLT_LO10:
          case BFD_RELOC_K1_FUNCDESC_GOT_LO10:
          case BFD_RELOC_K1_FUNCDESC_GOT_HI22:
	  case BFD_RELOC_K1_FUNCDESC_GOTOFF_LO10:
          case BFD_RELOC_K1_FUNCDESC_GOTOFF_HI22:
	  case BFD_RELOC_K1_FUNCDESC:
	  case BFD_RELOC_K1_GLOB_DAT:
  /* case BFD_RELOC_K1_GOTOFFX_HI23:
  * case BFD_RELOC_K1_GOTOFFX_LO9:
  * case BFD_RELOC_K1_FPTR32:  
  * case BFD_RELOC_K1_GOTOFF_FPTR_HI23:
  * case BFD_RELOC_K1_GOTOFF_FPTR_LO9:
  */
            return 1;
          default:
            return 0;
        }
    }
    sym = fixP->fx_addsy;
    if (sym)
    {
        symsec = S_GET_SEGMENT(sym);
        /* if (bfd_is_abs_section (symsec)) return 0; */
        if (!SEG_NORMAL(symsec))
            return 0;

        /*
         * for .debug_arrange, .debug_frame, .eh_frame sections, containing
         * expressions of the form "sym2 - sym1 + addend", solve them even when
         * --emit-all-relocs is set. Otherwise, a relocation on two symbols
         * is necessary and fails at elf level. Binopt should not be impacted by
         * the resolution of this relocatable expression on symbols inside a
         * function.
         */
        sec_name =  segment_name(sec);
        if ((strcmp(sec_name, ".eh_frame") == 0) ||
                (strcmp(sec_name, ".except_table") == 0) ||
                (strncmp(sec_name, ".debug_", strlen(".debug_")) == 0)) {
            return 0;
        }
    }
    return 1;
}

/*
 *    Support for unwind descriptors
 */

#define K1MAXUNWINDARGS 4

static int
k1_get_constant(const expressionS arg)
 {
    if (arg.X_op && !arg.X_add_symbol)
 {
        return arg.X_add_number;
    }
    else
 {
        as_warn("expected constant argument");
        return 0;
    }
}

static int
k1_default(char *s, int i)
 {
    as_warn(s);
    return i;
}

static void
k1_emit_uleb128(int i)
 {
    do
 {
        int tmp = i & 127;
        i = (i & 0x1fffffff) >> 7;
        FRAG_APPEND_1_CHAR((i != 0) ? (tmp + 1) : tmp);
    }
    while (i != 0);
}

static void
k1_unwind(int r)
 {
    int ntok;
    expressionS tok[K1MAXUNWINDARGS];
    char *f;

    int tmp;

    if (strcmp(segment_name(now_seg), ".K1.unwind_info") != 0)
 {
        as_warn("unwind directive not in .K1.unwind_info segment\n");
        return;
    }

    ntok = tokenize_arguments(input_line_pointer, tok, NULL, K1MAXUNWINDARGS);
    while (input_line_pointer && (input_line_pointer[0] != '\n'))
        input_line_pointer++;

#define K1GETCONST(i) (((i)<ntok) ? \
		       k1_get_constant(tok[(i)]) :  \
		       k1_default("too few operands", 0))

    switch ((enum unwrecord) r)
 {
        case UNW_HEADER:		/* postpone fixup work for now */
            tmp = (K1GETCONST(0) << 8);
            tmp += (K1GETCONST(1) & 1);
            tmp += ((K1GETCONST(2) & 1) << 1);
            emit_expr(tok + 3, 2);
            f = frag_more(2);
            md_number_to_chars(f, tmp, 2);
            break;

        case UNW_PROLOGUE:
            tmp = K1GETCONST(0);
            if (tmp < 32)
 {
                FRAG_APPEND_1_CHAR(tmp);
            }
            else
 {
                FRAG_APPEND_1_CHAR(0x40);
                k1_emit_uleb128(tmp);
            }
            break;

        case UNW_BODY:
            tmp = K1GETCONST(0);
            if (tmp < 32)
 {
                FRAG_APPEND_1_CHAR(tmp + 0x20);
            }
            else
 {
                FRAG_APPEND_1_CHAR(0x41);
                k1_emit_uleb128(tmp);
            }
            break;

        case UNW_MEM_STACK_F:
            FRAG_APPEND_1_CHAR(0xe0);
            k1_emit_uleb128(K1GETCONST(0));
            k1_emit_uleb128(K1GETCONST(1));
            break;

        case UNW_MEM_STACK_V:
            FRAG_APPEND_1_CHAR(0xe1);
            k1_emit_uleb128(K1GETCONST(0));
            break;

        case UNW_PSP_SPREL:
            FRAG_APPEND_1_CHAR(0xe2);
            k1_emit_uleb128(K1GETCONST(0));
            break;

        case UNW_RP_WHEN:
            FRAG_APPEND_1_CHAR(0xe3);
            k1_emit_uleb128(K1GETCONST(0));
            break;

        case UNW_RP_PSPREL:
            FRAG_APPEND_1_CHAR(0xe4);
            k1_emit_uleb128(K1GETCONST(0));
            break;

        case UNW_RP_SPREL:
            FRAG_APPEND_1_CHAR(0xe5);
            k1_emit_uleb128(K1GETCONST(0));
            break;

        case UNW_SPILL_BASE:
            FRAG_APPEND_1_CHAR(0xe6);
            k1_emit_uleb128(K1GETCONST(0));
            break;

        case UNW_PSP_GR:
            tmp = K1GETCONST(0);
            FRAG_APPEND_1_CHAR(0xb0);
            FRAG_APPEND_1_CHAR(tmp & 0x3f);
            break;

        case UNW_RP_GR:
            tmp = K1GETCONST(0);
            FRAG_APPEND_1_CHAR(0xb1);
            FRAG_APPEND_1_CHAR(tmp & 0x3f);
            break;

        case UNW_GR_MEM_S:
            tmp = K1GETCONST(0);
            FRAG_APPEND_1_CHAR(0xc0 + (tmp & 0x0f));
            break;

        case UNW_GR_MEM_L:
            tmp = K1GETCONST(0);
            FRAG_APPEND_1_CHAR(0x90 + (tmp & 7));
            k1_emit_uleb128(K1GETCONST(1));
            break;

        case UNW_SPILL_MASK:
            as_warn("unwind spill mask not interpreted yet \n");
            break;

        case UNW_EPILOGUE:
            tmp = K1GETCONST(1);
            if (tmp < 32)
 {
                FRAG_APPEND_1_CHAR(0xc0 + tmp);
                k1_emit_uleb128(K1GETCONST(0));
            }
            else
 {
                FRAG_APPEND_1_CHAR(0xe0);
                k1_emit_uleb128(K1GETCONST(0));
                k1_emit_uleb128(tmp);
            }
            break;

        case UNW_LABEL_STATE:
            tmp = K1GETCONST(0);
            if (tmp > 32)
 {
                FRAG_APPEND_1_CHAR(0xe1);
                k1_emit_uleb128(tmp);
            }
            else
 {
                FRAG_APPEND_1_CHAR(0x80 + tmp);
            }
            break;

        case UNW_COPY_STATE:
            tmp = K1GETCONST(0);
            if (tmp > 32)
 {
                FRAG_APPEND_1_CHAR(0xe9);
                k1_emit_uleb128(tmp);
            }
            else
 {
                FRAG_APPEND_1_CHAR(0xa0 + tmp);
            }
            break;

        case UNW_SPILL_PSREL:
            as_warn("unw_spill_psrel unimplemented\n");
            break;
        case UNW_SPILL_SPREL:
            as_warn("unw_spill_sprel unimplemented\n");
            break;
        default:
            as_warn("unrecognized unwind descriptor\n");
    }
}

static void
print_operand(expressionS * e, FILE * out) ATTRIBUTE_UNUSED;

/*
 * This is just used for debugging
 */

static void
print_operand(expressionS * e, FILE * out)
 {
    if (e)
 {
        switch (e->X_op)
 {
            case O_register:
                fprintf(out, "%s",  k1_registers[e->X_add_number].name);
                break;

            case O_constant:
                if (e->X_add_symbol)
 {
                    if (e->X_add_number)
                        fprintf(out, "(%s + %d)", S_GET_NAME(e->X_add_symbol),
                                (int) e->X_add_number);
                    else
                        fprintf(out, "%s", S_GET_NAME(e->X_add_symbol));
                }
                else
                    fprintf(out, "%d", (int) e->X_add_number);
                break;

            case O_symbol:
                if (e->X_add_symbol)
 {
                    if (e->X_add_number)
                        fprintf(out, "(%s + %d)", S_GET_NAME(e->X_add_symbol),
                                (int) e->X_add_number);
                    else
                        fprintf(out, "%s", S_GET_NAME(e->X_add_symbol));
                }
                else
                    fprintf(out, "%d", (int) e->X_add_number);
                break;

            default:
                fprintf(out, "o,ptype-%d", e->X_op);
        }
    }
}

void
k1_cfi_frame_initial_instructions(void)
 {
    cfi_add_CFA_def_cfa(12, 16);
}


int
k1_regname_to_dw2regnum(const char *regname)
 {
    unsigned int regnum = -1;
    const char *p;
    char *q;

    if (regname[0] == 'r')
 {
        p = regname + 1;
        regnum = strtoul(p, &q, 10);
        if (p == q || *q || regnum >= 64)
            return -1;
    }
    return regnum;
}
