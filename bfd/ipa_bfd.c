/* THIS FILE HAS BEEN MODIFIED OR ADDED BY STMicroelectronics, Inc. 1999-2003 */

#ifdef IPA_LINK

#if defined(__GNUC__)
#include <stdio.h>		/* for sys_errlist */
#endif
#include <stdlib.h>		/* for getenv(3) */
#include <unistd.h>		/* for unlink(2), rmdir(2), etc. */

#if defined(__CYGWIN__) || defined(_WIN32) 
#include "libiberty.h"		/* for basename(3) */
#else
#include <libgen.h>		/* for basename(3) */
#endif

#include <sys/stat.h>		/* for chmod(2) */

#include <fcntl.h>		/* for open(2) */
#if defined(sun) || defined(__CYGWIN__) || defined (_WIN32)
#include <sys/types.h>
#include <dirent.h>
#else
#include <sys/dir.h>		/* for opendir(2), readdir, closedir */
#endif
#include <signal.h>		/* for kill(2) */
#include <limits.h>		/* for PATH_MAX */
#include <errno.h>
#include <string.h>


#include "aout/ar.h"

#include "bfd.h"
#include "libbfd.h"
#include "elf-bfd.h"

#include "ipa_bfd.h"

#include "strings.h"            /* for memcopy() */

#if defined(__CYGWIN__)
#include <sys/cygwin.h>         /* [CL] for cygwin_conv_to_full_posix_path */
#endif

extern bfd_boolean is_ipa;

void *(*p_ipa_open_input)(char *, off_t *, int, char**) = NULL;
void (*p_ipa_init_link_line)(int, char **) = NULL;
void (*p_ipa_add_link_flag)(const char*) = NULL;
void (*p_ipa_driver)(int, char **) = NULL;
void (*p_process_whirl64)(void *, int, void *, int, const char *) = NULL;
int  (*p_Count_elf_external_gots)(void) = NULL;
void (*p_ipa_insert_whirl_marker)(void) = NULL;
void (*p_Sync_symbol_attributes)(unsigned int, unsigned int, bfd_boolean, unsigned int) = NULL;
bfd_boolean (*p_symbol_comdat)(unsigned int) = NULL;
bfd_boolean (*p_symbol_linkonce)(unsigned int) = NULL;
void (*p_ipa_initialize_external_symbols)(struct external_symbols_list*) = NULL;

static mode_t cmask = 0;	    /* file creation mode mask */

static string_t *tmp_list = 0;
static int tmp_list_size = 0;
static int tmp_list_max = 0;
string_t tmpdir = 0;
static int tmpdir_length = 0;

static bfd *p_current_bfd = NULL;

string_t outfilename = "./a.out";

int ipa_argc = 0;
char ** ipa_argv = NULL;

LD_IPA_OPTION ld_ipa_opt[] = {
/************************************************/
/*  ld_ipa_option_enum	    flag	    set */
/************************************************/
    {LD_IPA_SHARABLE, 	    F_NON_SHARED,   F_NON_SHARED}, 
    {LD_IPA_DEMANGLE, 	    0,		    0}, 
    {LD_IPA_SHOW, 	    0,		    0}, 
    {LD_IPA_HIDES, 	    0,		    0}, 
    {LD_IPA_TARGOS, 	    TOS_IA64_64,    0}, 
    {LD_IPA_VERBOSE,	    0,		    0},
    {LD_IPA_KEEP_TEMPS,	    0,		    0},
    {LD_IPA_ISA,	    0,		    0},
    {LD_IPA_RELOCATABLE,    0,		    0},
    {LD_IPA_EXPORTS, 	    0,		    0}, 
    {LD_IPA_XXXX, 	    0,		    0}
};


string_t
make_temp_file_with_suffix (string_t, char);
void
ipa_set_def_bfd(bfd *, struct bfd_link_hash_entry *);
void
ipa_process_whirl_in_archive ( bfd *archive, bfd *abfd);


	/*******************************************************
		Function: ipa_copy_of

		Allocate for and copy given string into a copy.

	 *******************************************************/
char *
ipa_copy_of (char *str)
{
    register int len;
    register char *p;

    len = strlen(str) + 1;
    p = (char *) MALLOC (len);
    MALLOC_ASSERT (p);
    MEMCPY (p, str, len);
    return p;
} /* ipa_copy_of */


	/*******************************************************
		Function: concat_names

		Create a new string by concating 2 other strings.

	 *******************************************************/
