#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "read_commands.h" 
#include "commands_execution.h"


char **command_array(struct word_item *commands, int count) {
    char **newArray = malloc((count + 1) * sizeof(char*));
    if (!newArray) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    struct word_item *tmp = commands;
    for (int i = 0; i < count; i++) {
        newArray[i] = tmp->word;
        tmp = tmp->next;
    }
    newArray[count] = NULL;
    return newArray;
}

void free_all_alloc_mem(int *n, int *arr_size, bool *flag, int *count, char *str, struct word_item *commands, char **arr_commands, int **pos_separators, int *size)
{
    free(n);
    free(arr_size);
    free(flag);
    free(count);
    free(*pos_separators);
    free(pos_separators);
    free(size);
    if(str != NULL) {
        free(str);
    }
    if(commands != NULL) {
        delete_commands(commands);
    }
    free(arr_commands);
    return;
}

int main()
{
    char c, *str = NULL, **arr_commands = NULL;
    struct word_item *commands = NULL;
    int *n = malloc(sizeof(int)), *arr_size =  malloc(sizeof(int)), *count = malloc(sizeof(int));
    bool *flag = malloc(sizeof(bool));
    int **pos_separators = malloc(sizeof(int)), *size = malloc(sizeof(int));
    *n = 0;
    *arr_size = 2;
    *flag = false;
    *count = 0;
    *size = 0;
    printf("> ");
    while((c = getchar()) != EOF) {
	str = read_commands(str, c, n, arr_size, flag, count, &commands, pos_separators, size);
	if(c == '\n') {
	    *n = 0;
	    *arr_size = 0;
            if(*flag == true) {
                printf("Error: Unclosed quotes\n");
                free(str);
		str = NULL;
            }
	    *flag = false;
	    if(commands) {
	        arr_commands = command_array(commands, *count);
	        if(arr_commands) {
	            exec_command(arr_commands, *count, size, pos_separators);
	        }
	        delete_commands(commands);
	        *count = 0;
	        free(arr_commands);
	        free(*pos_separators);
	        *pos_separators = NULL;
	        *size = 0;
	        arr_commands = NULL;
	    }
	    commands = NULL;
	    printf("> ");
	    continue;
	}
    }
    if(commands != NULL) {
        print_commands(&commands);
    }
    free_all_alloc_mem(n, arr_size, flag, count, str, commands, arr_commands, pos_separators, size);
    putchar('\n');
    return 0;
}
