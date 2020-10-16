#include <scheduler.h>
#include <memManager.h>
#include <window_manager.h>
#include <time.h>
#include <interrupts.h>

static ProcNode *currentProc, *lastProc, nodeAux;
static unsigned int lastPID = 1;
static int fgFlag = 0;                              //Flag que indica si debe haber cambio al proceso en foreground
static int shellFlag = 0;                           //Flag para indicar que quiero poner a la shell como proceso unico en fg

//Funcion para setear los datos en el Stack Frame de un nuevo proceso
static uint64_t createStackFrame(uint64_t frame, uint64_t mainptr, int argc, uint64_t argv);

//Funcion para liberar un nodo de la lista de procesos
static void freeResources(ProcNode **node);

//Fuerzo el cambio al proceso en foreground (el primero de mi lista)
static void switchForeground();

//Cierro todos los proc en fg menos la shell
static void returnToShell();

//Funcion auxiliar para contar caracteres de un string
static int strlen(const char* str);

//Funcion auxiliar para copiar los argumentos que recibe el proceso
static void copyArgs(int argc, const char ** from, char ***into);

unsigned int sys_startProcBg(uint64_t mainPtr, int argc, char const *argv[]){
    if((void *) mainPtr == NULL){
        sys_write(2,"Error en funcion a ejecutar\n", 28);
        return -1;
    }
    
    ProcNode *new = sys_malloc(sizeof(nodeAux));            //Creo el nuevo nodo
    if(new == NULL){
        sys_write(2,"Error en malloc de nodo\n", 24);
        return -1;
    }
    // Le seteo los datos
    new->pcb.pid = lastPID++;
    new->pcb.mem = sys_malloc(STACK_SIZE);

    if(new->pcb.mem == NULL){
        sys_write(2,"Error en malloc de PCB\n", 23);
        return -1;
    }

    new->pcb.rsp = (uint64_t) new->pcb.mem + STACK_SIZE;
    new->pcb.state = ACTIVE;
    new->pcb.mainPtr = mainPtr;
    
    new->pcb.argc = argc;
    copyArgs(argc, argv, &new->pcb.argv);

    new->pcb.priority = DEFAULT_QUANTUM;
    new->pcb.quantumCounter = DEFAULT_QUANTUM;

    //ALINEAR
    new->pcb.rsp &= -8;

    //  Armo el stack frame del proceso nuevo
    new->pcb.rsp = createStackFrame(new->pcb.rsp, mainPtr, argc, (uint64_t) argv);

    //  Si la lista está vacía
    if(lastProc == NULL){
        lastProc = new;
        new->next = new;
        new->previous = new;
    }else {
        new->next = lastProc->next;
        new->previous = lastProc;
        lastProc->next = new;
        new->next->previous = new;
        lastProc = new;
    }

    return new->pcb.pid;
}

unsigned int sys_startProcFg(uint64_t mainPtr, int argc, char const *argv[]){
    //Solamente el primer proceso de la lista puede iniciar a otros en foreground
    if(currentProc != NULL && currentProc != lastProc->next)
        return 0;

    //Primero inicio el proceso como si fuera en Background
    unsigned pid = sys_startProcBg(mainPtr, argc, argv);

    //Luego bloqueo el proceso que lo llamó (el primero de la lista) si no es el unico elemento en la lista
    if(lastProc != lastProc->next)
        lastProc->next->pcb.state = BLOCKED_BY_FG;
    //El nuevo proceso de foreground tiene la misma prioridad que el que reemplaza
    lastProc->pcb.priority = lastProc->next->pcb.priority;
    //Y mi nuevo proceso reemplaza al primero de la lista (muevo una posicion atras el puntero al ultimo)
    lastProc = lastProc->previous;

    //Cambio al nuevo proceso
    triggerForeground();
    sys_runNext();

    return pid;
}

