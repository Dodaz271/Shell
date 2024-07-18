#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>


void change_dir(char **arr_commands);
bool i_o_redirection(char **arr_commands, bool *input_flag, bool *output_flag, int **pos_separators, int i, int *fd, int j);
void exec_command(char **arr_commands, int count, int *size, int **pos_separators);

