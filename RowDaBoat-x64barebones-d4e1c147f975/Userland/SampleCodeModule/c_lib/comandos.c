#include <comandos.h>

void printTime() { //se podria modularizar en una funcion para obtener el tiempo correcto, que abarcaria hasta el while, y otra para generar el string formateado del tiempo, despues juega printString
	TimeFormat time;
	getTime(&time);

	//	Hora local Argentina
	if(time.hours >= 3)
		time.hours -= 3;
	else
		time.hours = 24 + time.hours - 3;

	char timeString[9];

	timeString[2] = ':';
	timeString[5] = ':';
	timeString[8] = 0;

	int data[3] = {time.hours, time.mins, time.secs};
	int j = 0;
	for(int i = 0; i < 3; i++) {
		if(data[i] < 10) {
			timeString[j++] = '0';
		} else {
			timeString[j++] = (data[i] / 10) + '0';
		}
		timeString[j++] = (data[i] % 10) + '0';
		j++;
	}

	puts(timeString);
	putchar('\n');
}

void printCPUInfo() {
	CpuInfo info;

	cpuVendor(info.vendor);
	cpuBrand(info.brand);
	cpuModel(&(info.family));

	printf("Vendor: %s\n", info.vendor);
	printf("FamilyID: %d\n", info.family);
	printf("Model: %d\n", info.model);
	printf("Brand: %s\n", info.brand);
}

static int pow(int potencia, int base) {
	int ans = 1;
	for(int i = 0; i < potencia; i++)
		ans = ans * base;

	return ans;
}

void printmem(char *address) {
	memType memory;

	uint64_t decimalAddress = 0;
	int length = strlen(address);
	int flag = 1; // asumo en un principio que esta correctamente escrito

	// reviso el address este escrito correctamente
	for(int i = length; i > 0 && flag; i--) {
		switch(address[i - 1]) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			decimalAddress += (address[i - 1] - '0') * pow(length - i, 16);
			break;
		case 'a':
		case 'A':
			decimalAddress += 10 * pow(length - i, 16);
			break;
		case 'b':
		case 'B':
			decimalAddress += 11 * pow(length - i, 16);
			break;
		case 'c':
		case 'C':
			decimalAddress += 12 * pow(length - i, 16);
			break;
		case 'd':
		case 'D':
			decimalAddress += 13 * pow(length - i, 16);
			break;
		case 'e':
		case 'E':
			decimalAddress += 14 * pow(length - i, 16);
			break;
		case 'f':
		case 'F':
			decimalAddress += 15 * pow(length - i, 16);
			break;

		default:
			// Encontre un caracter que no cumple con la escritura hexadecimal de un numero positivo
			flag = 0;
		}
	}

	if(flag) {
		getMemory(&memory, (char *)decimalAddress);

		changeWindowColor(0xa4de02);
		printf("%p", (char *)decimalAddress);
		changeWindowColor(0xffffff);
		printf(": ");
		for(int i = 0; i < 8; i++) {
			printf("%x ", 0xFF & memory.mem1);
			memory.mem1 = memory.mem1 >> 8;
		}
		putchar('\n');
		changeWindowColor(0xa4de02);
		printf("%p", (char *)decimalAddress + 8);
		changeWindowColor(0xffffff);
		printf(": ");
		for(int i = 0; i < 8; i++) {
			printf("%x ", 0xFF & memory.mem2);
			memory.mem2 = memory.mem2 >> 8;
		}
		putchar('\n');
		changeWindowColor(0xa4de02);
		printf("%p", (char *)decimalAddress + 16);
		changeWindowColor(0xffffff);
		printf(": ");
		for(int i = 0; i < 8; i++) {
			printf("%x ", 0xFF & memory.mem3);
			memory.mem3 = memory.mem3 >> 8;
		}
		putchar('\n');
		changeWindowColor(0xa4de02);
		printf("%p", (char *)decimalAddress + 24);
		changeWindowColor(0xffffff);
		printf(": ");
		for(int i = 0; i < 8; i++) {
			printf("%x ", 0xFF & memory.mem4);
			memory.mem4 = memory.mem4 >> 8;
		}
		putchar('\n');
	} else
		fprintf(2, "-printmem:Direccion invalida.\n");
}

