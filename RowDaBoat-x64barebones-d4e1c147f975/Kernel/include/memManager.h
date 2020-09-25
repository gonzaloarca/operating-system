#ifndef _MEM_MANAGER_H_
#define _MEM_MANAGER_H_ 

#include <stddef.h>

#define PWRTWO(x) (1 << (x))
#define MEM_SIZE_POW 23 // 2^23 B = 8 MB
#define MEM_BASE ((unsigned char*)0x700000)

#define sizeof(type) ((unsigned char *)(&type+1)-(unsigned char*)(&type))

void *sys_malloc(size_t size);
void sys_free(void *ptr);

typedef struct{
    size_t totalMem;
    size_t occMem;
    size_t freeMem;
} MemStatus;

void sys_getMemStatus(MemStatus *stat);

#endif
