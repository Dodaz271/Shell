#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "read_commands.h" 
#include "commands_execution.h"
#include "editor.h"

void free_all_alloc_mem(char *str, char ***arr_commands, int **pos_separators)
{
    free(*pos_separators);
    free(pos_separators);
    if(str != NULL) {
        free(str);
    }
    if((*arr_commands) != NULL) {
        for(int i = 0; (*arr_commands)[i] != NULL; i++) {
            free((*arr_commands)[i]);
        }
        free((*arr_commands));
    }
    return;
}

int main()
{
    struct termios origin_termios;
    char *str = NULL, **arr_commands = NULL;
    int count = 0;
    bool flag = false;
    int **pos_separators = malloc(sizeof(int)), size = 0;
    int i = 0;
    enable_canon_mode(&origin_termios);
    while((str = read_text()) != NULL) {
	    str = read_commands(str, &flag, &count, pos_separators, &size, &arr_commands);
	    if(flag == true) {
    		printf("Error: Unclosed quotes\n");
	    	free(str);
		    str = NULL;
	    }
	    flag = false;
	    if((str) && (str[0] != '\0')) {
		    if(arr_commands) {
		        exec_command(arr_commands, count, &size, pos_separators);
		    }
            count = 0;
		    free(*pos_separators);
    		*pos_separators = NULL;
	    	size = 0;
            i = 0;
            while(arr_commands[i] != NULL) {
                free(arr_commands[i]);
                i++;
            }
		    free(arr_commands); 
            arr_commands = NULL;
	    }
        free(str);
	}
    free_all_alloc_mem(str, &arr_commands, pos_separators);
    putchar('\n');
    tcsetattr(0, TCSANOW, &origin_termios);
    return 0;
}
