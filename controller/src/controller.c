#include "controller.h"



void free_request(void* item) {
    request* r = (request*)item;
    free(r->destination);
    free(r);
}





// CLIENT SESSION FUNCTIONS 
client_session* create_client_session(id_generator* g,
                                      const char* client_name,
                                      const int max_num_of_requests)
{
    if (client_name == NULL || max_num_of_requests <= 0) {
        perror(ERROR "Invalid args passed to create_client_session");
        exit(EXIT_FAILURE);
    }

    client_session* s = malloc(sizeof(client_session));
    if (s == NULL) {
        perror(ERROR "Could not allocate memory for the client session");
        exit(EXIT_FAILURE);
    }

    s->id = generate_id(g);

    // Store client name
    s->client_name = strdup(client_name);
    if (s->client_name == NULL) {
        perror(ERROR "Could not duplicate client name");
        exit(EXIT_FAILURE);
    }

    // Build pipe name: "<client>_to_controller"
    char pipe_name_buf[128];
    snprintf(pipe_name_buf, sizeof(pipe_name_buf),
             "%s_to_controller", client_name);

    s->client_to_controller_pipe = strdup(pipe_name_buf);
    if (s->client_to_controller_pipe == NULL) {
        perror(ERROR "Could not duplicate pipe name");
        exit(EXIT_FAILURE);
    }

    // Initialize request array with max slots
    s->requests = d_create_array(max_num_of_requests, free_request);

    s->is_active = 0;

    return s;
}  
void free_client_session(void* item) {
    client_session* s = (client_session*)item;
    if(s == NULL) {
        perror(ERROR "Attemted to free the client session, passed null pointer");
        exit(EXIT_FAILURE);
    }

    free(s->client_name);
    free(s->client_to_controller_pipe);
    d_destroy_array(s->requests);
    free(s);
    return;
}

// CONTROLLER FUNCTIONS

controller* create_controller(const int max_num_of_user_sessions,
                              const int max_num_of_vehicles,
                              const int max_num_of_services,
                              const int max_num_of_requests_per_user)
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
    c->services = a_create_array(max_num_of_services, NULL);
    if (!c->sessions || !c->vehicles || !c->services) {
        perror(ERROR "Failed to allocate controller arrays");
        exit(EXIT_FAILURE);
    }

    // Initialize ID generators
    c->vehicle_id_gen = malloc(sizeof(id_generator));
    c->session_id_gen = malloc(sizeof(id_generator));
    c->request_id_gen = malloc(sizeof(id_generator));

    if (!c->vehicle_id_gen || !c->session_id_gen || !c->request_id_gen) {
        perror(ERROR "Failed to allocate id_generators");
        exit(EXIT_FAILURE);
    }

    init_id_generator(c->vehicle_id_gen);
    init_id_generator(c->session_id_gen);
    init_id_generator(c->request_id_gen);

    c->max_num_of_sessions  = max_num_of_user_sessions;
    c->max_num_of_vehicles  = max_num_of_vehicles;
    c->max_num_of_services  = max_num_of_services;
    c->max_num_of_requests_per_user = max_num_of_requests_per_user;

    c->total_km_traveld_by_all_cars = 0;

    return c;
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
    if (!is_named_pipe_exists(client_name, PATH_TO_PROGRAM_PIPES_BASE)) {
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
        client_name,
        c->max_num_of_requests_per_user
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

void start_client_session(client_session* s) {
    if (s == NULL) {
        perror(ERROR "start_client_session: client session is NULL");
        exit(EXIT_FAILURE);
    }
    // Create the client pipe
    create_named_pipe(s->client_to_controller_pipe, PATH_TO_PROGRAM_PIPES_BASE);
    s->is_active = true;
    message_client_connection_accepted(s->client_name);
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
            remove_named_pipe(s->client_to_controller_pipe, PATH_TO_PROGRAM_PIPES_BASE);
            free_client_session(s);
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

    int count = d_size(c->sessions);
    for (int i = 0; i < count; i++) {
        client_session* s = d_get(c->sessions, i);
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


void process_clients_connection_requests(controller* c, queue* connection_request_queue) {
    char* client_name;
    while ((client_name = (char*)q_dequeue(connection_request_queue)) != NULL) {
        connect_client(c, client_name);
        free(client_name);
    }
}
void print_all_connected_clients(controller* c) {
    if (c == NULL) {
        fprintf(stderr, ERROR "print_all_connected_clients: controller is NULL\n");
        exit(EXIT_FAILURE);
    }

    int count = a_size(c->sessions);
    printf(CONTROLLER "Connected clients (%d):\n", count);
    for (int i = 0; i < count; i++) {
        client_session* s = (client_session*)a_get(c->sessions, i);
        if(s == NULL) {
            perror(ERROR "print_all_connected_clients: Read null session");
            exit(EXIT_FAILURE);
        }
        printf("  • Client: %s\n", s->client_name);
        printf("    ID: %d\n", s->id);
        printf("    Pipe: %s\n", s->client_to_controller_pipe);
        printf("    Active: %s\n", s->is_active ? "yes" : "no");
    }
}

// MESSAGES TO CLIENTS AND CONTROLLER ADMINISTRATOR
void message_client_connection_accepted(const char* client_name) {
    char message[128];
    snprintf(message, sizeof(message), CONTROLLER "Connection request accepted, you are now connected to the system.\n");
    write_to_fifo(client_name, PATH_TO_PROGRAM_PIPES_BASE, message);
}
void message_client_connection_rejected(const char* client_name, const char* reason) {
    char message[256];
    snprintf(message, sizeof(message), CONTROLLER "Connection request rejected: %s\n", reason);
    write_to_fifo(client_name, PATH_TO_PROGRAM_PIPES_BASE, message);
}






