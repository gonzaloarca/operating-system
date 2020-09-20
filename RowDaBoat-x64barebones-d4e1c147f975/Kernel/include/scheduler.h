#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <stdint.h>

#define STACK_SIZE 16384
#define ACTIVE  1
#define BLOCKED 0

void createStackFrame(uint64_t frame, uint64_t mainptr, int argc, int argv);

int sys_start(uint64_t mainPtr, int argc, char const *argv[]);

uint64_t getNextRSP(uint64_t rsp);

/*
void sys_exit();

int sys_kill(int pid);

int sys_getpid();

void sys_listProcess();

void sys_nice();

void sys_changeStatus();
*/
#endif