void printInforeg() {
	RegistersType *reg = getRegisters();
	printf("RAX: %lx\tRBX: %lx\tRCX: %lx\nRDX: %lx\tRBP: %lx\tRDI: %lx\nRSI: %lx\tRSP: %lx\tR8:  %lx\nR9:  %lx\tR10: %lx\tR11: %lx\nR12: %lx\tR13: %lx\tR14: %lx\nR15: %lx\tRIP: %lx\n",
	       (unsigned long)reg->rax, (unsigned long)reg->rbx, (unsigned long)reg->rcx,
	       (unsigned long)reg->rdx, (unsigned long)reg->rbp, (unsigned long)reg->rdi,
	       (unsigned long)reg->rsi, (unsigned long)reg->rsp, (unsigned long)reg->r8,
	       (unsigned long)reg->r9, (unsigned long)reg->r10, (unsigned long)reg->r11,
	       (unsigned long)reg->r12, (unsigned long)reg->r13, (unsigned long)reg->r14, (unsigned long)reg->r15, (unsigned long)reg->rip);
}

void printCPUTemp() {
	int temp = getCPUTemp();
	changeWindowColor(0xff4500);
	printf("Temperatura actual del CPU: ");
	changeWindowColor(0xffffff);
	printf("%d C\n", temp);
}

void printMemStatus() {
	MemStatus stat;
	getMemStatus(&stat);
	printf("Total = %lu B\tOcupada = %lu B\tLibre = %lu B\tPorcentaje libre: %f%%\n", stat.totalMem, stat.occMem, stat.freeMem, (double)(stat.freeMem * 100) / stat.totalMem);
}

void loop() {
	int pid = getpid();
	while(1) {
		for(int i = 0; i < LOOP_SLEEP; i++)
			;
		printf("%d ", pid);
	}
}

int unblock(int PID) {
	return kill(PID, ACTIVE);
}

int block(int PID) {
	return kill(PID, BLOCKED);
}

int wc() {
	char inputBuffer[INPUT_BUFFER_SIZE];
	int length, lines = 0;

	while((length = read(0, inputBuffer, INPUT_BUFFER_SIZE)) != 0) {
		for(int i = 0; i < length; i++)
			if(inputBuffer[i] == '\n')
				lines++;
	}

	printf("Cantidad de lineas: %d\n", lines);
	return lines;
}

static int isVowel(char c) {
	return c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u' || c == 'A' || c == 'E' || c == 'I' || c == 'O' || c == 'U';
}

static int min(int num1, int num2) {
	return (num1 > num2) ? num2 : num1;
}

void cat() {
	char inputBuffer[INPUT_BUFFER_SIZE];
	int length;

	while((length = read(0, inputBuffer, INPUT_BUFFER_SIZE)) != 0) {
		write(1, inputBuffer, length);
	}
}

void filter() {
	char inputBuffer[INPUT_BUFFER_SIZE], noVowelBuffer[MAX_OUTPUT_BUFFER];
	int length, j = 0;

	// El count de read se define entre el minimo de lo que se considera tamaño de una linea y la capacidad restante dentro del buffer respuesta
	while((length = read(0, inputBuffer, min(INPUT_BUFFER_SIZE, MAX_OUTPUT_BUFFER - j))) != 0 && j < MAX_OUTPUT_BUFFER) {
		for(int i = 0; i < length; i++)
			if(!isVowel(inputBuffer[i]))
				noVowelBuffer[j++] = inputBuffer[i];
	}
	noVowelBuffer[j] = 0;
	printf("%s", noVowelBuffer);
}

