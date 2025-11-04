#ifndef QUEUE_H
#define QUEUE_H

#include "settings.h"

#include <stdlib.h>
#include <pthread.h>

typedef void (*free_func_t)(void*);

// Thread safe ring-buffer queue implementation
// Each queue is created with a mutex to ensure thread safety
// That way many threads can have a reference to the same queue and safely enqueue/dequeue items
// This implementation is based on the https://www.geeksforgeeks.org/dsa/introduction-to-circular-queue/ article,
// But adapted for thread safety and void* data storage
typedef struct queue {
    void** data;       // Array of void pointers to hold the data [void*, void*, ...]
    int front;
    int rear;
    int count;
    int capacity;

    pthread_mutex_t mutex; // Mutex for thread safety
} queue;

// Allocates the queue and initializes the mutex
// This functions do not call each other to avoid dead locks. Such as, code from enqueue has 
queue* q_create_queue(int capacity);

int q_is_empty(queue* q);
int q_is_full(queue* q);
int q_queue_size(queue* q);
int q_enqueue(queue* q, void* item);
void* q_dequeue(queue* q);
void q_free_queue(queue* q, free_func_t free_func);

#endif // QUEUE_H
