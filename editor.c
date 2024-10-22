#include "editor.h"

enum esc_subsequence { arrow_up, arrow_down, arrow_right, arrow_left, delete_key, unknown };

struct termios origin_termios;
const char message[] = "\nGood bye\n";
void handler(int s)
{
    int save_errno = errno;
    signal(SIGINT, handler);
    write(1, message, sizeof(message)-1);
    errno = save_errno;
    tcsetattr(0, TCSANOW, &origin_termios);
    exit(0);
}


void enable_canon_mode(struct termios *origin_termios)
{
    struct termios ts;
    tcgetattr(0, origin_termios);
    ts = *origin_termios;
    ts.c_lflag &= ~(ICANON | ECHO);
    ts.c_lflag |= ISIG;
    tcsetattr(0, TCSANOW, &ts);
    return;
}

void newPath(char **path, struct dirent *dent, int path_len)
{
    char *new_path = realloc(*path, (strlen(dent->d_name) + path_len + 1) * sizeof(char));
    if (new_path == NULL) {
        // Handle the memory allocation failure
        free(*path);
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    } else {
        *path = new_path;
    }

    // Copy the new string to the allocated memory
    memcpy(*path + path_len, dent->d_name, strlen(dent->d_name));

    // Null-terminate the string
    (*path)[path_len + strlen(dent->d_name)] = '\0';
    return;
}

void update_display(const char *command, const char *buf, int *curr_pos, int *max_pos) {
    // Get the terminal size
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int terminal_width = w.ws_col;  // Width of the terminal in columns
    int n = (*max_pos) / terminal_width; 
    int i = 0;

    // Clear the line from the cursor to the end of the line
    printf("\033[K");

    // If the output spans multiple lines, clear them
    if(n > 0) {
        for(i = n; i > 0; i--) {
            printf("\033[K\033[A"); // Move cursor up and clear the line
        }
    }

    // Output the string with a NULL check
    if (command != NULL) {
        printf("\r> %s%s", command, buf);
    } else {
        printf("\r> %s", buf);
    }
    if((*max_pos) > (n * terminal_width)) {

    }
    // Adjust cursor position relative to terminal width
    printf("\033[%dG", (*curr_pos) - (n * terminal_width) + 3);
    fflush(stdout);
    return;
}

char *incCommand(char **command, char *buf, int *command_size, int *buf_size, bool is_space)
{
    int new_size = (*command_size) + (*buf_size) + (is_space ? 2 : 1);  // Space and null terminator if is_space is true
    char *newArray = (char*)malloc(new_size * sizeof(char));
    if (newArray == NULL) {
        free(*command);
        return NULL;
    }
    if((command) && (*command_size > 0)) {
        memcpy(newArray, *command, (*command_size));
    }
    if ((buf) && (*buf_size > 0)) {
        memcpy(newArray + (*command_size), buf, *buf_size);
	    (*command_size) += *buf_size;
	    if(is_space) {
            memset(buf, 0, *buf_size);
            *buf_size = 0;
            newArray[*command_size] = ' ';
	        (*command_size)++;
	    }
    }
    free(*command);
    newArray[*command_size] = '\0';
    return newArray;
}

char *incStr(char *str, int str_size)
{
    char *newArray = realloc(str, str_size * sizeof(char));
    if (newArray == NULL) {
        free(str);
        return NULL;
    }
    return newArray;
}

char *incBuf(char **buf, int *buf_size, int additional_len)
{
    int new_size = (*buf_size) + additional_len + 1;
    int i;
    char *newArray = malloc(new_size * sizeof(char));
    if (newArray == NULL) {
        return NULL;
    }
    for(i = 0; i < *buf_size; i++) {
        newArray[i] = (*buf)[i];
    }
    newArray[*buf_size] = '\0';
    if (*buf != NULL) {
        free(*buf);
        *buf = NULL;  // Set to NULL to avoid double-free
    }
    return newArray;
}

void handle_delete(int *curr_pos, int *max_pos, char **buf, char **command, int *n, int *len)
{
    if ((*curr_pos) >= 0 && (*curr_pos) < (*max_pos)) {
        if ((*curr_pos) < (*n)) {
            memmove(&(*command)[*curr_pos], &(*command)[*curr_pos + 1], (*n) - (*curr_pos));
            (*n)--;
        } else {
            int buf_index = (*curr_pos) - (*n);
            memmove(&(*buf)[buf_index], &(*buf)[buf_index + 1], (*max_pos) - (*curr_pos));
	    (*len)--;
        }
        (*max_pos)--;

        // Updating the display in the terminal
        update_display(*command, *buf, curr_pos, max_pos);
        fflush(stdout);
    }
    return;
}

