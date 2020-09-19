#ifndef _MEM_MANAGER_H_
#define _MEM_MANAGER_H_ 

#include <stddef.h>

#define PWRTWO(x) (1 << (x))
#define MEM_SIZE_POW 23 // 2^23 B = 8 MB
#define MEM_BASE ((unsigned char*)0x700000)

void *malloc(size_t size);
void free(void *ptr);

#endif