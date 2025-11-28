#include "utils.h"


int is_named_pipe_exists(char* pipe_path, char* pipe_name) {
    char full_path[256]; 
    snprintf(full_path, sizeof(full_path), "%s/%s", pipe_path, pipe_name);

    struct stat st;
    return stat(full_path, &st) == 0;
}

int is_controller_entry_pipe_exist(void) {
    return is_named_pipe_exists(PROGRAMS_BASE_PATH, CONTROLLER_ENTRY_FIFO_NAME);
}
int is_client_pipe_exist(char* client_name) {
    // Check if the named pipe (FIFO) exists
    return is_named_pipe_exists(PROGRAMS_BASE_PATH, client_name);
}




void create_named_pipe(char* pipe_path, char* pipe_name) {
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
void remove_named_pipe(char* pipe_path, char* pipe_name) {
    char full_path[256]; 
    snprintf(full_path, sizeof(full_path), "%s/%s", pipe_path, pipe_name);
    if (unlink(full_path) == -1 && errno != ENOENT) {
        printf(ERROR "Failed to remove existing file or pipe at %s: ", full_path);
        exit(EXIT_FAILURE);
    }
}


void print_usage() {
    printf("Usage:\n");
    printf("    agendar <hora> <local> <distancia>  -- Submit service request\n");
    printf("    consultar                           -- List all service requests\n");
    printf("    cancelar <id>                       -- Cancel the most recent service. If id is 0 - cancelles all.\n");
    printf("    terminar                            -- Exit the application\n");

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
