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
#include "logic.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf(ERROR " Invalid number of arguments.\n");
        printf(USAGE "%s <client_name>\n", argv[0]);
        return 1;
    }
    else {
        printf(INFO " Client name: %s\n", argv[1]);
    }

    char* client_name = argv[1];

    init(client_name);

    char client_to_controller_pipe_name[256];
    snprintf(client_to_controller_pipe_name, sizeof(client_to_controller_pipe_name), "%s_to_controller", client_name);

    char buf[256];
    printf("> ");

    // Get user commands
    // Works in tandem with message listener thread
    while (1) {
        fflush(stdout);

        if (!fgets(buf, sizeof(buf), stdin))
            break;

        // If user pressed Enter (empty input)
        if (strlen(buf) <= 1) {
            printf("> ");
            continue;
        }

        bool valid = validate_client_command(buf);
        if (valid) {
            fflush(stdout);
            if(!is_named_pipe_exists(PROGRAMS_BASE_PATH, client_to_controller_pipe_name)) {
                fprintf(stderr, "Controller to client pipe does not exist");
                application_termination_handler(0);
            }
            // Send command
            write_to_fifo(PROGRAMS_BASE_PATH, client_to_controller_pipe_name, buf);
        } else {
            buf[strcspn(buf, "\n")] = 0;
            printf("> Invalid command: %s\n", buf);
            print_usage();
        }

        if (terminate_requested) {
            application_termination_handler(0);
        }

        printf("> ");
        fflush(stdout);
    }
    while(1) pause();
    application_termination_handler(0);
    return 0;
}