void handleCtrlW(char **command, char **buf, int *n, int *len, int *curr_pos, int *max_pos)
{
    // Determining the position of the beginning of the previous word
    int start = *curr_pos - 1;
    while (start >= 0 && ((*curr_pos) > (*n) ? (*buf)[start - (*n)] == ' ' : (*command)[start] == ' ')) {
        start--;
    }
    while (start >= 0 && ((*curr_pos) > (*n) ? start - (*n) >= 0 : (*command)[start] != ' ')) {
        start--;
    }
    int num_chars_to_delete = (*curr_pos) - start - 1;

    for (int i = 0; i < num_chars_to_delete; i++) {
        if ((*curr_pos) > (*n)) {
            memmove(&(*buf)[start - (*n) + 1], &(*buf)[start - (*n) + 2], (*len) - (start - (*n) + 1));
            (*len)--;
        } else {
            memmove(&(*command)[start + 1], &(*command)[start + 2], (*n) - (start + 1));
            (*n)--;
        }
        (*curr_pos)--;
        (*max_pos)--;
    }

    // Moving the cursor
    printf("\033[%dG", (*curr_pos) + 3); // Moving the cursor
    printf("\033[K"); // Clear from cursor to end of line
    printf("\r> %s%s", *command, *buf);
    printf("\033[%dG", (*curr_pos) + 3); // Moving the cursor
    fflush(stdout);
    return;
}

void add_char_and_refresh(int *curr_pos, int *max_pos, char **buf, char **command, int *n, char c, int *len, bool is_slash)
{
    if (*curr_pos < *n) {
        *command = incCommand(command, NULL, n, &(int){1}, false);
        memmove(&(*command)[*curr_pos + 1], &(*command)[*curr_pos], (*n) - (*curr_pos));
        (*command)[*curr_pos] = c;
        (*n)++;
    	(*command)[*n] = '\0';
    } else {
        int buf_index = (*curr_pos) - (*n);
        memmove(&(*buf)[buf_index + 1], &(*buf)[buf_index], (*len) - buf_index);
        (*buf)[buf_index] = c;
        (*len)++;
	    (*buf)[(*len)] = '\0';
    }
    (*curr_pos)++;
    (*max_pos)++;

    // Updating the display in the terminal
    update_display(*command, *buf, curr_pos, max_pos);
    /*printf("\033[K"); // Clear from cursor to end of line
    if((command != NULL) && ((*command) != NULL)) {
        printf("\r> %s%s", *command, *buf);
    } else {
        printf("\r> %s", *buf);
    }
    printf("\033[%dG", (*curr_pos) + 3); // Moving the cursor
    fflush(stdout);*/
    return;
}

void remove_and_refresh(int *curr_pos, int *max_pos, char **buf, char **command, int *n, int *len)
{
    if ((*curr_pos) > 0) {
        if ((*curr_pos) <= (*n)) {
            memmove(&(*command)[*curr_pos - 1], &(*command)[*curr_pos], (*n) - (*curr_pos) + 1);
            (*n)--;
	        (*command)[*n] = '\0';
        } else {
            int buf_index = (*curr_pos) - (*n) - 1;
            memmove(&(*buf)[buf_index], &(*buf)[buf_index + 1], (*len) - buf_index);
            (*len)--;
	        (*buf)[(*len)] = '\0';
        }
        (*curr_pos)--;
        (*max_pos)--;

        // Updating the display in the terminal
    	printf("\033[%dG", (*curr_pos) + 3);
        update_display(*command, *buf, curr_pos, max_pos);
        fflush(stdout);
    }
    return;
}

enum esc_subsequence get_direction(const char* buf)
{
    if (strcmp(buf, "\033[A") == 0) {
        return arrow_up;
    } else if (strcmp(buf, "\033[B") == 0) {
        return arrow_down;
    } else if (strcmp(buf, "\033[C") == 0) {
        return arrow_right;
    } else if (strcmp(buf, "\033[D") == 0) {
        return arrow_left;
    } else if (strcmp(buf, "\033[3~") == 0) {
        return delete_key;
    } else {
        return unknown;
    }
}

