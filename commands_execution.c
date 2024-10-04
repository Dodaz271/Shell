#include "commands_execution.h"

// Function to change the current working directory
void change_dir(char **arr_commands) {
    // If no argument is provided to 'cd', change to the HOME directory
    if(arr_commands[1] == NULL) {
        char *home = getenv("HOME"); // Get the HOME environment variable
        if (home == NULL) {
            perror("getenv"); // Print error if HOME is not set
        }
        chdir(home); // Change directory to HOME
        return;
    }
    // If more than one argument is provided, print an error
    else if((arr_commands[2] != NULL)) {
        printf("cd: too many arguments\n");
        return;
    }
    // Otherwise, attempt to change to the specified directory
    else {
        if (chdir(arr_commands[1]) != 0) {
            perror("cd"); // Print error if chdir fails
        }
    }
    return;
}

// Function to handle input/output redirection
bool i_o_redirection(char ***arr_commands, bool *input_flag, bool *output_flag, int **pos_separators, int i, int *fd, int j) {
    // Check if the current token is a redirection operator and if the next token is a separator
    if (((strcmp((*arr_commands)[i], ">") == 0) || 
         (strcmp((*arr_commands)[i], "<") == 0) || 
         (strcmp((*arr_commands)[i], ">>") == 0)) && 
        ((i + 1) == (*pos_separators)[j])) {
        printf("Name of file can't be separator\n");
        perror("File name");
        *fd = -1; // Indicate an error by setting file descriptor to -1
        return false;
    }

    // Handle output redirection with '>'
    if ((i == (*pos_separators)[j]) && (strcmp((*arr_commands)[i], ">") == 0)) {
        *fd = open((*arr_commands)[i + 1], O_CREAT | O_WRONLY | O_TRUNC, 0666); // Open file for writing (truncate if exists)
        *output_flag = true; // Set output flag
        return true;
    }

    // Handle output redirection with '>>' (append)
    if ((i == (*pos_separators)[j]) && (strcmp((*arr_commands)[i], ">>") == 0)) {
        *fd = open((*arr_commands)[i + 1], O_CREAT | O_WRONLY | O_APPEND, 0666); // Open file for appending
        *output_flag = true; // Set output flag
        return true;
    }

    // Handle input redirection with '<'
    if ((i == (*pos_separators)[j]) && (strcmp((*arr_commands)[i], "<") == 0)) {
        *fd = open((*arr_commands)[i + 1], O_RDONLY); // Open file for reading
        *input_flag = true; // Set input flag
        return true;
    }

    return false; // No redirection detected
}

// Function to search for separators (like pipes, redirections, etc.)
bool search_separators(int *amount, int i, int *j, int **pos_separators, char **arr_commands, bool *is_ampersand, bool *input_flag, bool *output_flag, int *fd, bool *pipe_flag, int *size)
{
    // Check if the previous separator was a pipe and handle pipe flag
    if(((*j > 0) && 
        (strcmp(arr_commands[(*pos_separators)[*j-1]], "|") == 0) && 
        (((*pos_separators)[*j] == -1) || 
         (strcmp(arr_commands[(*pos_separators)[*j]], "|") != 0)))) {
        *pipe_flag = true; // Set pipe flag
    }

    // If current index matches a separator position
    if ((*j < ((*size)+1)) && (i == (*pos_separators)[*j])) {
        // Check for background execution operator '&'
        if ((i == ((*pos_separators)[*j])) && (strcmp(arr_commands[i], "&") == 0)) {
            *is_ampersand = true; // Set ampersand flag
        }

        // Handle input/output redirection
        if (i_o_redirection(&arr_commands, input_flag, output_flag, pos_separators, i, fd, *j)) {
            (*amount)++; // Increment the count of redirections
        }

        // Handle pipe '|'
        if ((i == (*pos_separators)[*j]) && (strcmp(arr_commands[i], "|") == 0)) {
            if(strcmp(arr_commands[(*pos_separators)[*j-1]], "<") != 0) {
                *pipe_flag = true; // Set pipe flag
            }
            (*amount)++; // Increment the count
        }

        (*j)++; // Move to the next separator
        return true; // Separator found
    }
    return false; // No separator at current index
}

