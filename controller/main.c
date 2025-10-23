#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>

#include <signal.h>
#include <unistd.h>
#include <sys/types.h>

#define CARS_NUM 4

#define COLOR_RED     "\x1b[31m"
#define COLOR_PURPLE  "\x1b[35m"
#define COLOR_RESET   "\x1b[0m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"

#define CONTROLLER COLOR_YELLOW "[CONTROLLER] " COLOR_RESET
#define ERROR COLOR_RED "[ERROR] " COLOR_RESET
#define USAGE COLOR_RED "[USAGE] " COLOR_RESET



int main(int argc, char *argv[]) {
    char *vehical_path = "../vehicle/build/vehicle"; 


    int pid = getpid();
    int ppid = getppid();
    printf(CONTROLLER "Controller process started. PID: %d, Parent PID: %d\n", pid, ppid);

    for(int i = 0; i < CARS_NUM; i++) {
        printf(CONTROLLER "Forking + execing vehicle process %d...\n", i+1);

        int vehical_pid = fork();
        if (vehical_pid < 0) {
            perror(ERROR "Fork failed");
            exit(1);
        } else if (vehical_pid == 0) {
            // Child process
            printf("Trying to exec: %s\n", vehical_path);
            execl(vehical_path, NULL);
            // If execl returns, an error occurred
            perror(ERROR "execl failed");
            exit(1);
        }
        // Parent process
        printf(CONTROLLER "Forked vehicle process with PID: %d\n", vehical_pid);
    }

    printf(CONTROLLER "Forked and execed all vehicle processes. Controller is now running...\n");
    for(;;){
        sleep(10);
    }

    return 0;
}
