#ifndef SEM_H

#define SEM_H

#include <stdint.h>

typedef unsigned int sem_t;

int sys_semBlock(sem_t *sem);

#endif
