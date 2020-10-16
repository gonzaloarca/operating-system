#include <lock.h>
#include <sig.h>
#include <memManager.h>
#include <scheduler.h>
#include <lock.h>

#define PIPE_SIZE 1024

typedef struct Pipe{
    char *buffer;
    unsigned int nRead;         // Indice en el cual se debe leer
    unsigned int nWrite;        // Indice en el cual se debe escribir
    unsigned int pipeId;
    int channel;                // canal de comunicacion para que los procesos de write esperen cuando el buffer esta lleno y los de read cuando esta vacio
    int lock;                   // lock que permite exclusion en escritura y lectura
    struct Pipe *next;
} Pipe;

static Pipe *first;
static unsigned int lastPipeId = 0;

int canRead(int nRead, int nWrite){
    acquire()
    return (nWrite - nRead) != 0;
}

int canWrite(int nRead, int nWrite){
    return (nWrite - nRead) != -1 && !(nWrite == PIPE_SIZE-1 && nRead == 0);
}

int createPipe(){
    Pipe *aux, *search, auxSizeOf = {0};

    if((aux = sys_malloc(sizeof(auxSizeOf))) == NULL){
        sys_write(2,"NO HAY ESPACIO DISPONIBLE PARA EL PIPE\n", 40);
        return -1;
    }

    if((aux->buffer = sys_malloc(PIPE_SIZE)) == NULL){
        sys_write(2,"NO HAY ESPACIO DISPONIBLE PARA EL BUFFER DEL PIPE\n", 40);
        sys_free(aux);
        return -1;
    };
    
    aux->nRead = 0;
    aux->nWrite = 0;
    aux->pipeId = lastPipeId++;
    if((aux->channel = sys_createChannel()) == -1){
        sys_free(aux->buffer);
        sys_free(aux);
        return -1;
    }

    if((aux->lock = createLock()) == NULL){
        sys_free(aux->buffer);
        sys_deleteChannel(aux->channel);
        sys_free(aux);
        return -1;
    };

    aux->next = NULL;

    // lo agrego a la lista
    if(first == NULL){
        first = aux;
    } else {
        search = first;
        while(search->next != NULL){
            search = search->next;
        }
        search->next = aux;
    }
    
    return aux->pipeId;
}