// Constantes utilizadas por el comando help
#define CLEAR_MSG "\tLimpia la pantalla."
#define CPUINFO_MSG "\tMuestra informacion de interes sobre el cpu."
#define CPUTEMP_MSG "\tMuestra la temperatura del cpu."
#define INFOREG_MSG "\tMuestra el estado de los registros del ultimo guardado(los guardados se realizan presionando F1)."
#define PRINTMEM_MSG "\tMuestra los 32 bytes siguientes a partir de la direccion de memoria(en hexadecimal) recibida por parametro."
#define PRINTTIME_MSG "\tMuestra la hora actual del dispositivo."
#define MEM_MSG "\tMuestra el estado de la memoria disponible para alocar."
#define PS_MSG "\tMuestra una lista de los procesos actuales."
#define PIPE_MSG "\tMuestra una lista de los pipes actuales."
#define SEM_MSG "\tMuestra una lista de los semaforos actuales."
#define KILL_MSG "\tFinaliza el proceso del pid indicado por parametro."
#define LOOP_MSG "\tCrea un proceso que imprime su PID cada cierto tiempo mientras se ejecuta."
#define BLOCK_MSG "\tBloquea el proceso del PID indicado"
#define UNBLOCK_MSG "\tDesloquea el proceso del PID indicado"
#define NICE_MSG "\tModifica la prioridad del proceso indicado, la prioridad es entre 0 y 5 siendo 0 la maxima prioridad."
#define CAT_MSG "\tFuncion que escribe lo que lee."
#define FILTER_MSG "\tFuncion que escribe lo que lee sin vocales."
#define WC_MSG "\tProceso que cuenta la cantidad de lineas del input."
#define EXCP_0_MSG "\tComando para verificar la rutina de excepcion de division por cero."
#define EXCP_6_MSG "\tComando para verificar la rutina de excepcion de operacion invalida(Undefined Instruction)."
#define TEST_MSG "Comandos para ejecutar tests:"
#define TECLA_F1 "\tEjecuta el guardado de los registros, para que sean impresos con inforeg."
#define TECLA_F2 "\tEjecuta el borrado total de la linea actual."
#define BACKGROUND "\tAgreguelo al final de un comando para correrlo en background."
#define CALCULATOR "\tSistema de calculadora simple."
#define FILOSOFOS "\tCorre una solucion para el problema de los filosofos."
#define TECLA_F3 "\tFrena la ejecucion del proceso actual en Foreground y regresa a la shell."
#define TECLA_F4 "\tManda un EOF por stdin."

