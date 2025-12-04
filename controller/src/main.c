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
    remove_named_pipe(PATH_TO_PROGRAM_PIPES_BASE, CONTROLLER_CONNECTION_PIPE_NAME);
    printf(CONTROLLER "Cleanup complete, exiting.\n");
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
    return;
}
int main(int argc, char *argv[]) {
    init();
    controller* c = create_controller(5, 10, 50);
    
    while(1) {
        // Connect bozos
        serve_connection_requests(c, client_connection_req_queue);
        
        sleep(1);
    }

}


