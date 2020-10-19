#include <std_io.h>
#include <syscalls.h>
#include <test_util.h>

//Test de conectar tres procesos por pipes:
// Escribe -> Mensajero -> Lee

#define SEM_ID 15
#define PIPE1_ID 50	 //Este pipe conecta escribe con mensajero
#define PIPE2_ID 51	 //Este pipe conecta mensajero con lee
#define PIPE_FINAL_ID 52 //Pipe para que el padre imprima el mensaje final

#define MENSAJE "Funcionan bien los pipes\n"
#define LENGTH 25

typedef int (*programStart)(int, const char **);

int hijoLee() {
	char buf[256];
	int n = read(0, buf, LENGTH);
	write(1, buf, n);
	return 0;
}

int hijoEscribe() {
	write(1, MENSAJE, LENGTH);
	return 0;
}

void wrapperhijoLee() {
	int p2[2], pf[2];
	Semaphore *sem;
	if((sem = semOpen(SEM_ID, 0)) == NULL) {
		printf("NO SE PUDO ABRIR EL SEMAFORO\n");
		return;
	}

	if(pipeOpen(PIPE2_ID, p2) == -1) {
		printf("NO SE PUDO ABRIR EL PIPE\n");
		semClose(sem);
		return;
	}

	if(pipeOpen(PIPE_FINAL_ID, pf) == -1) {
		printf("NO SE PUDO ABRIR EL PIPE\n");
		pipeClose(p2[0]);
		pipeClose(p2[1]);
		semClose(sem);
		return;
	}

	pipeClose(p2[1]);
	dup2(p2[0], 0);
	pipeClose(p2[0]);

	pipeClose(pf[0]);
	dup2(pf[1], 1);
	pipeClose(pf[1]);

	semPost(sem);
	semClose(sem);

	hijoLee();

	pipeClose(0);
}

void wrapperhijoMensajero() {
	int p1[2], p2[2];
	Semaphore *sem;
	if((sem = semOpen(SEM_ID, 0)) == NULL) {
		printf("NO SE PUDO ABRIR EL SEMAFORO\n");
		return;
	}

	if(pipeOpen(PIPE1_ID, p1) == -1) {
		printf("NO SE PUDO ABRIR EL PIPE\n");
		semClose(sem);
		return;
	}

	if(pipeOpen(PIPE2_ID, p2) == -1) {
		printf("NO SE PUDO ABRIR EL PIPE\n");
		pipeClose(p1[0]);
		pipeClose(p1[1]);
		semClose(sem);
		return;
	}

	pipeClose(p1[1]);
	pipeClose(p2[0]);
	dup2(p1[0], 0);
	dup2(p2[1], 1);
	pipeClose(p1[0]);
	pipeClose(p2[1]);

	semPost(sem);
	semClose(sem);

	hijoLee();

	pipeClose(1);
	pipeClose(0);
}

void wrapperhijoEscribe() {
	int p1[2];
	Semaphore *sem;

	if((sem = semOpen(SEM_ID, 0)) == NULL) {
		printf("NO SE PUDO ABRIR EL SEMAFORO\n");
		return;
	}

	if(pipeOpen(PIPE1_ID, p1) == -1) {
		printf("NO SE PUDO ABRIR EL PIPE\n");
		semClose(sem);
		return;
	}

	pipeClose(p1[0]);
	dup2(p1[1], 1);
	pipeClose(p1[1]);

	semPost(sem);
	semClose(sem);

	hijoEscribe();

	pipeClose(1);
}

