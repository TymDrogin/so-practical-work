#include "controller.h"



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
    // IS SET BY VEHICLE DURING MESSAGE READ
    r->is_completed = false;
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
    if (c == NULL) {
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

    // Session flag
    s->has_active_request = true;

    r->is_active = true;
    r->is_completed = false;
    r->vehicle_id = v->id;
    return;
}
void terminate_request(controller* c, request* r) {
    if(r->is_active) {
        vehicle_t* v = find_vehicle_by_id(c, r->vehicle_id);
        if(v == NULL) {
            perror(ERROR "Attempted to terminate request marked as active, with no vehicle avaible");
            return;
        }
        // Kills vehicle process 
        cancel_vehicle_service(v);
    }
    r->is_completed = true;
}




request* find_request_by_id(const controller* c, int id) {
    if(c == NULL) {
        perror(ERROR "find_request_by_id: Controller is NULL");
        exit(EXIT_FAILURE);
    }
    int size = a_size(c->request);
    
    for(int i = 0; i < size; i++) {
        request* r = (request*)a_get(c->request, i);
        if(r == NULL) continue;
        if(r->id != id) continue;
        return r;
        
    }
    return NULL;
}
request* find_request_by_vehicle_id(const controller* c, int vehicle_id) {
    if(c == NULL) {
        perror(ERROR "find_request_by_id: Controller is NULL");
        exit(EXIT_FAILURE);
    }
    int size = a_size(c->request);
    
    for(int i = 0; i < size; i++) {
        request* r = (request*)a_get(c->request, i);
        if(r == NULL) continue;
        if(r->vehicle_id != vehicle_id) continue;
        return r;
        
    }
    return NULL;
}
request* find_most_recent_inactive_request_for_client(const controller* c, const client_session* s) {
    if(c == NULL) {
        perror(ERROR "find_most_recent_inactive_request_for_client: Controller is NULL");
        exit(EXIT_FAILURE);
    }
    if(s == NULL) {
        perror(ERROR "find_most_recent_inactive_request_for_client: Controller is NULL");
        exit(EXIT_FAILURE);
    }


    request* most_recent_request = NULL;
    int most_recent_time = -1;

    int size = a_size(c->request);
    for(int i = 0; i < size; i++) {
        request* current_req = (request*)a_get(c->request, i);
        if(current_req == NULL) continue;
        if(current_req->client_session_id != s->id) continue;
        if(current_req->is_active) continue;

        // Update the newer request
        if(current_req->time_created > most_recent_time) {
            most_recent_time = current_req->time_created;
            most_recent_request = current_req;
        }
    }
    return most_recent_request;
}


// FIX WITH TERMINATE
void delete_request_by_id(controller* c, int id) {
    if (c == NULL) {
        fprintf(stderr, ERROR "delete_all_inactive_requests_for_client: controller is NULL\n");
        exit(EXIT_FAILURE);
    }
    if(id < 0) {
        fprintf(stderr, ERROR "delete_all_inactive_requests_for_client: id is less then 0.\n");
        exit(EXIT_FAILURE);

    }

    request* r = find_request_by_id(c, id);
    if(r == NULL) {
        perror("Can't delete request by id, as found none requests with this id");
        return;
    }
    client_session* s = get_client_session_by_id(c, r->client_session_id);
    if(s == NULL) {
        perror("Found orphaned request");
        exit(EXIT_FAILURE);
    }

    if(r->is_active) {
        delete_active_request_for_client(c, s);
    } else {
        delete_inactive_request_for_client_by_id(c, s, id);
    }

}
void delete_all_inactive_requests_for_client(controller* c, client_session* s) {
    if (c == NULL) {
        fprintf(stderr, ERROR "delete_all_inactive_requests_for_client: controller is NULL\n");
        exit(EXIT_FAILURE);
    }

    if (s == NULL) {
        fprintf(stderr, ERROR "delete_all_inactive_requests_for_client: client_session is NULL\n");
        exit(EXIT_FAILURE);
    }
    // Backwards iterating to avoid index shifting issues

    int found = false;
    
    int size = a_size(c->request);
    for(int i = size - 1; i >= 0; i--) {
        request* curr_request = (request*)a_get(c->request, i);
        if(curr_request == NULL) continue;
        if(curr_request->client_session_id != s->id) continue;
        if(curr_request->is_active) continue;

        found = true;
        
        // remove request
        message_client_request_deleted(s, curr_request);

        // TERMINATE
        a_remove(c->request, i);
    }
    if(!found) {
        message_client_no_requests_found(s);
    }

}
// FIX 
void delete_inactive_request_for_client_by_id(controller* c, client_session* s, int request_id) {
    if (c == NULL) {
        fprintf(stderr, ERROR "delete_inactive_request_for_client_by_id: controller is NULL\n");
        exit(EXIT_FAILURE);
    }

    if (s == NULL) {
        fprintf(stderr, ERROR "delete_inactive_request_for_client_by_id: client_session is NULL\n");
        exit(EXIT_FAILURE);
    }

    bool found = false;
    int size = a_size(c->request);
    
    for(int i = 0; i < size; i++) {
        request* r = (request*)a_get(c->request, i);
        if(r == NULL) continue;
        if(r->id != request_id) continue;
        if(r->client_session_id != s->id) continue;

        if(r->is_active) {
            message_client_error_occured(s, "Can't delete your request as it is active");
            return;
        }
        found = true;
        message_client_request_deleted(s, r);

        // TERMINATE
        a_remove(c->request, i);
        break;
    }
    if(!found) {
        message_client_no_requests_found(s);
    }
}


