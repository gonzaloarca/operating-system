#include <std_io.h>
#include <syscalls.h>
#include <shell.h>
#include <evaluator.h>

// int hijoLee(){
// 	char buf[256];
// 	read(1, buf, 13);
// 	printf("%s",buf);
// 	return 0;
// }

// int hijoEscribe(){
// 	write(0,"JULIANSICARDI", 13);
// 	return 0;
// }

// void wrapperhijoLee(char *fd){
// 	dup2(0,3);
// 	hijoLee();
// }

// void wrapperhijoEscribe(char *fd){
// 	dup2(1,4);
// 	hijoEscribe();
// }

// int test(){
// 	int fd[2];
// 	if(createPipe(fd) == -1)
// 		return -1;
	
// 	startProcessBg(wrapperhijoLee, 0, NULL);
// 	startProcessBg(wrapperhijoEscribe, 0, NULL);
	
// 	return 0;
// }

int main()
{
	//testPipe();
	runShell();

	return 0;
}
