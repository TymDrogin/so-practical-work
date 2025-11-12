#include "utils.h"

void init_id_generator(id_generator* generator) {
    generator->id = 0;
    if(pthread_mutex_init(&generator->mutex, NULL) != 0) {
        printf(ERROR "Failed to initialize id generator mutex.\n");
        free(generator);
        exit(1);
    };
}
int generate_id(id_generator* generator) {
    pthread_mutex_lock(&generator->mutex);
    int id = generator->id;
    generator->id++;
    pthread_mutex_unlock(&generator->mutex);
    return id; 
}

int is_named_pipe_exists(char* pipe_path) {
    struct stat st;
    return stat(pipe_path, &st) == 0;
}
void create_named_pipe(char* pipe_path) {
    if (pipe_path == NULL) {
        printf(ERROR "Pipe path is NULL.\n");
        exit(EXIT_FAILURE);
    }

    // Remove any existing file or pipe at the same path
    if (unlink(pipe_path) == -1 && errno != ENOENT) {
        printf(ERROR "Failed to remove existing file or pipe at %s: ", pipe_path);
        exit(EXIT_FAILURE);
    }

    // Create the FIFO (named pipe) with rw permissions for all
    if (mkfifo(pipe_path, 0666) == -1) {
        printf(ERROR "Failed to create named pipe at %s: ", pipe_path);
        exit(EXIT_FAILURE);
    }
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
        if (len == 0) continue; // Skip empty lines

        char* copy = malloc(len + 1);
        if (!copy) {
            printf(ERROR "Memory allocation failed");
            return -1;
        }

        memcpy(copy, line, len);
        copy[len] = '\0';
        // Enque the client
        if (!q_enqueue(q, copy)) {
            printf(ERROR "Could not enqueue line read from the buffer");
            free(copy);
            return -1;
        }
    }

    return 0;
}
