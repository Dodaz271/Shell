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

bool separators(char c, char *str, int *n, bool flag, int **pos_separators, int *count, int *size)
{
    const char *separators[] = {"&", "&&", ">", "<", ">>", "|", "||", ";", "(", ")"};
    if((c == '\n') || (c == ' ') || (c == '\t')) {
	if((!flag) && (str != NULL)) {
	    for (int i = 0; i < 10; i++) {
                if (strcmp(str, separators[i]) == 0) {
                    add_element(pos_separators, size, (*count));
                }
	    }
	}
        return true;
    }
    if((str != NULL) && (!flag)) {
	if(((strcmp(str, "&") == 0) && (c == '&')) || ((strcmp(str, ">") == 0) && (c == '>')) || ((strcmp(str, "|") == 0) && (c == '|'))) {
	    if(*n < 1) {
	        (*n)++;
	    }
	    return false;
	}
        for (int i = 0; i < 10; i++) {
            if (strcmp(str, separators[i]) == 0) {
		add_element(pos_separators, size, (*count));
                return true;
            }
        }
        if((c == '&') || (c == '>') || (c == '<') || (c == '|') || (c == ';') || (c == '(') || (c == ')')) {
            return true;
        }
    }
    return false;
}

void delete_commands(struct word_item *commands)
{
    struct word_item *current = commands; 
    struct word_item *next;
    while(current != NULL) {
        next = current->next;
	free(current->word);
	free(current);
	current = next;
    }
    return;
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

void add_node(struct word_item **first, const char *str) {
    // Allocate memory for the new node
    struct word_item *newNode = malloc(sizeof(struct word_item));
    if (!newNode) {
        perror("malloc");
        return;
    }
    newNode->word = str ? strdup(str) : NULL;
    newNode->next = NULL;

    // If the list is empty, set the new node as the first node
    if (!*first) {
        *first = newNode;
    } else {
        // Otherwise, append the new node to the end of the list
        struct word_item *last = *first;
        while (last->next) {
            last = last->next;
        }
        last->next = newNode;
    }
}


void print_commands(struct word_item **commands)
{
    struct word_item *tmp = *commands;
    while(tmp) {
        printf("[%s]\n", tmp->word);
	tmp = tmp->next;
    }
    return;

}

char *read_commands(char *str, char c, int *n, int *arr_size, bool *flag, int *count, struct word_item **commands, int **pos_separators, int *size)
{
    if((((str != NULL) && (str[*n-1] != '\\')) || (str == NULL)) && (c == '"')) {
        *flag = !(*flag);
	if((str == NULL) && (*flag == false)) {
	    add_node(commands, NULL);
	    free(str);
	    str = NULL;
	}
	if((str != NULL) && (separators(c, str, n, *flag, pos_separators, count, size)) && (c == '"')) {
	    if((*arr_size - *n) == 1) {
                *arr_size *= 2;
                str = newArray(str, *n, *arr_size);
            }
	    str[*n] = '"';
	    str[*n+1] = '\0';
	}
	return str;
    } 
    if((*flag == false) && (separators(c, str, n, *flag, pos_separators, count, size))) {
	if(str != NULL) {
	    if((*n > 1) && (str[*n-2] == '\\') && (str[*n-1] == '\\')) {
                str[*n-1] = '\0';
	    }
	    if((str != NULL) && (separators(c, str, n, *flag, pos_separators, count, size)) && ((str[1] == '\"') || (str[2] == '\"'))) {  // if str == separator\" (&\") or str = two separators\" (&&\") 
	        str[*n] = '\0';
	    }
            add_node(commands, str);
	    (*count)++;
	}
	*n = 0;
	*arr_size = 2;
	free(str);
	str = NULL;
	if(((c != ' ') && (c != '\n') && (c != '\t') && (separators(c, str, n, *flag, pos_separators, count, size))) || (!separators(c, str, n, *flag, pos_separators, count, size))) {
	    str = (char*)malloc(2 * sizeof(char));
	    str[0] = c;
	    str[1] = '\0';
	    (*n)++;
	}
	return str;
    }
    if(c != '\n') {
        if(str == NULL) {
            str = (char*)malloc(2 * sizeof(char));
        } else {
	    if((*arr_size - *n) == 1) {
	        *arr_size *= 2;
                str = newArray(str, *n, *arr_size);
	    }
            if((str[*n-1] == '\\') && (c != '\\')) {
	        (*n)--;
	    }
	    if((*n > 1) && (str[*n-2] == '\\') && (str[*n-1] == '\\')) {
	        (*n)--;
	    }
	}
        str[*n] = c;
	(*n)++;
        str[*n] = '\0';
    }
    return str;
}
