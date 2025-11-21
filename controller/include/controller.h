#include "data_structures.h"
#include "utils.h"
#include "timer.h"

#include "stdbool.h"

typedef struct service {
    int client_session_id;
    int vehicle_id;

    // Service id is assosiated with the request id 
    int request_id;
} service;


typedef struct request {
    char* destination;
    int distance_to_travel;
} request;
request* create_request(char* destination, int distance_to_travel);
void free_request(void* item);


typedef struct client_session {
    int id;

    char* client_name; // Also works as a client pipe name
    char* client_to_controller_pipe; // Name of the pipe client uses to send commands to controller
    // Is session is active or has been termineted by the client_session_worker thread 
    bool is_active;

    // Array of all the client requests
    array* requests;
} client_session;

// Fills the client session with data.
client_session* create_client_session(id_generator* g, const char* client_name, const int max_num_of_requests);
void free_client_session(void* item);

typedef struct controller {
    array* vehicles;            // Array of vehicles currently at work
    array* sessions;            // Array of currently active user sessions 
    array* services;            // Array of active services?

    id_generator* vehicle_id_gen;
    id_generator* session_id_gen;
    id_generator* request_id_gen;

    int max_num_of_sessions;
    int max_num_of_vehicles;
    int max_num_of_services;

    int max_num_of_requests_per_user;
    
    int total_km_traveld_by_all_cars // 

    
} controller;

controller* create_controller(const int max_num_of_user_sessions, const int max_num_of_vehicles, const max_num_of_services, const max_num_of_requests_per_user);


// Attempts to connect a client with given name to the controller.
// Returns true on success, false if client is already connected or no session slots are available.
// Also messages the client and the controller admin about the connection status.
bool connect_client(controller* c, const char* client_name);

// Setrs the flags, creates pipes. Does
void start_client_session(client_session* s);

// Termination routine for a client session.
// Does not free the client session struct itself,
// Maybe frees the resources associated with it,
// But has necessary steps to gracefully terminate the session.
void terminate_client_session(controller* c, client_session* s);

// Frees all resources associated with the client and disconnects them from the controller.
// TODO: Cancel all active services of the client before disconnecting?
// Messages the client about disconnection.
void disconnect_client_by_name(controller* c, const char* client_name);
bool is_client_connected_by_name(const controller* c, const char* client_name);



void process_clients_connection_requests(controller* c, queue* connection_request_queue);



void print_all_connected_clients(controller* c);


// MESSAGES TO CLIENTS AND CONTROLLER ADMINISTRATOR 

void message_client_connection_accepted(const char* client_name);
void message_client_connection_rejected(const char* client_name, const char* reason);




