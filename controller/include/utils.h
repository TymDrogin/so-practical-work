#ifndef UTILS_H
#define UTILS_H

#include "settings.h"
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

typedef struct id_generator {
    int id;
    pthread_mutex_t mutex;
    
} id_generator;

void init_id_generator(id_generator* generator);
int generate_id(id_generator* generator);


int is_named_pipe_exists(char* pipe_path);
void create_named_pipe(char* pipe_path);



// Works almost exactly like getline(), but works on buffers instead of file streams
const char* get_line_from_buffer(const char *buffer, size_t buf_len, size_t *pos, size_t *line_len);
int read_lines_from_buffer_to_queue(queue* q, const char* buffer, size_t buffer_size);



#endif // UTILS_H



