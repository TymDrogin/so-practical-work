#ifndef SETTINGS_H
#define SETTINGS_H


// Every "tick" of the controller timer will take this many milliseconds
#define TIMER_TICK_SPEED_SECONDS 1

#define COLOR_RED     "\x1b[31m"
#define COLOR_PURPLE  "\x1b[35m"
#define COLOR_RESET   "\x1b[0m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"

#define CONTROLLER COLOR_YELLOW "[CONTROLLER] " COLOR_RESET
#define ERROR COLOR_RED "[ERROR] " COLOR_RESET
#define USAGE COLOR_RED "[USAGE] " COLOR_RESET
#define VEHICLE COLOR_GREEN "[VEHICLE] " COLOR_RESET

#define TIMER_TICK_SPEED_MILLISECONDS 1000


#define CLIENT_NAME 1 
#define DESTINATION 2 
#define DISTANCE 3
#define VEHICLE_ID 4 




#endif // SETTINGS_H