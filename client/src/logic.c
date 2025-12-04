#include "logic.h"


static char* client_name_s = NULL;

void application_termination_handler(int signum) {
    printf("\n"INFO "Terminating client session \n");
    fflush(stdout);

    // Even though the remove exist here, there is basically zero chance that illigal client will remove the pipe
    if(is_client_pipe_exist(client_name_s)) {
        remove_named_pipe(PROGRAMS_BASE_PATH, client_name_s);
    }
    
    exit(1);
    
}

void init(char* client_name) {
    // Set termination signal handler
    struct sigaction sa = {0};
    sa.sa_handler = application_termination_handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    if(is_client_pipe_exist(client_name)) {
        printf(ERROR "Client with this name appears to be already connected to the system.\n");
        printf(ERROR "Please choose a different name and try again.\n");
        exit(1);
    }

    client_name_s = client_name;
    create_named_pipe(PROGRAMS_BASE_PATH, client_name);
    start_message_listner_thread(client_name);

    // send connection request 
    if(!is_controller_entry_pipe_exist()) {
        printf(ERROR "Controller appears to be offline. Please try again later.\n");
        application_termination_handler(0);
    }
    write_to_fifo(PROGRAMS_BASE_PATH, CONTROLLER_ENTRY_FIFO_NAME, client_name);

    // Wait for response
    sleep(1);

    char client_to_controller_pipe_name[256];
    // Build the full name: controller_to_<client_name>
    snprintf(client_to_controller_pipe_name, sizeof(client_to_controller_pipe_name), "%s_to_controller", client_name_s);

    if(!is_named_pipe_exists(PROGRAMS_BASE_PATH, client_to_controller_pipe_name)) {
        fprintf(stderr, "Controller hasn't created your pipe");
        application_termination_handler(0);
    }

    printf(INFO " You are now connected to the system! Have fun! \n");
}












bool validate_client_command(const char* command) {
    // Basic validation: command should not be NULL or empty
    if (command == NULL || strlen(command) == 0) {
        return false;
    }

    if(strncmp(command, "consultar", 9) == 0 || strncmp(command, "terminar", 8) == 0) {
        return true;
    }

    
    // Validate "agendar <hora> <local> <distancia>"
    char destination[32];
    int distance;
    int time;
    if(sscanf(command, "agendar %d %s %d", &time, destination, &distance) == 3) {
        if(distance > 0 && time >=0) {
            return true;
        } else {
            return false;
        }
    }

    // Validate "cancelar <id>"
    if(strncmp(command, "cancelar", 8) == 0) {
        int id = 0;
        int n = sscanf(command, "cancelar %d", &id);
        if(n <= 0) return true;   // cancelar with no id
        if(n == 1 && id >= 0) return true;  // cancelar with valid id
    }

    return false;
}