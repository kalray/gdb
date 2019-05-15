#ifndef _SOLIB_K1_BARE_H_
#define _SOLIB_K1_BARE_H_

#include "solib.h"
#include "inferior.h"

extern struct target_so_ops k1_bare_solib_ops;

void k1_bare_solib_load_debug_info (void);
const char *get_cluster_name (struct inferior *inf);

#endif // _SOLIB_K1_BARE_H_
