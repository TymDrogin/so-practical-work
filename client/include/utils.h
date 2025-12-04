#ifndef UTILS_H
#define UTILS_H

#include "common.h"
#include "data_structures.h"

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>


int is_named_pipe_exists(char* pipe_path, char* pipe_name);
int is_controller_entry_pipe_exist(void);
int is_client_pipe_exist(char* client_name);

void create_named_pipe(char* pipe_path, char* pipe_name);
void remove_named_pipe(char* pipe_path, char* pipe_name);

void write_to_fifo(const char *pipe_path,
                                 const char *pipe_name,
                                 const char *message);


void print_usage();


const char* get_line_from_buffer(const char *buffer, size_t buf_len, size_t *pos, size_t *line_len);


#endif // UTILS_H



