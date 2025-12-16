#include "message_listner.h"

static pthread_t message_listner_thread; 

// Ensures singleton-like behavior, only one thread can be spawned
static int is_spawned = 0;

// This function assumes there is going to be only one line per read call in the data
void* message_listener_worker(void* arg) {
    char* client_name = (char*)arg;
    char full_path[256]; 
    snprintf(full_path, sizeof(full_path), "%s/%s", PROGRAMS_BASE_PATH, client_name);

    int fd = open(full_path, O_RDONLY);
    if (fd == -1) { 
        perror(ERROR "Could not open client pipe to read messages");
        return NULL;
    }

    char buffer[256];
    char line_buffer[1024] = {0}; // persistent buffer for partial lines
    size_t line_len = 0;

    while (1) {
        ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
        if (bytes_read <= 0) {
            if (bytes_read == 0) {
                // EOF – writer closed the FIFO, reopen it
                close(fd);
                fd = open(full_path, O_RDONLY);
                if (fd == -1) {
                    perror(ERROR "Reopening FIFO failed");
                    return NULL;
                }
                continue;
            }
            if (errno == EINTR) continue;
            perror(ERROR "Could not read from the client pipe");
            close(fd);
            return NULL;
        }

        // Append read data to persistent buffer
        
        for (ssize_t i = 0; i < bytes_read; i++) {
            line_buffer[line_len++] = buffer[i];

            if (buffer[i] == '\n' || line_len >= sizeof(line_buffer)-1) {
                line_buffer[line_len] = '\0'; // terminate string

                // Remove trailing newline for pretty printing
                if (line_len > 0 && line_buffer[line_len-1] == '\n')
                    line_buffer[line_len-1] = '\0';

                // Print the complete line and reprint prompt (single `> `)
                printf("\n\x1b[33m[MSG]\x1b[0m %s\n> ", line_buffer);
                fflush(stdout);

                line_len = 0; // reset for next line
            }
        }
    }

    close(fd);
    return NULL;
}

// Perchance move pipe creation logic out of here to the controller initialization code
void start_message_listner_thread(char* client_name) {
    if(!is_client_pipe_exist(client_name)) {
        printf(ERROR "Client pipe has not been created, listening thread failed");
        exit(0);
    }
    if(is_spawned) {
        printf(ERROR "Creating a second message listner thread is forbiden.\n");
        exit(1);
    }

    int status = pthread_create(&message_listner_thread, NULL, message_listener_worker, client_name);
    is_spawned = 1;
    if (status != 0) {
        perror(ERROR "Failed to create entry fifo listener thread.\n");
        exit(1);
    };
}