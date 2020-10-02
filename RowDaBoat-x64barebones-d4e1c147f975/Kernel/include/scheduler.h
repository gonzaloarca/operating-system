#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <stdint.h>

#define STACK_SIZE 16384
#define MAX_QUANTUM 5
#define DEFAULT_QUANTUM 4

#define ACTIVE  1
#define BLOCKED 0
#define KILLED 2

typedef struct 
{
	unsigned int pid;				        //process ID del programa
    uint64_t *mem;                          //inicio de la memoria para el stack del proceso
    uint64_t rsp;                           //stack pointer del proceso
    char state;                             //activo o bloqueado
	uint64_t mainPtr;               		//puntero al inicio del programa
    int argc;
    char **argv;
    unsigned int priority;                  //dónde empieza a contar sus quantums
    unsigned int quantumCounter;            //contador para saber si terminó sus quantums
} PCB;

//  Nodo para la lista de procesos
typedef struct ProcNode
{
    struct ProcNode *next;
    struct ProcNode *previous;
    PCB pcb;
} ProcNode;

uint64_t createStackFrame(uint64_t frame, uint64_t mainptr, int argc, uint64_t argv);

int sys_start(uint64_t mainPtr, int argc, char const *argv[]);

void _start(int *(mainPtr)(int, char const **), int argc, char const *argv[]);

uint64_t getNextRSP(uint64_t rsp);

void freeResources(ProcNode **node);

void triggerForeground();

void switchForeground();

void sys_exit();

unsigned int sys_getpid();

int sys_kill(unsigned int pid, char state);

void sys_listProcess();

void sys_runNext();

int sys_nice(unsigned int pid, unsigned int priority);

#endif
