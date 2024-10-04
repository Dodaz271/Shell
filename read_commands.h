#ifndef READ_COMMANDS_H
#define READ_COMMANDS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>


void add_element(int **pos_separators, int *size, int new_value);
char *newArray(char *str, int n, int arr_size);
bool is_separators(char *str, int pos, bool is_prev);
char *read_commands(char *str, bool *flag, int *count, int **pos_separators, int *size, char ***arr_commands);

#endif
