#include <screenInfo.h>
#include <time.h>

static unsigned long ticks = 0;

void timer_handler() {
	ticks++;
	if((ticks % IDLE_SYMBOL_TIME) == 0)
		idleSymbol();
}

void decrease_ticks() {
	ticks--;
}

int ticks_elapsed() {
	return ticks;
}

int seconds_elapsed() {
	return ticks / 18;
}
