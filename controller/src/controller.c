#include "controller.h"


// Request related functions

request* create_request(id_generator* g,
                        int client_session_id,
                        const char* destination,
                        int distance_to_travel,
                        int desired_start_time,
                        int time_created)
{
    if (g == NULL) {
        perror(ERROR "create_request: id_generator is NULL");
        exit(EXIT_FAILURE);
    }

    request* r = malloc(sizeof(request));
    if (r == NULL) {
        perror(ERROR "Could not allocate memory for request");
        exit(EXIT_FAILURE);
    }

    r->id = generate_id(g);
    r->client_session_id = client_session_id;
    r->destination = strdup(destination);
    r->distance_to_travel = distance_to_travel;
    r->desired_start_time = desired_start_time;

    r->is_active = false;
    r->vehicle_id = -1; // No vehicle assigned yet
    r->time_created = time_created;

    return r;
}
void start_request_processing(request* r, int vehicle_id) {
    if (r == NULL) {
        perror(ERROR "start_request_processing: request is NULL");
        exit(EXIT_FAILURE);
    }
    r->is_active = true;
    r->vehicle_id = vehicle_id;
}

// CONTROLLER FUNCTIONS

controller* create_controller(const int max_num_of_user_sessions,
                              const int max_num_of_vehicles,
                              const int max_num_of_services,
                              const int max_num_of_requests)
{
    if (max_num_of_user_sessions <= 0 || max_num_of_vehicles <= 0 || max_num_of_services <= 0) {
        perror(ERROR "Invalid controller limits\n");
        exit(EXIT_FAILURE);
    }

    controller* c = malloc(sizeof(controller));
    if (c == NULL) {
        perror(ERROR "Could not allocate memory for controller");
        exit(EXIT_FAILURE);
    }

    // Initialize arrays
    // TODO: 
    c->sessions = a_create_array(max_num_of_user_sessions, free_client_session);
    c->vehicles = a_create_array(max_num_of_vehicles, NULL);
    c->request = a_create_array(max_num_of_services, NULL);

    // Initialize ID generators
    c->vehicle_id_gen = malloc(sizeof(id_generator));
    c->session_id_gen = malloc(sizeof(id_generator));
    c->request_id_gen = malloc(sizeof(id_generator));

    init_id_generator(c->vehicle_id_gen);
    init_id_generator(c->session_id_gen);
    init_id_generator(c->request_id_gen);

    c->max_num_of_sessions  = max_num_of_user_sessions;
    c->max_num_of_vehicles  = max_num_of_vehicles;
    c->max_num_of_services  = max_num_of_services;
    c->max_num_of_requests = max_num_of_requests;

    c->total_km_traveld_by_all_cars = 0;

    return c;
}


void start_client_session(client_session* s) {
    if (s == NULL) {
        perror(ERROR "start_client_session: client session is NULL");
        exit(EXIT_FAILURE);
    }
    // Create the client pipe
    create_named_pipe(PATH_TO_PROGRAM_PIPES_BASE, s->client_to_controller_pipe);

    s->is_active = true;
    message_client_connection_accepted(s->client_name);
}
void terminate_client_session(controller* c, client_session* s) {
    if (c == NULL) {
        fprintf(stderr, ERROR "terminate_client_session: controller is NULL\n");
        exit(EXIT_FAILURE);
    }

    if (s == NULL) {
        fprintf(stderr, ERROR "terminate_client_session: client_session is NULL\n");
        exit(EXIT_FAILURE);
    }

    // Removes the named pipe associated with the client session
    remove_named_pipe(PATH_TO_PROGRAM_PIPES_BASE, s->client_to_controller_pipe);

    // Notify the client about termination
    message_client_disconnection_notice(s->client_name);

    // Cancell all active services of the client? 

    // Remove all client requests from the system?




    // Free the session resources
    free_client_session(s);

    return;
}


bool connect_client(controller* c, const char* client_name) {
    if (c == NULL) {
        fprintf(stderr, ERROR "connect_client: controller is NULL\n");
        exit(EXIT_FAILURE);
    }

    if (client_name == NULL) {
        fprintf(stderr, ERROR "connect_client: client_name is NULL\n");
        exit(EXIT_FAILURE);
    }

    // Check if client is already connected
    if (is_client_connected_by_name(c, client_name)) {
        message_client_connection_rejected(client_name, "Client with this name is already connected.");
        return false;
    }
    // Check if client pipe exists
    if (!is_named_pipe_exists(PATH_TO_PROGRAM_PIPES_BASE, client_name)) {
        // TODO: Message controller admin about failed connection attempt
        return false;
    }

    // Check for available session slot
    if (a_is_full(c->sessions)) {
        message_client_connection_rejected(client_name, "No avaible connections slots.");
        return false;
    }

    // Create new client session
    client_session* s = create_client_session(
        c->session_id_gen,
        client_name
    );
    // Add session to controller's sessions array
    // a_push should never fail here because we already checked for capacity
    if (a_push(c->sessions, s) != 0) {
        message_client_connection_rejected(client_name, "Failed to create a new session.");
        free_client_session(s);
        return false;
    }
    start_client_session(s);
    return true;
}

