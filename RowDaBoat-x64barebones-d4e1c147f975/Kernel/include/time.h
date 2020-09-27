#ifndef _TIME_H
#define _TIME_H

#include <screenInfo.h>
#include <window_manager.h>

void timer_handler();
void decrease_ticks();
int ticks_elapsed();
int seconds_elapsed();

#endif
