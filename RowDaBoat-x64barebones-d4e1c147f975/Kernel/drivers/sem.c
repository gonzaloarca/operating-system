#include <sem.h>
#include <scheduler.h>

int sys_semBlock(sem_t *sem){
    //Se podria validar que el semaforo exista y valga 0
    return setSemaphore(sem);
}
