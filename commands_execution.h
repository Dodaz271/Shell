#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>


void change_dir(char **arr_commands);
bool i_o_redirection(char ***arr_commands, bool *input_flag, bool *output_flag, int **pos_separators, int i, int *fd, int j);
bool search_separators(int *amount, int i, int *j, int **pos_separators, char **arr_commands, bool *is_ampersand, bool *input_flag, bool *output_flag, int *fd, bool *pipe_flag, int *size);
void pipeline(int pipe_fd[2], char **token, int *pipe_input, char **arr_commands, int **pos_separators, int j, bool is_ampersand, int amount);
char *** newPipearray(char ****pipe_command, char **token, int *len);
int size_of_token(int count, int amount, int **pos_separators, int j, int *size);
void add_tokens(char ***token, int i, int **pos_separators, int *n, int j, char **arr_commands);
void free_pipe_commands(char ****pipe_commands, int len);
void execute_single_command(char **token, bool input_flag, bool output_flag, int fd, bool is_ampersand);
void exec_command(char **arr_commands, int count, int *size, int **pos_separators);

