#include "commands_execution.h"


void change_dir(char **arr_commands) {
    if (arr_commands[2] != NULL) {
        perror("cd");
        printf("cd: too many arguments\n");
    }
    if (arr_commands[1] == NULL) {
        char *home = getenv("HOME");
        if (home == NULL) {
            perror("getenv");
        }
        chdir(home);
        return;
    }
    if (chdir(arr_commands[1]) != 0) {
        perror("cd");
    }
}

bool i_o_redirection(char ***arr_commands, bool *input_flag, bool *output_flag, int **pos_separators, int i, int *fd, int j) {
    if (((strcmp((*arr_commands)[i], ">") == 0) || (strcmp((*arr_commands)[i], "<") == 0) || (strcmp((*arr_commands)[i], ">>") == 0)) && ((i + 1) == (*pos_separators)[j])) {
        printf("Name of file can't be separator\n");
        perror("File name");
        *fd = -1;
        return false;
    }
    if ((i == (*pos_separators)[j]) && (strcmp((*arr_commands)[i], ">") == 0)) {
        *fd = open((*arr_commands)[i + 1], O_CREAT | O_WRONLY | O_TRUNC, 0666);
        *input_flag = true;
        return true;
    }
    if ((i == (*pos_separators)[j]) && (strcmp((*arr_commands)[i], ">>") == 0)) {
        *fd = open((*arr_commands)[i + 1], O_CREAT | O_WRONLY | O_APPEND, 0666);
        *input_flag = true;
        return true;
    }
    if ((i == (*pos_separators)[j]) && (strcmp((*arr_commands)[i], "<") == 0)) {
        *fd = open((*arr_commands)[i + 1], O_RDONLY);
        *output_flag = true;
        return true;
    }
    return false;
}

bool search_separators(int *amount, int i, int *j, int **pos_separators, char **arr_commands, bool *is_ampersand, bool *input_flag, bool *output_flag, int *fd, bool *pipe_flag, int *size) 
{
    if(((*j > 0) && (strcmp(arr_commands[(*pos_separators)[*j-1]], "|") == 0)) || (((*pos_separators)[*j] == -1) && (strcmp(arr_commands[(*pos_separators)[*j-1]], "|") == 0))) {
        *pipe_flag = true;
    }
    if ((*j < ((*size)+1)) && (i == (*pos_separators)[*j])) {
	if ((i == ((*pos_separators)[*j])) && (strcmp(arr_commands[i], "&") == 0)) {
	    *is_ampersand = true;
	}
        if (i_o_redirection(&arr_commands, input_flag, output_flag, pos_separators, i, fd, *j)) {
            (*amount)++;
        }
        if ((i == (*pos_separators)[*j]) && (strcmp(arr_commands[i], "|") == 0)) {
	    *pipe_flag = true;
            (*amount)++;
        }
	(*j)++;
	return true;
    }
    return false;
}

void pipeline(int pipe_fd[2], char **token, int *pipe_input, char **arr_commands, int **pos_separators, int j, bool is_ampersand, int amount) {
    int pipe_pid, p;
    pipe(pipe_fd);
    if(pipe_fd == -1) {
        perror("pipe");
	exit(1);
    }
    pipe_pid = fork();
    if(pipe_pid == 0) {
	dup2(*pipe_input, 0);
        close(*pipe_input);
        if((((*pos_separators)[j] != -1) && ((strcmp(arr_commands[(*pos_separators)[j]], "|") == 0) || (strcmp(arr_commands[(*pos_separators)[j-1]], "|") == 0))) || (((*pos_separators)[j-1] == (amount-1)) && ((*pos_separators)[j] == -1) && (strcmp(arr_commands[(*pos_separators)[j-1]], "|") == 0))) {
            dup2(pipe_fd[1], 1);
        }
        close(pipe_fd[0]);
	close(pipe_fd[1]);
	execvp(token[0], token);
        perror(token[0]);
	exit(2);
    }
    if(*pipe_input != 0) {
        close(*pipe_input);
    }
    close(pipe_fd[1]);
    if (*pipe_input != 0) {
        close(*pipe_input); 
    }
    if (!is_ampersand) {
        do {
            p = wait(NULL);
        } while (p != pipe_pid);
    } else {
        printf("Process running in background with PID %d\n", pipe_pid);
    }
    *pipe_input = pipe_fd[0];
    return;
}

