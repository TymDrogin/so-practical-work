#include "data_structures.h"


queue* q_create_queue(int capacity, bool is_thread_safe, free_func_t free_func) {
    queue* q = (queue*)malloc(sizeof(queue));
    if(!q) {
        perror(ERROR "Failed to allocate memory for queue.\n");
        exit(EXIT_FAILURE);
    }
    q->data = (void**)malloc(sizeof(void*) * capacity);
    if(!q->data) {
        perror(ERROR "Failed to allocate memory for queue data.\n");
        free(q);
        exit(EXIT_FAILURE);
    }

    q->capacity = capacity;
    q->front = 0;
    q->rear = -1;
    q->count = 0;
    q->is_thread_safe = is_thread_safe;
    q->free_func = free_func;

    if (q->is_thread_safe) {
        if(pthread_mutex_init(&q->mutex, NULL) != 0) {
            perror(ERROR "Failed to initialize queue mutex.\n");
            free(q->data);
            free(q);
            exit(1);
    };

    }

    return q;
}
int q_is_empty(queue* q) {
    if (q->is_thread_safe) pthread_mutex_lock(&q->mutex);
    int empty = q->count == 0;
    if (q->is_thread_safe) pthread_mutex_unlock(&q->mutex);
    return empty;
}
int q_is_full(queue* q) {
    if (q->is_thread_safe) pthread_mutex_lock(&q->mutex);
    int full = q->count == q->capacity;
    if (q->is_thread_safe) pthread_mutex_unlock(&q->mutex);
    return full;
}
int q_size(queue* q) {
    if (q->is_thread_safe) pthread_mutex_lock(&q->mutex);
    int size = q->count;
    if (q->is_thread_safe) pthread_mutex_unlock(&q->mutex);
    return size;
}
int q_enqueue(queue* q, void* item) {
    if (q->is_thread_safe) pthread_mutex_lock(&q->mutex);

    if (q->count == q->capacity) {
        perror(ERROR "Queue is full, cannot enqueue item.\n");
        if (q->is_thread_safe) pthread_mutex_unlock(&q->mutex);
        return 0;
    }
    q->rear = (q->rear + 1) % q->capacity;
    q->data[q->rear] = item;
    q->count++;

    if (q->is_thread_safe) pthread_mutex_unlock(&q->mutex);
    return 1;
}
void* q_dequeue(queue* q) {
    if (q->is_thread_safe) pthread_mutex_lock(&q->mutex);

    if (q->count == 0) {
        if (q->is_thread_safe) pthread_mutex_unlock(&q->mutex);
        return NULL;
    }

    void* item = q->data[q->front];
    q->front = (q->front + 1) % q->capacity;
    q->count--;
    if (q->is_thread_safe) pthread_mutex_unlock(&q->mutex);
    return item;
}
void q_destroy_queue(queue* q) {
    if (!q) return;

    if (q->is_thread_safe) pthread_mutex_lock(&q->mutex);

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
    if (q->is_thread_safe) pthread_mutex_unlock(&q->mutex);
    if (q->is_thread_safe) pthread_mutex_destroy(&q->mutex);
    free(q);
}

array* a_create_array(int capacity, free_func_t free_func) {
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
int a_push(array* arr, void* item) {
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
    arr->data[arr->size] = item;
    arr->size++;
    return 0;
}
void* a_pop(array* arr) {
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

void* a_get(const array* arr, int index) {
    if (!arr) {
        perror(ERROR "d_get: arr is NULL\n");
        exit(EXIT_FAILURE);
    }
    if (index < 0 || index >= arr->size)
        return NULL;

    return arr->data[index];
}

int a_remove(array* arr, int index) {
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
int a_size(const array* arr) {
    return arr->size;
}
int a_capacity(const array* arr) {
    return arr->capacity;
}
bool a_is_full(const array* arr) {
    return arr->size >= arr->capacity;
}
void a_destroy_array(array* arr) {
    if (!arr) return;

    void* item;
    while ((item = a_pop(arr)) != NULL) {
        if (arr->free_func) {
            arr->free_func(item);
        } else {
            free(item);
        }
    }

    free(arr->data);
    free(arr);
}