void help() {
	puts("---------Informacion sobre comandos disponibles----------------\n");
	changeWindowColor(0xffd300);
	printf("%s\t\t", "clear");
	changeWindowColor(0xffffff);
	printf("%s\n", CLEAR_MSG);

	changeWindowColor(0xffd300);
	printf("%s  \t", "cpuinfo");
	changeWindowColor(0xffffff);
	printf("%s\n", CPUINFO_MSG);

	changeWindowColor(0xffd300);
	printf("%s  \t", "cputemp");
	changeWindowColor(0xffffff);
	printf("%s\n", CPUTEMP_MSG);

	changeWindowColor(0xffd300);
	printf("%s  \t", "inforeg");
	changeWindowColor(0xffffff);
	printf("%s\n", INFOREG_MSG);

	changeWindowColor(0xffd300);
	printf("%s dir ", "printmem");
	changeWindowColor(0xffffff);
	printf("%s\n", PRINTMEM_MSG);

	changeWindowColor(0xffd300);
	printf("%s\t", "printtime");
	changeWindowColor(0xffffff);
	printf("%s\n", PRINTTIME_MSG);

	changeWindowColor(0xffd300);
	printf("%s\t\t  ", "mem");
	changeWindowColor(0xffffff);
	printf("%s\n", MEM_MSG);

	changeWindowColor(0xffd300);
	printf("%s\t\t   ", "ps");
	changeWindowColor(0xffffff);
	printf("%s\n", PS_MSG);

	changeWindowColor(0xffd300);
	printf("%s\t\t ", "pipe");
	changeWindowColor(0xffffff);
	printf("%s\n", PIPE_MSG);

	changeWindowColor(0xffd300);
	printf("%s\t\t  ", "sem");
	changeWindowColor(0xffffff);
	printf("%s\n", SEM_MSG);

	changeWindowColor(0xffd300);
	printf("%s PID\t ", "kill");
	changeWindowColor(0xffffff);
	printf("%s\n", KILL_MSG);

	changeWindowColor(0xffd300);
	printf("%s\t\t ", "loop");
	changeWindowColor(0xffffff);
	printf("%s\n", LOOP_MSG);

	changeWindowColor(0xffd300);
	printf("%s\t", "block PID");
	changeWindowColor(0xffffff);
	printf("%s\n", BLOCK_MSG);

	changeWindowColor(0xffd300);
	printf("%s  ", "unblock PID");
	changeWindowColor(0xffffff);
	printf("%s\n", UNBLOCK_MSG);

	changeWindowColor(0xffd300);
	printf("%s", "nice PID PRIO");
	changeWindowColor(0xffffff);
	printf("%s\n", NICE_MSG);

	changeWindowColor(0xffd300);
	printf("%s\t\t   ", "wc");
	changeWindowColor(0xffffff);
	printf("%s\n", WC_MSG);

	changeWindowColor(0xffd300);
	printf("%s\t\t  ", "cat");
	changeWindowColor(0xffffff);
	printf("%s\n", CAT_MSG);

	changeWindowColor(0xffd300);
	printf("%s\t   ", "filter");
	changeWindowColor(0xffffff);
	printf("%s\n", FILTER_MSG);

	changeWindowColor(0xffd300);
	printf("%s\t\t ", "calc");
	changeWindowColor(0xffffff);
	printf("%s\n", CALCULATOR);

	changeWindowColor(0xffd300);
	printf("%s\t\t ", "phylo");
	changeWindowColor(0xffffff);
	printf("%s\n", FILOSOFOS);

	changeWindowColor(0xffd300);
	printf("%s", "executeZeroException");
	changeWindowColor(0xffffff);
	printf("%s\n", EXCP_0_MSG);

	changeWindowColor(0xffd300);
	printf("%s", "executeUIException  ");
	changeWindowColor(0xffffff);
	printf("%s\n", EXCP_6_MSG);

	printf("%s\t", TEST_MSG);
	changeWindowColor(0xffd300);
	printf("%s  ", "test_mm\t");

	printf("%s", "test_prio\t");

	printf("%s", "test_proc\t");

	printf("%s", "test_sync\t");

	printf("%s", "test_no_sync\t");

	printf("%s", "test_pipe\t");
	changeWindowColor(0xffffff);

	putchar('\n');

	printf("\n---------Informacion sobre teclas especiales disponibles-------\n");
	changeWindowColor(0x4ad5f2);
	printf("%s", "F1\t");
	changeWindowColor(0xffffff);
	printf("%s\n", TECLA_F1);

	changeWindowColor(0x4ad5f2);
	printf("%s", "F2\t");
	changeWindowColor(0xffffff);
	printf("%s\n", TECLA_F2);

	changeWindowColor(0x4ad5f2);
	printf("%s", "F3\t");
	changeWindowColor(0xffffff);
	printf("%s\n", TECLA_F3);

	changeWindowColor(0x4ad5f2);
	printf("%s", "F4\t");
	changeWindowColor(0xffffff);
	printf("%s\n", TECLA_F4);

	changeWindowColor(0x4ad5f2);
	printf("%s", " &\t");
	changeWindowColor(0xffffff);
	printf("%s\n", BACKGROUND);
}
