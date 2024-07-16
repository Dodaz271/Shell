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

void exec_command(char **arr_commands, bool is_ampersand, int count, bool i_o_redirection)
{
    int pid, p, fd;
    bool input_flag = false, output_flag = false;
    char *file_name = NULL;
    if(is_ampersand) {
        arr_commands[count-1] = NULL;
    }
    if(strcmp(arr_commands[0], "cd") == 0) {
        change_dir(arr_commands);
	return;
    }
    if(i_o_redirection) {
        for(int i = 0; i < count; i++) {
	    if(strcmp(arr_commands[i], ">") == 0) {
		fd = open(arr_commands[i+1], O_CREAT|O_WRONLY|O_TRUNC, 0666);  
		arr_commands[i+1] = NULL;
		arr_commands[i] = NULL;
		input_flag = true;
		break;
	    }
	    if(strcmp(arr_commands[i], ">>") == 0) {
	        fd = open(arr_commands[i+1], O_CREAT|O_WRONLY|O_APPEND, 0666);
                arr_commands[i+1] = NULL;
                arr_commands[i] = NULL;
		input_flag = true;
                break;
	    }
	    if(strcmp(arr_commands[i], "<") == 0) {
	        fd = open(arr_commands[i+1], O_RDONLY);
                arr_commands[i+1] = NULL;
                arr_commands[i] = NULL;
		output_flag = true;
		break;
	    }
	}
	if(fd == -1) {
	    perror("open");
	    return;
	}
    }
    pid = fork(); 
    if(pid == -1) {
        perror("fork");
	return;
    }
    if(pid == 0) {
	if(output_flag) {
	    dup2(fd, 0);
	    close(fd);
	}
	if(input_flag) {
            dup2(fd, 1);
	    close(fd);
        }
        execvp(arr_commands[0], arr_commands);
	perror(arr_commands[0]);
	exit(0);
    }
    /*if(output_flag) {
        dup2(save, 0);
    }
    if(input_flag) {
        dup2(save, 1);
    }*/
    close(fd);
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
