#include "queue.h"


queue* q_create_queue(int capacity) {
    queue* q = (queue*)malloc(sizeof(queue));
    if(!q) {
        perror(ERROR "Failed to allocate memory for queue.\n");
        return NULL;
    }
    q->data = (void**)malloc(sizeof(void*) * capacity);
    if(!q->data) {
        perror(ERROR "Failed to allocate memory for queue data.\n");
        free(q);
        return NULL;
    }

    q->capacity = capacity;
    q->front = 0;
    q->rear = -1;
    q->count = 0;
    if(pthread_mutex_init(&q->mutex, NULL) != 0) {
        perror(ERROR "Failed to initialize queue mutex.\n");
        free(q->data);
        free(q);
        exit(1);
    };

    return q;
}
int q_is_empty(queue* q) {
    pthread_mutex_lock(&q->mutex);
    int empty = q->count == 0;
    pthread_mutex_unlock(&q->mutex);
    return empty;
}
int q_is_full(queue* q) {
    pthread_mutex_lock(&q->mutex);
    int full = q->count == q->capacity;
    pthread_mutex_unlock(&q->mutex);
    return full;
}
int q_queue_size(queue* q) {
    pthread_mutex_lock(&q->mutex);
    int size = q->count;
    pthread_mutex_unlock(&q->mutex);
    return size;
}
int q_enqueue(queue* q, void* item) {
    pthread_mutex_lock(&q->mutex);

    // cannot use the is_full function here because of mutex deadlock

    if (q->count == q->capacity) {
        perror(ERROR "Queue is full, cannot enqueue item.\n");
        pthread_mutex_unlock(&q->mutex);
        return 0;
    }
    q->rear = (q->rear + 1) % q->capacity;
    q->data[q->rear] = item;
    q->count++;

    pthread_mutex_unlock(&q->mutex);
    return 1;
}
void* q_dequeue(queue* q) {
    pthread_mutex_lock(&q->mutex);
    int is_empty = q->count == 0;
    if (is_empty) {
        return NULL;
    }
    void* item = q->data[q->front];
    q->front = (q->front + 1) % q->capacity;
    q->count--;
    pthread_mutex_unlock(&q->mutex);
    return item;
}
void q_free_queue(queue* q, free_func_t free_func) {
    if (!q) return;

    pthread_mutex_lock(&q->mutex);

    // Dequeue and free each element
    while (q->count > 0) {
        void* item = q->data[q->front];
        q->front = (q->front + 1) % q->capacity;
        q->count--;

        if (item) {
            // use custom funciton if passed
            if (free_fun)
                free_fun(item);
            else
                free(item);
        }
    }

    free(q->data);
    pthread_mutex_unlock(&q->mutex);
    pthread_mutex_destroy(&q->mutex);
    free(q);
}