
#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "data_structures.h"
#include "utils.h"
#include "timer.h"
#include "session.h"

#include "stdbool.h"


// Since in the enunciado of the project the criteria to schedule 
// vehicles to clients is not specified, i define a function type for it here
// It takes a controller pointer as an argument and returns an integer
// The integer represents and id of a request that is going to be scheduled next
// It returns -1 if no request can be scheduled at the moment
// This function type can be used to implement different scheduling algorithms
// and pass them to the controller as needed
typedef struct request {
    int id;                 // Unique request ID
    int client_session_id;  // ID of the client session that created this request


    char* destination;
    int distance_to_travel;
    int desired_start_time; // Desired time to start the service provided by the client 

    bool is_active;        // Is the request currently being processed
    int vehicle_id;         // ID of the vehicle assigned to this request, -1 if none
    int time_created;       // Time when the request was created

} request;
request* create_request(id_generator* g, int client_session_id, const char* destination, int distance_to_travel, int desired_start_time, int time_created);
void free_request(void* item);

typedef struct controller {
    array* vehicles;            // Array of vehicles currently at work
    array* sessions;            // Array of currently active user sessions 
    array* request;            // Array of active services?

    id_generator* vehicle_id_gen;
    id_generator* session_id_gen;
    id_generator* request_id_gen;

    int max_num_of_sessions; // Maximum number of user sessions
    int max_num_of_vehicles; // Maximum number of vehicles controller can provide to the clients
    int max_num_of_services; // Should be equal to max_num_of_vehicles
    int max_num_of_requests; // Request might end up holding all the data, also about the services.
    // Howewe
    
    int total_km_traveld_by_all_cars; // 

    
} controller;

controller* create_controller(const int max_num_of_user_sessions, const int max_num_of_vehicles, const int max_num_of_services, const int max_num_of_requests);


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

// NOTES: Delete all request, terminate all active services, remove from controller sessions array
void disconnect_client_by_name(controller* c, const char* client_name);
bool is_client_connected_by_name(const controller* c, const char* client_name);





void process_clients_connection_requests(controller* c, queue* connection_request_queue);


// ADMIN COMMAND PROCESSING
void process_admin_command(controller* c, char* command);





// CLIENT COMMAND PROCESSING
void process_client_command(controller* c, client_session* s, char* command);









// MESSAGES TO CLIENTS AND CONTROLLER ADMINISTRATOR 
void message_client_connection_accepted(const char* client_name);
void message_client_connection_rejected(const char* client_name, const char* reason);
void message_client_disconnection_notice(const char* client_name);




#endif // CONTROLLER_H