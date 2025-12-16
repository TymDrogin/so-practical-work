#include "timer.h"

static pthread_t timer_thread;
static int is_spawned = 0;
static int is_paused = 0;

// Used to safely mutate/read timer ticks
static pthread_mutex_t timer_mutex = PTHREAD_MUTEX_INITIALIZER;
// Used to pause/resume the timer thread
static pthread_cond_t timer_cond = PTHREAD_COND_INITIALIZER;

static int timer_ticks = 0;

void* timer_worker(void* arg) {
    pthread_setname_np(pthread_self(), "timer-worker");
    int* ticks_counter = (int*)arg;

    while (1) {
        // Lock mutex to safely update ticks
        pthread_mutex_lock(&timer_mutex);

        // Only increment ticks if not paused
        while(is_paused) {
            pthread_cond_wait(&timer_cond, &timer_mutex);
        }
        (*ticks_counter)++;
        
        pthread_mutex_unlock(&timer_mutex);
        usleep(TIMER_TICK_SPEED_MILLISECONDS * 1000);
    }

    return NULL;
}


void spawn_timer_thread(void) {
    if (is_spawned) {
        printf(ERROR "Timer thread has already been spawned.\n");
        exit(1);
    }

    timer_thread = pthread_create(&timer_thread, NULL, timer_worker, (void*)&timer_ticks);
    if (timer_thread != 0) {
        perror(ERROR "Failed to create timer thread.\n");
        exit(1);
    }
}

void start_timer() {
    if (!timer_thread) {
        spawn_timer_thread();

        pthread_mutex_lock(&timer_mutex);
        is_paused = 0;
        is_spawned = 1;
        pthread_cond_signal(&timer_cond);
        pthread_mutex_unlock(&timer_mutex);
    } else {
        printf(ERROR "Timer has already been started. If you want to resume it, use resume_timer().\n");
        exit(1);
    }
}
void stop_timer() {
    if(!is_spawned) {
        printf(ERROR "Timer hasn't been started yet ");
        exit(1);
    }

    pthread_mutex_lock(&timer_mutex);
    is_paused = 1;
    pthread_mutex_unlock(&timer_mutex);
}
void resume_timer() {
    if(!is_spawned) {
        printf(ERROR "Timer hasn't been started yet ");
        exit(1);
    }

    pthread_mutex_lock(&timer_mutex);
    is_paused = 0;
    pthread_cond_signal(&timer_cond);
    pthread_mutex_unlock(&timer_mutex);
}
void reset_timer() {
    if(!is_spawned) {
        printf(ERROR "Timer hasn't been started yet ");
        exit(1);
    }

    pthread_mutex_lock(&timer_mutex);
    timer_ticks = 0;
    pthread_mutex_unlock(&timer_mutex);
}
void set_timer_ticks(int ticks) {
    if(!is_spawned) {
        printf(ERROR "Timer hasn't been started yet ");
        exit(1);
    }

    pthread_mutex_lock(&timer_mutex);
    timer_ticks = ticks;
    pthread_mutex_unlock(&timer_mutex);
}
int get_timer_ticks() {
    if(!is_spawned) {
        printf(ERROR "Timer hasn't been started yet ");
        exit(1);
    }
    int ticks;

    pthread_mutex_lock(&timer_mutex);
    ticks = timer_ticks;
    pthread_mutex_unlock(&timer_mutex);

    return ticks;
}

