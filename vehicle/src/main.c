#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>

#include <signal.h>
#include <unistd.h>
#include <sys/types.h>


#define COLOR_RED     "\x1b[31m"
#define COLOR_PURPLE  "\x1b[35m"
#define COLOR_RESET   "\x1b[0m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"

#define VEHICLE COLOR_GREEN "[VEHICLE] " COLOR_RESET


void kill_handler(int signum) {
    printf(VEHICLE "Vehicle process received termination signal. Exiting...\n");
    exit(0);
}


int main(int argc, char *argv[]) {

    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = kill_handler;
    sigaction(SIGINT, &sa, NULL);

    int pid = getpid();
    int ppid = getppid();

    printf(VEHICLE"Vehicle process started. PID: %d, Parent PID: %d\n", pid, ppid);   
    

    while (1)
    {
        sleep(5);
        printf(VEHICLE "Wroom wroom! Vehicle PID: %d is running...\n", pid);
    }
    

    return 0;
}