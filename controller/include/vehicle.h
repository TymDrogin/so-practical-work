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
    int id;                     // Unique vehicle id

    int pid;                    // Pid of the vehicle process
    int fd[2];                  // Anonymous pipe file descriptor 

    char* client_name;          // Name of the client this vehicle is dispatched for 
    char* destination;          // Destination of the vehicle 
    int distance_to_travel;     // Total distance for the vehicle to travel
    int distance_traveled;      // Actual traveled distance (controller gets notified via pipe and increments)

    bool is_alive;             // In case the process has been terminated (controller gets notified via pipe) set it to 0 
    bool is_performing_service  // Sets to 1 once the controller gets notify about client entry
} vehicle_t;

void print_vehicle_info(const vehicle_t* v);

// Starts the vehicle process immidiately, 
vehicle_t* create_vehicle(id_generator* g, char* client_name, char* destination, int distance_to_travel);
void start_vehicle(vehicle_t* v);
void stop_vehicle(vehicle_t* v);

// returns 1 if any messages have been read 
bool read_vehicle_messages(vehicle_t* v, queue* q);


// Frees the memory terminates the process if necceserry(vehicle can kill itself or client, meaning the funciton might not need to send kill)
void destroy_vehicle(vehicle_t* v);
void free_vehicle(void* v);





#endif // VEHICLE_PROCESS_H