// Function to handle pipeline execution
void pipeline(int pipe_fd[2], char **token, int *pipe_input, char **arr_commands, int **pos_separators, int j, bool is_ampersand, int amount) {
    int pipe_pid, p;
    pipe(pipe_fd); // Create a pipe
    pipe_pid = fork(); // Fork a new process

    if(pipe_pid == 0) { // Child process
        dup2(*pipe_input, 0); // Redirect stdin to the reading end of the previous pipe
        close(*pipe_input); // Close the original pipe input

        // Check various conditions to decide if stdout should be redirected to the pipe
        if(((((*pos_separators)[j] != -1) && 
             ((strcmp(arr_commands[(*pos_separators)[j]], "|") == 0) || 
              (strcmp(arr_commands[(*pos_separators)[j-1]], "|") == 0))) || 
            (((*pos_separators)[j-1] == (amount-1)) && 
             ((*pos_separators)[j] == -1) && 
             (strcmp(arr_commands[(*pos_separators)[j-1]], "|") == 0))) || 
           (strcmp(arr_commands[(*pos_separators)[j-1]], ">") == 0) || 
           (strcmp(arr_commands[(*pos_separators)[j-1]], ">>") == 0)){
            dup2(pipe_fd[1], 1); // Redirect stdout to the writing end of the pipe
        }

        close(pipe_fd[0]); // Close the reading end in the child
        close(pipe_fd[1]); // Close the writing end in the child

        execvp(token[0], token); // Execute the command
        perror(token[0]); // Print error if execvp fails
        exit(1); // Exit child process with error code
    }

    close(pipe_fd[1]); // Close the writing end in the parent
    *pipe_input = pipe_fd[0]; // Update pipe_input to the reading end for the next command
    if (!is_ampersand) { // If not running in background
        do {
            p = wait(NULL); // Wait for the child process to finish
        } while (p != pipe_pid);
    } else { // If running in background
        printf("Process running in background with PID %d\n", pipe_pid);
    }
    return;
}

// Function to determine the size of the current token segment
int size_of_token(int count, int amount, int **pos_separators, int j, int *size)
{
    int vol = 0;
    if (j == 0) {
        vol = ((*pos_separators)[j]); // Distance from start to first separator
    } else if ((*pos_separators)[j] != -1) {
        vol = ((*pos_separators)[j]) - amount; // Distance between separators
    } else {
        vol = count - ((*pos_separators)[j-1] + 1); // Remaining tokens after last separator
    }
    return vol;
}

// Function to add tokens to the current command
void add_tokens(char ***token, int i, int **pos_separators, int *n, int j, char **arr_commands)
{
    if (i != ((*pos_separators)[j])) { // If current index is not a separator
        (*token)[*n] = strdup(arr_commands[i]); // Duplicate the token
        (*n)++; // Increment token count
    } else { // If current index is a separator
        if((*n) > 0) {
            (*token)[*n] = NULL; // Null-terminate the token array
            *n = 0; // Reset token count
        } else {
            (*token) = NULL; // Set token to NULL if no tokens are present
        }
    }
    return;
}

// Function to execute a single command with possible redirections and pipes
void execute_single_command(char **token, bool input_flag, bool output_flag, int fd, bool is_ampersand, int pipe_fd[2], int *pipe_input, int j, int **pos_separators, char **arr_commands) {
    int pid, p, rc;
    char buf[1024];

    // If output redirection is set and the previous separator was a pipe
    if ((*pos_separators) && (output_flag || input_flag) && 
        ((*pos_separators)[j] != -1) && 
        strcmp(arr_commands[(*pos_separators)[j]], "|") == 0) {

        pipe(pipe_fd); // Create a new pipe
    }

    pid = fork(); // Fork a new process
    if (pid == -1) { // Check for fork failure
        perror("fork");
        return;
    }
    if (pid == 0) { // Child process
        // Handle output redirection
        if (output_flag) { 
            // If previous separator was a pipe
            if ((strcmp(arr_commands[(*pos_separators)[j-2]], "|") == 0)) {
                if (fd != -1) { // If file descriptor is valid
                    while((rc = read(*pipe_input, buf, sizeof(buf))) > 0) { // Read from stdin and write to stdout
                        write(fd, buf, rc);
                    }
                    close(*pipe_input);
                    exit(1);
                }
                close(*pipe_input);
            } else { // If not a pipe
                dup2(fd, 1); // Redirect stdout to the file
                close(fd); // Close the original file descriptor
            }
        }

        // Handle input redirection
        if (input_flag) { //< If input_flag is set
            dup2(fd, 0); // Redirect standard input to the file
            close(fd); // Close the original file descriptor
            // If the next separator is a pipe, redirect output to the pipe
            if ((*pos_separators) && (*pos_separators)[j] != -1 && 
                strcmp(arr_commands[(*pos_separators)[j]], "|") == 0) {
       
                close(pipe_fd[0]); // Close the read end of the pipe (not needed in child process)
                dup2(pipe_fd[1], 1); // Redirect standard output to the write end of the pipe
                close(pipe_fd[1]); // Close the write end of the pipe
            }
        }


        // Execute the command
        execvp(token[0], token); // Execute the command
        perror(token[0]); // Print error if execvp fails
        exit(EXIT_FAILURE); // Exit child process with failure
    }

    // Parent process continues here
    // If previous separator was a pipe, close pipe ends in the parent
    if ((*pos_separators) && 
        ((*pos_separators)[j] != -1) && 
        (strcmp(arr_commands[(*pos_separators)[j]], "|") == 0)) {

        close(pipe_fd[1]); // Close the writing end in the parent
        *pipe_input = pipe_fd[0]; // Optionally pass the reading end for the next command
    } else if(*pipe_input > 2) {
        close(*pipe_input);
    }

    // Wait for the child process if not running in background
    if ((!is_ampersand) && 
        ((!(*pos_separators)) || 
         ((*pos_separators) && 
          (strcmp(arr_commands[(*pos_separators)[j-2]], "|") != 0)))) {
        if(input_flag || output_flag) {
            close(fd); // Close the original file descriptor
        }
        do {
            p = wait(NULL); // Wait for any child process to finish
        } while (p != pid); // Continue waiting until the specific child finishes
    } else if(is_ampersand){ // If running in background
        printf("Process running in background with PID %d\n", pid);
    }
    return;
}