void delete_most_recent_inactive_request_for_client(controller* c, client_session* s) {
    if (c == NULL) {
        fprintf(stderr, ERROR "delete_most_recent_inactive_request_for_client: controller is NULL\n");
        exit(EXIT_FAILURE);
    }

    if (s == NULL) {
        fprintf(stderr, ERROR "delete_most_recent_inactive_request_for_client: client_session is NULL\n");
        exit(EXIT_FAILURE);
    }
    request* r = find_most_recent_inactive_request_for_client(c, s);
    if (r == NULL) {
        message_client_no_requests_found(s);
        return;
    }
    delete_inactive_request_for_client_by_id(c, s, r->id);
}
void delete_active_request_for_client(controller* c, client_session* s) {
    if (c == NULL) {
        fprintf(stderr, ERROR "delete_most_recent_inactive_request_for_client: controller is NULL\n");
        exit(EXIT_FAILURE);
    }

    if (s == NULL) {
        fprintf(stderr, ERROR "delete_most_recent_inactive_request_for_client: client_session is NULL\n");
        exit(EXIT_FAILURE);
    }

    int size = a_size(c->request);
    for(int i = 0; i < size; i++) {
        request* r = a_get(c->request, i);
        if(r == NULL) continue;
        if(r->client_session_id != s->id) continue;
        if(!r->is_active) continue;

        terminate_request(c, r);
        message_client_request_deleted(s, r);
        s->has_active_request = false;
        a_remove(c->request, i);
        break;
    }
}




// 
vehicle_t* find_vehicle_by_id(controller* c, int id) {
    int size = a_size(c->vehicles);
    for(int i = 0; i < size; i++) {
        vehicle_t* v = (vehicle_t*)a_get(c->vehicles, i);
        if(v == NULL) continue;
        if(v->id == id) {
            return v;
        }
    }
    return NULL;
}







// CONTROLLER FUNCTIONS

controller* create_controller(const int max_num_of_user_sessions,
                              const int max_num_of_vehicles,
                              const int max_num_of_requests)
{
    if (max_num_of_user_sessions <= 0 || max_num_of_vehicles <= 0 || max_num_of_requests <= 0) {
        perror(ERROR "Invalid controller limits\n");
        exit(EXIT_FAILURE);
    }

    controller* c = malloc(sizeof(controller));
    if (c == NULL) {
        perror(ERROR "Could not allocate memory for controller");
        exit(EXIT_FAILURE);
    };


    // Initialize arrays
    // TODO: 
    c->sessions = a_create_array(max_num_of_user_sessions, free_client_session);
    c->vehicles = a_create_array(max_num_of_vehicles, free_vehicle);
    c->request = a_create_array(max_num_of_requests, free_request);

    // Initialize ID generators
    c->vehicle_id_gen = malloc(sizeof(id_generator));
    c->session_id_gen = malloc(sizeof(id_generator));
    c->request_id_gen = malloc(sizeof(id_generator));

    init_id_generator(c->vehicle_id_gen);
    init_id_generator(c->session_id_gen);
    init_id_generator(c->request_id_gen);

    generate_id(c->vehicle_id_gen);
    generate_id(c->session_id_gen);
    generate_id(c->request_id_gen);

    c->max_num_of_sessions  = max_num_of_user_sessions;
    c->max_num_of_vehicles  = max_num_of_vehicles;
    c->max_num_of_requests  = max_num_of_requests;

    create_named_pipe(PATH_TO_PROGRAM_PIPES_BASE, CONTROLLER_CONNECTION_PIPE_NAME);

    c->total_km_traveld_by_all_cars = 0;

    return c;
}

