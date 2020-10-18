#include <evaluator.h>
#include <shell.h>
#include <std_io.h>
#include <syscalls.h>

#define mbeh(type) ((unsigned char *)(&type + 1) - (unsigned char *)(&type))

#define PIPE_ID 50

int hijoLee() {
	char buf[256];
	int n = read(0, buf, 47);
	write(1, buf, n);
	return 0;
}

int hijoEscribe() {
	write(1, "JULIANSICARDIELBETARDITENMBEHXDMBERTELBETOSAPE\n", 47);
	return 0;
}

void wrapperhijoLee() {
	int fd[2];
	Semaphore *mbeh;
	if((mbeh = semOpen(PIPE_ID, 0)) == NULL) {
		printf("NO SE PUDO ABRIR EL SEMAFORO\n");
		return;
	}

	if(openPipe(PIPE_ID, fd) == -1) {
		printf("NO SE PUDO ABRIR EL PIPE\n");
		return;
	}

	closePipe(fd[1]);
	dup2(fd[0], 0);
	semPost(mbeh);
	semClose(mbeh);
	hijoLee();

	closePipe(fd[0]);
}

void wrapperhijoEscribe() {
	int fd[2];
	Semaphore *mbeh;

	if((mbeh = semOpen(PIPE_ID, 0)) == NULL) {
		printf("NO SE PUDO ABRIR EL SEMAFORO\n");
		return;
	}

	if(openPipe(PIPE_ID, fd) == -1) {
		printf("NO SE PUDO ABRIR EL PIPE\n");
		return;
	}

	closePipe(fd[0]);
	dup2(fd[1], 1);
	semPost(mbeh);
	semClose(mbeh);
	hijoEscribe();

	closePipe(fd[1]);
}

int test() {
	int aux[2];
	openPipe(PIPE_ID, aux);
	Semaphore *mbeh;

	if((mbeh = semOpen(PIPE_ID, 0)) == NULL) {
		printf("NO SE PUDO ABRIR EL SEMAFORO\n");
		return;
	}

	if(openPipe(PIPE_ID, aux) == -1) {
		printf("NO SE PUDO ABRIR EL PIPE\n");
		return;
	}
	startProcessBg(wrapperhijoEscribe, 0, NULL);
	startProcessBg(wrapperhijoLee, 0, NULL);
	semWait(mbeh);
	semWait(mbeh);
	closePipe(aux[0]);
	closePipe(aux[1]);
	return 0;
}

int main() {
	//test();
	/*
	int fd[2];
	if(openPipe(PIPE_ID, fd) == -1){
		printf("ERROR AL ABRIR PIPE");
		return 0;
	}
	dup2(1, fd[1]);
	write(fd[1], "TEST DE PIPE", 12);
*/
	// while(1)
	// 	;

	runShell();

	return 0;
}
