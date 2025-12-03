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
void free_request(void* item) {
    if (item == NULL) return;

    request* r = (request*)item;
    free(r->destination);
    free(r);
}





// changes the flags, car must be created for
void serve_request(controller* c, request* r) {
    if (r == NULL) {
        perror(ERROR "serve_request: Request is NULL");
        exit(EXIT_FAILURE);
    }
    if (r == NULL) {
        perror(ERROR "serve_request: Controller is NULL");
        exit(EXIT_FAILURE);
    }

    if(r->is_active) {
        perror(ERROR "serve_request: Cannot serve this request, as it has already been started");
        return;
    }

    client_session* s = get_client_session_by_id(c, r->client_session_id);
    if(s == NULL) {
        perror(ERROR "serve_request: Can't serve this request as the client_session is NULL");
        return;
    }
    if(s->has_active_request) {
        fprintf(stderr, ERROR "Client with id:%d has already a service in action", s->id);
        return;
    }

    if(a_is_full(c->vehicles)) {
        perror(ERROR "serve_request: Cannot serve this request, no available vehicles");
        return;
    }

    vehicle_t* v = create_vehicle(c->vehicle_id_gen, s->client_name, r->destination, r->distance_to_travel);
    if(a_push(c->vehicles, v) != 0) {
        perror(ERROR "serve_request: Can't push the vehicle to the array");
        destroy_vehicle(v);
        return;
    }

    start_vehicle_service(v);

    s->has_active_request = true;

    r->is_active = true;
    r->is_comleted = false;
    r->vehicle_id = v->pid;
    return;
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
    c->sessions = a_create_array(max_num_of_user_sessions, NULL);
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

    // Connection part
    create_named_pipe(PATH_TO_PROGRAM_PIPES_BASE, s->client_to_controller_pipe);

    s->is_active = true;

    // Message success
    message_client_connection_accepted(s->client_name);


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
bool is_client_connected_by_id(const controller* c, const int id) {
    if (c == NULL) {
        fprintf(stderr, ERROR "is_client_connected_by_name: controller is NULL\n");
        exit(EXIT_FAILURE);
    }

    if (id <= 0) {
        fprintf(stderr, ERROR "is_client_connected_by_id: id is <= 0\n");
        exit(EXIT_FAILURE);
    }

    if (c->sessions == NULL) {
        fprintf(stderr, ERROR "is_client_connected_by_name: controller->sessions is NULL\n");
        exit(EXIT_FAILURE);
    }
    int count = a_size(c->sessions);
    for(int i = 0; i < count; i++) {
        client_session* s = a_get(c->sessions, i);
        if(s == NULL) {
            perror(ERROR "is_client_connected_by_id: a_get got NULL session");
            exit(EXIT_FAILURE);
        }
        if(s->id == id) {
            return true;
        }
    }
    return false;


}


client_session* get_client_session_by_name(const controller* c, const char* client_name) {
    if (c == NULL) {
        fprintf(stderr, ERROR "get_client_session_by_name: controller is NULL\n");
        exit(EXIT_FAILURE);
    }

    if (client_name == NULL) {
        fprintf(stderr, ERROR "get_client_session_by_name: client_name is NULL\n");
        exit(EXIT_FAILURE);
    }
    // Find the session by client name
    int count = a_size(c->sessions);
    for (int i = 0; i < count; i++) {
        client_session* s = a_get(c->sessions, i);
        if(s == NULL) {
            perror(ERROR "get_client_session_by_name: Read null session");
            exit(EXIT_FAILURE);
        }
        if(s->client_name == NULL) {
            perror(ERROR "get_client_session_by_name: Session client name is NULL");
            exit(EXIT_FAILURE);
        }

        if (strcmp(s->client_name, client_name) == 0) {
            return s;
        }
    }
    return NULL;

}
client_session* get_client_session_by_id(const controller* c, const int id) {
    if (c == NULL) {
        fprintf(stderr, ERROR "get_client_session_by_id: controller is NULL\n");
        exit(EXIT_FAILURE);
    }

    if (id <= 0) {
        fprintf(stderr, ERROR "get_client_session_by_id: client_name is NULL\n");
        exit(EXIT_FAILURE);
    }
    // Find the session by client name
    int count = a_size(c->sessions);
    for (int i = 0; i < count; i++) {
        client_session* s = a_get(c->sessions, i);
        if(s == NULL) {
            perror(ERROR "get_client_session_by_id: Read null session");
            exit(EXIT_FAILURE);
        }

        if (s->id == id) {
            return s;
        }
    }

    return NULL;
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

// CLIENT COMMANDS 
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

    // agendar <start_time> <destination> <distance> 
    if(strncmp(line, "agendar", 7 ) == 0) {
        int start_time;
        char destination[64];
        int distance;

        int n = sscanf(line, "agendar %d %63s %d", &start_time, destination, &distance);
        if(n != 3) {
            fprintf(stderr, ERROR "Invalid command format for 'agendar'. Usage: agendar <start_time> <destination> <distance>\n");
            return;
        }   
        agendar(c, s, start_time, destination, distance);
        return;
    }

    // Cancelar

    // consultar

    // terminar 
    



    // Notify client about unknown command
    return;
}
void agendar(controller* c, client_session* s, int start_time, const char* destination, int distance) {
    if (c == NULL) {
        fprintf(stderr, ERROR "agendar: controller is NULL\n");
        exit(EXIT_FAILURE);
    }

    if (s == NULL) {
        fprintf(stderr, ERROR "agendar: client_session is NULL\n");
        exit(EXIT_FAILURE);
    }

    if (destination == NULL) {
        fprintf(stderr, ERROR "agendar: destination is NULL\n");
        exit(EXIT_FAILURE);
    }

    // Check if there are available request slots
    if (a_is_full(c->request)) {
        message_client_request_creation_rejected(s->client_name, "Can't place your request now, system is at full capacity.");
        return;
    }

    // Create new request
    request* r = create_request(
        c->request_id_gen,
        s->id,
        destination,
        distance,
        start_time,
        get_timer_ticks()
    );

    // Add request to controller's requests array
    if (a_push(c->request, r) != 0) {
        message_client_request_creation_rejected(s->client_name, "Failed to create your request due to internal error.");
        free_request(r);
        return;
    }

    // Notify client about successful request creation
    message_client_request_creation_accepted(s->client_name, r->id);
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

void message_client_request_creation_accepted(const char* client_name, int request_id) {
    char message[128];
    snprintf(message, sizeof(message), CONTROLLER "Your request has been created successfully. Request ID: %d\n", request_id);
    write_to_fifo(PATH_TO_PROGRAM_PIPES_BASE, client_name,  message);
}

// Message client can't create request with reason
void message_client_request_creation_rejected(const char* client_name, const char* reason) {
    char message[256];
    snprintf(message, sizeof(message), CONTROLLER "Request creation rejected: %s\n", reason);
    write_to_fifo(PATH_TO_PROGRAM_PIPES_BASE, client_name,  message);
}