static uint64_t createStackFrame(uint64_t frame, uint64_t mainptr, int argc, uint64_t argv){
    uint64_t *framePtr = (uint64_t*)frame - 1;

    //Datos para el iretq
    *framePtr = 0; //SS
    framePtr--;
    *framePtr = (uint64_t)framePtr; //RSP
    framePtr--;
    *framePtr = 0x202; //RFLAGS
    framePtr--;
    *framePtr = 0x8; //CS
    framePtr--;
    *framePtr = (uint64_t)_start; //RIP
    framePtr--;

    //Lleno los registros con valores crecientes de 0 a 14
    for(int i = 0; i < 15; i++, framePtr--)
        *framePtr = i;
    
    //framePtr me apunta una posicion despues de r15, lo incremento para que apunte a r15
    framePtr++;

    //Cargo rdi, rsi y rdx con sus respectivos argumentos para que _start los levante
    *(framePtr+9) = mainptr; //rdi
    *(framePtr+8) = argc;
    *(framePtr+11) = argv;

    return (uint64_t)framePtr;
}

//  Guarda el rsp del proceso que lo llama por la interrupcion
//  Luego devuelve el rsp del proximo proceso a ejecutar
uint64_t getNextRSP(uint64_t rsp){

    if(currentProc == NULL){     //  Si todavia no está corriendo ningún proceso
        if(lastProc == NULL){    //  Todavia no hay procesos
            return 0;            //retorna 0 ya que se corresponderia con NULL en cuanto a direcciones, y esta funcion retorna una direccion
        }
        currentProc = lastProc->next;
    }else
        currentProc->pcb.rsp = rsp;

    //Si el proceso que acabo de correr se murio, tipicamente mediante un exit
    if(currentProc->pcb.state == KILLED){
        freeResources(&currentProc);
    }

    if(shellFlag){
        returnToShell();
        return currentProc->pcb.rsp;
    }

    if(fgFlag){                 //  Si tengo que cambiar al proceso en foreground
        switchForeground();
        return currentProc->pcb.rsp;
    }

                                //  En este if vemos si le toca cambiar al proceso
    if(currentProc->pcb.quantumCounter == MAX_QUANTUM || currentProc->pcb.state != ACTIVE ){  
        currentProc->pcb.quantumCounter = currentProc->pcb.priority; //Si llego a su maximo de quantums, lo reseteo
        
        do{                    //Encuentro algun proceso no bloqueado para correr
            currentProc = currentProc->next;
        }while(currentProc->pcb.state != ACTIVE);
    }

    currentProc->pcb.quantumCounter++;

    return currentProc->pcb.rsp;
}

static void freeResources(ProcNode **node){
    // Si el proceso a borrar es el ultimo, se debe modificar el lastProc para que sea el anterior a este
    if(*node == lastProc)
        lastProc = (*node)->previous;
    // Si voy a borrar el primer proceso (el que está en foreground) tengo que desbloquear al siguiente
    if(*node == lastProc->next || (*node)->next->pcb.state == BLOCKED_BY_FG)
        (*node)->next->pcb.state = ACTIVE;

    if((*node)->pcb.argv != NULL){
        for(int i = 0; i < (*node)->pcb.argc ; i++)
            sys_free((*node)->pcb.argv[i]);

        sys_free((*node)->pcb.argv);
    }

    sys_free((*node)->pcb.mem);
    *node = (*node)->next;
    (*node)->previous = (*node)->previous->previous;
    sys_free((*node)->previous->next);
    (*node)->previous->next = *node;
}

//  Se activa un flag para cambiar al proceso que corre en foreground
void triggerForeground(){
    fgFlag = 1;
    sys_runNext();
}

static void switchForeground(){
    currentProc->pcb.quantumCounter = currentProc->pcb.priority;
    currentProc = lastProc->next;
    currentProc->pcb.state = ACTIVE;
    currentProc->pcb.quantumCounter = currentProc->pcb.priority + 1;
    fgFlag = 0;
}

