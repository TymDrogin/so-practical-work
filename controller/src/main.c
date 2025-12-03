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
}
int main(int argc, char *argv[]) {
    init();
    void* msg;
    id_generator vid;
    init_id_generator(&vid);


    vehicle_t* vehicles[3];
    vehicles[0] = create_vehicle(&vid, "client1", "DestinationA", 10);
    vehicles[1] = create_vehicle(&vid, "client2", "DestinationB", 20);
    vehicles[2] = create_vehicle(&vid, "client3", "DestinationC", 30);

    //sleep(1);
    for(int i = 0; i < 3; i++) {
        start_vehicle_service(vehicles[i]);
        print_vehicle_info(vehicles[i]);
    }

    queue* vehicle_msg_queue = q_create_queue(10, false, NULL);
    while(1) {

        for(int i = 0; i < 3; i++) {
            fflush(stdout);
            if(read_vehicle_messages(vehicles[i], vehicle_msg_queue)) {
                char* line;
                while((line = (char*)q_dequeue(vehicle_msg_queue)) != NULL) {
                    printf(CONTROLLER "Received message from vehicle %d: %s \n", vehicles[i]->id, line);
                    fflush(stdout);
                    free(line);
                }
            }
        }
        sleep(1);
    }

}


