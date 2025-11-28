#include "message_listner.h"

static pthread_t message_listner_thread; 

// Ensures singleton-like behavior, only one thread can be spawned
static int is_spawned = 0;

// This function assumes there is going to be only one line per read call in the data
void* message_listner_worker(void* arg) {
    char* client_name = (char*)arg;

    char full_path[256]; 
    snprintf(full_path, sizeof(full_path), "%s/%s", PROGRAMS_BASE_PATH, client_name);


    int fd = open(full_path, O_RDONLY);
    if (fd == -1) {
        perror(ERROR "Could not open client pipe to read messages");
        return NULL;
    }
    char buffer[256];

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

            if (errno == EINTR)
                continue; // Retry after signal

            perror(ERROR "Could not read from the client pipe");
            close(fd);
            return NULL;
        }

        buffer[bytes_read] = '\0';

        // Remove the last newline character for pretty printing
        for(int i = bytes_read -1; i >=0; i--) {
            if(buffer[i] == '\n') {
                buffer[i] = '\0';
                break;
            }
        }
        

        // Move to line beggining
        printf("\r");
        printf("\x1b[33m[MSG]:\x1b[0m ");
        printf(buffer);

        // Reprint cursor symbol
        printf("\n>");
        fflush(stdout);
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

    int status = pthread_create(&message_listner_thread, NULL, message_listner_worker, client_name);
    is_spawned = 1;
    if (status != 0) {
        perror(ERROR "Failed to create entry fifo listener thread.\n");
        exit(1);
    };
}