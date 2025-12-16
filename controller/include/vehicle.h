#ifndef VEHICLE_PROCESS_H
#define VEHICLE_PROCESS_H

#include "utils.h"
#include <stdbool.h>


#include <pthread.h>   // pthread_t, pthread_create
#include <stdio.h>     // printf, perror
#include <stdlib.h>    // exit
#include <unistd.h>    // read, close, unlink
#include <fcntl.h>     // open, O_RDONLY, etc.
#include <errno.h>     // errno, EINTR
#include <sys/stat.h>  // mkfifo
#include <string.h>    // For string manipulation if needed (e.g., memset)
#include "settings.h"
#include <signal.h>


typedef struct vehicle_t {
    int id;                      // Unique vehicle id
 
    int pid;                     // Pid of the vehicle process
    int fd[2];                   // Anonymous pipe file descriptor 
 
    char* client_name;           // Name of the client this vehicle is dispatched for 
    char* destination;           // Destination of the vehicle 
    int distance_to_travel;      // Total distance for the vehicle to travel
    int distance_traveled;       // Actual traveled distance (controller gets notified via pipe and increments)

    bool is_alive;               // In case the process has been terminated (controller gets notified via pipe) set it to 0 
} vehicle_t;



// Starts the vehicle process immidiately, 
vehicle_t* create_vehicle(id_generator* g, char* client_name, char* destination, int distance_to_travel);
void run_vehicle_process(vehicle_t* v);


void print_vehicle_info(const vehicle_t* v);
bool read_vehicle_messages(vehicle_t* v, queue* q);

// Sends the aproppriate signals, and changes the state flags
void start_vehicle_service(vehicle_t* v);
void cancel_vehicle_service(vehicle_t* v);

// Sends the termination signal if neccesesry, as well as frees all the resources associated with the vehicle
void destroy_vehicle(vehicle_t* v);
void free_vehicle(void* item);





#endif // VEHICLE_PROCESS_H