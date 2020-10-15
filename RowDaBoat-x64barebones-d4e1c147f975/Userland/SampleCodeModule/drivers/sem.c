#include <syscalls.h> //para usar sleep, wakeup, malloc y free
#include <comandos.h>
#include <std_io.h>

typedef struct SemNode{
    unsigned int id;            //id del semaforo
    sem_t value;                //valor actual del semaforo
    unsigned int count;         //cantidad de procesos que abrieron el semaforo
    struct SemNode *next;       
} SemNode;

static SemNode *semList;         //Lista de semaforos
static void createNode(SemNode *node, unsigned int id, unsigned int init);

void semWait(sem_t *sem){
    unsigned int semAux;
    
    //Si el semaforo tenia 0, se bloquea
    while((semAux = _xchg(0, sem)) == 0){
        semBlock(sem);
    }
    
    //Una vez que el semaforo es distinto de 0, se disminuye su valor de forma atomica
    _xchg(semAux-1, sem);
}

void semPost(sem_t *sem){
    _inc(sem);
}


sem_t *semOpen(unsigned int id, unsigned int init){
    SemNode aux;
    //Inicializo la lista
    if(semList == NULL){
        if ( (semList = malloc(sizeof(aux))) == NULL)
            return NULL;
        createNode(semList, id, init);
        return &(semList->value);
    }    
    
    SemNode *search;
    //Busco el id en la lista. Si lo encuentro, me quedo con el valor que ya tenía
    for(search = semList; ; search = search->next){
        if(search->id == id){
            (search->count)++;
            return &(search->value);
        }
        if(search->next == NULL)
            break;
    }

    //Si no lo encontré, creo un nuevo semaforo
    search->next = malloc(sizeof(aux));
    search = search->next;
    createNode(search, id, init);
    return &(search->value);
}

static void createNode(SemNode *node, unsigned int id, unsigned int init){
    node->id = id;
    node->value = init;
    node->count = 0;
    node->next = NULL;
}

int semClose(sem_t *sem){
    SemNode *search, *previous = NULL;

    for(search = semList; search != NULL ; previous = search, search = search->next){
        if(&(search->value) == sem){
            (search->count)--;
            //Si ya lo cerraron todos los procesos, saco el semaforo de la lista
            if(search->count == 0){
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

