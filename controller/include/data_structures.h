#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include "settings.h"

#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

typedef void (*free_func_t)(void*);

// Thread safe ring-buffer queue implementation
// Each queue is created with a mutex to ensure thread safety
// That way many threads can have a reference to the same queue and safely enqueue/dequeue items
// This implementation is based on the https://www.geeksforgeeks.org/dsa/introduction-to-circular-queue/ article,
// But adapted for thread safety and void* data storage
typedef struct queue {
    void** data;       
    int front;
    int rear;
    int count;
    int capacity;

    bool is_thread_safe;

    pthread_mutex_t mutex; 
    free_func_t free_func;  
} queue;

queue* q_create_queue(int capacity, bool is_thread_safe, free_func_t free_func);
int q_is_empty(queue* q);
int q_is_full(queue* q);
int q_queue_size(queue* q);
int q_enqueue(queue* q, void* item);
void* q_dequeue(queue* q);
void q_destroy_queue(queue* q);

// Un
typedef struct array {
    void** data;

    int capacity;
    int size;

    free_func_t free_func;
} array;

array* d_create_array(int capacity, free_func_t free_func);

int d_push(array* arr, void* item);
void* d_pop(array* arr);

void* d_get(const array* arr, const int inxex);
int d_remove(array* arr, const int index);


int d_size(const array* arr);
int d_capacity(const array* arr);
void d_destroy_array(array* arr);

#endif // DATA_STRUCTURES_H
