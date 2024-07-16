#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "read_commands.h" 
#include "commands_execution.h"


char **command_array(struct word_item *commands, int count, bool is_ampersand)
{
    struct word_item *tmp = commands;
    int i = 0;
    char **newArray = malloc((count+1) * sizeof(char*));
    for(i = 0; i < count; i++) {
        newArray[i] = tmp->word;
	tmp = tmp->next;
	if((is_ampersand) && (strcmp(newArray[i], "&") == 0) && (tmp)) {
	    free(newArray);
	    return NULL;
	}
    }
    newArray[i] = NULL;
    return newArray;
}

void free_all_alloc_mem(int *n, int *arr_size, bool *flag, int *count, char *str, struct word_item *commands, char **arr_commands, bool *is_ampersand, bool *i_o_redirection)
{
    free(n);
    free(arr_size);
    free(flag);
    free(count);
    free(is_ampersand);
    free(i_o_redirection);
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
    bool *flag = malloc(sizeof(bool)), *is_ampersand = malloc(sizeof(bool)), *i_o_redirection = malloc(sizeof(bool));
    *n = 0;
    *arr_size = 2;
    *flag = false;
    *is_ampersand = false;
    *i_o_redirection = false;
    *count = 0;
    printf("> ");
    while((c = getchar()) != EOF) {
	str = read_commands(str, c, n, arr_size, flag, count, &commands, is_ampersand, i_o_redirection);
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
	    printf("Count: %d\nIs_ampersand: %s\nI_O_Redirection: %s\n", *count, *is_ampersand ? "true" : "false", *i_o_redirection ? "true" : "false");
	    if(commands) {
	        arr_commands = command_array(commands, *count, *is_ampersand);
	        if(arr_commands) {
	            exec_command(arr_commands, *is_ampersand, *count, *i_o_redirection);
	        } else {
	            printf("Error: ampersand is not last significant symbol\n");
	        }
	    }
	    delete_commands(commands);
            commands = NULL;
	    *count = 0;
	    free(arr_commands);
	    arr_commands = NULL;
	    *is_ampersand = false;
	    *i_o_redirection = false;
	    printf("> ");
	    continue;
	}
    }
    if(commands != NULL) {
        print_commands(&commands);
    }
    free_all_alloc_mem(n, arr_size, flag, count, str, commands, arr_commands, is_ampersand, i_o_redirection);
    putchar('\n');
    return 0;
}