string_t
concat_names(const string_t name1, const string_t name2)
{
    char *mangled_name = NULL;
    int len = strlen(name1)+strlen(name2)+1;

    mangled_name = (char *)MALLOC(len);
    MALLOC_ASSERT(mangled_name);

    strcpy(mangled_name, name1);
    strcat(mangled_name, name2);

    return(mangled_name);
}



	/*******************************************************
		Function: add_to_tmp_file_list

		Maintain list of temp. files created so they are all
		removed on error or when done.  Assume the first entry
		is "tmpdir".

	 *******************************************************/
void
add_to_tmp_file_list (string_t path)
{
    if (tmp_list_max == 0) {
	tmp_list_max = DEFAULT_TMP_LIST_SIZE;
	tmp_list = (string_t *) MALLOC (tmp_list_max * sizeof(string_t));
	MALLOC_ASSERT (tmp_list);
    } else if (tmp_list_size >= tmp_list_max) {
	tmp_list_max *= 2;
	tmp_list = (string_t *)REALLOC (tmp_list, tmp_list_max * sizeof(string_t));
	MALLOC_ASSERT (tmp_list);
    }

    tmp_list[tmp_list_size++] = path;

} /* add_to_tmp_file_list */

	/*******************************************************
		Function: make_temp_file_with_suffix

		Create a unique file

	 *******************************************************/
/* create a unique file */
string_t
make_temp_file_with_suffix (string_t name, char suffix)
{
    char path[PATH_MAX];
    int len;
    int count = 1;

    len = strlen (name);
    if (len+4 >= PATH_MAX) {
	fprintf(stderr,"%s %s\n","path name too long:", name);
	exit(1);
    }

    strcpy (path, name);

    if (suffix && len >= 2) {
	/* remove the original suffix */
	if (path[len-2] == '.') {
	    len -= 2;
	    path[len] = 0;
	}
    }

    if (suffix) {
	path[len] = '.';
	path[len+1] = suffix;
	path[len+2] = 0;
    }

    if (access (path, F_OK) != 0)
	return ipa_copy_of (path);

    do {
	if (suffix)
	    sprintf (&(path[len]), ".%d.%c", count, suffix);
	else
	    sprintf (&(path[len]), "%d", count);
	count++;
    } while (access (path, F_OK) == 0);

    return ipa_copy_of (path);

} /* make_temp_file_with_suffix */

/* ====================================================================
 *
 * create_tmpdir
 *
 * Create a temporary directory for (1) relocatable objects generated
 * from the backend under IPA control, (2) IR objects extracted from an
 * archive, and (3) IR objects generated by IPA.
 *
 * There are three cases.  If this is a directory to be kept, because
 * either -keep or a trace flag is specified for an IPA build, then the
 * directory is named <outfilename>.ipakeep, and if it already exists,
 * all files are cleared from it.  If it is a temporary directory for
 * an IPA build, a unique name is used with the template
 * $TMPDIR/<outfilename_with_path_stripped>.ipaXXXXXX .  For a normal link, 
 * a temporary directory
 * is created in the DEFAULT_TMPDIR with the name template XXXXXX.
 *
 * ====================================================================
 */