void controller_termination_routine(controller* c) {
    remove_named_pipe(PATH_TO_PROGRAM_PIPES_BASE, CONTROLLER_CONNECTION_PIPE_NAME);

    disconnect_all_clients(c);
    
    printf(CONTROLLER "Cleanup complete, exiting.\n");
    exit(0);
}




// CONTROLLER LOGIC
// Deletes completed request
void clear_completed_requests(controller* c) {
    int request_num = a_size(c->request);
    for(int i = request_num - 1; i >= 0; i--) {
        request* r = a_get(c->request, i);
        if(r==NULL) {
            perror(ERROR "dispatch_cars: request is null");
            continue;
        }

        client_session* s = get_client_session_by_id(c, r->client_session_id);
        if(s==NULL) {
            perror(ERROR "clean_requests: sessions is NULL");
            a_remove(c->request, i);
            continue;
        }
        if(r->is_completed) {
            a_remove(c->request, i);
            s->has_active_request = false;
            continue;
        }
    }
}

void dispatch_cars(controller* c) {
    int request_num = a_size(c->request);;

    for(int i = 0; i < request_num; i++) {
        request* r = a_get(c->request, i);
        if(r==NULL) {
            perror(ERROR "dispatch_cars: request is null");
            continue;
        }

        client_session* s = get_client_session_by_id(c, r->client_session_id);
        if(s==NULL) {
            perror(ERROR "dispatch_cars: sessions is NULL");
            continue;
        }

        if(!r->is_active && !s->has_active_request) {
            serve_request(c, r);
        }
    }
}
void clear_cars_that_completed_request(controller* c) {
    int cars_num = a_size(c->vehicles);

    for (int i = cars_num - 1; i >= 0; i--) {
        vehicle_t* v = (vehicle_t*)a_get(c->vehicles, i);
        if (v == NULL) {
            fprintf(stderr, ERROR "clear_cars_that_completed_request: vehicle is NULL\n");
            continue;
        }
        if(!v->is_alive) {
            cancel_vehicle_service(v);
            a_remove(c->vehicles, i);
        }
    }
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
        client_session* s = (client_session*)a_get(c->sessions, i);
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
        client_session* s = (client_session*)a_get(c->sessions, i);
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

    if (id < 0) {
        fprintf(stderr, ERROR "get_client_session_by_id: id is less then 0;\n");
        exit(EXIT_FAILURE);
    }
    // Find the session by client name
    int count = a_size(c->sessions);
    for (int i = 0; i < count; i++) {
        client_session* s = a_get(c->sessions, i);
        if(s == NULL) {
            perror(ERROR "get_client_session_by_id: Read null session\n");
            exit(EXIT_FAILURE);
        }

        if (s->id == id) {
            return s;
        }
    }

    return NULL;
}
// FIX
bool connect_client(controller* c, const char* client_name) {
    if (c == NULL) {
        fprintf(stderr, ERROR "connect_client: controller is NULL\n");
        exit(EXIT_FAILURE);
    }
    if (client_name == NULL) {
        fprintf(stderr, ERROR "connect_client: client_name is NULL\n");
        exit(EXIT_FAILURE);
    }

    // Check if client with this name is already connected
    if (is_client_connected_by_name(c, client_name)) {
        message_client_connection_rejected(client_name, "Client with this name is already connected.");
        message_controller_client_connection_denied(client_name, "Client with this name is already connected to the system.");
        return false;
    }

    // Check if the client pipe exists
    if (!is_named_pipe_exists(PATH_TO_PROGRAM_PIPES_BASE, client_name)) {
        message_controller_client_connection_denied(client_name, "Client with this name has no pipe.");
        return false;
    }

    // Check for available session slot
    if (a_is_full(c->sessions)) {
        message_client_connection_rejected(client_name, "No avaible connections slots.");
        message_controller_client_connection_denied(client_name, "No available session slots in the system.");
        return false;
    }

    // --  Session createtion  -- 

    client_session* s = create_client_session(c->session_id_gen, client_name);


    // creating <client>_to_controller
    if (mkfifo(s->client_to_controller_pipe_path, 0666) == -1 && errno != EEXIST) {
        perror(ERROR "mkfifo");
        free_client_session(s);
        return false;
    }

    // Opening <client>_to_controller (controller reads commands from here)
    s->client_to_controller_pipe_fd = open(s->client_to_controller_pipe_path, O_RDONLY | O_NONBLOCK);
    if (s->client_to_controller_pipe_fd == -1) {
        perror(ERROR "open client->controller");
        free_client_session(s);
        return false;
    }

    // Opening <client>  (Controller sends here messages)
    s->controller_to_client_pipe_fd = open(s->controller_to_client_pipe_path, O_WRONLY);
    if (s->controller_to_client_pipe_fd == -1) {
        perror(ERROR "open controller->client");
        close(s->client_to_controller_pipe_fd);
        free_client_session(s);
        return false;
    }


    // Add session to controller's sessions array
    // a_push should never fail here because we already checked for capacity
    if (a_push(c->sessions, s) != 0) {
        message_client_connection_rejected(s->client_name, "Failed to create a new session.");
        message_controller_client_connection_denied(s->client_name, "Failed to push the session to the array");
        close(s->client_to_controller_pipe_fd);
        close(s->controller_to_client_pipe_fd);

        unlink(s->client_to_controller_pipe_path);
        free_client_session(s);
        return false;
    }

    

    s->is_active = true;

    // Message success
    message_client_connection_accepted(s);
    message_controller_client_connected(s);

    fflush(stdout);

    return true;
}
void serve_connection_requests(controller* c, queue* client_connection_req_queue) {
    char* client_name;

    while((client_name = q_dequeue(client_connection_req_queue)) != NULL) {
        connect_client(c, client_name);
        free(client_name);
    }
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
            terminate_client_session(c, s);
            a_remove(c->sessions, i);
            return;
        }
    }
    // Client not found
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
    s->is_active = false;

    // Delete inactive requests
    message_client_disconnection_notice(s);

    // Remove pipe
    close(s->client_to_controller_pipe_fd);
    unlink(s->client_to_controller_pipe_path);

    close(s->controller_to_client_pipe_fd);

    delete_all_inactive_requests_for_client(c, s);
    delete_active_request_for_client(c, s);


    return;
}

