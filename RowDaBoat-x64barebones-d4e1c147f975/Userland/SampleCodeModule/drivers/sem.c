#include <syscalls.h>
#include <comandos.h>

#include <std_io.h>

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
