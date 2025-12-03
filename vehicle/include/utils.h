#include "settings.h"

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include <stdbool.h>


int read_tick_spead_from_enviroment();



int is_named_pipe_exists(const char* pipe_path,const char* pipe_name);
void create_named_pipe(const char* pipe_path, const char* pipe_name);
void remove_named_pipe(const char* pipe_path, const char* pipe_name);


void send_tenth_of_distance_traveled_message_to_controller(int traveled);

void send_arrived_message_to_controler();
void send_service_start_message_to_controller();
void send_termination_message_to_controller();


void send_arrived_message_to_client(
                        char* client_pipe_name, 
                        char* client_pipe_path_base, 
                        char* destination);
void send_service_start_message_to_client(
                        char* client_pipe_name, 
                        char* client_pipe_path_base, 
                        char* destination);
void send_termination_message_to_client(
                        char* client_pipe_name, 
                        char* client_pipe_path_base, 
                        char* destination, 
                        bool has_been_completed);
