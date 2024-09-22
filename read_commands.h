#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>


void add_element(int **pos_separators, int *size, int new_value);
bool separators(char *str, int n, bool flag, int **pos_separators, int *count, int *size, int pos_next_space);
char *newArray(char *str, int n, int arr_size);
char *read_commands(char *str, bool *flag, int *count, int **pos_separators, int *size, char ***arr_commands);
