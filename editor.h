#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <stdbool.h>


char *incCommand(char **command, char *buf, int *command_size, int *buf_size, bool is_space);
char *incStr(char *str, int str_size);
char *incBuf(char **buf, int *buf_size, int additional_len);
void handle_delete(int *curr_pos, int *max_pos, char **buf, char **command, int *n, int *len);
void handleCtrlW(char **command, char **buf, int *n, int *len, int *curr_pos, int *max_pos);
void add_char_and_refresh(int *curr_pos, int *max_pos, char **buf, char **command, int *n, char c, int *len, bool is_slash);
void remove_and_refresh(int *curr_pos, int *max_pos, char **buf, char **command, int *n, int *len);
enum esc_subsequence get_direction(const char* buf);
int last_slash_buf(char **buf, int *buf_len, int *pos_space, int *curr_pos, int *prev_pos_space);
char *handleTabCompletion(char **buf, int *len, char *command, int *n, int *max_pos, int *curr_pos, int *tab_press_count);
char *read_text();
void enable_canon_mode(struct termios *origin_termios);
