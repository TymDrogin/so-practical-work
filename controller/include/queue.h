#include "common.h"

#include <stdlib.h>


typedef struct queue {
    void** data;       // Array of void pointers to hold the data
    int front;
    int rear;
    int count;
    int capacity;
} queue;
