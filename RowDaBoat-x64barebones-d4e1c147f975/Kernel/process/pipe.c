#include <lock.h>
#include <memManager.h>
#include <pipe.h>
#include <scheduler.h>
#include <sig.h>
#include <stdint.h>
#include <window_manager.h>

static Pipe *createPipe(unsigned int pipeId);

static Pipe *first;

static int canRead(Pipe *pipe) {
	//Este caso indica que el buffer está vacio
	if(pipe->nWrite == pipe->nRead && pipe->isFull == 0)
		return 0;

	if(pipe->nRead >= pipe->nWrite)
		return PIPE_SIZE - (pipe->nRead - pipe->nWrite);
	else
		return pipe->nWrite - pipe->nRead;
}

// Funcion que retorna cuantos caracteres se pueden escribir en el buffer
static int canWrite(Pipe *pipe) {
	//Veo que no esté lleno el pipe
	if(pipe->isFull == 1)
		return 0;

	if(pipe->nWrite >= pipe->nRead)
		return PIPE_SIZE - (pipe->nWrite - pipe->nRead);
	else
		return pipe->nRead - pipe->nWrite;
}

static Pipe *findPipe(unsigned int pipeId) {
	Pipe *search = first;

	while(search != NULL && search->pipeId != pipeId)
		search = search->next;

	return search;
}

int sys_read(int fd, char *out_buffer, unsigned long int count) {
	Pipe *pipe;
	PipeEnd *end;
	int limit, ret = 0;

	if((end = getPipeEnd(fd)) == NULL)
		return -1;

	if(end->rw != READ)
		return -1;

	if(end->pipeId == STDIN_ID) {
		//Si voy a leer del teclado, tiene que ser por el proceso en foreground
		if(isForeground())
			return readKeyboard(out_buffer, count);
		//Si no está en foreground, lo voy a matar
		sys_exit();
	}

	if((pipe = findPipe(end->pipeId)) == NULL)
		return -1;

	acquire(pipe->lock);

	if(pipe->writers == 0) {
		release(pipe->lock);
		return -1;
	}

	while(ret < count) {
		while((limit = canRead(pipe)) == 0) {
			release(pipe->lock);
			sys_sleep(pipe->channelId);
			acquire(pipe->lock);
		}

		for(int i = 0; i < limit; i++) {
			out_buffer[i + ret] = pipe->buffer[(pipe->nRead + i) % PIPE_SIZE];
		}
		pipe->nRead = (pipe->nRead + limit) % PIPE_SIZE;
		ret += limit;
		pipe->isFull = 0;
	}
	release(pipe->lock);

	return ret;
}

int sys_write(int fd, const char *str, unsigned long count) {
	Pipe *pipe;
	PipeEnd *end;
	int ret = 0, limit;
	if((end = getPipeEnd(fd)) == NULL)
		return -1;

	if(end->rw != WRITE)
		return -1;

	if(end->pipeId == STDOUT_ID)
		return writeScreen(str, count);

	if((pipe = findPipe(end->pipeId)) == NULL)
		return -1;

	acquire(pipe->lock);

	if(pipe->readers == 0) {
		release(pipe->lock);
		return -1;
	}

	if((limit = canWrite(pipe)) > 0) {
		for(; ret < limit && ret < count; ret++) {
			(pipe->buffer)[pipe->nWrite] = str[ret];
			pipe->nWrite = (pipe->nWrite + 1) % PIPE_SIZE;
		}
		//Veo si se lleno el buffer
		if(pipe->nWrite == pipe->nRead)
			pipe->isFull = 1;
		sys_wakeup(pipe->channelId);
	}

	release(pipe->lock);

	return ret;
}

int sys_openPipe(unsigned int pipeId, int pipefd[2]) {
	Pipe *pipe;

	//No se pueden setear STDOUT y STDIN
	if(pipeId == STDOUT_ID || pipeId == STDIN_ID) {
		return -1;
	}

	if((pipe = findPipe(pipeId)) == NULL) {
		if((pipe = createPipe(pipeId)) == NULL)
			return -1;
	} else {
		acquire(pipe->lock);
		(pipe->readers)++;
		(pipe->writers)++;
		release(pipe->lock);
	}

	if(setPipe(pipeId, pipefd) == -1)
		return -1;

	return 0;
}

static Pipe *createPipe(unsigned int pipeId) {
	Pipe *aux, *search, auxSizeOf;

	if((aux = sys_malloc(sizeof(auxSizeOf))) == NULL) {
		writeScreen("NO HAY ESPACIO DISPONIBLE PARA EL PIPE\n", 40);
		return NULL;
	}

	aux->nRead = 0;
	aux->nWrite = 0;
	aux->isFull = 0;
	aux->pipeId = pipeId;
	aux->writers = 1;
	aux->readers = 1;
	if((aux->channelId = sys_createChannel()) == -1) {
		writeScreen("NO HAY ESPACIO DISPONIBLE PARA EL CHANNEL DEL PIPE\n", 51);
		sys_free(aux);
		return NULL;
	}

	if((aux->lock = createLock()) == NULL) {
		writeScreen("NO HAY ESPACIO DISPONIBLE PARA EL LOCK DEL PIPE\n", 48);
		sys_deleteChannel(aux->channelId);
		sys_free(aux);
		return NULL;
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

	return aux;
}

int sys_closePipe(int fd) {
	char rw;
	//Le quito al proceso de su PCB el pipe
	int pipeId = removePipe(fd, &rw);
	if(pipeId == -1) {
		return -1;
	}

	//Por como funciona nuestra logica hardcodeada con STDIN y STDOUT, estos
	//no deberian cerrarse
	if(pipeId == STDIN_ID || pipeId == STDOUT_ID) {
		return 0;
	}
	//Se elimina el pipe o decrementa sus escritores/lectores dependiendo del caso
	return updatePipeDelete(pipeId, rw);
}

int updatePipeCreate(int pipeId, char rw) {
	Pipe *pipe = findPipe(pipeId);
	if(pipe != NULL) {
		acquire(pipe->lock);
		if(rw == READ)
			(pipe->readers)++;
		else
			(pipe->writers)++;

		release(pipe->lock);
		return 0;
	}
	return -1;
}

int updatePipeDelete(int pipeId, char rw) {
	Pipe *search = first;
	char buff[16];
	Pipe *previous = NULL;
	while(search != NULL && search->pipeId != pipeId) {
		previous = search;
		search = search->next;
	}

	if(search != NULL) {
		//Primero modifico el contador indicado
		acquire(search->lock);
		if(rw == READ)
			(search->readers)--;
		else
			(search->writers)--;

		//Solo borro el pipe si no hay nadie afectado
		if(search->writers + search->readers == 0) {
			if(previous == NULL) {
				first = search->next;
			} else {
				previous->next = search->next;
			}
			release(search->lock);
			sys_free(search->buffer);
			deleteLock(search->lock);
			sys_deleteChannel(search->channelId);
			sys_free(search);
		} else
			release(search->lock);
	} else
		return -1;

	return 0;
}
