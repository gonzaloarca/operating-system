#include <interrupts.h>
#include <memManager.h>
#include <scheduler.h>
#include <time.h>
#include <window_manager.h>

static ProcNode *currentProc, *lastProc, nodeAux, *idleProc = NULL;
static unsigned int lastPID = 1;
static int fgFlag = 0; //Flag que indica si debe haber cambio al proceso en foreground

//Funcion para setear los datos en el Stack Frame de un nuevo proceso
static uint64_t createStackFrame(uint64_t frame, uint64_t mainptr, int argc, uint64_t argv);

//Funcion para liberar un nodo de la lista de procesos
static void freeResources(ProcNode **node);

//Fuerzo el cambio al proceso en foreground (el primero de mi lista)
static void switchForeground();

//Funcion auxiliar para contar caracteres de un string
static int strlen(const char *str);

//Funcion auxiliar para copiar los argumentos que recibe el proceso
static void copyArgs(int argc, const char **from, char ***into);

//Proceso idle para esperar si están todos los procesos bloqueados
void idle();

//Inicializa la tabla de filedescriptors del proceso con stdin, stdout y stderr
static int createFdTable(ProcNode *proc);

int sys_startProcBg(uint64_t mainPtr, int argc, char const *argv[]) {
	if((void *)mainPtr == NULL) {
		sys_write(2, "Error en funcion a ejecutar\n", 28);
		return -1;
	}

	ProcNode *new = sys_malloc(sizeof(nodeAux)); //Creo el nuevo nodo
	if(new == NULL) {
		sys_write(2, "Error en malloc de nodo\n", 24);
		return -1;
	}
	// Le seteo los datos
	new->pcb.pid = lastPID++;
	new->pcb.mem = sys_malloc(STACK_SIZE);

	if(new->pcb.mem == NULL) {
		sys_free(new);
		sys_write(2, "Error en malloc de PCB\n", 23);
		return -1;
	}

	new->pcb.rsp = (uint64_t) new->pcb.mem + STACK_SIZE;
	new->pcb.state = ACTIVE;
	new->pcb.mainPtr = mainPtr;

	new->pcb.argc = argc;
	copyArgs(argc, argv, &new->pcb.argv);

	new->pcb.priority = DEFAULT_QUANTUM;
	new->pcb.quantumCounter = DEFAULT_QUANTUM;

	//ALINEAR
	new->pcb.rsp &= -8;

	//  Armo el stack frame del proceso nuevo
	new->pcb.rsp = createStackFrame(new->pcb.rsp, mainPtr, argc, (uint64_t)argv);

	//Creo la tabla de file descriptors
	if(createFdTable(new) == -1) {
		writeScreen("NO SE LOGRO ALOCAR STDIN Y STDOUT\n", 34);
		sys_free(new->pcb.mem);
		sys_free(new);
		return -1;
	}

	//  Si la lista está vacía
	if(lastProc == NULL) {
		lastProc = new;
		new->next = new;
		new->previous = new;

		//Ahora creo el proceso para idle y lo saco de la lista
		sys_startProcBg((uint64_t)idle, 0, NULL);
		idleProc = lastProc;
		idleProc->next = NULL;
		idleProc->previous = NULL;

		//Y vuelvo a setar como estaba
		lastProc = new;
		new->next = new;
		new->previous = new;
	} else {
		new->next = lastProc->next;
		new->previous = lastProc;
		lastProc->next = new;
		new->next->previous = new;
		lastProc = new;
	}

	return new->pcb.pid;
}

int sys_startProcFg(uint64_t mainPtr, int argc, char const *argv[]) {
	//Solamente el primer proceso de la lista puede iniciar a otros en foreground
	if(currentProc != NULL && currentProc != lastProc->next)
		return 0;

	//Primero inicio el proceso como si fuera en Background
	unsigned pid = sys_startProcBg(mainPtr, argc, argv);

	//Luego bloqueo el proceso que lo llamó (el primero de la lista) si no es el unico elemento en la lista
	if(lastProc != lastProc->next)
		lastProc->next->pcb.state = BLOCKED_BY_FG;
	//El nuevo proceso de foreground tiene la misma prioridad que el que reemplaza
	lastProc->pcb.priority = lastProc->next->pcb.priority;
	//Y mi nuevo proceso reemplaza al primero de la lista (muevo una posicion atras el puntero al ultimo)
	lastProc = lastProc->previous;

	//Cambio al nuevo proceso, si es que no es el primero
	if(currentProc != NULL)
		triggerForeground();

	return pid;
}

