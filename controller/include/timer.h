#ifndef TIMER_H
#define TIMER_H


#define _GNU_SOURCE
#include "settings.h"

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

void start_timer();
void resume_timer();
void stop_timer();
void reset_timer();

int get_timer_ticks();
void set_timer_ticks(int ticks);

#endif // TIMER_H