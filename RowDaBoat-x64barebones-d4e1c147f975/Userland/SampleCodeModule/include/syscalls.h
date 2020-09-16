#ifndef SYSCALLS_H
#define SYSCALLS_H

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

//	Funcion para agregar un nuevo modulo a la lista de modulos
//	Regresa 0 si pudo agregarlo (hay espacio para correr mas procesos)
//	1 si no (no hay mas espacio)
int initProcess( void (*program)() );

// Funcion que se encarga de correr el primer proceso en la cola, en caso de existir
void runFirstProcess();

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

#endif
