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
    if(((strcmp(arr_commands[i], ">") == 0) || (strcmp(arr_commands[i], "<") == 0) || (strcmp(arr_commands[i], ">>") == 0)) && ((i+1) == (*pos_separators)[j+1])) {
	printf("Name of file can't be seporator\n");
	perror("File name");
	*fd = -1;
        return false;
    }
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

bool pipefunc(char **arr_commands, bool *pipe_flag, int i, int j, int **pos_separators)
{
    if((i == (*pos_separators)[j]) && (strcmp(arr_commands[i], "|") == 0)) {
	*pipe_flag = true;
        return true;
    }
    return false;
}

/*void pipeline()
{
    int fd[2];
    pipe(fd);
    if(fork()==0) {
        
    }

}*/

void exec_command(char **arr_commands, int count, int *size, int **pos_separators)
{
    int pid, p, fd, j = 0, n = 0, amount = 0, vol = 0;
    char **command = NULL;
    bool input_flag, output_flag, is_ampersand, pipe_flag;
    input_flag = output_flag = is_ampersand = pipe_flag = false;
    if(strcmp(arr_commands[0], "cd") == 0) {
        change_dir(arr_commands);
	return;
    }
    /*if(*pos_separators) {
        command = malloc((*pos_separators)[j] * sizeof(int*));
    }*/
    while(amount < count) {
        if((*pos_separators) && ((*pos_separators)[j] != 0)) {
	    if(command != NULL) {
                free(command);
                command = NULL;
            }
	    if((j == 0) && (amount < ((*pos_separators)[j]))) {
	        vol = ((*pos_separators)[j]);
	    } else if(j > 0) {
	        vol = ((*pos_separators)[j]) - ((*pos_separators)[j-1]);
	    } else {
	        vol = count - ((*pos_separators)[j]+1);
	    }
	    command = malloc((vol+1) * sizeof(char*));
            for(int i = amount; i < count; i++) {
                if((j < ((*size)-1)) && (i > (*pos_separators)[j])) {
		    printf("POS: %d\n", (*pos_separators)[j]);
	            j++;
		    /*if(command != NULL) {
		        free(command);
			command = NULL;
		    }
		    command = malloc((*pos_separators)[j] * sizeof(int*));*/
	        }
	        if(i != ((*pos_separators)[j])) {
	            command[n] = arr_commands[i];
		    n++;
	        } else {
		    command[n] = NULL;
		    n = 0;
		}
	        if((i == ((*pos_separators)[j])) && (strcmp(arr_commands[i], "&") == 0)) {
	            //arr_commands[i] = NULL;
		    is_ampersand = true;
		    //amount++;
		    break;
	        }
	        if(i_o_redirection(arr_commands, &input_flag, &output_flag, pos_separators, i, &fd, j)) {
		    amount++;
		    break;
		}
		if(pipefunc(arr_commands, &pipe_flag, i, j, pos_separators)) {
		    amount++;
		    break;
	        }
		amount++;
	    }
	    if(fd == -1) {
	        perror("open");
	        return;
	    }
        } else {
	    command = malloc(2 * sizeof(char*));
	    command[0] = arr_commands[0];
	    command[1] = NULL;
	}
        if(!pipe_flag) {
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
                execvp(command[0], command);
	        perror(command[0]);
	        exit(0);
            }
        }
        if(!is_ampersand) {
            do {
                p = wait(NULL);
            } while(p != pid);
        } else {
            printf("Process running in background with PID %d\n", pid);
        }
	amount++;
	input_flag = output_flag = is_ampersand = false;
	printf("WHILE\n");
    }
    free(command);
    printf("END WHILE\n");
    return;
}
