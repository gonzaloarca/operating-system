#ifndef SEM_H
#define SEM_H

// Tipo de dato utilizado para semaforos
typedef struct Semaphore {
	unsigned int semId;	// id del semaforo
	unsigned int value;	// valor del semaforo
	unsigned int channelId; // id del canal para señales del semaforo
	unsigned int count;	// ID del respectivo canal de comunicacion utilizado para las señales del semaforo
} Semaphore;

// Funcion que consulta el estado de un semaforo y bloquea si llega a valer 0.
// En caso de que el valor del semaforo sea mayor que 0, lo decrementa por 1
void semWait(Semaphore *sem);

// Funcion que incrementa atomicamente el valor de un semaforo en una unidad
void semPost(Semaphore *sem);

// Funcion abre un semaforo y lo devuelve. Si ya existe un semaforo con el id proporcionado, se ignora el valor init y preserva su valor anterior
Semaphore *semOpen(unsigned int id, unsigned int init);

// Funcion que cierra un semaforo y desaloca los recursos utilizados por el mismo
int semClose(Semaphore *sem);

#endif