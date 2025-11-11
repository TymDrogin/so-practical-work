#include "utils.h"


int is_named_pipe_exists(char* pipe_path) {
    struct stat st;
    return stat(pipe_path, &st) == 0;
}
void create_named_pipe(char* pipe_name, char* pipe_path_base) {
    if (pipe_name == NULL|| pipe_path_base == NULL) {
        printf(ERROR "One or more arguments passed to the create pipe function is NULL.\n");
        exit(EXIT_FAILURE);
    }


    char full_path[256]; 
    snprintf(full_path, sizeof(full_path), "%s/%s", pipe_path_base, pipe_name);

    // Remove any existing file or pipe at the same path
    if (unlink(full_path) == -1 && errno != ENOENT) {
        printf(ERROR "Failed to remove existing file or pipe at %s: ", full_path);
        exit(EXIT_FAILURE);
    }

    // Create the FIFO (named pipe) with rw permissions for all
    if (mkfifo(full_path, 0666) == -1) {
        printf(ERROR "Failed to create named pipe at %s: ", full_path);
        exit(EXIT_FAILURE);
    }
}

void remove_named_pipe(char* pipe_name, char* pipe_path_base) {
    char full_path[256]; 
    snprintf(full_path, sizeof(full_path), "%s/%s", pipe_path_base, pipe_name);
    if (unlink(full_path) == -1 && errno != ENOENT) {
        printf(ERROR "Failed to remove existing file or pipe at %s: ", full_path);
        exit(EXIT_FAILURE);
    }
}


int is_controller_entry_pipe_exist(void) {
    return is_named_pipe_exists(CONTROLLER_ENTRY_FIFO_PATH);
}
int is_client_pipe_exist(char* client_name) {
    char full_path[256]; 

    // Safely combine base path and client name
    snprintf(full_path, sizeof(full_path), "%s/%s", CLIENT_PIPE_PATH, client_name);

    // Check if the named pipe (FIFO) exists
    return is_named_pipe_exists(full_path);
}

const char *get_line_from_buffer(const char *buffer, size_t buf_len, size_t *pos, size_t *line_len) {
    if (!buffer || !pos || !line_len || *pos >= buf_len)
    	return NULL;

    const char *start = buffer + *pos;
    const char *newline = memchr(start, '\n', buf_len - *pos);

    if (newline) {
        *line_len = (size_t)(newline - start);
        *pos += *line_len + 1;  // move past '\n'
        return start;
    }
    
    if (*pos < buf_len) {
        *line_len = buf_len - *pos;
        *pos = buf_len;
        return start;
    }

    return NULL;
}
int read_lines_from_buffer_to_queue(queue* q, const char* buffer, size_t buffer_size) {
    if (!q || !buffer) {
        errno = EINVAL;
        printf(ERROR "Invalid arguments to read_lines_from_buffer_to_queue");
        return -1;
    }

    size_t pos = 0, len;
    const char* line;

    while ((line = get_line_from_buffer(buffer, buffer_size, &pos, &len)) != NULL) {
        if (len == 0) continue;

        char* copy = malloc(len + 1);
        if (!copy) {
            printf(ERROR "Memory allocation failed");
            return -1;
        }

        memcpy(copy, line, len);
        copy[len] = '\0';

        if (!q_enqueue(q, copy)) {
            printf(ERROR "Could not enqueue line read from the buffer");
            free(copy);
            return -1;
        }
    }

    return 0;
}
