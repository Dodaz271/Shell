#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "read_commands.h" 
#include "commands_execution.h"
#include "editor.h"

void find_pgid_shell() 
{
    int shell_pgid = getpid();
    if (setpgid(shell_pgid, shell_pgid) == -1) {
        perror("setpgid");
        exit(1);
    }
    tcsetpgrp(0, shell_pgid);
    return;
}

int main()
{
    char *str = NULL, **arr_commands = NULL;
    int count = 0;
    bool flag = false;
    int **pos_separators = malloc(sizeof(int)), size = 0;
    int i = 0;
    find_pgid_shell();
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
    free(pos_separators);
    putchar('\n');
    return 0;
}
