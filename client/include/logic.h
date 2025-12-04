#include "common.h"
#include "data_structures.h"
#include "message_listner.h"

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <signal.h>

#include <stdbool.h>



void application_termination_handler(int signum);



bool validate_client_command(const char* command);


