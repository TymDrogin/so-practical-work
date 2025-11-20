#include "utils.h"
#include "timer.h"
#include "connection_request_listener.h"
#include "data_structures.h"
#include "vehicle.h"

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


    //create_vehicle(&vid, "John", "Porto", 10);
    //create_vehicle(&vid, "Manuel", "Odessa", 200);
    //create_vehicle(&vid, "Pimba", "Lisbon", 20333);
    //create_vehicle(&vid, "Johnathan", "Jitomyr", 1);
    vehicle_t* v = create_vehicle(&vid, "Andrew", "Merda", 100);
    sleep(1);
    start_vehicle_service(v);

    while(1) pause();


    return 0;
}
