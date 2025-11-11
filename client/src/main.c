#define _POSIX_C_SOURCE 200809L


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

#include <signal.h>




#include "common.h"
#include "message_listner.h"
#include "utils.h"
static char* client_name;


void application_termination_handler(int signum) {
    printf(INFO "Terminating client session");
    remove_named_pipe(client_name, CLIENT_PIPE_PATH);
    exit(1);
    
}

void init() {
    struct sigaction sa = {0};
    sa.sa_handler = application_termination_handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    start_message_listner_thread(client_name);
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf(ERROR " Invalid number of arguments.\n");
        printf(USAGE "%s <client_name>\n", argv[0]);
        return 1;
    }
    else {
        printf(INFO " Client name: %s\n", argv[1]);
    }

    client_name = argv[1];

    if(is_client_pipe_exist(client_name)) {
        printf(INFO "Can't connect as the other client session with this name is running");
        exit(1);
    } else { 
        create_named_pipe(client_name, CLIENT_PIPE_PATH);
    }

    init();
    while(1) pause();


    return 0;
}