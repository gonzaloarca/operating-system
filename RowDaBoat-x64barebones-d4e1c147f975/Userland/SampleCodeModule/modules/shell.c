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
static int builtIn(char *command);
static programStart getProgram(char *command);
static void run(programStart mainPtr, const char *procName, int bgFlag);

void runShell() {
	printf("\nIngrese help y presione enter para una explicacion del programa\n");
	while(1) {
		puts(symbol);
		indexBuffer = read(0, inputBuffer, INPUT_BUFFER_SIZE);
		parse();
	}
}

static void parse() {
	programStart mainPtr;
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
		//Primero veo si es una función built-in
		if(builtIn(inputBuffer) == 0) {
			mainPtr = getProgram(inputBuffer);
			if(mainPtr != NULL)
				run(mainPtr, inputBuffer, bgFlag);
		}
	}
}

//	Ejecuta una funcion built-in de la shell y devuelve 1
//	Si no existe como built-in, devuelve 0
static int builtIn(char *command) {
	const char *name;

	if(strcmp(command, "help") == 0)
		help();
	else if(strcmp(command, "inforeg") == 0)
		printInforeg();
	else if(strcmp(command, "clear") == 0)
		clrScreen();
	else if(strcmp(command, "printtime") == 0)
		printTime();
	else if(strincl("printmem ", command) == 0) {		    // solo valido que lo q este al principio del input valide con el comando,
								    // el resto va a ser el argumento
		if(command[9] != 0) {				    // se ingreso un parametro
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
	else if(strcmp(command, "mem") == 0)
		printMemStatus();
	else if(strcmp(command, "ps") == 0)
		listProcess();
	else if(strcmp(command, "pipe") == 0)
		listPipes();
	else if(strcmp(command, "sem") == 0)
		listSems();
	else if(strincl("kill ", command) == 0) {
		int pid = strToPositiveInt(command + 5, NULL);
		if(pid == -1)
			printf("Error en argumentos\n");

		kill(pid, KILLED);
	} else if(strincl("block ", command) == 0) {
		int pid, aux = 0;
		pid = strToPositiveInt(command + 6, &aux);
		if(pid == -1)
			printf("Error en argumentos\n");
		if(block(pid) == -1)
			printf("Error en argumentos\n");
	} else if(strincl("unblock ", command) == 0) {
		int pid, aux = 0;
		pid = strToPositiveInt(command + 8, &aux);
		if(pid == -1)
			printf("Error en argumentos\n");
		if(unblock(pid) == -1)
			printf("Error en argumentos\n");
	} else if(strincl("nice ", command) == 0) {
		int pid, priority, aux = 0;
		pid = strToPositiveInt(command + 5, &aux);
		if(pid == -1)
			printf("Error en argumentos\n");

		priority = strToPositiveInt(command + 5 + aux + 1, NULL);
		if(priority == -1)
			printf("Error en argumentos\n");

		if(nice(pid, priority) == -1)
			printf("Error en argumentos\n"); //puede que no haya encontrado el pid o que la prioridad no sea valida
	} else if(strcmp(command, "executeZeroException") == 0) {
		name = "Zero Exc";
		startProcessFg((int (*)(int, const char **))executeZeroException, 1, &name);
	} else if(strcmp(command, "executeUIException") == 0) {
		name = "UI Exc";
		startProcessFg((int (*)(int, const char **))executeUIException, 1, &name);
	} else {
		return 0;
	}

	return 1;
}

//Recibe el nombre del comando como string y devuelve el puntero a su inicio (para funciones no built-in)
static programStart getProgram(char *command) {
	if(strcmp(command, "loop") == 0) {
		return (programStart)loop;
	} else if(strcmp(command, "test_mm") == 0) {
		return (programStart)test_mm;
	} else if(strcmp(command, "test_prio") == 0) {
		return (programStart)test_prio;
	} else if(strcmp(command, "test_proc") == 0) {
		return (programStart)test_processes;
	} else if(strcmp(command, "test_sync") == 0) {
		return (programStart)test_sync;
	} else if(strcmp(command, "test_no_sync") == 0) {
		return (programStart)test_no_sync;
	} else if(strcmp(command, "calc") == 0) {
		return (programStart)calculator;
	} else if(strcmp(command, "test_pipe") == 0) {
		return (programStart)test_pipe;
	}

	fprintf(2, "Comando no reconocido, ejecuta help para recibir informacion.\n");

	return NULL;
}

static void run(programStart mainPtr, const char *procName, int bgFlag) {
	if(bgFlag)
		startProcessBg(mainPtr, 1, &procName);
	else
		startProcessFg(mainPtr, 1, &procName);
}
