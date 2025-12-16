#include "utils.h"
#include "timer.h"
#include "connection_request_listener.h"
#include "data_structures.h"
#include "vehicle.h"
#include "settings.h"
#include "controller.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

#define CARS_NUM 4
#define CLIENT_CON_REQ_QUEUE_CAPACITY 100

static volatile sig_atomic_t is_active = 1;
static controller *c;
static queue *client_connection_req_queue;


static pthread_mutex_t controller_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  controller_cv    = PTHREAD_COND_INITIALIZER;


void termination_handler(int signum)
{
    (void)signum;
    is_active = 0;
}



void init(void)
{
    printf(CONTROLLER "Starting controller...\n");

    start_timer();

    client_connection_req_queue =
        q_create_queue(CLIENT_CON_REQ_QUEUE_CAPACITY, true, NULL);
    if (!client_connection_req_queue) {
        perror(ERROR "Could not create client connection request queue");
        exit(1);
    }

    start_connection_request_listener_thread(client_connection_req_queue);

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = termination_handler;

    sigaction(SIGINT,  &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    printf(CONTROLLER "Controller initialized successfully.\n");
}


void *controller_loop(void *arg)
{
    controller *c = (controller *)arg;

    while (is_active) {
        // Lock from admin commands
        pthread_mutex_lock(&controller_mutex);

        serve_connection_requests(c, client_connection_req_queue);
        process_all_clients_commands(c);
        process_all_vehicles_messages(c);
        clear_completed_requests(c);
        clear_cars_that_completed_request(c);
        dispatch_cars(c);

        pthread_mutex_unlock(&controller_mutex);

        sleep(1);
    }
    pthread_mutex_lock(&controller_mutex);
    controller_termination_routine(c);
    pthread_mutex_unlock(&controller_mutex);

    return NULL;
}





int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    pthread_t controller_thread;


    c = create_controller(5, 2, 50);

    init();


    if (pthread_create(&controller_thread, NULL, controller_loop, c) != 0) {
        perror(ERROR "Failed to start controller thread");
        exit(1);
    }

    /* User input loop */
    char buf[128];

    while (is_active) {
        printf("> ");
        fflush(stdout);

        if (!fgets(buf, sizeof(buf), stdin)) break;
        pthread_mutex_lock(&controller_mutex);
        process_admin_command(c, buf);
        printf("\n");
        pthread_mutex_unlock(&controller_mutex);

    }

    pthread_join(controller_thread, NULL);
    return 0;
}



