#include <time.h>
#include <screenInfo.h>

static unsigned long ticks = 0;

void timer_handler() {
	ticks++;
	if((ticks % IDLE_SYMBOL_TIME) == 0)
		idleSymbol();
}

int ticks_elapsed() {
	return ticks;
}

int seconds_elapsed() {
	return ticks / 18;
}