char ***newPipearray(char ****pipe_command, char **token, int *len) {
    int i;
    char ***tmp = (char***)malloc(((*len) + 2) * sizeof(char**));
    if (tmp == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    for (i = 0; i < (*len); i++) {
        tmp[i] = (*pipe_command)[i];
    }
    if(*len > 0) {
        free(*pipe_command);
	*pipe_command = NULL;
    }
    tmp[*len] = token;
    (*len)++;
    tmp[*len] = NULL;
    return tmp;
}

int size_of_token(int count, int amount, int **pos_separators, int j, int *size)
{
    int vol = 0;
    if (j == 0) {
        vol = ((*pos_separators)[j]);
    } else if ((*pos_separators)[j] != -1) {
        vol = ((*pos_separators)[j]) - amount;
    } else {
        vol = count - ((*pos_separators)[j-1] + 1);
    }
    return vol;
}

void add_tokens(char ***token, int i, int **pos_separators, int *n, int j, char **arr_commands)
{
    if (i != ((*pos_separators)[j])) {
        (*token)[*n] = strdup(arr_commands[i]);
        (*n)++;
    } else {
	if((*n) > 0) {
            (*token)[*n] = NULL;
            *n = 0;
	} else {
	    *token = NULL;
	}
    }
    return;
}

void free_pipe_commands(char ****pipe_commands, int len) {
    int i, j;
    for (i = 0; i < len; i++) {
        for (j = 0; (*pipe_commands)[i][j] != NULL; j++) {
            free((*pipe_commands)[i][j]);
	    (*pipe_commands)[i][j] = NULL;
        }
        free((*pipe_commands)[i]);
	(*pipe_commands)[i] = NULL;
    }
    free(*pipe_commands);
    (*pipe_commands) = NULL;
    return;
}

void execute_single_command(char **token, bool input_flag, bool output_flag, int fd, bool is_ampersand) {
    int pid, p;
    pid = fork();
    if (pid == -1) {
        perror("fork");
        return;
    }
    if (pid == 0) {
        /*if (output_flag) {
            dup2(fd, 0);
            close(fd);
        }*/
        /*if (input_flag) {
            dup2(fd, 1);
            close(fd);
        }*/
        execvp(token[0], token);
        perror(token[0]);
        exit(EXIT_FAILURE);
    } else if (!is_ampersand) {
        do {
            p = wait(NULL);
        } while (p != pid);
    } else {
        printf("Process running in background with PID %d\n", pid);
    }
    return;
}

void exec_command(char **arr_commands, int count, int *size, int **pos_separators) 
{
    int i, fd, j = 0, n = 0, amount = 0, vol = 0, k, len = 0, pipe_fd[2], pipe_input = 0;
    char **token = NULL;
    char ***pipe_command = NULL;
    bool input_flag, output_flag, is_ampersand, pipe_flag;
    input_flag = output_flag = is_ampersand = pipe_flag = false;
    if (strcmp(arr_commands[0], "cd") == 0) {
        change_dir(arr_commands);
        return;
    }
    while (amount < count) {
        if ((pos_separators) && (*pos_separators)) {
	    vol = size_of_token(count, amount, pos_separators, j, size);
            token = malloc((vol+1) * sizeof(char*));
            for (i = amount; i < count; i++) {
		add_tokens(&token, i, pos_separators, &n, j, arr_commands);
                if (search_separators(&amount, i, &j, pos_separators, arr_commands, &is_ampersand, &input_flag, &output_flag, &fd, &pipe_flag, size)) {
                    break;
                }
                amount++;
            }
            if (fd == -1) {
                perror("open");
                return;
            }
        } else {
            if (!token) {
                token = malloc((count + 1) * sizeof(char*));
            }
            for (k = 0; k < count; k++) {
                if (((k + amount) < count) && (((j > 0) && (k != ((*pos_separators)[j-1]))) || (true))) {
                    token[k] = strdup(arr_commands[k]);
                } else {
                    break;
                }
            }
	    amount = amount + k;
            token[k] = NULL;
        }
        if ((!pipe_flag) && (token)) {
	    execute_single_command(token, input_flag, output_flag, fd, is_ampersand);
	    amount++;
        } else {
            pipeline(pipe_fd, token, &pipe_input, arr_commands, pos_separators, j, is_ampersand, amount);
	    if((((*pos_separators)[j] == -1) && (((strcmp(arr_commands[(*pos_separators)[j-1]], "|") == 0) && ((amount-1) > (*pos_separators)[j-1])) || ((strcmp(arr_commands[(*pos_separators)[j-1]], "|") != 0) && ((amount) >= (*pos_separators)[j-1])))) || (((*pos_separators)[j] != -1) && (strcmp(arr_commands[(*pos_separators)[j]], "|") != 0) && (amount >= (*pos_separators)[j]))) {
                close(pipe_input);
		amount++;
            }
        }
        input_flag = output_flag = is_ampersand = false;
	if(pipe_command) {
            free_pipe_commands(&pipe_command, len);
            pipe_command = NULL;
        } else if(token) {
	    if(vol > 0) {
	        for(i = 0; i <= vol; i++) {
		    free(token[i]);
		}
		free(token);
		token = NULL;
	    } else {
	        for(i = 0; i <= count; i++) {
		    free(token[i]);
		}
		free(token);
		token = NULL;
	    }
        }

    }
    return;
}

