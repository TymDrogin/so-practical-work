#include "utils.h"
#include "timer.h"
#include "connection_request_listener.h"
#include "data_structures.h"
#include "vehicle.h"
#include "settings.h"
#include "controller.h"

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
    unlink(CONTROLLER_CONNECITON_FIFO_PATH); // Remove the FIFO
    exit(0);
}

void init(void) {
    printf(CONTROLLER "Starting controller...\n");  

    start_timer();

    client_connection_req_queue = q_create_queue(CLIENT_CON_REQ_QUEUE_CAPACITY, true, NULL);
    if (client_connection_req_queue == NULL) {
        perror(ERROR "Could not create client connection request queue");
        exit(1);
    }
    start_connection_request_listener_thread(client_connection_req_queue);

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
    id_generator vid;
    init_id_generator(&vid);


    const int TARGET_CONNECTIONS = 2;

    printf(CONTROLLER "Waiting for %d clients to connect...\n", TARGET_CONNECTIONS);

    // Busy wait with a small sleep – replace with condition variable if you prefer
    while (q_size(client_connection_req_queue) < TARGET_CONNECTIONS) {
        usleep(100 * 1000); // 100ms
    }
    

}

