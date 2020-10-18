#ifndef PIPE_H_
#define PIPE_H_

#define MAX_PIPES 16
#define PIPE_SIZE 1024

#define READ 0
#define WRITE 1

#define STDIN_ID 0
#define STDOUT_ID 1

typedef struct Pipe {
	char buffer[PIPE_SIZE];
	unsigned int nRead;  // Indice en el cual se debe leer
	unsigned int nWrite; // Indice en el cual se debe escribir
	int isFull;	     //	Flag necesario para saber si nRead-nWrite=0 representa lleno o vacio
	unsigned int pipeId;
	unsigned int writers; // cantidad de escritores del pipe que se estan usando
	unsigned int readers; //cantidad de lectores
	int channelId;	      // canal de comunicacion para que los procesos de write esperen cuando el buffer esta lleno y los de read cuando esta vacio
	int *lock;	      // lock que permite exclusion en escritura y lectura
	struct Pipe *next;
} Pipe;

typedef struct PipeEnd {
	unsigned int pipeId;
	char rw;
} PipeEnd;

int sys_read(int fd, char *out_buffer, unsigned long int count);

int sys_write(int fd, const char *str, unsigned long count);

// Syscall para abrir pipes
int sys_pipeOpen(unsigned int pipeId, int pipefd[2]);

// Syscall de cierre de pipe del proceso actual en el indice indicado (file descriptor)
int sys_pipeClose(int fd);

//	Actualiza los datos de un pipe luego de abrirlo/crearlo
int updatePipeCreate(int pipeId, char rw);

//	Actualiza los datos de un pipe luego de cerrar un fd
int updatePipeDelete(int pipeId, char rw);

// Syscall que imprime informacion sobre los pipes en el momento
void sys_listPipes();

#endif