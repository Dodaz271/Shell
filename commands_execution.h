#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>


void change_dir(char **arr_commands);
void exec_command(char **arr_commands, bool is_ampersand, int count);

