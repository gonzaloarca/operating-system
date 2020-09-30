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
	char *test = "test";
	for (size_t i = 0; i < 5; i++)
	{
		startProcess(testMain, 1, &test);	
	}
	nice(1, 0);
	runShell();

	return 0;
}
