#include <lock.h>
#include <memManager.h>
#include <libasm64.h>

int *createLock(){
    int *lock;
    if((lock = sys_malloc(sizeof(lock))) == NULL){
        return NULL;
    }

    *lock = 0;
    return lock;
}

void deleteLock(int *lock){
    sys_free(lock);
}

void acquire(int *lock){
    while(_xchg(1, lock) != 0);
}

void release(int *lock){
    _xchg(0, lock);
}