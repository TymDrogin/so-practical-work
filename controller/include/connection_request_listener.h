// Entry fifo is a main gateway for clients to connect to the controller
// It is a named pipe located at /tmp/controller_entry_fifo
// Clients write their names to this fifo to register themselves with the controller
// The controller reads from this fifo and processes client registrations
// This file contains a fifo listener thread, that is going to concurrently safe enqueue users that ask for connection
// While the main controller loop will dequeue and process them when it is ready

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>

#include "settings.h"
#include "queue.h"

// Start the entry fifo listener thread. It will read client names from the fifo and enqueue them to the provided client queue.
void start_connection_request_listener_thread(queue* client_connection_req_queue);
queue* dequeue_client_connection_request(queue* client_connection_req_queue);
