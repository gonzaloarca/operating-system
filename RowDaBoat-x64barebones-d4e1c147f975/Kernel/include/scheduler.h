#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <stdint.h>

#define STACK_SIZE 16384
#define MAX_QUANTUM 5
#define DEFAULT_QUANTUM 4

#define ACTIVE  1
#define BLOCKED 0
#define KILLED 2
#define BLOCKED_BY_FG 3

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

//Inicializa un proceso en background, devuelve su pid
unsigned int sys_startProcBg(uint64_t mainPtr, int argc, char const *argv[]);

//Inicializa un proceso en foreground, devuelve su pid
unsigned int sys_startProcFg(uint64_t mainPtr, int argc, char const *argv[]);

//Funcion wrapper de ASM para agarrar el return del main de los programas
void _start(int *(mainPtr)(int, char const **), int argc, char const *argv[]);

//Obtiene el RSP del proximo proceso a ejecutar
uint64_t getNextRSP(uint64_t rsp);

//Activa el flag para que el keyboard fuerce el cambio al proceso en foreground
void triggerForeground();

//Syscall para terminar el proceso actual
void sys_exit();

//Syscall para obtener el pid actual
unsigned int sys_getpid();

//Syscall para cambiar el estado de un proceso (no sólo para matarlo)
int sys_kill(unsigned int pid, char state);

//Syscall para listar los procesos en la lista actualmente
void sys_listProcess();

//Syscall para que el proceso actual le ceda el CPU al siguiente proceso
void sys_runNext();

//Syscall para cambiar la prioridad de un proceso
int sys_nice(unsigned int pid, unsigned int priority);

#endif
