#include "read_commands.h"

bool separators(char c, char *str, int *n, bool *is_ampersand, bool flag, bool *i_o_redirection)
{
    if((c == '\n') || (c == ' ') || (c == '\t')) {
	if((!flag) && (str != NULL) && (strcmp(str, "&") == 0)) {
            *is_ampersand = true;
        }
	if((!flag) && (str != NULL) && ((strcmp(str, ">") == 0) || (strcmp(str, ">>") == 0) || (strcmp(str, "<") == 0))) {
	    *i_o_redirection = true;
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
	const char *separators[] = {"&", "&&", ">", "<", ">>", "|", "||", ";", "(", ")"};
        for (int i = 0; i < 10; i++) {
            if (strcmp(str, separators[i]) == 0) {
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

void add_node(struct word_item **first, const char *str)
{
    struct word_item *newNode = (struct word_item*)malloc(sizeof(struct word_item));
    if(newNode == NULL) {
	perror("malloc");
        return;
    }
    struct word_item *last;
    if(str != NULL) {
        newNode->word = strdup(str);
    } else {
        newNode->word = NULL;
    }
    newNode->next = NULL;
    if(*first == NULL) {
        *first = newNode;
    } else {
        last = *first;
        while(last->next != NULL) {
	    last = last->next;
	}
	last->next = newNode;
    }
    return;
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

char *read_commands(char *str, char c, int *n, int *arr_size, bool *flag, int *count, struct word_item **commands, bool *is_ampersand, bool *i_o_redirection)
{
    if((((str != NULL) && (str[*n-1] != '\\')) || (str == NULL)) && (c == '"')) {
        *flag = !(*flag);
	if((str == NULL) && (*flag == false)) {
	    add_node(commands, NULL);
	    free(str);
	    str = NULL;
	}
	if((str != NULL) && (separators(c, str, n, is_ampersand, *flag, i_o_redirection)/*strcmp(str, "&") == 0*/) && (c == '"')) {
	    if((*arr_size - *n) == 1) {
                *arr_size *= 2;
                str = newArray(str, *n, *arr_size);
            }
	    str[*n] = '"';
	    str[*n+1] = '\0';
	}
	return str;
    } 
    if((*flag == false) && (separators(c, str, n, is_ampersand, *flag, i_o_redirection))) {
	if(str != NULL) {
	    if((*n > 1) && (str[*n-2] == '\\') && (str[*n-1] == '\\')) {
                str[*n-1] = '\0';
	    }
	    if((str != NULL) && (separators(c, str, n, is_ampersand, *flag, i_o_redirection)) && ((str[1] == '\"') || (str[2] == '\"'))) {
	        str[*n] = '\0';
	    }
            add_node(commands, str);
	    (*count)++;
	}
	*n = 0;
	*arr_size = 2;
	free(str);
	str = NULL;
	if(((c != ' ') && (c != '\n') && (c != '\t') && (separators(c, str, n, is_ampersand, *flag, i_o_redirection))) || (!separators(c, str, n, is_ampersand, *flag, i_o_redirection))) {
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
        }
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
        str[*n] = c;
        str[*n + 1] = '\0';
        (*n)++;
    }
    return str;
}
