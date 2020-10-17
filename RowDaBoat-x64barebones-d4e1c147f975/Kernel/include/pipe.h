#ifndef PIPE_H_
#define PIPE_H_

#define MAX_PIPES 16
#define PIPE_SIZE 1024

typedef struct Pipe {
	char buffer[PIPE_SIZE];
	unsigned int nRead;  // Indice en el cual se debe leer
	unsigned int nWrite; // Indice en el cual se debe escribir
	unsigned int pipeId;
	unsigned int processCount; // cantidad de procesos con el pipe abierto
	int channelId;		   // canal de comunicacion para que los procesos de write esperen cuando el buffer esta lleno y los de read cuando esta vacio
	int *lock;		   // lock que permite exclusion en escritura y lectura
	struct Pipe *next;
} Pipe;


int sys_read(int fd, char* out_buffer, unsigned long int count);

int sys_write(int fd, const char *str, unsigned long count);

// Syscall de creacion de pipe
int sys_createPipe(int pipefd[2]);

// Syscall de cierre de pipe del proceso actual en el indice indicado (file descriptor)
int sys_closePipe(int fd);

// Funcion que utiliza el scheduler para indicarle al pipe que se le agrego un nuevo proceso
int incPipeProcesses(int pipeId);

#endif