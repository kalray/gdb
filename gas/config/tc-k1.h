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

#define K1MAXSYLLABLES 8
#define K1MAXIMMX 4
#define NOIMMX -1

/* Author Jeff Brown */
/* Modified by Giuseppe Desoli */


#define TC_K1
#define TARGET_ARCH bfd_arch_k1
#define TARGET_FORMAT "elf32-k1"

/* Uncomment this if the compiler prepends an _ to global names */
/*
 * #define STRIP_UNDERSCORE 
 */

/* default little endian */

#define TARGET_BYTES_BIG_ENDIAN 0    

/* for now we have no BFD target */

/* lexing macros */

#define LEX_QM (LEX_BEGIN_NAME | LEX_NAME)
#define LEX_DOLLAR (LEX_BEGIN_NAME | LEX_NAME)
#define WORKING_DOT_WORD

#define md_end k1_end
extern void k1_end (void);

#define TC_FIX_TYPE struct _symbol_struct *
#define TC_SYMFILED_TYPE struct list_info_struct *
#define REPEAT_CONS_EXPRESSIONS
#define TC_INIT_FIX_DATA(FIXP) ((FIXP)->tc_fix_data = NULL)

#define tc_frob_label(sym) k1_frob_label(sym)
extern void k1_frob_label PARAMS ((struct symbol *));
/* listings */


int is_constant_expression(expressionS*);

/*
 *  Make sure that absolute symbols and imported symbols
 *  that are never used do not get written to .o file
 */

#ifdef OBJ_ELF
#define tc_frob_symbol(sym,punt) \
  { \
    if ((S_GET_SEGMENT (sym) == undefined_section \
	 && ! symbol_used_p (sym)\
	 && ELF_ST_VISIBILITY (S_GET_OTHER (sym)) == STV_DEFAULT) \
        || (S_GET_SEGMENT (sym) == absolute_section \
            && ! S_IS_EXTERNAL (sym))) \
      punt = 1; \
  }
#endif


#define LISTING_HEADER "K1 GAS LISTING"
#define LISTING_LHS_CONT_LINES 100
#define md_start_line_hook k1_md_start_line_hook
extern void k1_md_start_line_hook (void);

/* Values passed to md_apply_fix3 don't include the symbol value.  */
#define MD_APPLY_SYM_VALUE(FIX) 0

extern int k1_force_reloc PARAMS ((struct fix *));
#define TC_FORCE_RELOCATION(fixP)			\
		k1_force_reloc(fixP)

#define DIFF_EXPR_OK 1

#define TC_K1_K1
extern int k1_force_reloc_sub_same (struct fix *, segT);
#define TC_FORCE_RELOCATION_SUB_SAME(FIX, SEC)		\
  (! SEG_NORMAL (S_GET_SEGMENT((FIX)->fx_addsy))	\
   || k1_force_reloc_sub_same(FIX, SEC))

/* This expression evaluates to true if the relocation is for a local object
   for which we still want to do the relocation at runtime.  False if we
   are willing to perform this relocation while building the .o file.

   We can't resolve references to the GOT or the PLT when creating the
   object file, since these tables are only created by the linker.
   Also, if the symbol is global, weak, common or not defined, the
   assembler can't compute the appropriate reloc, since its location
   can only be determined at link time.  */

#define TC_FORCE_RELOCATION_LOCAL(FIX)			\
  (!(FIX)->fx_pcrel					\
   || TC_FORCE_RELOCATION (FIX))

extern int emit_all_relocs;

/* This expression evaluates to false if the relocation is for a local object
   for which we still want to do the relocation at runtime.  True if we
   are willing to perform this relocation while building the .o file.
   This is only used for pcrel relocations.
   Use this to ensure that a branch to a preemptible symbol is not
   resolved by the assembler. */

#define TC_RELOC_RTSYM_LOC_FIXUP(FIX)				\
  ((FIX)->fx_r_type != BFD_RELOC_K1_23_PCREL                    \
   || (FIX)->fx_addsy == NULL					\
   || (! S_IS_EXTERNAL ((FIX)->fx_addsy)			\
       && ! S_IS_WEAK ((FIX)->fx_addsy)			        \
       && S_IS_DEFINED ((FIX)->fx_addsy)			\
       && ! S_IS_COMMON ((FIX)->fx_addsy)))

#define tc_fix_adjustable(fixP)                         \
                k1_fix_adjustable (fixP)
extern int k1_fix_adjustable PARAMS((struct fix *fix));

/* This arranges for gas/write.c to not apply a relocation if
   tc_fix_adjustable() says it is not adjustable.
   The "! symbol_used_in_reloc_p" test is there specifically to cover
   the case of non-global symbols in linkonce sections.  It's the
   generally correct thing to do though;  If a reloc is going to be
   emitted against a symbol then we don't want to adjust the fixup by
   applying the reloc during assembly.  The reloc will be applied by
   the linker during final link.  */
#define TC_FIX_ADJUSTABLE(fixP) \
  (! symbol_used_in_reloc_p ((fixP)->fx_addsy) && tc_fix_adjustable (fixP))

extern void k1_validate_fix (struct fix *);
#define TC_VALIDATE_FIX(fix,seg,skip)	k1_validate_fix (fix)

/* Force this to avoid -g to fail because of dwarf2 expression .L0 - .L0 */
extern int k1_validate_sub_fix (struct fix *fix);
#define TC_VALIDATE_FIX_SUB(FIX, SEG)			\
  (((FIX)->fx_r_type == BFD_RELOC_32			\
    || (FIX)->fx_r_type == BFD_RELOC_16) &&		\
   k1_validate_sub_fix((FIX)))

#define TC_CONS_FIX_NEW(FRAG,OFF,LEN,EXP) k1_cons_fix_new(FRAG,OFF,LEN,EXP)
extern void k1_cons_fix_new (fragS *f, int where, int nbytes,
			     expressionS *exp);

/* No post-alignment of sections */
#define SUB_SEGMENT_ALIGN(SEG, FRCHAIN) 0

/* No Max for ST200 */
#  define MAX_MEM_FOR_RS_ALIGN_CODE  ((1 << alignment) - 1)

/* AP: The following is not needed for K1 */
#if 0
#define HANDLE_ALIGN(fragp) k1_handle_align (fragp)
extern void k1_handle_align PARAMS ((struct frag *));

#define md_do_align(N, FILL, LEN, MAX, LABEL)					\
  if (FILL == NULL && (N) != 0 && ! need_pass_2 && subseg_text_p (now_seg))	\
    {										\
      k1_frag_align_code (N, MAX);						\
      goto LABEL;								\
    }
extern void k1_frag_align_code PARAMS ((int, int));
#endif /* 0 */

#ifdef OBJ_ELF
#define TARGET_USE_CFIPOP 1
#define tc_cfi_frame_initial_instructions k1_cfi_frame_initial_instructions
extern void k1_cfi_frame_initial_instructions (void);

#define tc_regname_to_dw2regnum k1_regname_to_dw2regnum
extern int k1_regname_to_dw2regnum (const char *regname);

/* All K1 instructions are multiples of 32 bits.  */
#define DWARF2_LINE_MIN_INSN_LENGTH 1
#define DWARF2_DEFAULT_RETURN_COLUMN 63
#define DWARF2_CIE_DATA_ALIGNMENT -4
#endif
