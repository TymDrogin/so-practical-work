#define _POSIX_C_SOURCE 200809L

#include "common.h"
#include "timer.h"
#include "entry_fifo_listener.h"

#include <stdio.h>
#include <stdlib.h>

#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

#define CARS_NUM 4

void cleanup(int signum) {
    printf("\n" CONTROLLER "Caught signal %d, cleaning up...\n", signum);
    unlink(CONTROLLER_ENTRY_FIFO_PATH); // Remove the FIFO
    exit(0);
}

void init(void) {
    printf(CONTROLLER "Starting controller...\n");  
    
    printf(CONTROLLER "Starting timer...\n");
    start_timer();
    printf(CONTROLLER "Starting entry FIFO listener...\n");
    start_entry_fifo_listener_thread();

    printf(CONTROLLER "Controller initialized successfully.\n");

    struct sigaction sa;
    sa.sa_handler = cleanup;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, NULL);  // Ctrl+C
    sigaction(SIGTERM, &sa, NULL); // Kill signal
}




int main(int argc, char *argv[]) {
    init();

    while (1)
    {
        sleep(1);
    }

    return 0;
}
