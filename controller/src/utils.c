#include "utils.h"
// Id generator utilities
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


int is_named_pipe_exists(const char* pipe_path,const char* pipe_name);
void create_named_pipe(const char* pipe_path, const char* pipe_name);
void remove_named_pipe(const char* pipe_path, const char* pipe_name);


// Message utilities
void write_to_fifo(const char *pipe_path,
                                 const char *pipe_name,
                                 const char *message) {
    char full_path[256];

    // Build the full path: /base/path/pipe_name
    snprintf(full_path, sizeof(full_path), "%s/%s",
             pipe_path, pipe_name);

    int fd = open(full_path, O_WRONLY | O_NONBLOCK);
    if (fd < 0) {
        fprintf(stderr, "Can't open the pipe with path %s", full_path);
        return;
    }

    if (write(fd, message, strlen(message)) < 0) {
        perror("write to client fifo");
    }

    close(fd);
}


// Pipe utilities
int is_named_pipe_exists(const char* pipe_path, const char* pipe_name) {
    char full_path[256]; 
    snprintf(full_path, sizeof(full_path), "%s/%s", pipe_path, pipe_name);

    struct stat st;
    return stat(full_path, &st) == 0;
}
void create_named_pipe(const char* pipe_path, const char* pipe_name) {
    if (pipe_name == NULL|| pipe_path == NULL) {
        printf(ERROR "One or more arguments passed to the create pipe function is NULL.\n");
        exit(EXIT_FAILURE);
    }


    char full_path[256]; 
    snprintf(full_path, sizeof(full_path), "%s/%s", pipe_path, pipe_name);

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
void remove_named_pipe(const char* pipe_path, const char* pipe_name) {
    char full_path[256]; 
    snprintf(full_path, sizeof(full_path), "%s/%s", pipe_path, pipe_name);
    if (unlink(full_path) == -1 && errno != ENOENT) {
        printf(ERROR "Failed to remove existing file or pipe at %s: ", full_path);
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
