#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <stdint.h>

#define STACK_SIZE 16384
#define ACTIVE  1
#define BLOCKED 0
#define KILLED 2

uint64_t createStackFrame(uint64_t frame, uint64_t mainptr, int argc, int argv);

int sys_start(uint64_t mainPtr, int argc, char const *argv[]);

uint64_t getNextRSP(uint64_t rsp);

void sys_exit();

unsigned int sys_getpid();

int sys_kill(unsigned int pid, char state);

void sys_listProcess();

/*

void sys_nice();

void sys_changeStatus();
*/

#endif