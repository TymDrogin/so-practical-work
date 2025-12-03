#include "utils.h"


int read_tick_spead_from_enviroment() {
    // Read tick speed enviromental variable for the sync purpuses 
    const char* tick_str = getenv(TIMER_TICK_SPEED_ENV);
    if(!tick_str) {
        perror(ERROR "Could not read tick speed enviromental variable");
        return TIMER_TICK_SPEED_DEFAULT;
    }
    int tick_speed = atoi(tick_str);
    if(tick_speed <= 0) {
        perror(ERROR "Invalid tick speed has been passed to the vehicle via enviromental variable, defaulted to the 100");
        return TIMER_TICK_SPEED_DEFAULT;
    }
    return tick_speed;
}

int is_named_pipe_exists(const char* pipe_path, const char* pipe_name) {
    char full_path[256]; 
    snprintf(full_path, sizeof(full_path), "%s/%s", pipe_path, pipe_name);

    struct stat st;
    return stat(full_path, &st) == 0;
}
void create_named_pipe(const char* pipe_path, const char* pipe_name) {
    if (pipe_name == NULL|| pipe_path == NULL) {
        printf(ERROR "One or more arguments passed to the create pipe function is NULL.\n");
        exit(EXIT_FAILURE);
    }


    char full_path[256]; 
    snprintf(full_path, sizeof(full_path), "%s/%s", pipe_path, pipe_name);

    // Remove any existing file or pipe at the same path
    if (unlink(full_path) == -1 && errno != ENOENT) {
        printf(ERROR "Failed to remove existing file or pipe at %s: ", full_path);
        exit(EXIT_FAILURE);
    }

    // Create the FIFO (named pipe) with rw permissions for all
    if (mkfifo(full_path, 0666) == -1) {
        printf(ERROR "Failed to create named pipe at %s: ", full_path);
        exit(EXIT_FAILURE);
    }
}
void remove_named_pipe(const char* pipe_path, const char* pipe_name) {
    char full_path[256]; 
    snprintf(full_path, sizeof(full_path), "%s/%s", pipe_path, pipe_name);
    if (unlink(full_path) == -1 && errno != ENOENT) {
        printf(ERROR "Failed to remove existing file or pipe at %s: ", full_path);
        exit(EXIT_FAILURE);
    }
}


static void write_to_client_fifo(const char *client_pipe_name,
                                 const char *client_pipe_path_base,
                                 const char *message)
{
    char full_path[256];

    // Build the full path: /base/path/pipe_name
    snprintf(full_path, sizeof(full_path), "%s/%s",
             client_pipe_path_base, client_pipe_name);

    int fd = open(full_path, O_WRONLY | O_NONBLOCK);
    if (fd < 0) {
        perror("open client fifo");
        return;
    }

    if (write(fd, message, strlen(message)) < 0) {
        perror("write to client fifo");
    }

    close(fd);
}


void send_tenth_of_distance_traveled_message_to_controller(int traveled) {
	printf("DISTANCE=%d\n", traveled);
    fflush(stdout);
}
void send_arrived_message_to_controler() {
	printf("INFO=ARRIVED\n");
    fflush(stdout);
}
void send_service_start_message_to_controller() {
	printf("INFO=START\n");
    fflush(stdout);
}
void send_termination_message_to_controller() {
	printf("INFO=TERMINATED\n");
    fflush(stdout);
}



void send_arrived_message_to_client(char* client_pipe_name, char* client_pipe_path_base, char* destination) {
    char full_path[256]; 
    snprintf(full_path, sizeof(full_path), "%s/%s", client_pipe_name, client_pipe_path_base);

    char message[128];
    snprintf(message, sizeof(message), "\033[32m[VEHICLE]\033[0m Vehicle to the %s has arrived to your destination\n", destination);

    write_to_client_fifo(client_pipe_name, client_pipe_path_base, message);
}
void send_service_start_message_to_client(char* client_pipe_name, char* client_pipe_path_base, char* destination) {
    char message[128];
    snprintf(message, sizeof(message), "\033[32m[VEHICLE]\033[0m Starting journey to %s\n", destination);

    write_to_client_fifo(client_pipe_name, client_pipe_path_base, message);
}
void send_termination_message_to_client(char* client_pipe_name, char* client_pipe_path_base, char* destination, bool has_been_completed) {
    char message[128];
    if(has_been_completed) {
        snprintf(message, sizeof(message), "\033[32m[VEHICLE]\033[0m The journey to %s has been completed\n", destination);
    } else {
        snprintf(message, sizeof(message), "\033[31m[VEHICLE]\033[0m The journey to %s has been terminated prematurely\n", destination);
    }
    write_to_client_fifo(client_pipe_name, client_pipe_path_base, message);
}



