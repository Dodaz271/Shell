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
	free(home);
	return;
    }
    chdir(arr_commands[1]);
    return;
}

void exec_command(char **arr_commands)
{
    int pid;
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
	return;
    }
    wait(NULL);
    return;
}
