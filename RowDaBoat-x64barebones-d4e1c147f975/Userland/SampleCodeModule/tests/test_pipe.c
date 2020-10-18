#include <std_io.h>
#include <syscalls.h>

#define SEM_ID 15
#define PIPE1_ID 50
#define PIPE2_ID 51

int hijoLee() {
	char buf[256];
	int n = read(0, buf, 8);
	write(1, buf, n);
	return 0;
}

int hijoEscribe() {
	write(1, "ta bien\n", 8);
	return 0;
}

void wrapperhijoLee() {
	int p[2];
	Semaphore *sem;
	if((sem = semOpen(SEM_ID, 0)) == NULL) {
		printf("NO SE PUDO ABRIR EL SEMAFORO\n");
		return;
	}

	if(pipeOpen(PIPE1_ID, p) == -1) {
		printf("NO SE PUDO ABRIR EL PIPE\n");
		semClose(sem);
		return;
	}

	pipeClose(p[1]);
	dup2(p[0], 0);
	semPost(sem);
	semClose(sem);
	hijoLee();

	pipeClose(p[0]);
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

	semPost(sem);
	semClose(sem);

	hijoLee();

	pipeClose(p1[0]);
	pipeClose(p2[1]);
}

void wrapperhijoEscribe() {
	int p[2];
	Semaphore *sem;

	if((sem = semOpen(SEM_ID, 0)) == NULL) {
		printf("NO SE PUDO ABRIR EL SEMAFORO\n");
		return;
	}

	if(pipeOpen(PIPE1_ID, p) == -1) {
		printf("NO SE PUDO ABRIR EL PIPE\n");
		semClose(sem);
		return;
	}

	pipeClose(p[0]);
	dup2(p[1], 1);
	semPost(sem);
	semClose(sem);
	hijoEscribe();

	pipeClose(p[1]);
}

void test_pipe() {
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

	// if(pipeOpen(PIPE2_ID, p2) == -1) {
	// 	printf("NO SE PUDO ABRIR EL PIPE\n");
	// 	pipeClose(p1[0]);
	// 	pipeClose(p1[1]);
	// 	semClose(sem);
	// 	return;
	// }

	startProcessBg(wrapperhijoEscribe, 0, NULL);
	//startProcessBg(wrapperhijoMensajero, 0, NULL);
	startProcessBg(wrapperhijoLee, 0, NULL);

	semWait(sem);
	//semWait(sem);
	semWait(sem);

	pipeClose(p1[0]);
	pipeClose(p1[1]);
	// pipeClose(p2[0]);
	// pipeClose(p2[1]);
}