static uint64_t createStackFrame(uint64_t frame, uint64_t mainptr, int argc, uint64_t argv) {
	uint64_t *framePtr = (uint64_t *)frame - 1;

	//Datos para el iretq
	*framePtr = 0; //SS
	framePtr--;
	*framePtr = (uint64_t)framePtr; //RSP
	framePtr--;
	*framePtr = 0x202; //RFLAGS
	framePtr--;
	*framePtr = 0x8; //CS
	framePtr--;
	*framePtr = (uint64_t)_start; //RIP
	framePtr--;

	//Lleno los registros con valores crecientes de 0 a 14
	for(int i = 0; i < 15; i++, framePtr--)
		*framePtr = i;

	//framePtr me apunta una posicion despues de r15, lo incremento para que apunte a r15
	framePtr++;

	//Cargo rdi, rsi y rdx con sus respectivos argumentos para que _start los levante
	*(framePtr + 9) = mainptr; //rdi
	*(framePtr + 8) = argc;
	*(framePtr + 11) = argv;

	return (uint64_t)framePtr;
}

//  Guarda el rsp del proceso que lo llama por la interrupcion
//  Luego devuelve el rsp del proximo proceso a ejecutar
uint64_t getNextRSP(uint64_t rsp) {

	if(currentProc == NULL) {      //  Si todavia no está corriendo ningún proceso
		if(lastProc == NULL) { //  Todavia no hay procesos
			return 0;      //retorna 0 ya que se corresponderia con NULL en cuanto a direcciones, y esta funcion retorna una direccion
		}
		currentProc = lastProc->next;
	} else
		currentProc->pcb.rsp = rsp;

	//Si el proceso que acabo de correr se murio, tipicamente mediante un exit
	if(currentProc->pcb.state == KILLED) {
		freeResources(&currentProc);
	}

	if(fgFlag) { //  Si tengo que cambiar al proceso en foreground
		switchForeground();
		return currentProc->pcb.rsp;
	}
	//  En este if vemos si le toca cambiar al proceso
	if(currentProc->pcb.quantumCounter == MAX_QUANTUM || currentProc->pcb.state != ACTIVE) {
		currentProc->pcb.quantumCounter = currentProc->pcb.priority; //Si llego a su maximo de quantums, lo reseteo

		//Si es el proceso idle el que volvió, vuelvo a buscar desde el principio de la lista
		if(currentProc == idleProc)
			currentProc = lastProc;

		ProcNode *search = currentProc->next;
		//Encuentro algun proceso no bloqueado para correr
		while(search != currentProc) {
			if(search->pcb.state == ACTIVE)
				break;
			search = search->next;
		}

		//Me fijo si encontré uno activo o están todos bloquedos
		if(search->pcb.state == ACTIVE) {
			currentProc = search;
		} else {
			//vamos al proceso idle
			currentProc = idleProc;
		}
	}
	currentProc->pcb.quantumCounter++;

	return currentProc->pcb.rsp;
}

static void freeResources(ProcNode **node) {
	// Si el proceso a borrar es el ultimo, se debe modificar el lastProc para que sea el anterior a este
	if(*node == lastProc)
		lastProc = (*node)->previous;
	// Si voy a borrar el primer proceso (el que está en foreground) tengo que desbloquear al siguiente
	if(*node == lastProc->next || (*node)->next->pcb.state == BLOCKED_BY_FG)
		(*node)->next->pcb.state = ACTIVE;

	if((*node)->pcb.argv != NULL) {
		for(int i = 0; i < (*node)->pcb.argc; i++)
			sys_free((*node)->pcb.argv[i]);

		sys_free((*node)->pcb.argv);
	}

	//Borramos los fds que hayan quedado abiertos
	for(size_t i = 0; i < MAX_PIPES; i++) {
		if((*node)->pcb.pipeList[i] != NULL) {
			if((*node)->pcb.pipeList[i]->pipeId != STDIN_ID && (*node)->pcb.pipeList[i]->pipeId != STDOUT_ID) {
				updatePipeDelete((*node)->pcb.pipeList[i]->pipeId, (*node)->pcb.pipeList[i]->rw);
			}
			sys_free((*node)->pcb.pipeList[i]);
		}
	}

	sys_free((*node)->pcb.mem);
	*node = (*node)->next;
	(*node)->previous = (*node)->previous->previous;
	sys_free((*node)->previous->next);
	(*node)->previous->next = *node;
}

