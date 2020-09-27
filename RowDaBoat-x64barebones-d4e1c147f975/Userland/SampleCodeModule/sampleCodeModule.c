#include <std_io.h>
#include <syscalls.h>
#include <shell.h>
#include <evaluator.h>

int testMain(int argc, char const *argv[]){
	
	while(1){
//		runNext();
	}
	return 0;
}

int main()
{
	startProcess(testMain, 0, NULL);
	startProcess(testMain, 0, NULL);
	startProcess(testMain, 0, NULL);
	startProcess(testMain, 0, NULL);
	startProcess(testMain, 0, NULL);
	nice(1, 0);
	runShell();

	return 0;
}
