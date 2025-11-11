#ifndef MESSAGE_LISTNER_H
#define MESSAGE_LISTNER_H


#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>

#include "common.h"
#include "utils.h"



void start_message_listner_thread(char* client_name);


#endif // MESSAGE_LISTNER_H