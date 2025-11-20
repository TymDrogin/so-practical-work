#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include "settings.h"
#include "utils.h"
#include "logic.h"


// ARGS ./vehicle <client_name> <destination> <distance_to_travel> <id_of_car>
//       0        1             2             3                    4 

int main(int argc, char *argv[]) {
    init(argc, argv);
    main_loop();

    return 0;
}