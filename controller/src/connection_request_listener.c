#include "connection_request_listener.h"

static pthread_t connection_request_listener_thread; 

// Ensures singleton-like behavior, only one thread can be spawned
static int is_spawned = 0;

// This function assumes there is going to be only one line per read call in the data
void* connection_request_worker(void* arg) {
    queue* client_queue = (queue*)arg;
    char buffer[512];

    int fd = open(CONTROLLER_ENTRY_FIFO_PATH, O_RDONLY);
    if (fd == -1) {
        perror(ERROR "Could not open controller entry FIFO for reading");
        return NULL;
    }

    while (1) {
        // Read from pipe
        ssize_t bytes_read = read(fd, buffer, sizeof(buffer));
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
                continue; 

            perror(ERROR "Error reading from controller entry FIFO");
            close(fd);
            return NULL;
        }
        buffer[bytes_read] = '\0';
        // Process buffered up data 
        if (read_lines_from_buffer_to_queue(client_queue, buffer, bytes_read) != 0) {
            close(fd);
            return NULL;
        }
    }

    close(fd);
    return NULL;
}


// Perchance move pipe creation logic out of here to the controller initialization code
void start_connection_request_listener_thread(queue* client_connection_req_queue) {
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

    connection_request_listener_thread = pthread_create(&connection_request_listener_thread, NULL, connection_request_worker, client_connection_req_queue);
    is_spawned = 1;
    if (connection_request_listener_thread != 0) {
        perror(ERROR "Failed to create entry fifo listener thread.\n");
        exit(1);
    };
}

