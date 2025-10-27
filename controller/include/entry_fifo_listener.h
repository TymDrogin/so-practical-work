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

#include <fcntl.h>
#include <sys/types.h>

#include "common.h"
#include "settings.h"

// Start the entry fifo listener thread
// In the future, this function will accept a pointer to a queue where it will enqueue incoming client connection requests
// For now, it just prints the received client names to stdout
void start_entry_fifo_listener_thread(void);