void test_pipe() {
	int p1[2], p2[2], pf[2], len;
	char buffer[100];
	Semaphore *sem;

	if((sem = semOpen(SEM_ID, 0)) == NULL) {
		printf("NO SE PUDO ABRIR EL SEMAFORO\n");
		return;
	}

	if(pipeOpen(PIPE1_ID, p1) == -1) {
		printf("NO SE PUDO ABRIR EL PIPE\n");
		semClose(sem);
		return;
	}

	if(pipeOpen(PIPE2_ID, p2) == -1) {
		printf("NO SE PUDO ABRIR EL PIPE\n");
		pipeClose(p1[0]);
		pipeClose(p1[1]);
		semClose(sem);
		return;
	}

	if(pipeOpen(PIPE_FINAL_ID, pf) == -1) {
		printf("NO SE PUDO ABRIR EL PIPE\n");
		pipeClose(p1[0]);
		pipeClose(p1[1]);
		pipeClose(p2[0]);
		pipeClose(p2[1]);
		semClose(sem);
		return;
	}

	const char *name = "escribe";
	startProcessBg((programStart)wrapperhijoEscribe, 1, &name);
	name = "mensajero";
	startProcessBg((programStart)wrapperhijoMensajero, 1, &name);
	name = "lee";
	startProcessBg((programStart)wrapperhijoLee, 1, &name);

	semWait(sem);
	semWait(sem);
	semWait(sem);

	semClose(sem);

	len = read(pf[0], buffer, 100);
	write(1, buffer, len);

	pipeClose(p1[0]);
	pipeClose(p1[1]);
	pipeClose(p2[0]);
	pipeClose(p2[1]);
	pipeClose(pf[0]);
	pipeClose(pf[1]);
}

/* Test de solo 2 procesos pipeados

#define mbeh(type) ((unsigned char *)(&type + 1) - (unsigned char *)(&type))

#define PIPE_ID 50

int hijoLee() {
	char buf[256];
	int n = read(0, buf, LENGTH);
	write(1, buf, n);
	return 0;
}

int hijoEscribe() {
	write(1, MENSAJE, LENGTH);
	return 0;
}

void wrapperhijoLee() {
	int fd[2];
	Semaphore *mbeh;
	if((mbeh = semOpen(PIPE_ID, 0)) == NULL) {
		printf("NO SE PUDO ABRIR EL SEMAFORO\n");
		return;
	}

	if(pipeOpen(PIPE_ID, fd) == -1) {
		printf("NO SE PUDO ABRIR EL PIPE\n");
		return;
	}

	pipeClose(fd[1]);
	dup2(fd[0], 0);
	semPost(mbeh);
	semClose(mbeh);
	hijoLee();

	pipeClose(fd[0]);
	pipeClose(0);

	fprintf(2, "termino hijo lee\n");
}

void wrapperhijoEscribe() {
	int fd[2];
	Semaphore *mbeh;

	if((mbeh = semOpen(PIPE_ID, 0)) == NULL) {
		printf("NO SE PUDO ABRIR EL SEMAFORO\n");
		return;
	}

	if(pipeOpen(PIPE_ID, fd) == -1) {
		printf("NO SE PUDO ABRIR EL PIPE\n");
		return;
	}

	pipeClose(fd[0]);
	dup2(fd[1], 1);
	semPost(mbeh);
	semClose(mbeh);
	hijoEscribe();

	pipeClose(fd[1]);
	pipeClose(1);

	fprintf(2, "termino hijo escribe\n");
}

int test_pipe() {
	int aux[2];
	Semaphore *mbeh;

	if((mbeh = semOpen(PIPE_ID, 0)) == NULL) {
		printf("NO SE PUDO ABRIR EL SEMAFORO\n");
		return;
	}

	if(pipeOpen(PIPE_ID, aux) == -1) {
		printf("NO SE PUDO ABRIR EL PIPE\n");
		return;
	}
	startProcessBg(wrapperhijoEscribe, 0, NULL);
	startProcessBg(wrapperhijoLee, 0, NULL);
	semWait(mbeh);
	semWait(mbeh);
	semClose(mbeh);
	pipeClose(aux[0]);
	pipeClose(aux[1]);
	fprintf(2, "termino el viejo\n");
	return 0;
}
*/
