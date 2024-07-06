#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "read_commands.h" 


void free_all_alloc_mem(int *n, int *arr_size, bool *flag, int *count, char *str, struct word_item *commands)
{
    free(n);
    free(arr_size);
    free(flag);
    free(count);
    if(str != NULL) {
        free(str);
    }
    if(commands != NULL) {
        delete_commands(commands);
    }
    return;
}

int main()
{
    char c;
    char *str = NULL;
    struct word_item *commands = NULL;
    int *n = malloc(sizeof(int)), *arr_size =  malloc(sizeof(int)), *count = malloc(sizeof(int));
    bool *flag = malloc(sizeof(bool));
    *n = 0;
    *arr_size = 2;
    *flag = false;
    printf("> ");
    while((c = getchar()) != EOF) {
	str = read_commands(str, c, n, arr_size, flag, count, &commands);
	if(c == '\n') {
            print_commands(&commands);
	    delete_commands(commands);
	    commands = NULL;
	    printf("> ");
	    str = NULL;
	    *n = 0;
	    *arr_size = 0;
	    *flag = false;
	    continue;
	}
    }
    if(commands != NULL) {
        print_commands(&commands);
    }
    free_all_alloc_mem(n, arr_size, flag, count, str, commands);
    putchar('\n');
    return 0;
}
