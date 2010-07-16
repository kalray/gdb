/* THIS FILE HAS BEEN MODIFIED OR ADDED BY STMicroelectronics, Inc. 1999-2003 */

#ifndef __IPA_BFD_H__
#define __IPA_BFD_H__

#define BCOPY(src, dst, len) \
    bcopy((const void *)(src), (void *)(dst), (int)(len))    	       

#define FREE(ptr) \
    free((void *) (ptr))

#define MALLOC_ASSERT(addr) \
    if (addr == 0) { perror("malloc failed: "); exit(1);}

#define MALLOC(nbytes) \
    malloc((size_t)(nbytes))

#define REALLOC(ptr, size) \
    realloc((void *)(ptr), (size_t)(size))

#define UNLINK(path) \
    unlink((const char *)(path))

#if defined (_WIN32)
#define MKDIR(path, mode) \
    mkdir((const char *)(path))
#else /* _WIN32 */
#define MKDIR(path, mode) \
    mkdir((const char *)(path), (mode_t)(mode))
#endif /* _WIN32 */

#define RMDIR(path) \
    rmdir((const char *)(path))	       

#define OPEN(path, oflag, mode) \
    open((char *)(path), (int)(oflag), (int)(mode))

#define CLOSE(fid) \
    close((int)(fid))

#define READ(fildes, buf, nbyte) \
    read((int) (fildes), (void *)(buf), (size_t) (nbyte))

#define ALLOCA(size) \
    alloca((unsigned int)(size))	       
	       
#define FCHMOD(fid, mode) \
    fchmod((int)(fid), (mode_t)(mode))

#define MMAP(addr, len, prot, flags, fd, off) \
    mmap((void *)(addr), (int)(len), (int)(prot), (int)(flags), (int)(fd), \
	 (off_t)(off))

#define MUNMAP(addr, len) \
    munmap((void *)(addr), (int)(len))

#define MEMCPY(s1, s2, n) \
    memcpy((void *)(s1), (void *)(s2), (size_t)(n))

#define ELF_WORD int

#define OBJ_ASSERT(EX, obj, str) \
    if (!(EX)) {fprintf(stderr,"%s: %s\n", obj->filename, str); exit(1);}

#define DEFAULT_TMP_LIST_SIZE 32
#define DEFAULT_TMPDIR "./ldtmp"

#define arch_eltdata(bfd) ((struct areltdata *)((bfd)->arelt_data))
#define arch_hdr(_bfd) ((struct ar_hdr *)arch_eltdata(_bfd)->arch_header)

#define FALSE 0
#define TRUE 1

typedef char *string_t;

extern int ipa_argc;
extern char **ipa_argv;

/* These are taken from ipc_sumtab_merge.h in the ipa tree. */


/*
 * These are for passing option information from the
 * static linker to ipa without involving the linkers
 * internal option table. Any enumerated types defined
 * here can be added to,  but not altered in any other
 * way. This allows the linker to change without affecting
 * ipa.
 */

typedef enum{
    LD_IPA_SHARABLE, 
    LD_IPA_DEMANGLE, 
    LD_IPA_SHOW, 
    LD_IPA_HIDES, 
    LD_IPA_TARGOS, 
    LD_IPA_VERBOSE, 
    LD_IPA_KEEP_TEMPS, 
    LD_IPA_ISA,
    LD_IPA_RELOCATABLE, /* [CL] generate a relocatable file */
    LD_IPA_EXPORTS,     /* [CL] all symbols are exported (-E)*/
    LD_IPA_XXXX, 
    MAX_LD_IPA
}ld_ipa_option_enum;

typedef struct ld_ipa_option {
    ld_ipa_option_enum opt_ndx;
    unsigned    flag		: 4;    /*  */
    unsigned     set		: 4;    /*  */
} LD_IPA_OPTION;

extern LD_IPA_OPTION ld_ipa_opt[MAX_LD_IPA];

#define HS_DEFAULT 0
#define HS_HIDES 1
#define HS_EXPORTS 2
#define HS_IGNORE 3

                 /* these are set to bit fields for */
                 /* easy table initialization */
#define          F_RELOCATABLE       1
#define          F_NON_SHARED        2
#define          F_CALL_SHARED       4
#define          F_MAKE_SHARABLE     8
#define          F_STATIC    (F_NON_SHARED | F_RELOCATABLE)
#define          F_DYNAMIC   (~(F_STATIC))
#define          F_MAIN      (F_NON_SHARED | F_CALL_SHARED)
#define		 F_EXEC	     (~F_RELOCATABLE)
#define          F_ALL       (F_STATIC | F_DYNAMIC)
#define          F_CALL_SHARED_RELOC (F_RELOCATABLE | F_CALL_SHARED)

typedef enum {
	TOS_IA64_64,
	TOS_IA64_32, 
	TOS_MAX
}targos_enum;

extern string_t tos_string[TOS_MAX];
extern string_t toolroot;
extern string_t tmpdir;
extern string_t outfilename;
extern char * __Release_ID;

