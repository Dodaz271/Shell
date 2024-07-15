#include "commands_execution.h"

void change_dir(char **arr_commands)
{
    if(arr_commands[2] != NULL) {
        perror("cd");
	printf("cd: too many arguments\n");
    }
    if(arr_commands[1] == NULL) {
	char *home = getenv("HOME");
	if(home == NULL) {
	    perror("getenv");
	}
        chdir(home);
	return;
    }
    if(chdir(arr_commands[1]) != 0) {
        perror("cd");
    }
    return;
}

void exec_command(char **arr_commands, bool is_ampersand, int count)
{
    int pid, p;
    if(is_ampersand) {
        arr_commands[count-1] = NULL;
    }
    if(strcmp(arr_commands[0], "cd") == 0) {
        change_dir(arr_commands);
	return;
    }
    pid = fork(); 
    if(pid == -1) {
        perror("fork");
	return;
    }
    if(pid == 0) {
        execvp(arr_commands[0], arr_commands);
	perror(arr_commands[0]);
	exit(0);
    }
    if(!is_ampersand) {
        do {
            p = wait(NULL);
        } while(p != pid);
	return;
    } else {
        printf("Process running in background with PID %d\n", pid);
    }
    return;
}
