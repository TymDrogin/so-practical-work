#include "session.h"


client_session* create_client_session(id_generator* g, const char* client_name)
{
    if (client_name == NULL || g == NULL) {
        fprintf(stderr, ERROR "Invalid args passed to create_client_session\n");
        exit(EXIT_FAILURE);
    }

    client_session* s = malloc(sizeof(client_session));
    if (s == NULL) {
        perror(ERROR "Could not allocate memory for the client session");
        exit(EXIT_FAILURE);
    }

    // Session id
    s->id = generate_id(g);

    // Client name
    s->client_name = strdup(client_name);
    if (s->client_name == NULL) { perror("strdup"); exit(EXIT_FAILURE); }

    // Controller -> Client FIFO
    char controller_to_client_pipe_path[256];
    snprintf(controller_to_client_pipe_path, sizeof(controller_to_client_pipe_path),
             "%s/%s", PATH_TO_PROGRAM_PIPES_BASE, client_name);
    s->controller_to_client_pipe_path = strdup(controller_to_client_pipe_path);
    s->controller_to_client_pipe_fd = -1;

    // Client -> Controller FIFO
    char client_to_controller_pipe_path[256];
    snprintf(client_to_controller_pipe_path, sizeof(client_to_controller_pipe_path),
             "%s/%s_to_controller", PATH_TO_PROGRAM_PIPES_BASE, client_name);
    s->client_to_controller_pipe_path = strdup(client_to_controller_pipe_path);
    s->client_to_controller_pipe_fd = -1;

    // Session state
    s->is_active = false;
    s->has_active_request = false;

    return s;
}

// FIX
void free_client_session(void* item) {
    client_session* s = (client_session*)item;
    if(s == NULL) {
        perror(ERROR "Attemted to free the client session, passed null pointer");
        exit(EXIT_FAILURE);
    }
    free(s->client_name);
    free(s->client_to_controller_pipe_path);
    free(s->controller_to_client_pipe_path);
    free(s);
    return;
}