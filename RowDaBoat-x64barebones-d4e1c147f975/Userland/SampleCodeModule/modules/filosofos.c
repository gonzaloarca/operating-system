/*	
	Programa que presenta una solucion para el problema de los filosofos comensales
	Crea N filosofos que se turnan para comer mientras que el de su lado no esté comiendo
*/

#include <sem.h>
#include <std_io.h>
#include <std_num.h>
#include <syscalls.h>

#define MIN 2
#define MAX 20
#define IZQ (i - 1 + N) % N //Plato de la izquierda
#define DER (i + 1) % N	    //Plato de la derecha

#define PENSANDO 0
#define BUSCANDO 1
#define COMIENDO 2

#define PHYLO_MUTEX_ID 99
#define PHYLO_SEM_BASE 100
#define PHYLO_PIPE_ID 547

static int filosofo(int argc, char const *argv[]);
static void agarrar_cubiertos(int i);
static void dejar_cubiertos(int i);
static void probar(int i);
static void comer();

static int N = 5;	      //Cantidad de filosofos/cubiertos
static Semaphore *mutex;      //Semaforo para variables compartidas
static Semaphore **cubiertos; //Semaforos para frenar si no tienen cubiertos
static char *status;	      //Estados de los filosofos

void phylo() {
	char c, *arguments[2], buffer[100];
	int len, flag = 1, fd[2];

	N = 5;

	// Inicializo los semaforos
	mutex = semOpen(PHYLO_MUTEX_ID, 1);
	cubiertos = malloc(MAX * sizeof(mutex));
	status = malloc(MAX * sizeof(c));
	for(int i = 0; i < MAX; i++) {
		cubiertos[i] = semOpen(PHYLO_SEM_BASE + i, 0);
		status[i] = PENSANDO;
	}

	//	Abro un pipe para recibir los resultados de los filosofos
	if(pipeOpen(PHYLO_PIPE_ID, fd) == -1) {
		printf("NO SE PUDO ABRIR EL PIPE\n");
		return;
	}

	arguments[0] = "filosofo";
	arguments[1] = malloc(10 * sizeof(c));

	printf("Use 'a' para agregar filosofos, 'r' para borrarlos y 'x' para salir\n");
	printf("Ejemplo: 'aar' agrega dos filosofos y resta uno\n");

	while(flag) {
		putchar('\n');
		//	Creo a los filosofos
		for(int i = 0; i < N; i++) {
			intToString(i, arguments[1]);
			startProcessBg((programStart)filosofo, 2, (const char **)arguments);
		}
		//	Espero a recibir los resultados
		for(int i = 0; i < N; i++) {
			len = read(fd[0], buffer, 100);
			write(1, buffer, len);
		}

		len = read(0, buffer, 100);
		if(len != 0) {
			for(int i = 0; i < len; i++) {
				if(buffer[i] == 'a' && N < MAX)
					N++;
				else if(buffer[i] == 'r' && N > MIN)
					N--;
				else if(buffer[i] == 'x') {
					flag = 0;
					break;
				}
			}
		}
	}

	semClose(mutex);
	for(int i = 0; i < MAX; i++) {
		semClose(cubiertos[i]);
	}
	free(cubiertos);
	free(status);
	free(arguments[1]);
	pipeClose(fd[0]);
	pipeClose(fd[1]);

	return;
}

//	Accion de un filosofo
static int filosofo(int argc, char const *argv[]) {
	int i = strToPositiveInt((char *)argv[1], NULL);
	int fd[2];

	if(pipeOpen(PHYLO_PIPE_ID, fd) == -1) {
		printf("NO SE PUDO ABRIR EL PIPE\n");
		return 1;
	}

	dup2(fd[1], 1);

	agarrar_cubiertos(i);
	comer();
	dejar_cubiertos(i);

	pipeClose(fd[0]);
	pipeClose(fd[1]);
	pipeClose(1);

	return 0;
}

static void agarrar_cubiertos(int i) {
	semWait(mutex);

	status[i] = BUSCANDO;
	probar(i);

	semPost(mutex);

	//Si no tengo cubiertos, tengo que esperar a que me lleguen
	semWait(cubiertos[i]);
}

static void dejar_cubiertos(int i) {
	semWait(mutex);

	status[i] = PENSANDO; //Ya terminé

	//Veo si mis adyacentes pueden comer
	probar(IZQ);
	probar(DER);

	semPost(mutex);
}

static void probar(int i) {
	//	Si puedo comer, lo hago
	if(status[i] == BUSCANDO && status[IZQ] != COMIENDO && status[DER] != COMIENDO) {
		status[i] = COMIENDO;
		semPost(cubiertos[i]); //Indico que conseguí cubiertos
	}
}

//	La funcion comer va a imprimir el estado actual de los filosofos
static void comer() {
	semWait(mutex);

	for(int i = 0; i < N; i++) {
		if(status[i] == COMIENDO)
			putchar('E');
		else
			putchar('.');
		putchar(' ');
	}

	putchar('\n');

	semPost(mutex);

	// Hacemos un runNext para que puedan comer dos filosofos a la vez
	runNext();
}
