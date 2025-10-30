#define _POSIX_C_SOURCE 200809L

#include "common.h"
#include "timer.h"
#include "entry_fifo_listener.h"
#include "queue.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

#define CARS_NUM 4
#define CLIENT_CON_REQ_QUEUE_CAPACITY 100

static queue* client_connection_req_queue;

void cleanup(int signum) {
    printf("\n" CONTROLLER "Caught signal %d, cleaning up...\n", signum);
    unlink(CONTROLLER_ENTRY_FIFO_PATH); // Remove the FIFO
    exit(0);
}

void init(void) {
    printf(CONTROLLER "Starting controller...\n");  

    start_timer();

    client_connection_req_queue = create_queue(CLIENT_CON_REQ_QUEUE_CAPACITY);
    if (client_connection_req_queue == NULL) {
        perror(ERROR "Could not create client connection request queue");
        exit(1);
    }
    start_entry_fifo_listener_thread(client_connection_req_queue);

    struct sigaction sa;
    sa.sa_handler = cleanup;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, NULL);  // Ctrl+C
    sigaction(SIGTERM, &sa, NULL); // Kill signal

    printf(CONTROLLER "Controller initialized successfully.\n");
}




int main(int argc, char *argv[]) {
    init();
    void* msg;
    while(queue_size(client_connection_req_queue) < 5) {
        sleep(1);
        printf("Waiting for client connection requests... Current queue size: %d\n", queue_size(client_connection_req_queue));
    }
    while ((msg = dequeue(client_connection_req_queue)) != NULL) {
        printf("Processing client connection request: %s\n", (char*)msg);
    }
        


    return 0;
}
