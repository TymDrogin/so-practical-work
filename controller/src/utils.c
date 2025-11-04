#include "utils.h"

void init_id_generator(id_generator* generator) {
    generator->id = 0;
    if(pthread_mutex_init(&generator->mutex, NULL) != 0) {
        perror(ERROR "Failed to initialize id generator mutex.\n");
        free(generator);
        exit(1);
    };
}
int generate_id(id_generator* generator) {
    pthread_mutex_lock(&generator->mutex);
    int id = generator->id;
    generator->id++;
    pthread_mutex_unlock(&generator->mutex);
    return id; 
}
