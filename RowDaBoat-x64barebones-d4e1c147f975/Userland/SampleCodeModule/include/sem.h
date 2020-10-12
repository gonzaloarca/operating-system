#ifndef SEM_H
#define SEM_H

// Tipo de dato utilizado para semaforos
typedef unsigned int sem_t;

// Funcion que consulta el estado de un semaforo y bloquea si llega a valer 0.
// En caso de que el valor del semaforo sea mayor que 0, lo decrementa por 1
void semWait(sem_t *sem);

// Funcion que incrementa atomicamente el valor de un semaforo en una unidad
void semPost(sem_t *sem);

#endif