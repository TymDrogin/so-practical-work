#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "common.h"





int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf(ERROR " Invalid number of arguments.\n");
        printf(USAGE "%s <client_name>\n", argv[0]);
        return 1;
    }
    else {
        printf(INFO " Client name: %s\n", argv[1]);
    }


    return 0;
}