int
create_tmpdir ( int tracing )
{
    int fixedname = is_ipa && ( ld_ipa_opt[LD_IPA_KEEP_TEMPS].flag );

    /* [CL] If we have already created tmpdir, return immediately:
       this condition occurs when extracting Whirl files from an
       archive; in this case, create_tmpdir() is called twice:
       first when extracting from archive, second from ipa_driver()
    */
    if (tmpdir) return 0;

    if ( is_ipa ) {
	if ( fixedname ) {
	    tmpdir = concat_names ( outfilename, ".ipakeep" );
	} else {
	    char *tmpdir_env_var;
	    if ((tmpdir_env_var = getenv("TMPDIR")) != NULL) {
		char *filename;
	        tmpdir_env_var = concat_names ( tmpdir_env_var, "/");
		if ((filename = strrchr(outfilename, '/')) != NULL)
		    filename++;
		else
		    filename = outfilename;
		
	        tmpdir = concat_names ( tmpdir_env_var, filename);
	    }
	    else
	        tmpdir = outfilename;
	    tmpdir = concat_names ( tmpdir, ".ipaXXXXXX" );
	}
    } else {
	tmpdir = concat_names ( DEFAULT_TMPDIR, "XXXXXX" );
    }
#if defined(__CYGWIN__)
    char posix_driver_directory[_POSIX_PATH_MAX] ;
    cygwin_conv_to_full_posix_path(tmpdir, posix_driver_directory) ;
    tmpdir = ipa_copy_of(posix_driver_directory);
#endif
    if ( ! fixedname ) {
	tmpdir = mktemp ( tmpdir );
    }

    tmpdir_length = strlen ( tmpdir );

    if ( cmask == 0 ) {
	cmask = umask (0);
	(void) umask (cmask);
    }

    if ( MKDIR (tmpdir, 0777 & ~cmask) != 0 ) {
	if ( errno == EEXIST && fixedname ) {
	    /* We have an old instance of this directory -- clear it out: */
	    DIR *dirp;
#if defined(sun) || defined(__CYGWIN__) || defined (_WIN32)
	    struct dirent *entryp;
#else
	    struct direct *entryp;
#endif
	    char *prefix;

	    dirp = opendir ( tmpdir );
	    if ( dirp != NULL ) {
		prefix = concat_names ( tmpdir, "/" );
		while ( ( entryp = readdir(dirp) ) != NULL ) {
		    /* Don't bother with names of one or two characters, e.g. '.'
		     * and '..', since we don't create temporary files with such
		     * names:
		     */
#if defined(sun) || defined(__CYGWIN__) || defined (_WIN32)
		  if ( strlen(entryp->d_name) > 2)
#elif defined(_DIRENT_HAVE_D_NAMLEN)
		  if ( entryp->d_namlen > 2 )
#else
		  if (_D_EXACT_NAMLEN(entryp) > 2)
#endif
		    {
			string_t fname = concat_names ( prefix, entryp->d_name);
			unlink (fname);
			FREE (fname);
		    }
		}
		FREE (prefix);
		closedir ( dirp );
	    }
	} else {
    	    perror("cannot create temporary directory for code generation");
	    return -1;
	}
    }

    add_to_tmp_file_list ( tmpdir );

    if (tracing) {
#if 0
      printf("Created tmp dir %s\n", tmpdir);
#endif
    }
    return 0;

} /* create_tmpdir */

	/*******************************************************
		Function: create_unique_file

		

	 *******************************************************/
string_t
create_unique_file (const string_t path, char suffix)
{
    string_t p;
    string_t base = basename (path);
    string_t new_path;
    int fd;

    /* length of tmpdir + basename of path and '/' between the dir
       and the basename + null terminator */
    p = (string_t) MALLOC (strlen(tmpdir) + strlen(base) + 2);
    MALLOC_ASSERT (p);
    strcpy (p, tmpdir);
    strcat (p, "/");
    strcat (p, base);
    new_path = make_temp_file_with_suffix (p, suffix);
    FREE (p);

    if ((fd = creat (new_path, 0666 & ~cmask)) == -1) {
	perror(new_path);
	exit(1);
    }

    CLOSE (fd);
    
    return new_path;

} /* create_unique_file */


	/*******************************************************
		Function: create_unique_file_in_curdir
                [CL] do not create file in tmpdir, but in
                current directory. Useful for cleanup		

	 *******************************************************/
string_t
create_unique_file_in_curdir (const string_t path, char suffix)
{
    string_t p;
    string_t base = basename (path);
    string_t new_path;
    int fd;

    /* length of tmpdir + basename of path and '/' between the dir
       and the basename + null terminator */
    p = (string_t) MALLOC (strlen(base) + 2);
    MALLOC_ASSERT (p);
    strcpy (p, base);
    new_path = make_temp_file_with_suffix (p, suffix);
    FREE (p);

    if ((fd = creat (new_path, 0666 & ~cmask)) == -1) {
	perror(new_path);
	exit(1);
    }

    CLOSE (fd);
    
    return new_path;

} /* create_unique_file_in_curdir */



	/*******************************************************
		Function: ipa_set_ndx

		This field cannot be used beyond
		the pass1 phase.
	 *******************************************************/
int
ipa_set_ndx (bfd *abfd)
{
    if (ipa_is_whirl(abfd))
      	return WHIRL_ST_IDX_UNINITIALIZED;
    else
      	return WHIRL_ST_IDX_NOT_AVAILABLE;
}

	/*******************************************************
		Function: ipa_set_def_bfd


	 *******************************************************/
void
ipa_set_def_bfd(bfd *abfd, struct bfd_link_hash_entry *p_bfd_hash)
{
    if (is_ipa)
    	p_bfd_hash->u.def.section = (asection *)abfd;
}

	/*******************************************************
		Function: ld_set_cur_bfd


	 *******************************************************/
void 
ld_set_cur_obj(bfd *abfd)
{
    p_current_bfd = (bfd *)abfd;
}



	/*******************************************************
		Function: ld_get_cur_bfd


	 *******************************************************/
void *
ld_get_cur_obj(void)
{
    return p_current_bfd;
}

	/*******************************************************
		Function: ipa_is_whirl


	 *******************************************************/
#define ET_SGI_IR   (ET_LOPROC + 0)

