#include <syscalls.h> //para usar sleep, wakeup, malloc y free
#include <comandos.h>
#include <std_io.h>
#include <sem.h>

typedef struct SemNode{
    Semaphore sem;                //valor actual del semaforo
    struct SemNode *next;       
} SemNode;

static SemNode *semList;         //Lista de semaforos
static void createNode(SemNode *node, unsigned int id, unsigned int init, unsigned int channelId);

void semWait(Semaphore *sem){
    unsigned int semAux;
    
    //Si el semaforo tenia 0, se bloquea
    while((semAux = _xchg(0, &(sem->value))) == 0){
        sleep(sem->channelId);
    }
    printf("hola soy %d y el semaforo ahora vale %d\n", getpid(), sem->value);
    //Una vez que el semaforo es distinto de 0, se disminuye su valor de forma atomica
    _xchg(semAux-1, &(sem->value));
}

void semPost(Semaphore *sem){
    printf("post\n");
    _inc(&(sem->value));
    printf("hola soy %d y el semaforo ahora vale %d\n", getpid(), sem->value);
    wakeup(sem->channelId);
    printf("wakeup\n");
}


Semaphore *semOpen(unsigned int id, unsigned int init){
    SemNode aux;
    unsigned int channelId;
    //Inicializo la lista
    if(semList == NULL){
        if((semList = malloc(sizeof(aux))) == NULL)
            return NULL;

        if((channelId = createChannel()) == -1){
            free(semList); //Se pudo alocar la lista pero no se pudo crear el canal, por ende deshago lo creado
            semList = NULL;
            return NULL;
        }

        createNode(semList, id, init, channelId);
        return &(semList->sem);
    }    
    
    SemNode *search;
    //Busco el id en la lista. Si lo encuentro, me quedo con el valor que ya tenía
    for(search = semList; ; search = search->next){
        if(search->sem.semId == id){
            (search->sem.count)++;
            return &(search->sem);
        }
        if(search->next == NULL)
            break;
    }

    //Si no lo encontré, creo un nuevo semaforo y un canal de comunicacion
    if((search->next = malloc(sizeof(aux))) == NULL ){
        return NULL;
    }

    if((channelId = createChannel()) == -1){
        free(search->next); //Se pudo alocar espacio para el nodo pero no se pudo crear el canal, por ende deshago lo creado
        search->next = NULL;
        return NULL;
    }
    
    search = search->next;
    createNode(search, id, init, channelId);
    return &(search->sem);
}

static void createNode(SemNode *node, unsigned int id, unsigned int init, unsigned int channelId){
    node->sem.semId = id;
    node->sem.value = init;
    node->sem.count = 0;
    node->sem.channelId = channelId;
    node->next = NULL;
}

int semClose(Semaphore *sem){
    SemNode *search, *previous = NULL;

    for(search = semList; search != NULL ; previous = search, search = search->next){
        if(&(search->sem) == sem){
            (search->sem.count)--;
            //Si ya lo cerraron todos los procesos, saco el semaforo de la lista
            if(search->sem.count == 0){
                deleteChannel(search->sem.channelId);
                if(previous == NULL){  
                    //Este caso es cuando el id es el del primer semaforo de la lista
                    semList = search->next;
                } else{
                    previous->next = search->next;
                }
                free(search);
            }
            return 0;
        }
    }
    return -1;
}

