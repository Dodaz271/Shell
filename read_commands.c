#include "read_commands.h"

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
	/*if(str[i] == '\0') {
	    break;
	}*/
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
    newNode->word = strdup(str);
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

char *read_commands(char *str, char c, int *n, int *arr_size, bool *flag, int *count, struct word_item **commands)
{
    if((str != NULL) && (*flag == false) && ((c == '\n') || (c == ' '))) {
        add_node(commands, str);
	*n = 0;
	*arr_size = 2;
	(*count)++;
	free(str);
	return NULL; //str = NULL
    }
    if(str == NULL) {
        str = (char*)malloc(2 * sizeof(char));
    }
    if((*arr_size - *n) == 1) {
	*arr_size *= 2;
        str = newArray(str, *n, *arr_size);
    }
    str[*n] = c;
    str[*n + 1] = '\0';
    (*n)++;
    return str;
}