void disconnect_all_clients(controller* c) {
    int session_num = a_size(c->sessions);
    for(int i = session_num - 1; i >= 0; i--) {
        client_session* s = a_get(c->sessions, i);
        if(s==NULL) continue;

        terminate_client_session(c, s);
        a_remove(c->sessions, i);
    }
}




// MISC


// Vehcile messages
void process_vehicle_message(controller* c, vehicle_t* v, const char* message){
    if(c == NULL) {
        perror(ERROR "process_vehicle_message: controller is null\n");
        exit(EXIT_FAILURE);
    }
    if(v == NULL) {
        perror(ERROR "process_vehicle_message: vehicle is is null\n");
        exit(EXIT_FAILURE);
    }
    if(c == NULL) {
        perror(ERROR "process_vehicle_message: controller is null\n");
        exit(EXIT_FAILURE);
    }


    request* r = find_request_by_vehicle_id(c, v->id);
    if(r == NULL) {
        perror(ERROR "process_vehicle_message: Request is null, no assosiated request with this vehicle.");
        exit(EXIT_FAILURE);
    }

    client_session* s = get_client_session_by_name(c, v->client_name);

    if(s == NULL) {
        perror(ERROR "process_vehicle_messages: No client session assosiated with this vehicle");
        v->is_alive = false;
        return;
    }


    if(strncmp(message, "INFO=ARRIVED", 12) == 0) {
        v->is_alive = true;
        return;
    }
    if(strncmp(message, "INFO=START", 10) == 0) {
        r->is_active = true;
        s->has_active_request = true;
        return;
    }
    if(strncmp(message, "INFO=TERMINATED", 15) == 0) {

        // User has no active requests
        s->has_active_request = false;

        // REQUEST IS MARKED AS COMPLETED
        r->is_completed = true;


        // Mark vehicle as dead
        v->is_alive = false;
        return;

    }
    int tenth_of_distance;
    if(sscanf(message, "DISTANCE=%d", &tenth_of_distance) == 1) {
        v->distance_traveled += tenth_of_distance;
        c->total_km_traveld_by_all_cars += tenth_of_distance;
        return;
    }
    perror(ERROR "Unknown vehicle message processed");
    exit(EXIT_FAILURE);

}
void process_vehicle_messages(controller* c, vehicle_t* v, queue* messages) {
    if(c == NULL) {
        perror(ERROR "process_vehicle_messages: Controller is NULL");
        exit(EXIT_FAILURE);
    }
    if(v == NULL) {
        perror(ERROR "process_vehicle_messages: Vehicle is NULL");
        exit(EXIT_FAILURE);
    }
    if(messages == NULL) {
        perror(ERROR "process_vehicle_messages: message queue is NULL");
        exit(EXIT_FAILURE);
    }
    char* message = NULL;
    while((message = (char*)q_dequeue(messages)) != NULL) {
        process_vehicle_message(c, v, message);
        free(message);
    };
}
void process_all_vehicles_messages(controller* c) {

    int size = a_size(c->vehicles);
    for(int i = 0; i < size; i++) {
        vehicle_t* v = a_get(c->vehicles, i);
        
        queue* messages = q_create_queue(14, false, NULL);
        read_vehicle_messages(v, messages);
        process_vehicle_messages(c, v, messages);

    }
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
        listar(c);
        return;

    }
    if(strcmp(cmd, "utiliz") == 0) {
        utiliz(c);
        return;
    }
    if(strcmp(cmd, "frota") == 0) {
        frota(c);
        return;
    }
    if(strcmp(cmd, "km") == 0) {
        // Prints total km traveled by all vehicles
        printf(CONTROLLER "Distance traveled by all vehicles: %d", c->total_km_traveld_by_all_cars);
        return;
    }
    if(strcmp(cmd, "hora") == 0) {
        // Prints current controller time
        printf(CONTROLLER "Current time:%d", get_timer_ticks());
        return;
    }
    if(strcmp("terminar", cmd) == 0) {
        kill(getpid(), SIGINT);
        return;
    }

    if(strcmp(cmd, "cancelar") == 0) {
        if(n == 2 && id >= 0) {
            if(id == 0) {
                int size = a_size(c->sessions);
                for(int i = 0; i < size; i++) {
                    client_session* s = a_get(c->sessions, i);
                    if(s == NULL) continue;
                    delete_all_inactive_requests_for_client(c, s);
                    delete_active_request_for_client(c, s);
                }
            } else {
                delete_request_by_id(c, id);
            }
            return;
        } else {
            fprintf(stderr, ERROR "Invalid command format for 'cancelar'. Usage: cancelar <service_id>\n");
            return;
        }
    }
    printf("Unknown command \n");

    // Notify admin about unknown command
}

