#ifndef RTC_DRIVER_H_
#define RTC_DRIVER_H_

#include <stdint.h>
#include <libasm64.h>

void sys_getTime(TimeFormat *time);

#endif
