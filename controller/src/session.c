#include "session.h"



client_session_manager* create_session_manager(int capacity) {
    client_session_manager* manager = malloc(sizeof(client_session_manager));
    if(!manager) {
        perror(ERROR "Could not allocate memory for the session manager");
        exit(1);
    };

    manager->sessions = calloc(capacity, sizeof(client_session));
    if(!manager->sessions) {
        perror(ERROR "Could not allocate memory for the client sessions");
    };

    manager->capacity = capacity;
    manager->count = 0;

    return manager;
}



