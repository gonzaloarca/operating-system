#include <lock.h>
#include <memManager.h>
#include <scheduler.h>
#include <sig.h>
#include <window_manager.h>

typedef struct ChannelNode {
	unsigned int pid;
	struct ChannelNode *next;
} ChannelNode;

typedef struct ChannelNodeHeader {
	unsigned int channelId;
	unsigned int sigCounter;
	int *lock;
	ChannelNode *first;
	struct ChannelNodeHeader *next;
} ChannelNodeHeader;

static ChannelNodeHeader *first = {0};
static unsigned int lastId = 0;

int sys_createChannel() {
	ChannelNodeHeader *aux;
	ChannelNodeHeader *search;
	ChannelNodeHeader auxSizeOf;

	if((aux = sys_malloc(sizeof(auxSizeOf))) == NULL) {
		sys_write(2, "NO HAY ESPACIO DISPONIBLE PARA EL CANAL\n", 40);
		return -1;
	}

	aux->channelId = lastId++;
	aux->sigCounter = 0;
	aux->first = NULL;
	aux->next = NULL;
	if((aux->lock = createLock()) == NULL) {
		sys_free(aux);
		return -1;
	};

	//Si es el primer canal
	if(first == NULL) {
		first = aux;
	} else {
		search = first;
		while(search->next != NULL) {
			search = search->next;
		}
		search->next = aux;
	}

	return aux->channelId;
}

int sys_deleteChannel(unsigned int channelId) {
	if(channelId >= lastId) {
		sys_write(2, "NO EXISTE EL CANAL\n", 19);
		return -1;
	}

	ChannelNodeHeader *search = first;
	ChannelNodeHeader *previous = NULL;
	while(search->channelId < channelId) {
		previous = search;
		search = search->next;
	}

	//Solo borro el canal si no hay nadie durmiendo
	if(search->channelId == channelId && search->first == NULL) {
		if(previous == NULL) {
			first = search->next;
		} else {
			previous->next = search->next;
		}
		deleteLock(search->lock);
		sys_free(search);
	} else {
		sys_write(2, "HAY PROCESOS BLOQUEADOS POR EL CANAL\n", 37);
		return -1;
	}

	return 0;
}

static ChannelNodeHeader *findChannel(unsigned int channelId) {
	if(channelId >= lastId) {
		sys_write(2, "NO EXISTE EL CANAL\n", 19);
		return NULL;
	}

	//Busco el canal solicitado
	ChannelNodeHeader *searchHeader = first;
	while(searchHeader->channelId < channelId) {
		searchHeader = searchHeader->next;
	}

	if(searchHeader->channelId != channelId) {
		sys_write(2, "NO EXISTE EL CANAL\n", 19);
		return NULL;
	}

	return searchHeader;
}

int sys_sleep(unsigned int channelId) {

	ChannelNodeHeader *channel;
	if((channel = findChannel(channelId)) == NULL) {
		sys_write(2, "CANAL NO ENCONTRADO\n", 20);
		return -1;
	}

	acquire(channel->lock);
	if(channel->sigCounter > 0) {
		channel->sigCounter--;
		release(channel->lock);
		return 1;
	}
	release(channel->lock);

	ChannelNode *new;
	ChannelNode aux;
	if((new = sys_malloc(sizeof(aux))) == NULL) {
		sys_write(2, "NO HAY MEMORIA DISPONIBLE PARA EL NODO\n", 39);
		return -1;
	}

	new->pid = sys_getpid();
	new->next = NULL;

	//Busco el ultimo en el canal
	ChannelNode *searchProc = channel->first;

	if(searchProc == NULL) {
		channel->first = new;
	} else {
		while(searchProc->next != NULL) {
			searchProc = searchProc->next;
		}
		searchProc->next = new;
	}

	sys_kill(new->pid, BLOCKED);

	return 0;
}

int sys_wakeup(unsigned int channelId) {
	ChannelNodeHeader *channel;
	if((channel = findChannel(channelId)) == NULL) {
		sys_write(2, "NO EXISTE EL CANAL\n", 19);
		return -1;
	}

	acquire(channel->lock);
	//Si se hace un wakeup y no habia nadie esperando, se incrementa
	//el contador de señales, para prevenir un signal lost
	if(channel->first == NULL) {
		channel->sigCounter++;
		release(channel->lock);
		return 1;
	}
	release(channel->lock);

	ChannelNode *current = channel->first;
	ChannelNode *aux;
	channel->first = NULL;

	//Recorro la lista activando todos los procesos durmientes y eliminandolos de la lista
	while(current != NULL) {
		sys_kill(current->pid, ACTIVE);
		aux = current;
		current = current->next;
		sys_free(aux);
	}

	return 0;
}