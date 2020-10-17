#include <pipe.h>

static Pipe *first;
static unsigned int lastPipeId = 0;

int canRead(int nRead, int nWrite){
    return (nWrite - nRead) != 0;
}

int canWrite(int nRead, int nWrite){
    return (nWrite - nRead) != -1 && !(nWrite == PIPE_SIZE-1 && nRead == 0);
}

int sys_createPipe(){
    Pipe *aux, *search, auxSizeOf;

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
    aux->processCount = 1;
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

    return setPipe(aux); // QUE CARAJO RETORNA ESTO, 2 indices???
}

int sys_closePipe(unsigned int index){
    int pipeId = removePipe(index);
    if(pipeId == -1){
        return -1;
    }

    Pipe *search = first;
    Pipe *previous = NULL;
    while(search->pipeId < pipeId){
        previous = search;
        search = search->next;     
    }

    if(search->pipeId == pipeId){
    //Solo borro el pipe si no hay nadie afectado
        if(search->processCount == 0){
            if(previous == NULL){
                first = search->next;
            } else {
                previous->next = search->next;
            }
            sys_free(search->buffer);
            deleteLock(search->lock);
            sys_deleteChannel(search->channel);
            sys_free(search);
        }else
            search->processCount--;
        
    } else {
        sys_write(2,"TODAVIA HAY PROCESOS AFECTADOS POR EL PIPE\n", 37);
        return -1;
    }

    return 0;
}