void disconnect_client_by_name(controller* c, const char* client_name) {
    if (c == NULL) {
        fprintf(stderr, ERROR "disconnect_client: controller is NULL\n");
        exit(EXIT_FAILURE);
    }

    if (client_name == NULL) {
        fprintf(stderr, ERROR "disconnect_client: client_name is NULL\n");
        exit(EXIT_FAILURE);
    }
    // Find the session by client name
    int count = a_size(c->sessions);
    for (int i = 0; i < count; i++) {
        client_session* s = a_get(c->sessions, i);
        if(s == NULL) {
            perror(ERROR "disconnect_client: Read null session");
            exit(EXIT_FAILURE);
        }
        if(s->client_name == NULL) {
            perror(ERROR "disconnect_client: Session client name is NULL");
            exit(EXIT_FAILURE);
        }
        if (strcmp(s->client_name, client_name) == 0) {
            // Found the session, remove it
            a_remove(c->sessions, i);
            terminate_client_session(c, s);
            return;
        }
    }



    // Client not found
}

bool is_client_connected_by_name(const controller* c, const char* client_name) {
    if (c == NULL) {
        fprintf(stderr, ERROR "is_client_connected_by_name: controller is NULL\n");
        exit(EXIT_FAILURE);
    }

    if (client_name == NULL) {
        fprintf(stderr, ERROR "is_client_connected_by_name: client_name is NULL\n");
        exit(EXIT_FAILURE);
    }

    if (c->sessions == NULL) {
        fprintf(stderr, ERROR "is_client_connected_by_name: controller->sessions is NULL\n");
        exit(EXIT_FAILURE);
    }

    int count = a_size(c->sessions);
    for (int i = 0; i < count; i++) {
        client_session* s = a_get(c->sessions, i);
        if(s == NULL) {
            perror(ERROR "is_client_connected_by_name: Read null session");
            exit(EXIT_FAILURE);
        }
        if(s->client_name == NULL) {
            perror(ERROR "is_client_connected_by_name: Session client name is NULL");
            exit(EXIT_FAILURE);
        }
        if (strcmp(s->client_name, client_name) == 0) {
            return true;
        }
    }
    return false;
}







void process_admin_command(controller* c, char* line) {
    if (c == NULL) {
        fprintf(stderr, ERROR "process_admin_command: controller is NULL\n");
        exit(EXIT_FAILURE);
    }

    if (line == NULL) {
        fprintf(stderr, ERROR "process_admin_command: command is NULL\n");
        exit(EXIT_FAILURE);
    }
    char cmd[32];
    int id;

    // Try parsing "<cmd> <id>" or "<cmd>"
    int n = sscanf(line, "%31s %d", cmd, &id);


    if(strcmp(cmd, "listar") == 0) {
        // Execute listar
        // Prints all active servises information

    }
    if(strcmp(cmd, "utiliz") == 0) {
        // Prints information about connected clients, 
        // As well as the status of their servise
    }
    if(strcmp(cmd, "frota") == 0) {
        // Prints information about all vehicles
        // id, destination, percentage traveled
    }
    if(strcmp(cmd, "km") == 0) {
        // Prints total km traveled by all vehicles
    }
    if(strcmp(cmd, "hora") == 0) {
        // Prints current controller time
    }
    if(strcmp("terminar", cmd) == 0) {
        // Gracefully terminates the controller
        // Disconnects all clients, frees all resources
    }

    if(strcmp(cmd, "cancelar") == 0) {
        if(n == 2 && id >= 0) {
            // Cancel service with given id
            // Calls service cancellation routine. Removes service, cancelles if active, frees resources, notifies client

            // If id is 0, cancel all services
        } else {
            fprintf(stderr, ERROR "Invalid command format for 'cancelar'. Usage: cancelar <service_id>\n");
        }
    }

    // Notify admin about unknown command
}
void process_client_command(controller* c, client_session* s, char* line) {
    if (c == NULL) {
        fprintf(stderr, ERROR "process_client_command: controller is NULL\n");
        exit(EXIT_FAILURE);
    }

    if (s == NULL) {
        fprintf(stderr, ERROR "process_client_command: client_session is NULL\n");
        exit(EXIT_FAILURE);
    }

    if (line == NULL) {
        fprintf(stderr, ERROR "process_client_command: command is NULL\n");
        exit(EXIT_FAILURE);
    }

    char cmd[32];
    // Try parsing "<cmd> <args...>"
    int n = sscanf(line, "%31s", cmd);

    if(strcmp(cmd, "solicitar") == 0) {
        // Client requests a new service
        // Parse arguments, create request, add to controller's request array
    }
    if(strcmp(cmd, "estado") == 0) {
        // Client requests status of their active service
        // Find the active request for this client session and report its status
    }
    if(strcmp(cmd, "desligar") == 0) {
        // Client requests disconnection from the controller
        disconnect_client_by_name(c, s->client_name);
    }

    // Notify client about unknown command
    return;
}



// MESSAGES TO CLIENTS AND CONTROLLER ADMINISTRATOR
void message_client_connection_accepted(const char* client_name) {
    char message[128];
    snprintf(message, sizeof(message), CONTROLLER "Connection request accepted, you are now connected to the system.\n");
    write_to_fifo(PATH_TO_PROGRAM_PIPES_BASE, client_name,  message);
}
void message_client_connection_rejected(const char* client_name, const char* reason) {
    char message[256];
    snprintf(message, sizeof(message), CONTROLLER "Connection request rejected: %s\n", reason);
    write_to_fifo(PATH_TO_PROGRAM_PIPES_BASE, client_name,  message);
}

void message_client_disconnection_notice(const char* client_name) {
    char message[128];
    snprintf(message, sizeof(message), CONTROLLER "You have been disconnected from the system. Futher commands from you will not be processed.\n");
    write_to_fifo(PATH_TO_PROGRAM_PIPES_BASE, client_name,  message);
};