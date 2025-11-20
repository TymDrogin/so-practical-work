#include "controller.h"



void free_request(void* item) {
    request* r = (request*)item;
    free(r->destination);
    free(r);
}


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



controller* create_controller(const int max_num_of_user_sessions,
                              const int max_num_of_vehicles,
                              const int max_num_of_services)
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
    c->sessions = d_create_array(max_num_of_user_sessions, free_client_session);
    c->vehicles = d_create_array(max_num_of_vehicles, NULL);
    c->services = d_create_array(max_num_of_services, NULL);
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

    c->total_km_traveld_by_all_cars = 0;

    return c;
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





