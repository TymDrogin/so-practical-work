#include "settings.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <string.h>
#include <unistd.h>
#include <fcntl.h>



int read_tick_spead_from_enviroment();


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
