#include "data_structures.h"
#include "utils.h"
#include "timer.h"

#include "stdbool.h"


typedef struct client_session {
    int id;

    char* client_name; // Also works as a client pipe name
    char* client_to_controller_pipe; // Name of the pipe client uses to send commands to controller
    // Is session is active or has been termineted by the client_session_worker thread 
    int is_active;

    // Array of all the client requests
    array* requests;
} client_session;

// Fills the client session with data.
// Connect client has 
client_session* create_client_session(id_generator* g, const char* client_name, const int max_num_of_requests);
void free_client_session(void* item);


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

request* create_request();
void free_request(void* item);



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
    
    int total_km_traveld_by_all_cars // 

    
} controller;

controller* create_controller(const int max_num_of_user_sessions, const int max_num_of_vehicles, const int max_num_of_requests_per_user);


// Returns one on success, return false in case of client already connected | no avaible sessions
// Adds the session to the array, as well has the connection logic
bool connect_client(controller* c, char* client_name);
bool is_client_connected_by_name(const controller* c, const char* client_name);
// Disconnection routine for the client
void disconnect_client(controller* c, char* client_name);

void disconnect_all_clients(controller* c);


void print_all_connected_clients(controller* c);