void listar(controller* c) {
    int size = a_size(c->request);
    printf("List of services: \n");
    for(int i = 0; i < size; i++) {
        request* r = a_get(c->request, i);
        if(r==NULL) continue;
        printf("Request ID: %d, is active: %d, is completed: %d, client ID: %d, vehicle id: %d \n",
                r->id, r->is_active, r->is_completed, r->client_session_id, r->vehicle_id);

    }
}
void utiliz(controller* c) {
    int size = a_size(c->sessions);
    printf("List of connected clients: \n");
    for(int i = 0; i < size; i++) {
        client_session* s = a_get(c->sessions, i);
        if(s==NULL) continue;
        printf("Client: %s, ID: %d, has active service: %d", s->client_name, s->id, s->has_active_request);

    }
}

void frota(controller* c) {
    int size = a_size(c->vehicles);
    printf("List of vehicles: \n");
    for(int i = 0; i < size; i++) {
        vehicle_t* v = a_get(c->vehicles, i);
        if(v==NULL) continue;

        float distance_traveleded = (float)v->distance_traveled / (float)v->distance_to_travel;
        printf("Vehicle ID: %d, destination: %s, client name: %s, trip traveled percentage: %f", v->id, v->destination, v->client_name, distance_traveleded);

    }
}










// FIX
// CLIENT COMMANDS 
bool read_client_commands_sent(const client_session* s, queue* q) {
    if(s == NULL) {
        perror(ERROR "read_client_commands_sent: Session is NULL");
        exit(EXIT_FAILURE);
    }
    if(q == NULL) {
        perror(ERROR "read_client_commands_sent: Queue is NULL");
    }
    if(s->client_to_controller_pipe_fd == -1) {
        perror(ERROR "read_client_commands_sent: Client to controller pipe fd is invalid");
    }

    char buffer[512];

    ssize_t bytes_read = read(s->client_to_controller_pipe_fd, buffer, sizeof(buffer) - 1);

    if (bytes_read < 0) {
        if (errno == EINTR || errno == EAGAIN) {
            // No data available right now
            return false;
        }
        fprintf(stderr, "ERROR read_client_commands_sent: %s\n", strerror(errno));
        return false;
    }

    if (bytes_read == 0) {
        // Writer closed the pipe (client terminated)
        // You can mark the session inactive somewhere
        // s->is_active = false;   // if s is not const
        return false;
    }

    // bytes_read > 0
    buffer[bytes_read] = '\0';
    read_lines_from_buffer_to_queue(q, buffer, bytes_read);

    return true;
}
// FIX
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


    if(strncmp(line, "cancelar", 8) == 0) {
        int id;
        if(sscanf(line, "cancelar %d", &id) == 1) {
            c_cancelar(c, s, id);
        } else {
            c_cancelar(c, s, -1);
        };
        return;     
    }

    if(strcmp(line, "consultar") == 0) {
        consultar(c, s);
        return;
    }

    // terminar 
    if(strcmp(line, "terminar") == 0){
        // TErminates client session
        disconnect_client_by_name(c, s->client_name);
        // Termination routine

        return;

    }
    return;
}


