#ifndef PIPE_H_
#define PIPE_H_

#include <lock.h>
#include <sig.h>
#include <memManager.h>
#include <scheduler.h>
#include <lock.h>
#include <stddef.h>
#include <stdint.h>

#define MAX_PIPES 16
#define PIPE_SIZE 1024

typedef struct Pipe{
    char *buffer;
    unsigned int nRead;         // Indice en el cual se debe leer
    unsigned int nWrite;        // Indice en el cual se debe escribir
    unsigned int pipeId;
    unsigned int processCount;           // cantidad de procesos con el pipe abierto
    int channel;                // canal de comunicacion para que los procesos de write esperen cuando el buffer esta lleno y los de read cuando esta vacio
    int lock;                   // lock que permite exclusion en escritura y lectura
    struct Pipe *next;
} Pipe;


#endif