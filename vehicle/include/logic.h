#ifndef LOGIC_H
#define LOGIC_h

#include "settings.h"
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>

void init(int argc, char* argv[]);
bool read_cmdline_args(int argc, char* argv[]);

#endif // LOGIC_H

