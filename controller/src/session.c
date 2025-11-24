#include "session.h"




client_session* create_client_session(id_generator* g, const char* client_name)
{
    if (client_name == NULL || g == NULL) {
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
    s->is_active = 0;
    s->has_active_request = false;

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
    free(s);
    return;
}