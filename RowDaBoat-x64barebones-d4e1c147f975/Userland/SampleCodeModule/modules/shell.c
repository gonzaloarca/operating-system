#include <comandos.h>
#include <evaluator.h>
#include <filosofos.h>
#include <sem.h>
#include <std_io.h>
#include <syscalls.h>
#include <test_util.h>

#define INPUT_BUFFER_SIZE 100
#define BG_SYMBOL '&'
#define PIPE_SYMBOL '|'
#define SHELL_PIPE_ID 9348

char inputBuffer[INPUT_BUFFER_SIZE + 1];
int indexBuffer;
static char *symbol = "$>";

static void parse();
static int builtIn(char *command);
static programStart getProgram(char *command);
static void run(programStart mainPtr, const char *procName, int bgFlag);
static void errorMsg();
static void startPipe(char *cmd1, char *cmd2, int bgFlag);
static int wrapperFirstProgram(int argc, const char *argv[]);
static int wrapperSecondProgram(int argc, const char *argv[]);

void runShell() {
	printf("\nIngrese help y presione enter para una explicacion del programa\n");
	puts(symbol);
	while(1) {
		indexBuffer = read(0, inputBuffer, INPUT_BUFFER_SIZE);
		if(indexBuffer != 0 && inputBuffer[indexBuffer - 1] == '\n') {
			parse();
			puts(symbol);
		}
	}
}

static void parse() {
	programStart mainPtr;
	int bgFlag = 0, pipeFlag = 0;
	char cmd1[INPUT_BUFFER_SIZE], cmd2[INPUT_BUFFER_SIZE];

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

	for(int i = 0; i < indexBuffer; i++) {
		if(inputBuffer[i] == PIPE_SYMBOL) {
			pipeFlag = i;
			break;
		}
		//Voy copiando el primer (o unico) comando
		cmd1[i] = inputBuffer[i];
	}

	if(pipeFlag) {
		cmd1[pipeFlag - 1] = 0;		       //Hago que el comando sea null-terminated antes del simbolo pipe
		if(inputBuffer[pipeFlag + 1] != ' ') { //Tiene que haber un espacio antes del comando
			errorMsg();
			return;
		}
		strcopy(inputBuffer + pipeFlag + 2, cmd2);

		//Solamente hago el piping si ninguno era built-in
		if(builtIn(cmd1) == 0 && builtIn(cmd2) == 0) {
			startPipe(cmd1, cmd2, bgFlag);
		}

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
	else if(strincl("printmem ", command)) {		    // solo valido que lo q este al principio del input valide con el comando,
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
	else if(strincl("kill ", command)) {
		int pid = strToPositiveInt(command + 5, NULL);
		if(pid == -1)
			printf("Error en argumentos\n");

		kill(pid, KILLED);
	} else if(strincl("block ", command)) {
		int pid, aux = 0;
		pid = strToPositiveInt(command + 6, &aux);
		if(pid == -1)
			printf("Error en argumentos\n");
		if(block(pid) == -1)
			printf("Error en argumentos\n");
	} else if(strincl("unblock ", command)) {
		int pid, aux = 0;
		pid = strToPositiveInt(command + 8, &aux);
		if(pid == -1)
			printf("Error en argumentos\n");
		if(unblock(pid) == -1)
			printf("Error en argumentos\n");
	} else if(strincl("nice ", command)) {
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
	} else if(strcmp(command, "wc") == 0) {
		return (programStart)wc;
	} else if(strcmp(command, "cat") == 0) {
		return (programStart)cat;
	} else if(strcmp(command, "filter") == 0) {
		return (programStart)filter;
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
	} else if(strcmp(command, "phylo") == 0) {
		return (programStart)phylo;
	}

	errorMsg();

	return NULL;
}

static void run(programStart mainPtr, const char *procName, int bgFlag) {
	if(bgFlag)
		startProcessBg(mainPtr, 1, &procName);
	else
		startProcessFg(mainPtr, 1, &procName);
}

static void errorMsg() {
	fprintf(2, "Comando no reconocido, ejecuta help para recibir informacion.\n");
}

// Esta función me inicia el pipeado de los comandos que se les pasa
static void startPipe(char *cmd1, char *cmd2, int bgFlag) {
	int aux[2];
	Semaphore *sem;

	if((sem = semOpen(SHELL_PIPE_ID, 0)) == NULL) {
		printf("NO SE PUDO ABRIR EL SEMAFORO\n");
		return;
	}

	if(pipeOpen(SHELL_PIPE_ID, aux) == -1) {
		printf("NO SE PUDO ABRIR EL PIPE\n");
		semClose(sem);
		return;
	}

	//Corro los programas, teniendo en cuenta que el segundo siempre es en Background
	//Y el primero depende si se incluyó el & al final del comando
	run(wrapperSecondProgram, cmd2, 1);
	run(wrapperFirstProgram, cmd1, bgFlag);

	//Tengo que esperar para cerrar el pipe
	semWait(sem);
	semWait(sem);
	semClose(sem);

	pipeClose(aux[0]);
	pipeClose(aux[1]);
}

static int wrapperFirstProgram(int argc, const char *argv[]) {
	int fd[2];
	Semaphore *sem;
	programStart prog;
	char cmd[INPUT_BUFFER_SIZE];

	strcopy(argv[0], cmd);

	//El nombre del programa me define el comando a correr
	if((prog = getProgram(cmd)) == NULL) {
		errorMsg();
		return 1;
	}

	if((sem = semOpen(SHELL_PIPE_ID, 0)) == NULL) {
		printf("NO SE PUDO ABRIR EL SEMAFORO\n");
		return 1;
	}

	if(pipeOpen(SHELL_PIPE_ID, fd) == -1) {
		printf("NO SE PUDO ABRIR EL PIPE\n");
		semClose(sem);
		return 1;
	}

	pipeClose(fd[0]);
	dup2(fd[1], 1);
	semPost(sem);
	semClose(sem);

	prog(argc, argv);

	pipeClose(fd[1]);
	pipeClose(1);

	return 0;
}

static int wrapperSecondProgram(int argc, const char *argv[]) {
	int fd[2];
	Semaphore *sem;
	programStart prog;
	char cmd[INPUT_BUFFER_SIZE];

	strcopy(argv[0], cmd);

	//El nombre del programa me define el comando a correr
	if((prog = getProgram(cmd)) == NULL) {
		errorMsg();
		return 1;
	}

	if((sem = semOpen(SHELL_PIPE_ID, 0)) == NULL) {
		printf("NO SE PUDO ABRIR EL SEMAFORO\n");
		return 1;
	}

	if(pipeOpen(SHELL_PIPE_ID, fd) == -1) {
		printf("NO SE PUDO ABRIR EL PIPE\n");
		return 1;
	}

	pipeClose(fd[1]);
	dup2(fd[0], 0);
	semPost(sem);
	semClose(sem);

	prog(argc, argv);

	pipeClose(fd[0]);
	pipeClose(0);

	return 0;
}
