#ifndef UTILS_H
#define UTILS_H

#include "settings.h"

#include <pthread.h>
#include <stdlib.h>

typedef struct id_generator {
    // Previous id
    int id;
    pthread_mutex_t mutex;
    
} id_generator;

void init_id_generator(id_generator* generator);
int generate_id(id_generator* generator);



#endif // UTILS_H



