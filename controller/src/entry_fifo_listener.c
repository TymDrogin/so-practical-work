#include "entry_fifo_listener.h"

static pthread_t entry_fifo_listner_thread; 
static int is_spawned = 0;

void* entry_fifo_worker(void* arg) {
    char buffer[256];
    int fd = open(CONTROLLER_ENTRY_FIFO_PATH, O_RDONLY);

    while (1) {
        ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);

        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            printf("Received from FIFO: %s\n", buffer);
        } else if (bytes_read == 0) {
            // Writer closed the pipe — reopen to wait for the next one
            close(fd);
            fd = open(CONTROLLER_ENTRY_FIFO_PATH, O_RDONLY);
            if (fd == -1) {
                perror("reopen");
                exit(EXIT_FAILURE);
            }
        } else {
            perror("read");
            break;
        };
    }
    return NULL;
}

// Perchance move pipe creation logic out of here to the controller initialization code
void start_entry_fifo_listener_thread(void) {
    // Remove the pipe if it already exists
    unlink(CONTROLLER_ENTRY_FIFO_PATH); 

    if(mkfifo(CONTROLLER_ENTRY_FIFO_PATH, 0666) == -1) {
        perror(ERROR "Could not create controller entry FIFO");
        return 1;
    };

    if (is_spawned) {
        printf(ERROR "Entry FIFO listener thread has already been started.\n");
        exit(1);
    }

    entry_fifo_listner_thread = pthread_create(&entry_fifo_listner_thread, NULL, entry_fifo_worker, NULL);
    is_spawned = 1;
    if (entry_fifo_listner_thread != 0) {
        perror(ERROR "Failed to create entry fifo listener thread.\n");
        exit(1);
    };
}