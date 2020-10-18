#include <evaluator.h>
#include <shell.h>
#include <std_io.h>
#include <syscalls.h>

#define mbeh(type) ((unsigned char *)(&type + 1) - (unsigned char *)(&type))

#define PIPE_ID 50

int hijoLee() {
	char buf[256];
	int n = read(0, buf, 47);
	write(1, buf, 47);
	return 0;
}

int hijoEscribe() {
	int n = write(1, "JULIANSICARDIELBETARDITENMBEHXDMBERTELBETOSAPE\n", 47);
	fprintf(2, "n = %d\n", n);
	return 0;
}

void wrapperhijoLee() {
	int fd[2];
	if(openPipe(PIPE_ID, fd) == -1) {
		printf("NO SE PUDO ABRIR EL PIPE\n");
		return;
	}

	closePipe(fd[1]);
	dup2(fd[0], 0);
	hijoLee();

	// closePipe(fd[0]);
	while(1)
		;
}

void wrapperhijoEscribe() {
	int fd[2];
	if(openPipe(PIPE_ID, fd) == -1) {
		printf("NO SE PUDO ABRIR EL PIPE\n");
		return;
	}

	closePipe(fd[0]);
	dup2(fd[1], 1);
	hijoEscribe();
	while(1)
		;
	// closePipe(fd[1]);
}

int test() {
	startProcessBg(wrapperhijoEscribe, 0, NULL);
	startProcessBg(wrapperhijoLee, 0, NULL);

	return 0;
}

int main() {
	test();
	/*
	int fd[2];
	if(openPipe(PIPE_ID, fd) == -1){
		printf("ERROR AL ABRIR PIPE");
		return 0;
	}
	dup2(1, fd[1]);
	write(fd[1], "TEST DE PIPE", 12);
*/
	while(1)
		;

	//	runShell();

	return 0;
}