void process_client_commands(controller* c, client_session* s) {
    if (c == NULL || s == NULL) {
        fprintf(stderr, "ERROR process_client_command: Controller or Session is NULL\n");
        exit(EXIT_FAILURE);
    }

    queue* q = q_create_queue(10, false, NULL);
    if (q == NULL) {
        fprintf(stderr, "ERROR: failed to allocate queue\n");
        exit(EXIT_FAILURE);
    }

    bool has_data = read_client_commands_sent(s, q);

    if (has_data) {
        char* cmd = NULL;

        while ((cmd = q_dequeue(q)) != NULL) {
            process_client_command(c, s, cmd);
            free(cmd);
        }
    }

    q_destroy_queue(q);
}
void process_all_clients_commands(controller* c) {
    if(c == NULL) {
        perror(ERROR "process_all_client_commands: Controller is NULL");
        exit(EXIT_FAILURE);
    };

    int size = a_size(c->sessions);
    for(int i = 0; i < size; i++) {
        client_session* s = (client_session*)a_get(c->sessions, i);
        if(s != NULL && s->is_active) {
            process_client_commands(c, s);
        }
    }
}

// FIX
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
        message_client_request_creation_rejected(s, "Can't place your request now, system is at full capacity.");
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
        message_client_request_creation_rejected(s, "Failed to create your request due to internal error.");
        free_request(r);
        return;
    }

    // Notify client about successful request creation
    message_client_request_creation_accepted(s, r->id);
}

void consultar(const controller* c, const client_session* s) {
    if(s == NULL || c == NULL) {
        perror(ERROR "consultar: Got null");
        exit(0);
    }


    bool found = false;
    request* r = NULL;
    
    int size = a_size(c->request);
    for(int i = 0; i < size; i++) {
        r = (request*)a_get(c->request, i);
        if(r == NULL) continue;

        if(r->client_session_id == s->id) {
            if(!found) {
                message_client_requests_found(s);
            }
            found = true;
            message_client_request_info(s, r);
        }
    };
    if(!found) {
        message_client_no_requests_found(s);
    }
}

void c_cancelar(controller* c, client_session* s, int id) {
    if (c == NULL) {
        fprintf(stderr, ERROR "agendar: controller is NULL\n");
        exit(EXIT_FAILURE);
    }

    if (s == NULL) {
        fprintf(stderr, ERROR "agendar: client_session is NULL\n");
        exit(EXIT_FAILURE);
    }

    if(id < 0) {
        delete_most_recent_inactive_request_for_client(c, s);
        return;
    } else if(id == 0) {
        delete_all_inactive_requests_for_client(c, s);
        return;
    } else {
        delete_inactive_request_for_client_by_id(c, s, id);
        return;
    }
}


