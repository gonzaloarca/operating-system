#include <sem.h>
#include <memManager.h>

typedef struct SemNode{
    unsigned int id;
    sem_t *value;
    SemNode *next;
} SemNode;

static SemNode *sem_list;         //Lista de semaforos

static void createNode(SemNode *node, unsigned int id, unsigned int init);

sem_t *sys_sem_open(unsigned int id, unsigned int init){
    SemNode aux;
    //Inicializo la lista
    if(sem_list == NULL){
        if ( (sem_list = sys_malloc(sizeof(aux))) == NULL)
            return NULL;
        createNode(sem_list, id, init);
        return &(sem_list->value);
    }    
    
    SemNode *search;
    //Busco el id en la lista. Si lo encuentro, me quedo con el valor que ya tenía
    for(search = sem_list; ; search = search->next){
        if(search->id == id)
            return &(search->value);
        if(search->next == NULL)
            break;
    }

    //Si no lo encontré, creo un nuevo semaforo
    search->next = sys_malloc(sizeof(aux));
    search = search->next;
    createNode(search, id, init);
    return &(search->value);
}

static void createNode(SemNode *node, unsigned int id, unsigned int init){
    node->id = id;
    node->value = init;
    node->next = NULL;
}

int sys_sem_close(unsigned int id){
    SemNode *search, *previous = NULL;

    for(search = sem_list; search != NULL ; previous = search, search = search->next){
        if(search->id == id){
            if(previous == NULL){  
                //Este caso es cuando el id es el del primer semaforo de la lista
                sem_list = search->next;
            } else{
                previous->next = search->next;
            }
            sys_free(search);
            return 0;
        }
    }

    return -1;
}
