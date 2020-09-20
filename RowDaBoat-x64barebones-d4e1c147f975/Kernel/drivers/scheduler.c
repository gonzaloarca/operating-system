#include <scheduler.h>
#include <memManager.h>

#include <window_manager.h>

typedef struct 
{
	unsigned int pid;				        //process ID del programa
    uint64_t *mem;                          //inicio de la memoria para el stack del proceso
    uint64_t rsp;                           //stack pointer del proceso
    int state;                             //activo o bloqueado
	uint64_t mainPtr;               		//puntero al inicio del programa
    int argc;
    uint64_t argv;
} PCB;

//  Nodo para la lista de procesos
typedef struct ProcNode
{
    struct ProcNode *next;
    PCB pcb;
} ProcNode;

static ProcNode *currentProc, *lastProc, nodeAux;
static unsigned int lastPID = 0;

int sys_start(uint64_t mainPtr, int argc, char const *argv[]){
    if((void *) mainPtr == NULL)
        return -1;
    
    ProcNode *new = sys_malloc(sizeof(nodeAux));            //Creo el nuevo nodo
    if(new == NULL){
        return -1;
    }
    // Le seteo los datos
    new->pcb.pid = lastPID++;
    new->pcb.mem = sys_malloc(STACK_SIZE);
    if(new->pcb.mem == NULL){
        return -1;
    }
    new->pcb.rsp = (uint64_t) new->pcb.mem + STACK_SIZE;
    new->pcb.state = ACTIVE;
    new->pcb.mainPtr = mainPtr;
    new->pcb.argc = argc;
    new->pcb.argv = (uint64_t) argv;

    createStackFrame(new->pcb.rsp, mainPtr, argc, (uint64_t) argv);

    new->pcb.rsp -= 8*20;

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
        if (lastProc == NULL)   //  Todavia no hay procesos
            return 0;
        currentProc = lastProc->next;
    }

    currentProc->pcb.rsp = rsp;
    
    do{
        currentProc = currentProc->next;
    } while(currentProc->pcb.state == BLOCKED);

    return currentProc->pcb.rsp;
}
