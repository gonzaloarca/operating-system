#include <scheduler.h>
#include <memManager.h>

typedef struct 
{
	unsigned int pid;				        //process ID del programa
    uint64_t *mem;                          //inicio de la memoria para el stack del proceso
    uint64_t rsp;                           //stack pointer del proceso
    int state;                             //activo o bloqueado
	int (*mainPtr)(int, char const **);		//puntero al inicio del programa
    int argc;
    int argv;
} PCB;

//  Nodo para la lista de procesos
typedef struct 
{
    ProcNode *next;
    PCB pcb;
} ProcNode;

static ProcNode *currentProc, *lastProc, nodeAux;
static unsigned int lastPID = 0;

int sys_start(int *(mainPtr)(int, char const **), int argc, char const *argv[]){
    if(mainPtr == NULL)
        return -1;
    
    ProcNode *new = sys_malloc(sizeof(nodeAux));            //Creo el nuevo nodo
    if(new == NULL)
        return -1;

    // Le seteo los datos
    new->pcb.pid = lastPID++;
    new->pcb.mem = sys_malloc(STACK_SIZE);
    if(new->pcb.mem == NULL)
        return -1;
    new->pcb.rsp = (uint64_t) new->pcb.mem + STACK_SIZE;
    new->pcb.state = ACTIVE;
    new->pcb.mainPtr = mainPtr;
    new->pcb.argc = argc;
    new->pcb.argv = argv;

    createStackFrame((uint64_t*) new->pcb.rsp, (uint64_t) mainptr, (uint64_t) argc, (uint64_t) argv);

    //  Si la lista está vacía
    if(lastProc == NULL){
        lastProc = new;
        new->next = new;
    }else{
        new->next = lastProc->next;
        lastProc->next = new;
        lastProc = new;
    }

    return 0;
}

//  Guarda el rsp del proceso que lo llama por la interrupcion
//  Luego devuelve el rsp del proximo proceso a ejecutar
uint64_t getNextRSP(uint64_t rsp){
    //  Si todavia no está corriendo ningún proceso
    if(currentProc == NULL){
        //Hay que chequear si lastProc = NULL? (no hay procesos en la lista)
        currentProc = lastProc->next;
    }

    currentProc->pcb.rsp = rsp;
    
    do{
        currentProc = currentProc->next;
    } while(currentProc->pcb.state == BLOCKED);

    return currentProc->pcb.rsp;
}
