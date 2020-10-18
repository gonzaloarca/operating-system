#include <comandos.h>
#include <lock.h>
#include <sem.h>
#include <std_io.h>
#include <syscalls.h> //para usar sleep, wakeup, malloc y free

typedef struct SemNode {
	Semaphore sem; //valor actual del semaforo
	struct SemNode *next;
} SemNode;

static SemNode *semList = {0}; //Lista de semaforos
static unsigned int *lock = {0};
static void createNode(SemNode *node, unsigned int id, unsigned int init, unsigned int channelId);

void semWait(Semaphore *sem) {
	unsigned int semAux;

	//Si el semaforo tenia 0, se bloquea
	while((semAux = _xchg(0, &(sem->value))) == 0) {
		sleep(sem->channelId);
	}
	//Una vez que el semaforo es distinto de 0, se disminuye su valor de forma atomica
	_xchg(semAux - 1, &(sem->value));
}

void semPost(Semaphore *sem) {
	_inc(&(sem->value));
	wakeup(sem->channelId);
}

Semaphore *semOpen(unsigned int id, unsigned int init) {
	SemNode aux;
	unsigned int channelId;

	//Si todavia no esta inicializado el spinlock, lo inicializo
	if(lock == NULL) {
		if((lock = createLock()) == NULL)
			return NULL;
	}

	//Al estar en Userland, nos puede interrumpir mientras corremos semOpen, y semOpen
	//modifica estructuras globales, por lo tanto es una zona critica
	acquire(lock);
	//Inicializo la lista
	if(semList == NULL) {
		if((semList = malloc(sizeof(aux))) == NULL) {
			release(lock);
			return NULL;
		}

		if((channelId = createChannel()) == -1) {
			free(semList); //Se pudo alocar la lista pero no se pudo crear el canal, por ende deshago lo creado
			semList = NULL;
			release(lock);
			return NULL;
		}

		createNode(semList, id, init, channelId);
		release(lock);
		return &(semList->sem);
	}

	SemNode *search;
	//Busco el id en la lista. Si lo encuentro, me quedo con el valor que ya tenía
	for(search = semList;; search = search->next) {
		if(search->sem.semId == id) {
			(search->sem.count)++;
			release(lock);
			return &(search->sem);
		}
		if(search->next == NULL)
			break;
	}

	//Si no lo encontré, creo un nuevo semaforo y un canal de comunicacion
	if((search->next = malloc(sizeof(aux))) == NULL) {
		release(lock);
		return NULL;
	}

	if((channelId = createChannel()) == -1) {
		free(search->next); //Se pudo alocar espacio para el nodo pero no se pudo crear el canal, por ende deshago lo creado
		search->next = NULL;
		release(lock);
		return NULL;
	}

	search = search->next;
	createNode(search, id, init, channelId);
	release(lock);

	return &(search->sem);
}

static void createNode(SemNode *node, unsigned int id, unsigned int init, unsigned int channelId) {
	node->sem.semId = id;
	node->sem.value = init;
	node->sem.count = 1;
	node->sem.channelId = channelId;
	node->next = NULL;
}

int semClose(Semaphore *sem) {
	SemNode *search, *previous = NULL;

	//En el caso de que se quiera llamar a semClose sin que este inicializado el lock
	//que se crea en semOpen, retorna -1 a modo de error
	if(lock == NULL) {
		return -1;
	}
	//Al estar en Userland, nos puede interrumpir mientras corremos semClose, y semClose
	//modifica estructuras globales, por lo tanto es una zona critica
	acquire(lock);
	for(search = semList; search != NULL; previous = search, search = search->next) {
		if(&(search->sem) == sem) {
			(search->sem.count)--;
			//Si ya lo cerraron todos los procesos, saco el semaforo de la lista y elimino el canal de señales -- OJO CONDICIONES DE CORRERA CON SEMOPEN
			if(search->sem.count == 0) {
				deleteChannel(search->sem.channelId);
				if(previous == NULL) {
					//Este caso es cuando el id es el del primer semaforo de la lista
					semList = search->next;
					//Si la lista quedo vacia, libero los recursos del lock
					if(semList == NULL) {
						release(lock);
					} else {
						release(lock);
					}
					free(search);
					return 0;

				} else {
					previous->next = search->next;
				}
				free(search);
			}
			release(lock);
			return 0;
		}
	}
	release(lock);
	return -1;
}
