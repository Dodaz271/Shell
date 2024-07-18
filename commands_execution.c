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

bool i_o_redirection(char **arr_commands, bool *input_flag, bool *output_flag, int **pos_separators, int i, int *fd, int j)
{
    if((i == ((*pos_separators)[j])) && (strcmp(arr_commands[i], ">") == 0)) {
        *fd = open(arr_commands[i+1], O_CREAT|O_WRONLY|O_TRUNC, 0666);
        arr_commands[i+1] = NULL;
        arr_commands[i] = NULL;
        *input_flag = true;
	return true;
    }
    if((i == ((*pos_separators)[j])) && (strcmp(arr_commands[i], ">>") == 0)) {
        *fd = open(arr_commands[i+1], O_CREAT|O_WRONLY|O_APPEND, 0666);
        arr_commands[i+1] = NULL;
        arr_commands[i] = NULL;
        *input_flag = true;
	return true;
    }
    if((i == ((*pos_separators)[j])) && (strcmp(arr_commands[i], "<") == 0)) {
        *fd = open(arr_commands[i+1], O_RDONLY);
        arr_commands[i+1] = NULL;
        arr_commands[i] = NULL;
        *output_flag = true;
	return true;
    }
    return false;
}

void exec_command(char **arr_commands, int count, int *size, int **pos_separators)
{
    int pid, p, fd, j = 0;
    bool input_flag, output_flag, is_ampersand;
    input_flag = output_flag = is_ampersand = false;
    if(strcmp(arr_commands[0], "cd") == 0) {
        change_dir(arr_commands);
	return;
    }
    if(*pos_separators) {
	printf("Check\n");
        for(int i = 0; i < count; i++) {
            if((j < ((*size)-1)) && (i > (*pos_separators)[j])) {
		printf("POS: %d\n", (*pos_separators)[j]);
	        j++;
	    }
	    if((i == ((*pos_separators)[j])) && (strcmp(arr_commands[i], "&") == 0)) {
	        arr_commands[i] = NULL;
		is_ampersand = true;
		break;
	    }
	    if(i_o_redirection(arr_commands, &input_flag, &output_flag, pos_separators, i, &fd, j)) {
		break;
	    }
	    /*if((i == ((*pos_separators)[j])) && (strcmp(arr_commands[i], ">") == 0)) {
		fd = open(arr_commands[i+1], O_CREAT|O_WRONLY|O_TRUNC, 0666);  
		arr_commands[i+1] = NULL;
		arr_commands[i] = NULL;
		input_flag = true;
		break;
	    }
	    if((i == ((*pos_separators)[j])) && (strcmp(arr_commands[i], ">>") == 0)) {
	        fd = open(arr_commands[i+1], O_CREAT|O_WRONLY|O_APPEND, 0666);
                arr_commands[i+1] = NULL;
                arr_commands[i] = NULL;
		input_flag = true;
                break;
	    }
	    if((i == ((*pos_separators)[j])) && (strcmp(arr_commands[i], "<") == 0)) {
	        fd = open(arr_commands[i+1], O_RDONLY);
                arr_commands[i+1] = NULL;
                arr_commands[i] = NULL;
		output_flag = true;
		break;
	    }*/
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
    if(!is_ampersand) {
        do {
            p = wait(NULL);
        } while(p != pid);
	return;
    } else {
        printf("Process running in background with PID %d\n", pid);
    }
    /*free(input_flag);
    free(output_flag);
    free(is_ampersand);*/
    return;
}
