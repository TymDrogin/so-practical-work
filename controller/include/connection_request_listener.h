// Entry fifo is a main gateway for clients to connect to the controller
// It is a named pipe located at /tmp/controller_entry_fifo
// Clients write their names to this fifo to register themselves with the controller
// The controller reads from this fifo and processes client registrations
// This file contains a fifo listener thread, that is going to concurrently safe enqueue users that ask for connection
// While the main controller loop will dequeue and process them when it is ready

#define _GNU_SOURCE

#include <pthread.h>   // pthread_t, pthread_create
#include <stdio.h>     // printf, perror
#include <stdlib.h>    // exit
#include <unistd.h>    // read, close, unlink
#include <fcntl.h>     // open, O_RDONLY, etc.
#include <errno.h>     // errno, EINTR
#include <sys/stat.h>  // mkfifo
#include <string.h>    // For string manipulation if needed (e.g., memset)

#include "settings.h"
#include "utils.h"
#include "data_structures.h"

// Start the connection request fifo listener thread. It will read client names from the fifo and enqueue them to the provided client queue.

void start_connection_request_listener_thread(queue* client_connection_req_queue);
