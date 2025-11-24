#ifndef SETTINGS_H
#define SETTINGS_H

#define _POSIX_C_SOURCE 200809L


// Every "tick" of the controller timer will take this many milliseconds
// Sets it for the vehicle too via enviromental variable before calling exec for sincronisation purpuses  
#define TIMER_TICK_SPEED_MILLISECONDS 500



#define CLIENT_NAME_MAX_LENGTH 32
#define MAX_USER_SESSIONS_NUM 10 


// Paths 
#define CONTROLLER_CONNECTION_PIPE_NAME "controller_connection_gateway"
#define PATH_TO_VEHICLE_EXECUTABLE "../vehicle/build/vehicle"
#define PATH_TO_PROGRAM_PIPES_BASE "/tmp"



// Pretty terminal tags
#define COLOR_RED     "\x1b[31m"
#define COLOR_PURPLE  "\x1b[35m"
#define COLOR_RESET   "\x1b[0m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"

#define CONTROLLER COLOR_YELLOW "[CONTROLLER] " COLOR_RESET
#define ERROR COLOR_RED "[ERROR] " COLOR_RESET
#define USAGE COLOR_RED "[USAGE] " COLOR_RESET



#endif // SETTINGS_H