
#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "data_structures.h"
#include "utils.h"
#include "timer.h"
#include "session.h"
#include "vehicle.h"
#include "stdbool.h"


// Request holds all the information about a client service request,
// As well as its status and additional metadata
// Vehicle holds more complete 
typedef struct request {
    int id;                 // Unique request ID
    int client_session_id;  // ID of the client session that created this request


    char* destination;
    int distance_to_travel;
    int desired_start_time; // Desired time to start the service provided by the client 
    int time_created;       // Time when the request was created

    bool is_active;         // Is the request currently being processed
    bool is_comleted;
    int vehicle_id;         // ID of the vehicle assigned to this request, -1 if none




} request;
request* create_request(id_generator* g, int client_session_id, const char* destination, int distance_to_travel, int desired_start_time, int time_created);
void free_request(void* item);


// Controller struct holds all the necessary data structures and state information
// It manages client sessions, vehicle, requests, interractions between them
// It is a heart of the system
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

// Creates and initializes a new controller instance with given parameters
controller* create_controller(const int max_num_of_user_sessions,
    const int max_num_of_vehicles, 
    const int max_num_of_requests);


// CONTROLLER LOOP
// Connect clients
// check vehicle messages. IF the vehicle terminated, free the resources, notify the client, frees the car etc

// Read client messages
// Read controller admin messages
// Do the cleanup(optional)



bool is_client_connected_by_name(const controller* c, const char* client_name);
bool is_client_connected_by_id(const controller* c, const int id);

client_session* get_client_session_by_name(const controller* c, const char* client_name);
client_session* get_client_session_by_id(const controller* c, const int id);

request* get_request_by_id(const controller* c, const int id);



// -- CLIENT CONNECTION/DISCONNECTION LOGIC 
bool connect_client(controller* c, const char* client_name);
void serve_connection_requests(controller* c, queue* client_connection_req_queue);


// Frees all resources associated with the client and disconnects them from the controller.
// TODO: Cancel all active services of the client before disconnecting?
// Messages the client about disconnection.
// NOTES: Delete all request, terminate all active services, remove from controller sessions array
void disconnect_client_by_name(controller* c, const char* client_name);
void terminate_client_session(controller* c, client_session* s);


void disconnect_all_clients(controller* c);





void print_full_session_info(client_session* s);




void process_vehicle_message(controller* c, vehicle_t* v, request* r, const char* message);

// arrived - set status to alive
// Begin - set service to active 
// distance - update both internal and global distance travelled
// terminated - frees the resources, the request, notifies the client




// ADMIN COMMAND PROCESSING
void process_admin_command(controller* c, char* command);

void utiliz(const controller* c);





// CLIENT COMMAND PROCESSING
void process_client_command(controller* c, client_session* s, char* command);

// -- creates a new request and adds it to the controller's requests array 
void agendar(controller* c, client_session* s, int start_time, const char* destination, int distance);


// MESSAGES TO CLIENTS
void message_client_connection_accepted(const char* client_name);
void message_client_connection_rejected(const char* client_name, const char* reason);
void message_client_disconnection_notice(const char* client_name);
void message_client_request_creation_accepted(const char* client_name, int request_id);
void message_client_request_creation_rejected(const char* client_name, const char* reason);

// CONTROLLER MESSAGES 

void message_controller_client_connected(const client_session* s);
void message_controller_client_connection_denied(const char* client_name, const char* reason);




#endif // CONTROLLER_H