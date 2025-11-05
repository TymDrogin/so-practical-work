#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include "settings.h"

static char* client_name;
static char* destination;
static int distance_to_travel;
static int tenth_of_distance_to_travel;
static int vehicle_id;
static int distance_traveled = 0;


// Service cancelation routine
void service_termination_handler(int signum) {
    char* termination_message = "Vehicle process received termination signal. Exiting...\n";
    write(STDOUT_FILENO, "Vehicle process received termination signal. Exiting...\n", strlen(termination_message));
    exit(0);
}

// Main vehicle loop 
void service_start_handler(int signum) {
    printf(VEHICLE "Client entered the vehicle, starting the ride\n");
    fflush(stdout);

    while (1) {
        distance_traveled++;

        if (distance_traveled % tenth_of_distance_to_travel == 0) {
            printf("Traveled tenth of the distance (%d). Target: %d\n",
                   tenth_of_distance_to_travel, distance_to_travel);
            printf("Total distance traveled: %d\n", distance_traveled);
            fflush(stdout);
        }

        if (distance_traveled >= distance_to_travel)
            break;

        usleep(TIMER_TICK_SPEED_MILLISECONDS * 1000);
    }

    printf("Finished the service, informing client\n");
    fflush(stdout);
}

void init() {
    struct sigaction start_action = {0};
    start_action.sa_handler = service_start_handler;
    sigaction(SIGUSR2, &start_action, NULL);

    struct sigaction termination_action = {0};
    termination_action.sa_handler = service_termination_handler;
    sigaction(SIGUSR1, &termination_action, NULL);
}


// ARGS ./vehicle <client_name> <destination> <distance_to_travel> <id_of_car>
//       0        1             2             3                    4 

// Check if client pipe exists and open, steps 
// Write a message (car to destination is ready + pid of car)
// Client sends a message enter to the controller that changes the state of the client session info 
// And sends kill SIGUSR2 to start the car. 
// Client also can exit or cancel the service that might 
int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Error: Invalid number of arguments, expected 5, got %d\n", argc);
        exit(1);
    }

    client_name = argv[CLIENT_NAME];
    destination = argv[DESTINATION];
    distance_to_travel = atoi(argv[DISTANCE]);
    vehicle_id = atoi(argv[VEHICLE_ID]);
    tenth_of_distance_to_travel = distance_to_travel / 10;

    setbuf(stdout, NULL); // disable buffering

    printf("Vehicle process has been started, pid: %d\n", getpid());
    printf("Client: %s, Destination: %s, Distance: %d, ID: %d\n",
           client_name, destination, distance_to_travel, vehicle_id);

    init();

    while(1)
        pause();

    return 0;
}