bfd_boolean
ipa_is_whirl(bfd *abfd)
{
    Elf_Internal_Ehdr *i_ehdrp;	/* Elf file header, internal form */

    i_ehdrp = elf_elfheader (abfd);

    if (i_ehdrp->e_type == ET_SGI_IR) {
    	    return(TRUE);
    }

    return(FALSE);
}


	/*******************************************************
		Function: ipa_process_whirl

		I need to read the WHIRL symbol table so the
		internal mechanisms of IPA will have their 
		data structures correctly filled out.
		
		Since IPA needs an mmapped view of the object
		I'm trying to remap it here. It is not ready
		for archives yet.
		
		I am overloading the usrdata field of the bfd
		with the assumption that bfd is done with it.
		
	 *******************************************************/
void
ipa_process_whirl ( bfd *abfd) 
{

    off_t mapped_size;
    abfd->usrdata = (PTR)(*p_ipa_open_input)((char *)abfd->filename, &mapped_size, ipa_argc, ipa_argv);
    (*p_process_whirl64) ( 
    	    	(void *)abfd, 
    	    	elf_elfheader (abfd)->e_shnum, 
		abfd->usrdata+elf_elfheader(abfd)->e_shoff,
	        0, /* check_whirl_revision */
		abfd->filename);
}



#define MAX_RBUF_SIZE 1*1024*1024
	/*******************************************************
		Function: ipa_process_whirl_in_archive

	 *******************************************************/
void
ipa_process_whirl_in_archive ( bfd *archive, bfd *abfd) 
{
  FILE *fd = NULL, *tmp_fd = NULL;
  int elem_size = arelt_size (abfd);
  off_t mapped_size;
  string_t input_path;
  string_t file_name = (string_t)abfd->filename;
  void *rbuf = NULL;
  size_t size_buf = 0;
  long cur_size = 0;
  int seek_pos = 0;

#define ARCHIVE_ERROR_MESSAGE " cannot process archive member "
  int n=strlen(archive->filename) + strlen(abfd->filename) + strlen(ARCHIVE_ERROR_MESSAGE);
  char archive_msg[n];
  sprintf(archive_msg, "%s" ARCHIVE_ERROR_MESSAGE "%s",
	  archive->filename, abfd->filename);

  if ( (fd = fopen(archive->filename, "rb")) == NULL) {
    perror(archive_msg);
    exit(1);
  }

  if (elem_size > MAX_RBUF_SIZE) {
      rbuf = (void *) MALLOC(MAX_RBUF_SIZE);
      MALLOC_ASSERT(rbuf);
      size_buf = MAX_RBUF_SIZE;
  } else {
      rbuf = (void *) malloc(elem_size);
      size_buf = elem_size;
  }

  memset(rbuf, 0, size_buf);

  /* Start copying from the begining of the current object file in the
     archive */
  seek_pos = fseek(fd, abfd->origin, SEEK_SET);
  if (seek_pos != 0) {
      perror(archive_msg);
      exit(1);
  }

  /* Code borrowed from ld_compile() -- [CL] */
    
  if (tmpdir == 0)
    if (create_tmpdir (FALSE) != 0) {
      perror(archive_msg);
      exit(1);
    }

  if ((input_path = create_unique_file (file_name, 'B')) == 0) {
    perror(archive_msg);
    exit(1);
  }

  tmp_fd = fopen (input_path, "wb");
  if (tmp_fd == NULL) {
    perror(archive_msg);
    exit(1);
  }

  cur_size = elem_size;
  while (cur_size > 0) {
      size_t ret;

      ret = fread (rbuf, 1, size_buf, fd);

      if (ret > 0) {
	  size_t w_ret;

	  w_ret = fwrite(rbuf , 1, ret, tmp_fd);
	  if ((w_ret == 0) || (w_ret != ret)) {
	      perror(archive_msg);
	      exit(1);
	  }
	  cur_size -= ret;

	  /* [CL] If we read nothing, but cur_size is still > 0, it
	     means there was an error */
      } else if ((ret == 0) ) {
	  perror(archive_msg);
	  exit(1);
      }
  }

  FREE(rbuf);
  fclose (fd);
  fclose (tmp_fd);

  abfd->usrdata = (PTR)(*p_ipa_open_input)((char *)input_path, &mapped_size, ipa_argc, ipa_argv);
  if ((long) abfd->usrdata == -1) {
    perror(archive_msg);
    exit(1);
  }

  (*p_process_whirl64) ( 
    	    	(void *)abfd, 
    	    	elf_elfheader (abfd)->e_shnum, 
		abfd->usrdata+elf_elfheader(abfd)->e_shoff,
	        0, /* check_whirl_revision */
		abfd->filename);
}

#endif
