#include <scheduler.h>
#include <memManager.h>
#include <window_manager.h>
#include <time.h>

typedef struct 
{
	unsigned int pid;				        //process ID del programa
    uint64_t *mem;                          //inicio de la memoria para el stack del proceso
    uint64_t rsp;                           //stack pointer del proceso
    char state;                             //activo o bloqueado
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
static unsigned int lastPID = 1;

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

    //ALINEAR
    new->pcb.rsp &= -8;

    //  Armo el stack frame del proceso nuevo
    new->pcb.rsp = createStackFrame(new->pcb.rsp, mainPtr, argc, (uint64_t) argv);

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
        if(lastProc == NULL){   //  Todavia no hay procesos
            return 0;
        }
        currentProc = lastProc->next;
    }else
        currentProc->pcb.rsp = rsp;
    
    ProcNode *previous = currentProc;

    do{
        currentProc = currentProc->next;

        if(currentProc->pcb.state == KILLED){
            if(currentProc == lastProc)
                lastProc = previous;            // Si el proceso a borrar es el ultimo, se debe modificar el lastProc para que sea el anterior a este

            sys_free(currentProc->pcb.mem);
            currentProc = currentProc->next;
            sys_free(previous->next);
            previous->next = currentProc;
        }else
            previous = previous->next;
    }while(currentProc->pcb.state != ACTIVE);

    return currentProc->pcb.rsp;
}

//  Syscall para eliminar el proceso actual de la lista de procesos
void sys_exit(){
    if(currentProc == NULL)
        return;         //No hay nada corriendo

    currentProc->pcb.state = KILLED;
}

//  Syscall que retorna PID del proceso actual
unsigned int sys_getpid(){
    return currentProc->pcb.pid;
}

// Syscall que imprime los procesos actuales
void sys_listProcess(){
    ProcNode *aux = lastProc->next; // Arranco desde el primero
    printProcessListHeader();
    do{
        printProcess((char **)aux->pcb.argv, aux->pcb.pid, 13, aux->pcb.rsp, aux->pcb.mem[0], 7);
        aux = aux->next;
    }while(aux != lastProc->next);
}

// Syscall para cambiar el estado de un pid especifico
int sys_kill(unsigned int pid, char state){
    if(lastProc == NULL)
        return 0;

    ProcNode *search = lastProc;
    //  Realizo la busqueda del proceso con el pid y lo marco como KILLED
    do{
        search = search->next;
        if(search->pcb.pid == pid){
            if(search->pcb.state == KILLED)
                return 0;
            else
            {
                if(search->pcb.pid == 1 && state == KILLED)
                    return 0;
                
                search->pcb.state = state;
                return 1;
            }
        }
    }while(search != lastProc);

    return 0;
}

//Syscall para que el proceso corriendo en el momento renuncie al CPU y se corra el siguiente proceso
void sys_runNext(){
    decrease_ticks();
    forceTick();
}