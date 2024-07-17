#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

struct word_item {
    char *word;
    struct word_item *next;
};

void add_element(int **pos_separators, int *size, int new_value);
bool separators(char c, char *str, int *n,/* bool *is_ampersand,*/ bool flag, /*bool *i_o_redirection, */int **pos_separators, int *count, int *size);
void delete_commands(struct word_item *commands);
char *newArray(char *str, int n, int arr_size);
void add_node(struct word_item **first, const char *str);
void print_commands(struct word_item **commands);
char *read_commands(char *str, char c, int *n, int *arr_size, bool *flag, int *count, struct word_item **commands,/* bool *is_ampersand, bool *i_o_redirection,*/ int **pos_separators, int *size);
