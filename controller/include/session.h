#ifndef SESSION_H
#define SESSION_H


#include "data_structures.h"
#include "utils.h"
#include "settings.h"


typedef struct client_session {
    int id;                          // Unique session ID, can be used to simplify lookups

    char* client_name;               // Also works as a client pipe name
    char* client_to_controller_pipe; // Name of the pipe client uses to send commands to controller
    bool is_active;                  // Is the session currently active

    bool has_active_request;        // Does the client have an active request being processed by a vehicle
    
} client_session;

client_session* create_client_session(id_generator* g, const char* client_name);
void free_client_session(void* item);







#endif // SESSION_H