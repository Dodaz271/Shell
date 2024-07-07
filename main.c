#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "read_commands.h" 
#include "commands_execution.h"


char **command_array(struct word_item *commands, int count)
{
    struct word_item *tmp = commands;
    int i = 0;
    char **newArray = malloc((count+1) * sizeof(char*));
    for(i = 0; i < count; i++) {
        newArray[i] = tmp->word;
	tmp = tmp->next;
    }
    newArray[i] = NULL;
    return newArray;
}

void free_all_alloc_mem(int *n, int *arr_size, bool *flag, int *count, char *str, struct word_item *commands, char **arr_commands)
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
    free(arr_commands);
    return;
}

int main()
{
    char c, *str = NULL, **arr_commands = NULL;
    struct word_item *commands = NULL;
    int *n = malloc(sizeof(int)), *arr_size =  malloc(sizeof(int)), *count = malloc(sizeof(int));
    bool *flag = malloc(sizeof(bool));
    *n = 0;
    *arr_size = 2;
    *flag = false;
    *count = 0;
    printf("> ");
    while((c = getchar()) != EOF) {
	str = read_commands(str, c, n, arr_size, flag, count, &commands);
	if(c == '\n') {
            print_commands(&commands);
	    *n = 0;
	    *arr_size = 0;
            if(*flag == true) {
                printf("Error: Unclosed quotes\n");
                free(str);
		str = NULL;
            }
	    *flag = false;
	    printf("Count: %d\n", *count);
	    arr_commands = command_array(commands, *count);
	    exec_command(arr_commands);
	    delete_commands(commands);
            commands = NULL;
	    *count = 0;
	    free(arr_commands);
	    arr_commands = NULL;
	    printf("> ");
	    continue;
	}
    }
    if(commands != NULL) {
        print_commands(&commands);
    }
    free_all_alloc_mem(n, arr_size, flag, count, str, commands, arr_commands);
    putchar('\n');
    return 0;
}
