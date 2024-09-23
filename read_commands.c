#include "read_commands.h"

void add_element(int **pos_separators, int *size, int new_value) {
    int *temp = (int*)malloc(((*size) + 2) * sizeof(int));
    int i;
    if (temp == NULL) {
        printf("Memory reallocation failed\n");
        free(pos_separators);
        exit(1);
    }
    if((*size > 0) && (new_value == (*pos_separators)[*size-1])) {
	    free(temp);
    	return;
    }
    for(i = 0; i < (*size); i++) {
	    if(new_value == (*pos_separators)[i]) {
	        free(temp);
    	    return;
	    }
        temp[i] = (*pos_separators)[i];
    }
    if(*size > 0) {
        free(*pos_separators);
    }
    temp[*size] = new_value;
    (*size)++;
    temp[*size] = -1;
    *pos_separators = temp;
    return;
}

bool separators(char *str, int n, bool flag, int **pos_separators, int *count, int *size, int pos_next_word)
{
    const char *separators[] = {"&", "&&", ">", "<", ">>", "|", "||", ";", "(", ")"};
    if((str[n] == '\n') || (str[n] == ' ') || (str[n+1] == '\0')) {
	    if((flag == false) && (str != NULL)) {
	        for(int i = 0; i < 10; i++) {
                if(strncmp(separators[i], str + n - 1, pos_next_word - n + 1) == 0) {
                    add_element(pos_separators, size, (*count));
                }
	        }
	    }
        if((str[n-1] != ' ') || ((str[n] != ' ') && (str[n+1] == '\0'))) {
            (*count)++;
        }
        return true;
    }
    return false;
}


char *newArray(char *str, int n, int arr_size)
{
    char *newArray = (char*)malloc(arr_size * sizeof(char));
    int i;
    for(i = 0; i < n; i++) {
        newArray[i] = str[i];
    }
    free(str);
    return newArray;
}

int str_capacity(char *str)
{
    int i = 0;
    while(str[i] != '\0') {
        i++;
    }
    return i;
}

char **inc_command_array(char *str, int pos_prev_word, int pos_next_word, char ***arr_commands, int count) {
    // Allocate memory for the new array (count + 1 for the new command and NULL terminator)
    char **newArray = malloc((count + 1) * sizeof(char*));
    if (newArray == NULL) {
        // Return NULL if allocation fails
        return NULL;
    }

    int i = 0, j = 0, n = pos_prev_word;

    // Skip leading space if present
    if (str[n] == ' ') {
        n++;
    }

    // Copy existing commands into the new array
    while ((arr_commands != NULL) && (i < count-1)) {
        newArray[i] = (*arr_commands)[i];
        i++;
    }

    // Allocate memory for the new command and copy it from the string
    newArray[i] = malloc((pos_next_word - pos_prev_word + 1) * sizeof(char));
    if (newArray[i] == NULL) {
        // If allocation fails, free previously allocated memory
        for (int k = 0; k < i; k++) {
            free(newArray[k]);
        }
        free(newArray);
        return NULL;
    }

    // Copy the string into the new command
    for (j = 0; j < (pos_next_word - (pos_prev_word == 0 ? pos_prev_word : pos_prev_word+1)); j++) {
        newArray[i][j] = str[n];
        n++;
    }
    newArray[i][j] = '\0';  // Null-terminate the new command

    // Mark the end of the array with NULL
    newArray[count] = NULL;

    // Free the old array memory (not the strings themselves, as they are copied)
    free(*arr_commands);
    
    return newArray;
}


char *read_commands(char *str, bool *flag, int *count, int **pos_separators, int *size, char ***arr_commands)
{
    int pos_next_word = 0, n = 0, pos_prev_word = -1;
    int max_capacity = 0, count_quotes = 0;
    bool is_quotes = false;
    max_capacity = str_capacity(str); 
    while(str[n] != '\0') {
        if((((str != NULL) && (n > 0) && (str[n-1] != '\\')) || (str == NULL)) && (str[n] == '"')) {
            *flag = !(*flag);
            is_quotes = *flag;
            memmove(&str[n], &str[n+1], max_capacity - n);
        }
        if (n > pos_next_word) {
            pos_prev_word = pos_next_word;
            do {
                pos_next_word++;
                if(str[pos_next_word] == '"' && str[pos_next_word-1] != '\\') {
                    is_quotes = !is_quotes;
                    count_quotes++;
                }
            } while((str[pos_next_word] != '\0') &&
                   ((is_quotes && str[pos_next_word] != '\0') ||  // Inside quotes, move forward
                    (!is_quotes && (str[pos_next_word] != ' ' || (str[pos_next_word - 1] == ' ')))  // Outside quotes
                   ));
            pos_next_word = pos_next_word - count_quotes;
            count_quotes = 0;
        }
        if(str[n-1] == '\\') {
            memmove(&str[n-1], &str[n], max_capacity - n);
            pos_next_word--;
        }
        if(*flag == false) {
            if(separators(str, n, *flag, pos_separators, count, size, pos_next_word)) {
                (*arr_commands) = inc_command_array(str, pos_prev_word, pos_next_word, arr_commands, *count);
            }
        }
        n++;
    }
    //(*count)++;
    return str;
}
