#include "queue.h"


queue* create_queue(int capacity) {
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

    return q;
}

int is_empty(queue* q) {
    return q->count == 0;
}
int is_full(queue* q) {
    return q->count == q->capacity;
}
int enqueue(queue* q, void* item) {
    if (is_full(q)) {
        perror(ERROR "Queue is full, cannot enqueue item.\n");
        return 0;
    }
    q->rear = (q->rear + 1) % q->capacity;
    q->data[q->rear] = item;
    q->count++;
    return 1;
}
void* dequeue(queue* q) {
    if (is_empty(q)) {
        perror(ERROR "Queue is empty, cannot dequeue item.\n");
        return NULL;
    }
    void* item = q->data[q->front];
    q->front = (q->front + 1) % q->capacity;
    q->count--;
    return item;
}
void free_queue(queue* q) {
    if(!q) return;
    if(q->data) {
        free(q->data);
    }
    free(q);
}