//  Se activa un flag para cambiar al proceso que corre en foreground
void triggerForeground() {
	fgFlag = 1;
	sys_runNext();
}

static void switchForeground() {
	currentProc->pcb.quantumCounter = currentProc->pcb.priority;
	currentProc = lastProc->next;
	currentProc->pcb.state = ACTIVE;
	currentProc->pcb.quantumCounter = currentProc->pcb.priority + 1;
	fgFlag = 0;
}

//  Syscall para eliminar el proceso actual de la lista de procesos
void sys_exit() {
	if(currentProc == NULL)
		return; //No hay nada corriendo

	currentProc->pcb.state = KILLED;
	sys_runNext();
}

//  Syscall que retorna PID del proceso actual
unsigned int sys_getpid() {
	return currentProc->pcb.pid;
}

// Syscall que imprime los procesos actuales
void sys_listProcess() {
	char fg;
	ProcNode *aux = lastProc->next; // Arranco desde el primero
	printProcessListHeader();
	do {
		if(aux == lastProc->next || aux->pcb.state == BLOCKED_BY_FG)
			fg = 1;
		else
			fg = 0;
		printProcess(aux->pcb.argv, aux->pcb.pid, aux->pcb.priority, aux->pcb.rsp, (uint64_t)(((char *)aux->pcb.mem) + STACK_SIZE - 8), fg, aux->pcb.state);
		aux = aux->next;
	} while(aux != lastProc->next);
}

// Syscall para cambiar el estado de un pid especifico
int sys_kill(unsigned int pid, char state) {
	if(lastProc == NULL || state == BLOCKED_BY_FG)
		return -1;

	ProcNode *search = lastProc;
	//  Realizo la busqueda del proceso con el pid y lo marco como KILLED
	do {
		search = search->next;
		if(search->pcb.pid == pid) {
			if(state == KILLED) {
				if(pid == 1) //Si es la shell, no la mato
					return -1;

				if(search == currentProc) //Si un proceso se quiere matar a si mismo, el scheduler deberia encargarse de el
					sys_exit();

				freeResources(&search);
				return 0;
			} else {
				search->pcb.state = state;

				if(state == BLOCKED && search == currentProc) //Si un proceso se bloquea a si mismo, se fuerza un cambio de contexto
					sys_runNext();

				return 0;
			}
		}
	} while(search != lastProc);

	return -1;
}

//Syscall para que el proceso corriendo en el momento renuncie al CPU y se corra el siguiente proceso
void sys_runNext() {
	currentProc->pcb.quantumCounter = MAX_QUANTUM;
	decrease_ticks();
	forceTick();
}

//Syscall para cambiar la prioridad de un proceso
int sys_nice(unsigned int pid, unsigned int priority) {
	if(pid > lastPID || priority >= MAX_QUANTUM)
		return -1;

	ProcNode *search = lastProc;
	//  Realizo la busqueda del proceso con el pid
	do {
		search = search->next;
		if(search->pcb.pid == pid) {
			if(search->pcb.state == KILLED)
				return -1;
			else {
				search->pcb.priority = priority;
				search->pcb.quantumCounter = priority;
				return 1;
			}
		}
	} while(search != lastProc);

	return -1;
}

static void copyArgs(int argc, const char **from, char ***into) {
	if(argc == 0 || from == NULL) {
		*into = NULL;
		return;
	}

	const char *aux = "aux";
	*into = (char **)sys_malloc((argc + 1) * sizeof(aux));
	for(int i = 0, j = 0, length; i < argc; i++) {

		length = strlen(from[i]);
		(*into)[i] = sys_malloc((length + 1) * sizeof(aux[0]));

		for(j = 0; j < length; j++)
			(*into)[i][j] = from[i][j];

		(*into)[i][j] = 0;
	}
	(*into)[argc] = NULL;
}

static int strlen(const char *str) {
	int ans = 0;
	for(; str[ans] != 0; ans++)
		;
	return ans;
}

void idle() {
	while(1)
		_hlt();
	return;
}

