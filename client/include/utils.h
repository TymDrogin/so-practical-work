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


int is_named_pipe_exists(char* pipe_path);
void create_named_pipe(char* pipe_name, char* pipe_path_base);
void remove_named_pipe(char* pipe_name, char* pipe_path_base);

int is_controller_entry_pipe_exist(void);
int is_client_pipe_exist(char* client_name);




const char* get_line_from_buffer(const char *buffer, size_t buf_len, size_t *pos, size_t *line_len);
// Works almost exactly like getline(), but works on buffers instead of file streams
// I have asked chatgpt to help me write this as i haven't encountered any implementations online 
// How to use: 
// int main(void) {
//     char buffer[1024];
//     ssize_t n = read(STDIN_FILENO, buffer, sizeof(buffer));
// 
//     if (n <= 0) return 0;
// 
//     size_t pos = 0, len;
//     const char *line;
// 
//     while ((line = get_line_from_buffer(buffer, n, &pos, &len)) != NULL) {
// 		if(len ==0) continue;
// 		printf("Line (%zu): %.*s\n", len, (int)len, line);
//      To copy line don't forget to allocate len+1 and insert null terminator or \n
//     }
// 
//     return 0;
// }

int read_lines_from_buffer_to_queue(queue* q, const char* buffer, size_t buffer_size);



#endif // UTILS_H



