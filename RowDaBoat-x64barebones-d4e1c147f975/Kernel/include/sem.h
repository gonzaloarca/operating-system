#ifndef SEM_H

#define SEM_H

#include <stdint.h>

typedef unsigned int sem_t;

sem_t *sys_sem_open(unsigned int id, unsigned int init);

int sys_sem_close(unsigned int id);

#endif
