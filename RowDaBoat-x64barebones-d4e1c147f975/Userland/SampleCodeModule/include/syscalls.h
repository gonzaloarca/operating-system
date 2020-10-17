#ifndef SYSCALLS_H
#define SYSCALLS_H

#define ACTIVE  1
#define BLOCKED 0
#define KILLED 2

#include <stddef.h>
#include <stdint.h>
#include <sem.h>

// Estructura utilizada para manejar una copia de los registros realizada en algun momento
typedef struct RegistersType {
	uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rbp;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t rsp;
    uint64_t rip;
} RegistersType;

// Funcion que permite escribir count caracteres de un string en el file descriptor fd
int write(unsigned int fd, char *buffer, unsigned long count);

//	Syscall para leer de entrada estandar en un buffer hasta que se llegue a "count" caracteres o se llegue al caracter "delim"
int read( char *buffer, unsigned long int count, char delim );

//	Limpia la pantalla actual
void clrScreen();

//	Cambiar el color de letra de la ventana
int changeWindowColor(int rgb);

//	Temperatura del CPU
int getCPUTemp();

// Estructura en la que se retorna la informacion del tiempo
typedef struct{
    uint32_t secs;
    uint32_t mins;
    uint32_t hours;
}TimeFormat;

//	Syscall para obtener la hora actual
void getTime(TimeFormat *time); //llama a syscall que llena la estructura de TimeFormat

//  Funcion que retorna una estructura con los valores de los registros de la ultima vez que se presiono la tecla especial F1
RegistersType* getRegisters();

// Estructura utilizada para imprimir los valores de 32 bytes de memoria
typedef struct memType {
	uint64_t mem1;
	uint64_t mem2;
	uint64_t mem3;
	uint64_t mem4;
} memType;

// Syscall para volcar en la estructura recibida los 32 bytes de informacion encontrados a partir de la direccion recibida
void getMemory(memType* answer, char * address);

// Syscall que alloca memoria suficiente para almacenar el size indicado por parametro, en caso de no haber memoria suficiente disponible
//retorna NULL
void *malloc(size_t size);

// Syscall que libera la memoria alocada que arranca en la direccion indicada por parametro, si no existe realiza undefined behaviour
void free(void *ptr);

// Estructura utilizada para imprimir el estado de la memoria disponible para alocar
typedef struct{
    size_t totalMem;
    size_t occMem;
    size_t freeMem;
} MemStatus;

// Syscall que escribe en la estructura indicada los valores que permiten saber el estado de la memoria disponible para alocar en dicho instante
void getMemStatus(MemStatus *stat);

//  Syscall para correr un proceso en foreground (bloquea al proceso en foreground actual)
//  Devuelve el pid del proceso nuevo
unsigned int startProcessFg(int (*mainptr)(int, char const **), int argc, char const *argv[]);

// Syscall que permite iniciar un proceso en background que corre la funcion mainptr con argc argumentos que recibe en argv
// Devuelve su pid
unsigned int startProcessBg(int (*mainptr)(int, char const **), int argc, char const *argv[]);

// Syscall que finaliza la ejecucion de un proceso, marcando su estado como "KILLED" y luego
// llamando a que se ejecute el siguiente proceso en la cola de listos.
void exit();

// Syscall que retorna el pid del proceso que la llama
unsigned int getpid();

// Syscall que imprime los procesos corriendo en el momento
void listProcess();

// Syscall para cambiar el estado de un proceso según su pid
int kill(unsigned int pid, char state);

// Syscall para que el proceso corriendo en el momento renuncie al CPU y se corra el siguiente proceso
void runNext();

// Syscall para cambiar la prioridad de un proceso según su pid, retorna 0 en caso de error
int nice(unsigned int pid, unsigned int priority);

// Syscall para crear un canal de comunicacion para señales de sleep y wakeup, 
// devuelve el ID del canal, y en caso de error devuelve -1
int createChannel();

// Syscall destruye un canal de comunicacion para señales de sleep y wakeup dado su ID
// Si el ID no se corresponde con ningun canal existente, devuelve -1. Sino devuelve 0 
int deleteChannel(unsigned int id);

// Syscall que manda a dormir al proceso actual en el caso que corresponda.
// Si ya habian señales pendientes, retorna 1. En caso contrario, devuelve 0. En caso de error, devuelve -1.
int sleep(unsigned int id);

// Syscall que despierta a los procesos esperando en el canal pasado como argumento por su ID
// Si no habia nadie durmiendo, incrementa el contador de señales. En este caso, retorna 1.
// Si habia alguien durmiendo, lo despierta y retorna 0. En caso de error retorna -1.
// Recibe el id del canal correspondiente
int wakeup(unsigned int id);

// Syscall crea un pipe de comunicacion, 
// devuelve un puntero a un vector que en la primer posicion contiene el indice del
// pipe de lectura y en el segundo el de escritura, y en caso de error devuelve -1
int createPipe(int pipefd[2]);

// Syscall 42 cierra para el proceso actual el acceso al pipe que se encuentra
// en el indice indicado por paramtro dentro de su vector de pipes
int closePipe(unsigned int index);

#endif
