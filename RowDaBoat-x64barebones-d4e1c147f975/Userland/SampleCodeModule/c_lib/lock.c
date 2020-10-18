#include <comandos.h>
#include <lock.h>
#include <syscalls.h>

unsigned int *createLock() {
	unsigned int *lock;
	unsigned int aux;
	if((lock = malloc(sizeof(aux))) == NULL) {
		return NULL;
	}

	*lock = 0;
	return lock;
}

void deleteLock(unsigned int *lock) {
	free(lock);
}

void acquire(unsigned int *lock) {
	while(_xchg(1, lock) != 0)
		;
}

void release(unsigned int *lock) {
	_xchg(0, lock);
}