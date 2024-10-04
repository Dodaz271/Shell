#include "read_commands.h"

// Function to check if a single character is a separator
bool is_single_char_separator(char c) {
    const char char_separators[] = {'&', '>', '<', '|', ';', '(', ')'};
    int num_char_separators = sizeof(char_separators) / sizeof(char_separators[0]);
    for(int i = 0; i < num_char_separators; i++) {
        if(char_separators[i] == c) {
            return true;
        }
    }
    return false;
}

// Function to add a new element to pos_separators
void add_element(int **pos_separators, int *size, int new_value) {
    int *temp = (int*)malloc(((*size) + 2) * sizeof(int)); // +1 for new element, +1 for sentinel
    if (temp == NULL) {
        printf("Memory allocation failed\n");
        free(*pos_separators); // Correctly free the original array
        exit(1);
    }

    // Copy existing elements
    for(int i = 0; i < *size; i++) {
        temp[i] = (*pos_separators)[i];
    }

    // Add the new element
    temp[*size] = new_value;
    (*size)++;

    // Add sentinel value
    temp[*size] = -1;

    // Free the old array if it exists
    if(*size > 1) { // Only free if there was a previous allocation
        free(*pos_separators);
    }

    *pos_separators = temp;
}

// Function to check and handle separators at current position
bool is_separator_at_pos(char *str, int pos, int *matched_length) {
    // Define all separators, including multi-character ones
    const char *separators_list[] = {"&&", "||", ">>", "&", ">", "<", "|", ";", "(", ")"};
    const int num_separators = sizeof(separators_list) / sizeof(separators_list[0]);
    for(int i = 0; i < num_separators; i++) {
        int sep_len = strlen(separators_list[i]);
        if(strncmp(separators_list[i], str + pos, sep_len) == 0) {
            *matched_length = sep_len;
            return true;
        }
    }
    return false;
}

// Function to add commands and separators to arr_commands
char **inc_command_array(char *str, int pos_prev_word, int pos_next_word, char ***arr_commands, int count, bool is_separator) {
    // Allocate memory for the new array (count + 2 for the new command and NULL terminator)
    char **newArray = realloc(*arr_commands, (count + 2) * sizeof(char*));
    if (newArray == NULL) {
        perror("Realloc failed");
        exit(EXIT_FAILURE);
    }
    *arr_commands = newArray;

    // Allocate memory for the new command
    int length = pos_next_word - pos_prev_word;
    (*arr_commands)[count] = malloc((length + 1) * sizeof(char));
    if((*arr_commands)[count] == NULL) {
        perror("Malloc failed");
        exit(EXIT_FAILURE);
    }

    // Copy the substring
    strncpy((*arr_commands)[count], str + pos_prev_word, length);
    (*arr_commands)[count][length] = '\0'; // Null-terminate

    // Optionally, trim leading spaces if not a separator
    if(!is_separator) {
        // Trim leading spaces
        int start = 0;
        while((*arr_commands)[count][start] == ' ') start++;
        if(start > 0) {
            memmove((*arr_commands)[count], (*arr_commands)[count] + start, length - start + 1);
        }
    }

    // Null-terminate the array
    (*arr_commands)[count + 1] = NULL;

    return newArray;
}

// Function to parse commands from the input string
char *read_commands(char *str, bool *flag, int *count, int **pos_separators, int *size, char ***arr_commands) {
    int pos_next_word = 0, n = 0, pos_prev_word = 0;
    int max_capacity = strlen(str);
    bool is_quotes = false;

    while(str[n] != '\0') {
        // Handle quotes
        if(str[n] == '"' && (n == 0 || str[n-1] != '\\')) {
            is_quotes = !is_quotes;
            // Remove the quote from the string
            memmove(&str[n], &str[n+1], max_capacity - n);
            max_capacity--;
            continue;
        }

        // If not inside quotes, check for separators
        if(!is_quotes) {
            int matched_length = 0;
            if(is_separator_at_pos(str, n, &matched_length)) {
                // Add the preceding command if any
                if(n > pos_prev_word) {
                    *arr_commands = inc_command_array(str, pos_prev_word, n, arr_commands, *count, false);
                    (*count)++;
                }

                // Add the separator
                *arr_commands = inc_command_array(str, n, n + matched_length, arr_commands, *count, true);
                (*count)++;

                // Record the separator position
                add_element(pos_separators, size, *count - 1); // Assuming *count -1 is the separator's index

                // Move the position forward
                n += matched_length;
                pos_prev_word = n;
                continue;
            }

            // Handle spaces
            if(str[n] == ' ') {
                // Add the preceding command if any
                if(n > pos_prev_word) {
                    *arr_commands = inc_command_array(str, pos_prev_word, n, arr_commands, *count, false);
                    (*count)++;
                }
                pos_prev_word = n + 1;
                n++;
                continue;
            }
        }

        // Handle escaped characters (e.g., \ )
        if(str[n] == '\\' && str[n+1] != '\0') {
            // Remove the escape character
            memmove(&str[n], &str[n+1], max_capacity - n);
            max_capacity--;
            // Optionally, handle the escaped character
            // For simplicity, we skip this in the current implementation
        } else {
            n++;
        }
    }

    // Add the last command if any
    if(n > pos_prev_word) {
        *arr_commands = inc_command_array(str, pos_prev_word, n, arr_commands, *count, false);
        (*count)++;
    }

    return str;
}

// Helper function to calculate the length of the string
int str_capacity(char *str)
{
    int i = 0;
    while(str[i] != '\0') {
        i++;
    }
    return i;
}
