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




    char buf[256];
    printf("> ");

    // Get user commands
    // Works in tandem with message listener thread
    while (1) {
        fflush(stdout);

        if (!fgets(buf, sizeof(buf), stdin))
            break;

        // Remove newline
        buf[strcspn(buf, "\n")] = 0;

        // If user pressed Enter (empty input)
        if (buf[0] == '\0') {
            printf("> ");
            continue;
        }
        bool valid = validate_client_command(buf);
        if (valid) {
            printf("> Command valid!\n");
        } else {
            printf("> Invalid command!\n");
        }

        printf("> ");
        fflush(stdout);
    }

    while(1) pause();


    return 0;
}