// MESSAGES TO CLIENTS AND CONTROLLER ADMINISTRATOR
// FIX to the session 
// -- messages using session FD --

void message_client_connection_accepted(client_session* s) {
    if (!s) return;
    char message[128];
    snprintf(message, sizeof(message),
             CONTROLLER "Connection request accepted, you are now connected to the system.\n");
    write(s->controller_to_client_pipe_fd, message, strlen(message));
}
void message_client_disconnection_notice(client_session* s) {
    if (!s) return;
    char message[128];
    snprintf(message, sizeof(message),
             CONTROLLER "You have been disconnected from the system. Further commands from you will not be processed.\n");
    write(s->controller_to_client_pipe_fd, message, strlen(message));
}
void message_client_request_creation_accepted(client_session* s, int request_id) {
    if (!s) return;
    char message[128];
    snprintf(message, sizeof(message),
             CONTROLLER "Your request has been created successfully. Request ID: %d\n", request_id);
    write(s->controller_to_client_pipe_fd, message, strlen(message));
}
void message_client_request_creation_rejected(client_session* s, const char* reason) {
    if (!s) return;
    char message[256];
    snprintf(message, sizeof(message),
             CONTROLLER "Request creation rejected: %s\n", reason);
    write(s->controller_to_client_pipe_fd, message, strlen(message));
}
void message_client_request_deleted(client_session* s, request* r) {
    if(!s || !r) return;
        char message[256];
    snprintf(message, sizeof(message),
             CONTROLLER "Your request to: %s, with id: %d has been deleted. \n", r->destination, r->id);
    write(s->controller_to_client_pipe_fd, message, strlen(message));
}
void message_client_no_requests_to_delete(client_session* s) {
    if(!s) return;
    char message[256];
    snprintf(message, sizeof(message),
             CONTROLLER "No requests to delete\n");
    write(s->controller_to_client_pipe_fd, message, strlen(message));

}
void message_client_request_info(client_session* s, request* r) {
    if (!s || !r) return;
    char message[256];
    snprintf(message, sizeof(message),
             CONTROLLER "Request ID: %d, To: %s, Distance: %d, Is active: %d, Is completed: %d\n",
             r->id, r->destination, r->distance_to_travel, r->is_active, r->is_completed);
    write(s->controller_to_client_pipe_fd, message, strlen(message));
}
void message_client_requests_found(client_session* s) {
    if (!s) return;
    char message[256];
    snprintf(message, sizeof(message),
             CONTROLLER "Here are your requests:\n");
    write(s->controller_to_client_pipe_fd, message, strlen(message));
}
void message_client_no_requests_found(client_session* s) {
    if (!s) return;
    char message[256];
    snprintf(message, sizeof(message),
             CONTROLLER "No requests have been found.\n");
    write(s->controller_to_client_pipe_fd, message, strlen(message));
}
void message_client_error_occured(client_session* s, char* description) {
    if (!s) return;
    char message[256];
    snprintf(message, sizeof(message),
             "CONTROLLER ERROR %s\n", description);
    write(s->controller_to_client_pipe_fd, message, strlen(message));
}
// -- messages for pre-session (before session creation) --

void message_client_connection_rejected(const char* client_name, const char* reason) {
    char message[256];
    snprintf(message, sizeof(message),
             CONTROLLER "Connection request rejected: %s\n", reason);
    write_to_fifo(PATH_TO_PROGRAM_PIPES_BASE, client_name, message);
}
// CONTROLLER MESSAGES

void message_controller_client_connected(const client_session* s) {
    if(s == NULL) {
        perror(ERROR "message_controller_client_connected: Can't notify about client connection, session is NULL");
        exit(0);
    }
    if(s->client_name == NULL) {
        perror(ERROR "message_controller_client_connected: Client name is null");
        exit(0);
    }
    printf(CONTROLLER "Client %s connected to the system. Session id: %d \n", s->client_name, s->id);

}
void message_controller_client_connection_denied(const char* client_name, const char* reason) {
    printf(CONTROLLER "Could not connect client %s, reason: %s \n", client_name, reason);
}