extern void *(*p_ipa_open_input)(char *, off_t *, int, char**);
extern void (*p_ipa_init_link_line)(int, char **);
extern void (*p_ipa_add_link_flag)(const char*);
extern void (*p_ipa_driver)(int, char **);
extern void (*p_process_whirl64)(void *, int, void *, int, const char *);
extern void (*p_ipa_insert_whirl_marker)(void);
extern void (*p_Sync_symbol_attributes)(unsigned int, unsigned int, bfd_boolean, unsigned int);
/* [CL] handle comdat attribute */
extern bfd_boolean (*p_symbol_comdat)(unsigned int);
/* [CL] handle comdat attribute */
extern bfd_boolean (*p_symbol_linkonce)(unsigned int);

/* [CL] pointers to external symbols used by ipa.so */
typedef LD_IPA_OPTION ipa_option_array[MAX_LD_IPA];

#define IPA_EXTERNAL_SYMBOLS_VERSION 1

struct external_symbols_list {
  /* Size and version of this struct, for sanity checks */
    int size;
    int version;

    /* pass1.h */
    unsigned int *used_gp_area;

    /* ld_ipa_option.h */
    ipa_option_array *ld_ipa_opt;
/*     ipacom_flags */
    string_t *WB_flags;
    string_t *Y_flags;

    /* error.h */
/*     msg */

    /* ext_tbl.h */
    void (*merge_ext)(void*, char *, int, void*);
/*     enter_mext */
/*     slookup_mext */
/*     slookup_mext_idx */
/*     get_mext */
/*     ext_tbl */
    void* (*ld_slookup_mext)(char *, bfd_boolean);

    /* obj_file.h */
/*     num_ir */
/*     get_next_ir */
/*     is_archive_member */

    /* process.h */
    int (*create_tmpdir)(int);
    string_t (*create_unique_file)(string_t, char);
    string_t (*create_unique_file_in_curdir)(string_t, char);
    void (*add_to_tmp_file_list)(string_t);
    string_t *tmpdir;
/*     get_command_line */
/*     make_link */

    /* ld_util.h */
    string_t (*concat_names)(string_t, string_t);

    /* ld_main.h */
    int *arg_count;
    char** *arg_vector;
    char** *environ_vars;
    int *max_gpa_size;

    /* read.h */
    void (*read_one_section)(int, void*);
/*     read_headers */
/*     unread_sections */
/*     unread_obj */
/*     objs_mapped_total_size */
/*     copy_section */

    /* dem.h */
    char* (*always_demangle)(char *, char );

    /* elfhash.h */
/*     elfhash */

    /* ld_ipa_interface.h */
  unsigned long long (*ld_get_section_size) (void* pobj, int index);

  char *(*ld_get_section_name) (void* pobj, int index);

  char *(*ld_get_section_base) (void* pobj, int index);

  void *(*ld_get_mmap_addr) (void* pobj);

/* extern unsigned int */
/* ld_get_sym_attr (void* pext); */

/* extern int */
/* ld_is_weak_symbol (void* pext); */

/* extern ST_EXPORT */
/* ld_get_export (void* pext); */

  void (*ld_set_st_idx) (void* pext, int st_idx);

  int (*ld_get_st_idx) (void* pext);

  bfd_boolean (*ld_resolved_to_obj) (void* pext, void* pobj);

/*   extern void */
/*   cleanup_symtab_for_ipa (void); */

  int (*Count_elf_external_gots) (void);

  string_t *outfilename;
};

extern void (*p_ipa_initialize_external_symbols)(struct external_symbols_list*);


/* Function declarations
 */
 
extern char *
ipa_copy_of(char *);

extern string_t
concat_names(const string_t, const string_t);

extern void
add_to_tmp_file_list (string_t);

extern int
create_tmpdir ( int);

extern string_t
create_unique_file (const string_t, char);

extern string_t
create_unique_file_in_curdir (const string_t, char);

extern int
ipa_set_ndx (bfd *);

extern void 
ld_set_cur_obj(bfd *);

void *
ld_get_cur_obj(void);

extern bfd_boolean
ipa_is_whirl(bfd *);

extern void
ipa_process_whirl ( bfd *);

/* The following constant values are shared by both ld and ipa.  Each
   symtab entry in ld's merged symbol table has an ST_IDX field pointing
   back to the corresponding entry (if any) in the WHIRL merged symbol
   table.  When a new entry is added to ld's merged symbol table, if the
   symbol comes from an Elf object, we set the ST_IDX field to
   WHIRL_ST_IDX_NOT_AVAILABLE because there is no corresponding ST entry in
   the WHIRL merged symbol table. If the symbol comes from a WHIRL object,
   we set the ST_IDX field to WHIRL_ST_IDX_UNINITIALIZED, which means that
   there will be a corresponding entry in the WHIRL merged symtab but we
   don't know the value yet.
 */

#define WHIRL_ST_IDX_UNINITIALIZED (0)
#define WHIRL_ST_IDX_NOT_AVAILABLE (-1)



#endif /* __IPA_BFD_H__ */