// Function to execute the parsed command
void exec_command(char **arr_commands, int count, int *size, int **pos_separators)
{
    int i, fd, j = 0, n = 0, amount = 0, vol = 0, k, pipe_fd[2], pipe_input = 0, u = 0;
    char **token = NULL;
    bool input_flag, output_flag, is_ampersand, pipe_flag;

    // Initialize flags
    input_flag = output_flag = is_ampersand = pipe_flag = false;

    // Handle 'cd' command separately
    if (strcmp(arr_commands[0], "cd") == 0) {
        change_dir(arr_commands);
        return;
    }

    // Iterate through all commands
    while (amount < count) {
        if ((pos_separators) && (*pos_separators)) { // If there are separators
            vol = size_of_token(count, amount, pos_separators, j, size); // Determine the size of the current token
            if(vol > 0) {
                token = malloc((vol+1) * sizeof(char*)); // Allocate memory for tokens
                for(i = 0; i <= vol; i++) {
                    token[i] = NULL;
                }
            }
            u = j;
            while(((*pos_separators)[u] != -1) && ((strcmp(arr_commands[(*pos_separators)[u]], ">") == 0) || (strcmp(arr_commands[(*pos_separators)[u]], "<") == 0) || (strcmp(arr_commands[(*pos_separators)[u]], "|") == 0))) {
                if(((*pos_separators)[u+1] != -1) && (strcmp(arr_commands[(*pos_separators)[u+1]], "&") == 0)) {
                    is_ampersand = true;
                    break;
                }
                u++;
            }
            // Iterate through tokens and add them
            for (i = amount; i < count; i++) {
                add_tokens(&token, i, pos_separators, &n, j, arr_commands); // Add tokens to the current command
                if (search_separators(&amount, i, &j, pos_separators, arr_commands, &is_ampersand, &input_flag, &output_flag, &fd, &pipe_flag, size)) {
                    break; // If a separator is found, stop adding tokens
                }
                amount++; // Move to the next token
            }

            // If there was an error opening a file, print error and exit
            if (fd == -1) {
                perror("open");
                return;
            }
        } else { // If there are no separators
            if (!token) {
                token = malloc((count + 1) * sizeof(char*)); // Allocate memory for all tokens
            }
            // Duplicate all commands into tokens
            for (k = 0; k < count; k++) {
                if (((k + amount) < count) && 
                    (((j > 0) && (k != ((*pos_separators)[j-1]))) || (true))) {
                    token[k] = strdup(arr_commands[k]); // Duplicate the command
                } else {
                    break; // Stop if out of bounds
                }
            }
            amount = amount + k; // Update the amount processed
            token[k] = NULL; // Null-terminate the token array
        }

        // If not a pipe, execute the single command
        if ((!pipe_flag) && (token)) {
            execute_single_command(token, input_flag, output_flag, fd, is_ampersand, pipe_fd, &pipe_input, j, pos_separators, arr_commands);
            amount++; // Move to the next command
            input_flag = output_flag = false; // Reset flags
        } else if((pipe_flag) && (token)){ // If it's a pipe, handle pipeline
            pipeline(pipe_fd, token, &pipe_input, arr_commands, pos_separators, j, is_ampersand, amount);
            // Handle specific cases where the pipe should be closed
            if((((*pos_separators)[j] == -1) && 
                ((amount-1) > ((*pos_separators)[j-1]))) || 
               (((*pos_separators)[j] == -1) && 
                (strcmp(arr_commands[(*pos_separators)[j-1]], "|") != 0 && 
                 !output_flag && !input_flag))) {
                close(pipe_input); // Close the reading end of the pipe
                amount++; // Move to the next command
            }
        }

        // Reset pipe and ampersand flags
        pipe_flag = is_ampersand = false;

        // Free allocated memory for tokens
        if(token) {
            if(vol > 0) { // If tokens were allocated based on 'vol'
                for(i = 0; i <= vol; i++) {
                    free(token[i]); // Free each token string
                }
                free(token); // Free the token array
                token = NULL; // Reset token pointer
            } else { // If tokens were allocated for all commands
                for(i = 0; i <= count; i++) {
                    free(token[i]); // Free each token string
                    token[i] = NULL;
                }
                free(token); // Free the token array
                token = NULL; // Reset token pointer
            }
        }

    }
    return;
}

