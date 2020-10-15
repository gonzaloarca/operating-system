#ifndef LOCK_H
#define LOCK_H

// Funcion que crea un spinlock. Si no hay espacio retorna NULL
int *createLock();

// Funcion que elimina un spinlock. Si el spinlock que se le pasa no existe, el comportamiento es indefinido
void deleteLock(int *lock);

// Funcion que espera activamente por el acceso a un spinlock
void acquire(int *lock);

// Funcion que habilita el acceso a un spinlock
void release(int *lock);

#endif