#include "entry_fifo_listener.h"

static pthread_t entry_fifo_listner_thread; 

// Ensures singleton-like behavior, only one thread can be spawned
static int is_spawned = 0;

void* entry_fifo_worker(void* arg) {
    queue* client_queue = (queue*)arg;
    char buffer[128];

    int fd = open(CONTROLLER_ENTRY_FIFO_PATH, O_RDONLY);
    if (fd == -1) {
        perror(ERROR "Entry fifo listner could not open controller entry FIFO for reading");
        return NULL;
    }

    while (1) {
        ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
        // Error handling
        if (bytes_read <= 0) {
            if (bytes_read == 0) continue; // EOF, continue reading

            if (errno == EINTR) continue; // interrupted by signal, safe to retry

            // Other reading errors
            perror(ERROR "Error reading from controller entry FIFO");
            exit(1);
        }
        // Enqueue data to the client queue
        buffer[bytes_read] = '\0';
        char* client_name = malloc(bytes_read + 1);
        if (!client_name) {
            perror(ERROR "Could not allocate memory for client name");
            exit(1);
        }
        strcpy(client_name, buffer);
        
        // Thread safe enqueue
        if(!enqueue(client_queue, client_name)) {
            perror(ERROR "Could not enqueue client connection request");
            free(client_name);
        };
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

