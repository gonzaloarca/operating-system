#include <comandos.h>
#include <evaluator.h>
#include <std_io.h>
#include <syscalls.h>
#include <test_util.h>

#define INPUT_BUFFER_SIZE 100
#define BG_SYMBOL '&'
#define PIPE_SYMBOL '|'

char inputBuffer[INPUT_BUFFER_SIZE + 1];
int indexBuffer;
static char *symbol = "$>";

static void parse();
static void run(char *command, int bgFlag);
static void startCommand(int (*mainPtr)(int, const char **), char *procName, int bgFlag);

void runShell() {
	printf("\nIngrese help y presione enter para una explicacion del programa\n");
	while(1) {
		puts(symbol);
		indexBuffer = read(inputBuffer, INPUT_BUFFER_SIZE, '\n');
		parse();
	}
}

static void parse() {
	int bgFlag = 0;
	int pipeFlag = 0;

	inputBuffer[--indexBuffer] = 0; //Me saco el \n del comando

	if(indexBuffer == 0) //Comando vacio
		return;

	//Veo si me pide correr en background
	if(inputBuffer[indexBuffer - 1] == BG_SYMBOL && inputBuffer[indexBuffer - 2] == ' ') {
		bgFlag = 1;
		//Quito el simbolo del string
		indexBuffer -= 2;
		inputBuffer[indexBuffer] = 0;
	}

	int i;
	for(i = 0; i < indexBuffer; i++) {
		if(inputBuffer[i] == PIPE_SYMBOL) {
			pipeFlag = 1;
			break;
		}
	}

	if(pipeFlag) {
		//Implementar sistema de piping, en i se encuentra la posicion del simbolo
		fprintf(2, "Comando no reconocido, ejecuta help para recibir informacion.\n");
	} else {
		run(inputBuffer, bgFlag);
	}
}

//Recibe el nombre del comando como string y un flag indicando si es bg o fg
static void run(char *command, int bgFlag) {
	const char *name;

	if(strcmp(command, "help") == 0)
		help();
	else if(strcmp(command, "inforeg") == 0)
		printInforeg();
	else if(strcmp(command, "clear") == 0)
		clrScreen();
	else if(strcmp(command, "printtime") == 0) {
		printTime();
		putchar('\n');
	} else if(strcmp("printmem", command) == 0) {		    // solo valido que lo q este al principio del input valide con el comando,
								    // el resto va a ser el argumento
		if(command[8] == ' ' && command[9] != 0) {	    // se ingreso un parametro
			char address[INPUT_BUFFER_SIZE - 10] = {0}; // maximo tamanio posible del argumento("printmem " son 9 caracteres y tampoco cuento el \n)
			int i = 0;
			for(; command[9 + i] != 0; i++)
				address[i] = command[9 + i];

			address[i] = 0;
			printmem(address);
		} else
			fprintf(2, "-printmem: Falta ingresar la direccion como parametro.\n");
	} else if(strcmp(command, "cpuinfo") == 0)
		printCPUInfo();
	else if(strcmp(command, "cputemp") == 0)
		printCPUTemp();
	else if(strcmp(command, "executeZeroException") == 0) {
		name = "Zero Exc";
		startProcessFg((int (*)(int, const char **))executeZeroException, 1, &name);
	} else if(strcmp(command, "executeUIException") == 0) {
		name = "UI Exc";
		startProcessFg((int (*)(int, const char **))executeUIException, 1, &name);
	} else if(strcmp(command, "mem") == 0)
		printMemStatus();
	else if(strcmp(command, "ps") == 0)
		listProcess();
	else if(strcmp("kill ", command) == 0) {
		int pid = strToPositiveInt(command + 5, NULL);
		if(pid == -1)
			printf("Error en argumentos\n");

		kill(pid, KILLED);
	} else if(strcmp(command, "loop") == 0) {
		startCommand((int (*)(int, const char **))loop, "loop", bgFlag);
	} else if(strcmp("block ", command) == 0) {
		int pid, aux = 0;
		pid = strToPositiveInt(command + 6, &aux);
		if(pid == -1)
			printf("Error en argumentos\n");
		if(block(pid) == -1)
			printf("Error en argumentos\n");
	} else if(strcmp("unblock ", command) == 0) {
		int pid, aux = 0;
		pid = strToPositiveInt(command + 8, &aux);
		if(pid == -1)
			printf("Error en argumentos\n");
		if(unblock(pid) == -1)
			printf("Error en argumentos\n");
	} else if(strcmp("nice ", command) == 0) {
		int pid, priority, aux = 0;
		pid = strToPositiveInt(command + 5, &aux);
		if(pid == -1)
			printf("Error en argumentos\n");

		priority = strToPositiveInt(command + 5 + aux + 1, NULL);
		if(priority == -1)
			printf("Error en argumentos\n");

		if(nice(pid, priority) == -1)
			printf("Error en argumentos\n"); //puede que no haya encontrado el pid o que la prioridad no sea valida
	} else if(strcmp(command, "test_mm") == 0) {
		startCommand((int (*)(int, const char **))test_mm, "test_mm", bgFlag);
	} else if(strcmp(command, "test_prio") == 0) {
		startCommand((int (*)(int, const char **))test_prio, "test_prio", bgFlag);
	} else if(strcmp(command, "test_proc") == 0) {
		startCommand((int (*)(int, const char **))test_processes, "test_processes", bgFlag);
	} else if(strcmp(command, "test_sync") == 0) {
		startCommand((int (*)(int, const char **))test_sync, "test_sync", bgFlag);
	} else if(strcmp(command, "test_no_sync") == 0) {
		startCommand((int (*)(int, const char **))test_no_sync, "test_no_sync", bgFlag);
	} else if(strcmp(command, "calc") == 0) {
		startCommand((int (*)(int, const char **))calculator, "calc", 0);
	} else
		fprintf(2, "Comando no reconocido, ejecuta help para recibir informacion.\n");
}

static void startCommand(int (*mainPtr)(int, const char **), char *procName, int bgFlag) {
	const char *name = procName;

	if(bgFlag)
		startProcessBg(mainPtr, 1, &name);
	else
		startProcessFg(mainPtr, 1, &name);
}