int last_slash_buf(char **buf, int *buf_len, int *pos_space, int *curr_pos, int *prev_pos_space)
{
    int last_pos_slash = -1;
    int i = 0;
    if((buf) && (*buf) && (*buf)[i] != '\0') {
        if(*curr_pos < *buf_len) {
            i = *curr_pos;
            i--;
            while(((*buf)[i-1] != ' ') && (i > 0/*1*/)) {
                i--;
            }
        }
        *prev_pos_space = i-1;
    }
    while((buf) && (*buf) && (*buf)[i] != '\0' && i < *curr_pos) {
        if((*buf)[i] == '/') {
	        last_pos_slash = i;
	    }
	    if((*buf)[i] == ' ') {
	        *pos_space = i;
	        break;
	    }
	    i++;
    }
    return last_pos_slash;
}

bool matchExists(char **matches, int match_count, const char *new_match) {
    for (int i = 0; i < match_count; i++) {
        if (strcmp(matches[i], new_match) == 0) {
            return true;
        }
    }
    return false;
}

char *handleTabCompletion(char **buf, int *len, char *command, int *n, int *max_pos, int *curr_pos, int *tab_press_count)
{
    int last_pos_slash = 0, path_len, pos_space = -1, additional_len, prev_pos_space = 0;
    struct stat file_stat;
    char *path = NULL;
    DIR *dir;
    struct dirent *dent;

    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    int terminal_width = w.ws_col;  // Width of the terminal in columns
    int max_name_len = 0, name_len = 0;

    if((*curr_pos) >= *n) {
        last_pos_slash = last_slash_buf(buf, len, &pos_space, curr_pos, &prev_pos_space);
    } else {
        last_pos_slash = last_slash_buf(&command, n, &pos_space, curr_pos, &prev_pos_space);
    }
    if(last_pos_slash == -1) {
        if(*n > 0) {
            path = strdup("./");
	        path_len = 2;
        }
        if((*n > 1) && (command[0] == 'c') && (command[1] == 'd')) {
            last_pos_slash = -2;
        }
    } else {
        path = malloc((last_pos_slash + 2) * sizeof(char));
    	if((*curr_pos) > *n) {
    	    memcpy(path, *buf, last_pos_slash+1);
            path_len = last_pos_slash+1;
        } else {
    	    memcpy(path, command + prev_pos_space + 1, pos_space - prev_pos_space - 2);
            path_len = pos_space - prev_pos_space - 2;
        }
        path[path_len] = '\0';
    }
    if(path != NULL) {
        dir = opendir(path);
    }
    if (!dir) {
        perror(path);
        return command;
    }
    int i, j;
    size_t buf_len = *len - 1;
    char **matches = NULL;
    size_t match_count = 0;
    size_t match_capacity = 0;
    
    if((*n == 0) && (last_pos_slash == -1)) {
        char *path_env = getenv("PATH");
        char *path_copy = strdup(path_env);
        char *token = strtok(path_copy, ":");

        while (token != NULL) {
            dir = opendir(token);
            if (dir) {
                while ((dent = readdir(dir)) != NULL) {
                    if (dent->d_type == DT_REG || dent->d_type == DT_LNK) {
                        if (!(matchExists(matches, match_count, dent->d_name))) {
                            if (strncmp(dent->d_name, *buf, buf_len + 1) == 0) {
                                if (match_count == match_capacity) {
                                    match_capacity = (match_capacity == 0) ? 10 : match_capacity * 2;
                                    matches = realloc(matches, match_capacity * sizeof(char *));
                                }
                                matches[match_count] = strdup(dent->d_name);
                                match_count++;
                            }
                        }
                    }
                }
                closedir(dir);
            }
            token = strtok(NULL, ":");
        }
        free(path_copy);
    } else {
        // Read the directory and look for matches
        while ((dent = readdir(dir)) != NULL) {
            // Skip the current and parent directory entries ("." and "..")
            if ((strcmp(dent->d_name, ".") == 0) || (strcmp(dent->d_name, "..") == 0)) {
                continue;
            }

            // Allocate memory for the full path by appending the directory entry name
	        newPath(&path, dent, path_len);

            // Use stat() to get file status; return command if stat fails
            if ((stat(path, &file_stat)) != 0) {
                for (i = 0; i < match_count; i++) {
                    free(matches[i]);
                }
                free(matches);
                if(last_pos_slash != -1) {
                    free(path);
                }
                closedir(dir);
                return command;
            }
    
            // Check if the directory entry matches the buffer and is a directory or executable
            if (((*buf)[0] == '\0') || ((((*curr_pos > *n) && (strncmp(dent->d_name, *buf + ((last_pos_slash == -2) ? 0 : (last_pos_slash + 1)), buf_len - ((last_pos_slash == -2) ? -1 : (last_pos_slash))) == 0))                                || ((*curr_pos < *n) && (strncmp(dent->d_name, command + last_pos_slash + 1, pos_space - last_pos_slash - 1) == 0)))
                && (((last_pos_slash != -1) && (S_ISDIR(file_stat.st_mode) || (file_stat.st_mode & S_IXUSR))) 
                || ((last_pos_slash == -1) && (!S_ISDIR(file_stat.st_mode)))))) {
            
                // Reallocate the matches array if capacity is exceeded
                if (match_count == match_capacity) {
                    match_capacity = (match_capacity == 0) ? 10 : match_capacity * 2;
                    matches = realloc(matches, match_capacity * sizeof(char *));
                }

                // Duplicate the directory entry name into the matches array
                matches[match_count] = strdup(dent->d_name);
                if (matches[match_count] != NULL) {
                    size_t len = strlen(matches[match_count]);

                    // Reallocate the memory to add '/' for directories
                    matches[match_count] = realloc(matches[match_count], len + 2); // +2 for '/' and '\0'
                    if (matches[match_count] == NULL) {
                        perror("Memory reallocation failed");
                        exit(EXIT_FAILURE);
                    }
    
                    // If it's a directory, append '/' to the entry name
                    if (S_ISDIR(file_stat.st_mode)) {
                        matches[match_count][len] = '/';
                        matches[match_count][len + 1] = '\0';
		            }
                    name_len = strlen(matches[match_count]);
                    if(name_len > max_name_len) {
                        max_name_len = name_len;
                    }
                }
                match_count++;
            }
        }
        // Close the directory stream
        closedir(dir);
    }
    if (match_count == 0) {
        // No match
        return command;
    } else if (match_count == 1) {
        // Only one match — complete the line completely
        size_t match_len = strlen(matches[0]);
	    if(*curr_pos > *n) {
            additional_len = match_len - buf_len + (last_pos_slash == -2 ? -1 : last_pos_slash);
	    } else {
	        additional_len = match_len - pos_space + (last_pos_slash == -2 ? -1 : last_pos_slash);
	    }
        if (additional_len > 0) {
	        if(*curr_pos > *n) {
                *buf = incBuf(buf, len, additional_len);
                memcpy(*buf + *len, matches[0] + buf_len - (last_pos_slash == -2 ? -1 : last_pos_slash), additional_len);
                *len += additional_len;
                (*buf)[*len] = '\0';
		        // Update the display in the terminal
                printf("%s", matches[0] + buf_len - (last_pos_slash == -2 ? -1 : last_pos_slash));
                if ((stat(*buf, &file_stat)) != 0) {
                    return command;
                }
                if((!S_ISDIR(file_stat.st_mode))) {
                    putchar(' ');
                    command = incCommand(&command, *buf, n, len, true);
                    (*max_pos)++;
                    (*curr_pos)++;
                }
		        *max_pos += additional_len;
                *curr_pos += additional_len;
	        } else if((n > 0) && (*curr_pos < *n)) {
	            //changing command 
     	        command = incCommand(&command, NULL, n, &additional_len, false);
 	            char *str = malloc(((*n) - pos_space) * sizeof(char));
	            memcpy(str, command + pos_space, (*n) - pos_space);
	            memcpy(command + pos_space, matches[0] + pos_space - last_pos_slash - 1, additional_len + 2);
    	        memcpy(command + pos_space + additional_len + 1, str, (*n) - pos_space);
	            (*n) += additional_len + 1; 
	            command[*n] = '\0';
        		free(str);
		        *max_pos += additional_len + 1;
                *curr_pos += additional_len + 1;
        		//printf("\033[%dG", (*curr_pos) + 3); // Moving the cursor
	            printf("\033[K"); // Clear from cursor to end of line
    	        printf("\r> %s", command); 
                if((buf != NULL) && (*buf != NULL) && ((*buf)[0] != '\0')) {
                    printf("%s", *buf);
                }
                printf("\033[%dG", (*curr_pos) + 3); // Moving the cursor

	        }
            /**max_pos += additional_len;
            *curr_pos += additional_len;*/

            fflush(stdout);
        }
        *tab_press_count = 0;
    } else {
        if ((*tab_press_count) == 1) {
            // First Tab: append the line to the common prefix
            size_t prefix_len = buf_len - last_pos_slash;
            for (i = buf_len - last_pos_slash; ; i++) {
                char c = matches[0][i];
                int all_match = 1;
                for (j = 1; j < match_count; j++) {
                    if (matches[j][i] != c) {
                        all_match = 0;
                        break;
                    }
                }
                if (all_match) {
                    prefix_len++;
                } else {
                    break;
                }
            }

            size_t additional_len = prefix_len - (buf_len - last_pos_slash);
            if (additional_len > 0) {
                *buf = incBuf(buf, len, additional_len);
                memcpy(*buf + *len, matches[0] + buf_len - last_pos_slash, additional_len);
                *len += additional_len;
                (*buf)[*len] = '\0';
                *max_pos += additional_len;
                *curr_pos += additional_len;

                // Update the display in the terminal
                printf("%s", *buf + last_pos_slash + prefix_len - additional_len + 1);
                fflush(stdout);
            }
        } else {
            // Second Tab: Display all matches
            printf("\n");
            max_name_len += 2;
            int cols = terminal_width / max_name_len;
            if (cols == 0) {
                cols = 1;
            }
            for (i = 0; i < match_count; i++) {
                printf("%-*s", max_name_len, matches[i]);
                if (((i + 1) % cols == 0) && (i < match_count -2)) {
                    printf("\n");
                }
            }
            printf("\n> ");
            if (command) {
                printf("%s", command);
            }
            printf("%s", *buf);
	        *tab_press_count = 0;
	        printf("\033[%dG", (*curr_pos) + 3); // Moving the cursor
            fflush(stdout);
        }
    }

    // Freeing up memory for matches
    for (i = 0; i < match_count; i++) {
        free(matches[i]);
    }
    free(matches);
    if(path != NULL) {
        free(path);
    }
    return command;
}


