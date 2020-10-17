#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <pipe.h>
#include <stdint.h>

#define STACK_SIZE 32760 // 32 KB - 8 bytes (sino el buddy system pasa a dar 64 KB). 32 KB = 32768, 16 KB = 16384
#define MAX_QUANTUM 5
#define DEFAULT_QUANTUM 4

#define ACTIVE 1
#define BLOCKED 0
#define KILLED 2
#define BLOCKED_BY_FG 3

typedef struct {
	unsigned int pid; //process ID del programa
	uint64_t *mem;	  //inicio de la memoria para el stack del proceso
	uint64_t rsp;	  //stack pointer del proceso
	char state;	  //activo o bloqueado
	uint64_t mainPtr; //puntero al inicio del programa
	int argc;
	char **argv;
	unsigned int priority;	     //dónde empieza a contar sus quantums
	unsigned int quantumCounter; //contador para saber si terminó sus quantums
	int pipeList[MAX_PIPES];     //arreglo donde cada proceso almacenara los pipes correspondientes a cada file descriptor, -1 indica vacio
} PCB;

//  Nodo para la lista de procesos
typedef struct ProcNode {
	struct ProcNode *next;
	struct ProcNode *previous;
	PCB pcb;
} ProcNode;

//Inicializa un proceso en background, devuelve su pid
int sys_startProcBg(uint64_t mainPtr, int argc, char const *argv[]);

//Inicializa un proceso en foreground, devuelve su pid
int sys_startProcFg(uint64_t mainPtr, int argc, char const *argv[]);

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

//Funcion que usa pipe.c para asignarle al proceso actual el nuevo pipe creado, retorna un puntero al vector [ReadIdx, WriteIdx]
int setPipe(unsigned int newPipeId, int pipefd[2]);

// Funcion que utiliza Pipe para sacarle al proceso actual el pipe que se encuentra en el indice indicado
int removePipe(int fd);

// Funcion que le proporciona a pipe.c el pipe en el indice indicado
int getPipeId(int fd);

// Funcion que pisa oldfd con newfd en el proceso actual
int sys_dup2(int oldfd, int newfd);

#endif
