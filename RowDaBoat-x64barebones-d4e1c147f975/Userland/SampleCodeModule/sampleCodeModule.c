#include <std_io.h>
#include <syscalls.h>
#include <shell.h>
#include <evaluator.h>

#define PIPE_ID 50

int hijoLee(){
	char buf[256];
	read(1, buf, 13);
	printf("%s",buf);
	return 0;
}

int hijoEscribe(){
	write(0,"JULIANSICARDI", 13);
	return 0;
}

void wrapperhijoLee(){

	int fd[2];
	if(openPipe(PIPE_ID, fd) == -1)
		return;

//	closePipe(fd[1]);
	dup2(fd[0], 0);
	printf("\nlee\n");

	hijoLee();
}

void wrapperhijoEscribe(){

	int fd[2];
	if(openPipe(PIPE_ID, fd) == -1)
		return;


//	closePipe(fd[0]);
	dup2(fd[1], 1);

	printf("\nescribe\n");

	hijoEscribe();
}

int test(){
	printf("\ntest\n");

	startProcessBg(wrapperhijoLee, 0, NULL);
	startProcessBg(wrapperhijoEscribe, 0, NULL);
	
	return 0;
}

int main()
{
//	test();
	runShell();

	return 0;
}
