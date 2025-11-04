#include "entry_fifo_listener.h"

static pthread_t entry_fifo_listner_thread; 

// Ensures singleton-like behavior, only one thread can be spawned
static int is_spawned = 0;

void* entry_fifo_worker(void* arg) {
    queue* client_queue = (queue*)arg;
    char buffer[128];

    int fd = open(CONTROLLER_ENTRY_FIFO_PATH, O_RDONLY);
    if (fd == -1) {
        perror(ERROR "Could not open controller entry FIFO for reading");
        return NULL;
    }

    while (1) {
        ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
        if (bytes_read <= 0) {
            if (bytes_read == 0) {
                // EOF – writer closed the FIFO, reopen it
                close(fd);
                fd = open(CONTROLLER_ENTRY_FIFO_PATH, O_RDONLY);
                if (fd == -1) {
                    perror(ERROR "Reopening FIFO failed");
                    return NULL;
                }
                continue;
            }

            if (errno == EINTR)
                continue; // Retry after signal

            perror(ERROR "Error reading from controller entry FIFO");
            close(fd);
            return NULL;
        }

        buffer[bytes_read] = '\0';
        size_t len = strlen(buffer);
        char* client_name = malloc(len + 1);
        if (!client_name) {
            perror(ERROR "Memory allocation failed");
            continue;
        }
        strcpy(client_name, buffer);

        if (!q_enqueue(client_queue, client_name)) {
            perror(ERROR "Could not enqueue client connection request");
            free(client_name);
        }
    }

    close(fd);
    return NULL;
}

// Perchance move pipe creation logic out of here to the controller initialization code
void start_entry_fifo_listener_thread(queue* client_connection_req_queue) {
    // Remove the pipe if it already exists
    unlink(CONTROLLER_ENTRY_FIFO_PATH); 

    if(mkfifo(CONTROLLER_ENTRY_FIFO_PATH, 0666) == -1) {
        perror(ERROR "Could not create controller entry FIFO");
        exit(1);
    };

    if (is_spawned) {
        printf(ERROR "Entry FIFO listener thread has already been started.\n");
        exit(1);
    }
    if(client_connection_req_queue == NULL) {
        printf(ERROR "Client connection request queue is NULL.\n");
        exit(1);
    }

    entry_fifo_listner_thread = pthread_create(&entry_fifo_listner_thread, NULL, entry_fifo_worker, client_connection_req_queue);
    is_spawned = 1;
    if (entry_fifo_listner_thread != 0) {
        perror(ERROR "Failed to create entry fifo listener thread.\n");
        exit(1);
    };
}