//  Syscall para eliminar el proceso actual de la lista de procesos
void sys_exit(){
    if(currentProc == NULL)
        return;         //No hay nada corriendo

    currentProc->pcb.state = KILLED;
    sys_runNext();
}

//  Syscall que retorna PID del proceso actual
unsigned int sys_getpid(){
    return currentProc->pcb.pid;
}

// Syscall que imprime los procesos actuales
void sys_listProcess(){
    char fg;
    ProcNode *aux = lastProc->next; // Arranco desde el primero
    printProcessListHeader();
    do{
        if(aux == lastProc->next || aux->pcb.state == BLOCKED_BY_FG)
            fg = 1;
        else
            fg = 0;
        printProcess(aux->pcb.argv, aux->pcb.pid, aux->pcb.priority, aux->pcb.rsp, (uint64_t)(((char*)aux->pcb.mem) + STACK_SIZE - 8), fg, aux->pcb.state);
        aux = aux->next;
    }while(aux != lastProc->next);
}

// Syscall para cambiar el estado de un pid especifico
int sys_kill(unsigned int pid, char state){
    if(lastProc == NULL || state == BLOCKED_BY_FG)
        return -1;

    ProcNode *search = lastProc;
    //  Realizo la busqueda del proceso con el pid y lo marco como KILLED
    do{
        search = search->next;
        if(search->pcb.pid == pid){
            if(state == KILLED){
                if(pid == 1){ //Si es la shell, no la mato
                   return -1;
                }

                if(search == currentProc){ //Si un proceso se quiere matar a si mismo, el scheduler deberia encargarse de el
                    sys_exit();
                }

                freeResources(&search);
                return 0;
            } else{
                search->pcb.state = state;
                return 0;
            }
        }
    }while(search != lastProc);

    return -1;
}

//Syscall para que el proceso corriendo en el momento renuncie al CPU y se corra el siguiente proceso
void sys_runNext(){
    currentProc->pcb.quantumCounter = MAX_QUANTUM;
    decrease_ticks();
    forceTick();
}

//Syscall para cambiar la prioridad de un proceso
int sys_nice(unsigned int pid, unsigned int priority){
    if(pid > lastPID || priority >= MAX_QUANTUM)
        return -1;

    ProcNode *search = lastProc;
    //  Realizo la busqueda del proceso con el pid
    do{
        search = search->next;
        if(search->pcb.pid == pid){
            if(search->pcb.state == KILLED)
                return -1;
            else
            {                
                search->pcb.priority = priority;
                search->pcb.quantumCounter = priority;
                return 1;
            }
        }
    }while(search != lastProc);

    return -1;
}

static void copyArgs(int argc, const char ** from, char ***into){
    if(argc == 0 || from == NULL){
        *into = NULL;
        return;
    }

    const char * aux = "aux";
    *into = (char **) sys_malloc((argc+1)* sizeof(aux));
    for(int i = 0, j = 0, length; i < argc ; i++){
        
        length = strlen(from[i]);
        (*into)[i] = sys_malloc((length+1)*sizeof(aux[0]));

        for(j = 0; j < length ; j++)
            (*into)[i][j] = from[i][j];

        (*into)[i][j+1] = 0;
    }
    (*into)[argc] = NULL;
}

static int strlen(const char* str){
    int ans = 0;
    for(; str[ans] != 0 ; ans++);
    return ans;
}

void triggerShell(){
    shellFlag = 1;
    sys_runNext();
}

static void returnToShell(){
    ProcNode *aux;

    currentProc->pcb.quantumCounter = currentProc->pcb.priority;

    currentProc = lastProc->next;       //Voy al primer proceso
    while(currentProc->pcb.pid != 1){
        aux = currentProc;
        //Si no es la shell, voy al siguiente
        currentProc = currentProc->next;
        //Y borro el anterior (que no era la shell)
        freeResources(&aux);
    }

    currentProc->pcb.state = ACTIVE;
    currentProc->pcb.quantumCounter = currentProc->pcb.priority + 1;
    shellFlag = 0;
}