char *read_text()
{
    //struct termios origin_termios;
    char *buf = malloc(sizeof(char));
    buf[0] = '\0';
    char *command = NULL;
    bool flag = false;
    int len = 0, n = 0, curr_pos = 0, max_pos = 0, tab_press_count = 0;
    char c;
    bool tab_flag = false;
    enum esc_subsequence esc_sub;
    char esc_seq[6];
    int esc_seq_index = 0;

    enable_canon_mode(&origin_termios);
    signal(SIGINT, handler);
    write(1, "> ", 2);
    while (read(0, &c, 1) > 0) {
        if (c == '\033') { // Start of escape sequence
            esc_seq[0] = c;
            esc_seq_index = 1;
            while (esc_seq_index < 5 && read(0, &c, 1) > 0) {
                esc_seq[esc_seq_index++] = c;
                if ((c >= 'A' && c <= 'D') || c == '~') {
                    break;
                }
            }
            esc_seq[esc_seq_index] = '\0';
            esc_sub = get_direction(esc_seq);
            switch (esc_sub) {
                case arrow_left:
                    if (curr_pos > 0) {
                        curr_pos--;
                        printf("\033[D");
                        fflush(stdout);
                    }
                    break;
                case arrow_right:
                    if (curr_pos < max_pos) {
                        curr_pos++;
                        printf("\033[C");
                        fflush(stdout);
                    }
                    break;
                case delete_key:
                    handle_delete(&curr_pos, &max_pos, &buf, &command, &n, &len);
                    break;
                default:
                    break;
            }
        } else if (c == 10) { // Enter
            command = incCommand(&command, buf, &n, &len, false);
	        free(buf);
            len = 0;
            curr_pos = max_pos = 0;

            // Print new prompt line
            putchar('\n');
            fflush(stdout);
            tcsetattr(0, TCSANOW, &origin_termios);
	        return command;
        } else if((c == 32) && (curr_pos == max_pos) && (flag == false) && (buf[0] != '\0')) {
            if(!tab_flag) {
                command = incCommand(&command, buf, &n, &len, true);
            }
            curr_pos++;
            max_pos++;
            write(1, &c, 1);
	    } else if (c == '\t') { // Tab
	        tab_press_count++;
            command = handleTabCompletion(&buf, &len, command, &n, &max_pos, &curr_pos, &tab_press_count);
        } else if (c == 127 || c == '\b') { // Backspace
            remove_and_refresh(&curr_pos, &max_pos, &buf, &command, &n, &len);
        } else if (c == 23) { // Ctrl+W
            handleCtrlW(&command, &buf, &n, &len, &curr_pos, &max_pos);
        } else if (c == 4) { // Ctrl+D
            putchar('\n');
            free(command);
	        free(buf);
            tcsetattr(0, TCSANOW, &origin_termios);
            return NULL;
        } else {
	        if(curr_pos > n) {
	            buf = incBuf(&buf, &len, 1);
	        }
            if(c == '"') {
                flag = !flag;
            }
            add_char_and_refresh(&curr_pos, &max_pos, &buf, &command, &n, c, &len, false);
        }
    }
    free(buf);
    tcsetattr(0, TCSANOW, &origin_termios);
    return command;
}