static int createFdTable(ProcNode *proc) {
	PipeEnd aux;

	for(int i = 0; i < MAX_PIPES; i++)
		proc->pcb.pipeList[i] = NULL;

	//Se ponen los valores para los "pipes" de teclado y pantalla
	//En realidad, no son pipes, si no que son hard-codeados para read()/write()

	//STDIN
	if((proc->pcb.pipeList[0] = sys_malloc(sizeof(aux))) == NULL)
		return -1;
	proc->pcb.pipeList[0]->pipeId = STDIN_ID;
	proc->pcb.pipeList[0]->rw = READ;

	//STDOUT
	if((proc->pcb.pipeList[1] = sys_malloc(sizeof(aux))) == NULL) {
		sys_free(proc->pcb.pipeList[0]);
		return -1;
	}
	proc->pcb.pipeList[1]->pipeId = STDOUT_ID;
	proc->pcb.pipeList[1]->rw = WRITE;

	//STDERR
	if((proc->pcb.pipeList[2] = sys_malloc(sizeof(aux))) == NULL) {
		sys_free(proc->pcb.pipeList[0]);
		sys_free(proc->pcb.pipeList[1]);
		return -1;
	}
	proc->pcb.pipeList[2]->pipeId = STDOUT_ID;
	proc->pcb.pipeList[2]->rw = WRITE;

	return 0;
}

// Funcion que utiliza Pipe para agregar al pcb que lo creo el nuevo pipe como file descriptor, retorna por argumento el vector [ReadFD, WriteFD]
int setPipe(unsigned int newPipeId, int pipefd[2]) {
	ProcNode *proc;
	PipeEnd aux;
	if(currentProc == NULL)
		proc = lastProc;
	else
		proc = currentProc;

	int i;
	for(i = 0; proc->pcb.pipeList[i] != NULL && i < MAX_PIPES; i++)
		;

	if(i == MAX_PIPES)
		return -1;

	if((proc->pcb.pipeList[i] = sys_malloc(sizeof(aux))) == NULL)
		return -1;

	proc->pcb.pipeList[i]->pipeId = newPipeId;
	proc->pcb.pipeList[i]->rw = READ;
	pipefd[0] = i;

	for(; proc->pcb.pipeList[i] != NULL && i < MAX_PIPES; i++)
		;

	if(i == MAX_PIPES)
		return -1;

	if((proc->pcb.pipeList[i] = sys_malloc(sizeof(aux))) == NULL)
		return -1;

	proc->pcb.pipeList[i]->pipeId = newPipeId;
	proc->pcb.pipeList[i]->rw = WRITE;
	pipefd[1] = i;

	return 0;
}

// Funcion que utiliza Pipe para sacarle al proceso actual el pipe que se encuentra en el indice indicado
int removePipe(int fd, char *rw) {
	if(fd < 0 || fd > MAX_PIPES)
		return -1;

	if(currentProc->pcb.pipeList[fd] == NULL) {
		return -1;
	}

	int aux = currentProc->pcb.pipeList[fd]->pipeId;
	*rw = currentProc->pcb.pipeList[fd]->rw;

	sys_free(currentProc->pcb.pipeList[fd]);
	currentProc->pcb.pipeList[fd] = NULL;

	return aux;
}

// Funcion que le proporciona a pipe.c el pipe en el indice indicado
PipeEnd *getPipeEnd(int fd) {
	if(fd < 0 || fd > MAX_PIPES)
		return NULL;

	return currentProc->pcb.pipeList[fd];
}

// Funcion que copia oldfd en newfd
int sys_dup2(int oldfd, int newfd) {
	if(oldfd < 0 || newfd < 0 || oldfd >= MAX_PIPES || newfd >= MAX_PIPES || currentProc->pcb.pipeList[oldfd] == NULL)
		return -1;

	if(newfd == oldfd)
		return newfd;

	PipeEnd aux;
	//Cierro el fd que voy a sobreescribir
	sys_pipeClose(newfd);
	//Aloco lugar para reubicar el fd
	if((currentProc->pcb.pipeList[newfd] = sys_malloc(sizeof(aux))) == NULL)
		return -1;

	currentProc->pcb.pipeList[newfd]->pipeId = currentProc->pcb.pipeList[oldfd]->pipeId;
	currentProc->pcb.pipeList[newfd]->rw = currentProc->pcb.pipeList[oldfd]->rw;

	updatePipeCreate(currentProc->pcb.pipeList[newfd]->pipeId, currentProc->pcb.pipeList[newfd]->rw);

	return newfd;
}

int isForeground() {
	return currentProc == lastProc->next;
}
