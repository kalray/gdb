#ifndef _K1_TARGET_H_
#define _K1_TARGET_H_

struct inferior_data
{
  const char *cluster;
  int booted;
  int sym_file_loaded;
  int cluster_break_on_spawn;
  int cluster_stop_all;
};

extern int after_first_resume;
extern char cjtag_over_iss;

void _initialize__k1_target (void);
void send_cluster_debug_level (int level);
void send_cluster_postponed_debug_level (struct inferior *inf, int level);
int get_is_hot_attached (struct inferior *inf);
int get_thread_mode_used_for_ptid (ptid_t ptid);
void send_cluster_break_on_spawn (struct inferior *inf, int v);
void send_cluster_stop_all (struct inferior *inf, int v);
char get_jtag_over_iss (void);

struct inferior_data *mppa_inferior_data (struct inferior *inf);

int read_memory_no_dcache (uint64_t addr, unsigned char *gdb_byte, int len);

void set_general_thread (struct ptid ptid);

#endif //_K1_TARGET_H_
