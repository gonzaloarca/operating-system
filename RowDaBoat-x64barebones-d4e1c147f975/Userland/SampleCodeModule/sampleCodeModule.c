#include <std_io.h>
#include <syscalls.h>
#include <shell.h>
#include <evaluator.h>

int testMain(int argc, char const *argv[]){
	printf("WORKS LIKE A CHARM\n");
	return 0;
}

int main()
{
	startProcess(testMain, 0, NULL);
	runShell();

	return 0;
}
