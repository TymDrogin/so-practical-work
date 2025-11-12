#include "data_structures.h"


queue* q_create_queue(int capacity, bool thread_safe, free_func_t free_func) {
    queue* q = (queue*)malloc(sizeof(queue));
    if(!q) {
        perror(ERROR "Failed to allocate memory for queue.\n");
        exit(1);
    }
    q->data = (void**)malloc(sizeof(void*) * capacity);
    if(!q->data) {
        perror(ERROR "Failed to allocate memory for queue data.\n");
        free(q);
        exit(1);
    }

    q->capacity = capacity;
    q->front = 0;
    q->rear = -1;
    q->count = 0;
    q->free_func = free_func;

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
void q_destroy_queue(queue* q) {
    if (!q) return;

    pthread_mutex_lock(&q->mutex);

    // Dequeue and free each element
    while (q->count > 0) {
        void* item = q->data[q->front];
        q->front = (q->front + 1) % q->capacity;
        q->count--;

        if (item) {
            if (q->free_func)
                q->free_func(item);
            else
                free(item);
        }
    }

    free(q->data);
    pthread_mutex_unlock(&q->mutex);
    pthread_mutex_destroy(&q->mutex);
    free(q);
}



array* d_create_array(int capacity, free_func_t free_func) {
    array* arr = (array*)malloc(sizeof(array));

    if(capacity <= 0) {
        perror(ERROR" Invalid capacity of %d has been set for the dynamic array.\n", capacity);
        exit(1);
    }
    if(!arr) {
        perror(ERROR "Failed to allocate memory for the dynamic array.\n");
        exit(1);
    }
    arr->data = (void**)malloc(sizeof(void*) * capacity);
    if(!arr->data) {
        perror(ERROR "Failed to allocate memory for the array data.\n");
        exit(1);
    }

    arr->capacity = capacity;
    arr->size = 0;
    arr->free_func = free_func;

    return arr;
}

array* d_create_array(int capacity, free_func_t free_func) {
    if (capacity <= 0) {
        perror(ERROR "Attemt to create an array with invalid capacity");
        exit(EXIT_FAILURE);
    }

    array* arr = malloc(sizeof(array));
    if (!arr) {
        perror("Failed to allocate array struct");
        exit(EXIT_FAILURE);
    }

    arr->data = malloc(sizeof(void*) * capacity);
    if (!arr->data) {
        perror("Failed to allocate array data");
        free(arr);
        exit(EXIT_FAILURE);
    }

    arr->capacity = capacity;
    arr->size = 0;
    arr->free_func = free_func;
    return arr;
}
int d_push(array* arr, void* item) {
    if (!arr) {
        perror(ERROR "d_push: arr is NULL\n");
        exit(EXIT_FAILURE);
    }
    if (!item) {
        perror(ERROR "d_push: item is NULL\n");
        exit(EXIT_FAILURE);
    }
    if (arr->size >= arr->capacity) {
        perror(ERROR "d_push: array full (capacity=%d)\n", arr->capacity);
        return -1;
    }
    arr->size++;
    arr->data[arr->size] = item;
    return 0;
}
void* d_pop(array* arr) {
    if (!arr) {
        perror(ERROR "d_pop: arr is NULL\n");
        exit(EXIT_FAILURE);
    }

    if (arr->size == 0)
        return NULL;

    void* item = arr->data[arr->size];
    arr->size--;

    return item;
}

void* d_get(const array* arr, int index) {
    if (!arr) {
        perror(ERROR "d_get: arr is NULL\n");
        exit(EXIT_FAILURE);
    }
    if (index < 0 || index >= arr->size)
        return NULL;

    return arr->data[index];
}

int d_remove(array* arr, int index) {
    if (!arr) {
        perror(ERROR "d_remove: arr is NULL\n");
        return -1;
    }
    if (index < 0 || index >= arr->size) {
        perror(ERROR "d_remove: invalid index %d\n", index);
        return -1;
    }

    if (arr->free_func) {
        arr->free_func(arr->data[index]);
    } else {
        free(arr->data[index]);
    }

    // shift all elements after index to the left
    for (int i = index; i < arr->size - 1; i++) {
        arr->data[i] = arr->data[i + 1];
    }

    arr->size--;

    return 0;
}
int d_size(const array* arr) {
    return arr->size;
}
int d_capacity(const array* arr) {
    return arr->capacity;
}
void d_destroy_array(array* arr) {
    if (!arr) return;

    void* item;
    while ((item = d_pop(arr)) != NULL) {
        if (arr->free_func) {
            arr->free_func(item);
        } else {
            free(item);
        }
    }

    free(arr->data);
    free(arr);
}