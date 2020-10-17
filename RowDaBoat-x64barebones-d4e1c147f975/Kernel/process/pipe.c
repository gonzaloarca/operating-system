#include <lock.h>
#include <memManager.h>
#include <pipe.h>
#include <scheduler.h>
#include <sig.h>
#include <stdint.h>
#include <window_manager.h>

static Pipe *first;
static unsigned int lastPipeId = 0;

int canRead(int nRead, int nWrite) {
	return (nWrite - nRead) != 0;
}

int canWrite(int nRead, int nWrite) {
	return (nWrite - nRead) != -1 && !(nWrite == PIPE_SIZE - 1 && nRead == 0);
}

static Pipe *findPipe(int pipeId) {
	if(first == NULL)
		return NULL;

	Pipe *search = first;
	while(search->pipeId < pipeId)
		search = search->next;

	if(search->pipeId != pipeId)
		return NULL;
	else
		return search;
}

int sys_write(int fd, const char *str, unsigned long count) {
	Pipe *search;
	int written = 0, pipeId = getPipeId(fd);
	if(pipeId == -1)
		return -1;

	if(pipeId == 0) {
		// siempre puedo escribir a la pantalla
		return writeScreen(str, count);
	}

	search = findPipe(pipeId);

	acquire(search->lock);
	while(canWrite(search->nRead, search->nWrite) && written < count) {
		search->buffer[search->nWrite] = str[written];
		search->nWrite = (search->nWrite + 1) % PIPE_SIZE;
		written++;
	}
	release(search->lock);

	return written;
}

int sys_createPipe(int pipefd[2]) {
	Pipe *aux, *search, auxSizeOf;

	if((aux = sys_malloc(sizeof(auxSizeOf))) == NULL) {
		writeScreen("NO HAY ESPACIO DISPONIBLE PARA EL PIPE\n", 40);
		return -1;
	}

	aux->nRead = 0;
	aux->nWrite = 0;
	aux->pipeId = lastPipeId++;
	aux->processCount = 2; // el proceso que lo crea tiene acceso a escritura y lectura
	if((aux->channelId = sys_createChannel()) == -1) {
		writeScreen("NO HAY ESPACIO DISPONIBLE PARA EL CHANNEL DEL PIPE\n", 51);
		sys_free(aux);
		return -1;
	}

	if((aux->lock = createLock()) == NULL) {
		writeScreen("NO HAY ESPACIO DISPONIBLE PARA EL LOCK DEL PIPE\n", 48);
		sys_deleteChannel(aux->channelId);
		sys_free(aux);
		return -1;
	}

	aux->next = NULL;

	// lo agrego a la lista
	if(first == NULL) {
		first = aux;
	} else {
		search = first;
		while(search->next != NULL) {
			search = search->next;
		}
		search->next = aux;
	}

	return setPipe(aux->pipeId, pipefd);
}

int sys_closePipe(int fd) {
	int pipeId = removePipe(fd);
	if(pipeId == -1) {
		return -1;
	}

	Pipe *search = first;
	Pipe *previous = NULL;
	while(search->pipeId < pipeId) {
		previous = search;
		search = search->next;
	}

	if(search->pipeId == pipeId) {
		//Solo borro el pipe si no hay nadie afectado
		if(search->processCount == 0) {
			if(previous == NULL) {
				first = search->next;
			} else {
				previous->next = search->next;
			}
			sys_free(search->buffer);
			deleteLock(search->lock);
			sys_deleteChannel(search->channelId);
			sys_free(search);
		} else
			search->processCount--;

	} else {
		sys_write(2, "TODAVIA HAY PROCESOS AFECTADOS POR EL PIPE\n", 37);
		return -1;
	}

	return 0;
}

int incPipeProcesses(int pipeId) {
	Pipe *search = findPipe(pipeId);
	if(search == NULL)
		return -1;

	search->processCount++;
	return 0;
}