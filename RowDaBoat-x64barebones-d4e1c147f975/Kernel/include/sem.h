#ifndef SEM_H

#define SEM_H

#include <stdint.h>

typedef unsigned int sem_t;

sem_t *sys_semOpen(unsigned int id, unsigned int init);

int sys_semClose(sem_t *sem);

int sys_semBlock(sem_t *sem);

#endif
