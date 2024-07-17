#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>


void change_dir(char **arr_commands);
void exec_command(char **arr_commands, int count, int *size, int **pos_separators);

