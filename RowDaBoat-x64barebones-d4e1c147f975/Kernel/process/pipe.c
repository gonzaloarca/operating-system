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
	if( nWrite == nRead )
		return 0;

	else if( nWrite > nRead)
		return nWrite - nRead -1;

	else
		// nWrite < nRead
		return PIPE_SIZE - (nRead - nWrite);
}

// Funcion que retorna cuantos caracteres se pueden escribir en el buffer
static int canWrite(int nRead, int nWrite) {
	if( nWrite == nRead )
		return 0;

	else if( nWrite > nRead)
		return PIPE_SIZE - (nWrite - nRead);

	else
		// nWrite < nRead
		return nRead - nWrite -1;
}

static Pipe *findPipe(int pipeId) {
	if(pipeId == -1 || pipeId > lastPipeId)
		return NULL;

	Pipe *search = first;
	while(search->pipeId < pipeId)
		search = search->next;

	if(search->pipeId != pipeId)
		return NULL;
	else
		return search;
}


int sys_read(int fd, char* out_buffer, unsigned long int count){
	Pipe *pipe;
	int pipeId, limit, ret = 0;
	if((pipeId = getPipeId(fd)) == -1)
		return -1;

	if(pipeId == 0)
		// siempre puedo leer de keyboard
		return readKeyboard(out_buffer, count);

	if((pipe = findPipe(pipeId)) == NULL)
		return -1;

	acquire(pipe->lock);
	while( ret < count ){
		while( (limit = canRead(pipe->nRead, pipe->nWrite)) == 0) {
			release(pipe->lock);
			sys_sleep(pipe->channelId);
			acquire(pipe->lock);
		}
		for(int i = 0; i < limit ; i++){
			out_buffer[i+ret] = pipe->buffer[(pipe->nRead + i) % PIPE_SIZE];			
		}
		pipe->nRead = (pipe->nRead + limit) % PIPE_SIZE;
		ret += limit;
	}
	release(pipe->lock);

	return ret;
}

int sys_write(int fd, const char *str, unsigned long count) {
	Pipe *pipe;
	int ret = 0, pipeId, limit;
	if((pipeId = getPipeId(fd)) == -1)
		return -1;

	if(pipeId == 0) 
		// siempre puedo escribir a la pantalla
		return writeScreen(str, count);
	
	if((pipe = findPipe(pipeId)) == NULL)
		return -1;

	acquire(pipe->lock);

	if((limit = canWrite(pipe->nRead, pipe->nWrite)) >0 ) {
		for(; ret < limit  && ret < count; ret++){
			pipe->buffer[pipe->nWrite] = str[ret];
			pipe->nWrite = (pipe->nWrite + 1) % PIPE_SIZE;
		}
		sys_wakeup(pipe->channelId);
	}

	release(pipe->lock);

	return ret;
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