#include <keyboard.h>
#include <libasm64.h>
#include <memManager.h>
#include <pipe.h>
#include <registers.h>
#include <rtc_driver.h>
#include <scheduler.h>
#include <sig.h>
#include <stdint.h>
#include <video_driver.h>
#include <window_manager.h>

typedef struct {
	uint64_t rbx;
	uint64_t rcx;
	uint64_t rdx;
	uint64_t rsi;
	uint64_t rdi;
} * Registers;

// La syscall 3 es read
uint64_t syscall_03(uint64_t rbx, uint64_t rcx, uint64_t rdx) {
	return sys_read((int)rbx, (char *)rcx, (unsigned long)rdx);
}

//	La syscall 4 es write
uint64_t syscall_04(uint64_t rbx, uint64_t rcx, uint64_t rdx) {
	return sys_write((int)rbx, (const char *)rcx, (unsigned long)rdx);
}

//	Limpia la ventana actual
uint64_t syscall_07() {
	sys_clrScreen();
	return 1;
}

// La syscall 9 permite el cambio de color de los caracteres a escribir en la ventana actual
uint64_t syscall_09(uint64_t rbx) {
	return sys_changeWindowColor((int)rbx);
}

//	La syscall 10 vacia el buffer de teclado
uint64_t syscall_10() {
	sys_emptyBuffer();
	return 0;
}

//	La syscall 12 recibe un puntero a estructura de tipo TimeFormat y la rellena con los datos actuales del RTC
uint64_t syscall_12(uint64_t rbx) {
	sys_getTime((TimeFormat *)rbx);
	return 0;
}

//	La syscall 13 devuelve la temperatura del CPU como un conjunto en celsius
uint64_t syscall_13() {
	return sys_getCPUTemp();
}

// La syscall 14 devuelve los registros con los valores de cuando se presiono por ultima vez F1
uint64_t syscall_14() {
	return (uint64_t)sys_getRegisters();
}

//	La syscall 25 rellena en la estructura indicada por parametro los 32 bytes de informacion que se obtienen a partir de address
uint64_t syscall_25(uint64_t rbx, uint64_t rcx) {
	sys_getMemory((memType *)rbx, (char *)rcx);
	return 0;
}

// La syscall 26 permite alocar memoria suficiente para que entre la indicada por parametro, en caso de ser posible
uint64_t syscall_26(uint64_t rbx) {
	return (uint64_t)sys_malloc((size_t)rbx);
}

// La syscall 27 permite liberar la memoria alocada que arranca en la direccion indicada por parametro, en caso de no existir realiza undefined behaviour
uint64_t syscall_27(uint64_t rbx) {
	sys_free((void *)rbx);
	return 0;
}

// La syscall 28 rellena en la estructura indicada por parametro los valores del estado de la memoria disponible para alocar en dicho instante
uint64_t syscall_28(uint64_t rbx) {
	sys_getMemStatus((MemStatus *)rbx);
	return 0;
}

// La syscall 30 es para iniciar un proceso en foreground (bloquea al proceso en foreground).
//Se le pasa el punteor al main del programa y los atrbutos de este (argc y argv).
//Devuelve el pid del nuevo proceso.
uint64_t syscall_29(uint64_t rbx, uint64_t rcx, uint64_t rdx) {
	return sys_startProcFg(rbx, (int)rcx, (char const **)rdx);
}

// La syscall 30 es para iniciar un proceso en background (no bloquea el proceso en foreground).
//Se le pasa el punteor al main del programa y los atrbutos de este (argc y argv).
//Devuelve el pid del nuevo proceso.
uint64_t syscall_30(uint64_t rbx, uint64_t rcx, uint64_t rdx) {
	return sys_startProcBg(rbx, (int)rcx, (char const **)rdx);
}

//	La syscall 31 pone el estado del proceso actual en KILLED para luego quitarlo de la lista en la proxima iteracion
uint64_t syscall_31() {
	sys_exit();
	return 0;
}

//	La syscall 32 retorna el pid del proceso que la ejecuto
uint64_t syscall_32() {
	return sys_getpid();
}

// La syscall 33 imprime los procesos actuales
uint64_t syscall_33() {
	sys_listProcess();
	return 0;
}

//	La syscall 34 cambia el estado del proceso en especifico segun el pid indicado. Devuelve 1 si pudo encontrar el proceso
uint64_t syscall_34(uint64_t rbx, uint64_t rcx) {
	return sys_kill((unsigned int)rbx, (char)rcx);
}

//	La syscall 35 cede el CPU al siguiente proceso
uint64_t syscall_35() {
	sys_runNext();
	return 0;
}

//	La syscall 36 cambia la prioridad de un proceso
uint64_t syscall_36(uint64_t rbx, uint64_t rcx) {
	return sys_nice((unsigned int)rbx, (unsigned int)rcx);
}

// La syscall 37 crea un canal de comunicacion para señales de sleep y wakeup,
// devuelve el ID del canal, y en caso de error devuelve -1
uint64_t syscall_37() {
	return sys_createChannel();
}

// La syscall 38 destruye un canal de comunicacion para señales de sleep y wakeup dado su ID
// Si el ID no se corresponde con ningun canal existente, devuelve -1. Sino devuelve 0
uint64_t syscall_38(uint64_t rbx) {
	return sys_deleteChannel((unsigned int)rbx);
}

// La syscall 39 manda a dormir al proceso actual en el caso que corresponda.
// Si ya habian señales pendientes, retorna 1. En caso contrario, devuelve 0. En caso de error, devuelve -1.
uint64_t syscall_39(uint64_t rbx) {
	return sys_sleep((unsigned int)rbx);
}

// La syscall 40 despierta a los procesos esperando en el canal pasado como argumento por su ID
// Si no habia nadie durmiendo, incrementa el contador de señales. En este caso, retorna 1.
// Si habia alguien durmiendo, lo despierta y retorna 0. En caso de error retorna -1.
uint64_t syscall_40(uint64_t rbx) {
	return sys_wakeup((unsigned int)rbx);
}

// La syscall 41 imprime en pantalla los pids de los procesos bloqueados por el canal indicado
uint64_t syscall_41(uint64_t rbx) {
	sys_printChannelPIDs((unsigned int)rbx);
	return 0;
}

// La syscall 42 crea un pipe para el proceso que la llama y retorna en un vector de dos posiciones los
// respectivos indices de lectura y escritura
uint64_t syscall_42(uint64_t rbx, uint64_t rcx) {
	return sys_pipeOpen((unsigned int)rbx, (int *)rcx);
}

// La syscall 43 cierra para el proceso actual el pipe que se encuentra en el indice indicado por parametro
// En caso de ser el ultimo que lo tenia abierto, se borra el pipe completamente
uint64_t syscall_43(uint64_t rbx) {
	return sys_pipeClose((int)rbx);
}

// La syscall 44 pisa el fd en rbx con el de rcx del proceso actual y actualiza la informacion de los pipes correspondientes
uint64_t syscall_44(uint64_t rbx, uint64_t rcx) {
	return sys_dup2((int)rbx, (int)rcx);
}

// La syscall 45 imprime informacion sobre los pipes actuales
uint64_t syscall_45() {
	sys_listPipes();
	return 0;
}

//	scNumber indica a cual syscall se llamo
//	parameters es una estructura con los parametros para la syscall
//	Cada syscall se encarga de interpretar a la estructura
uint64_t sysCallDispatcher(uint64_t scNumber, Registers reg) {

	switch(scNumber) {
	case 3: return syscall_03(reg->rbx, reg->rcx, reg->rdx);

	case 4: return syscall_04(reg->rbx, reg->rcx, reg->rdx);

	case 7: return syscall_07();

	case 9: return syscall_09(reg->rbx);

	case 10: return syscall_10();

	case 12: return syscall_12(reg->rbx);

	case 13: return syscall_13();

	case 14: return syscall_14();

	case 25: return syscall_25(reg->rbx, reg->rcx);

	case 26: return syscall_26(reg->rbx);

	case 27: return syscall_27(reg->rbx);

	case 28: return syscall_28(reg->rbx);

	case 29: return syscall_29(reg->rbx, reg->rcx, reg->rdx);

	case 30: return syscall_30(reg->rbx, reg->rcx, reg->rdx);

	case 31: return syscall_31();

	case 32: return syscall_32();

	case 33: return syscall_33();

	case 34: return syscall_34(reg->rbx, reg->rcx);

	case 35: return syscall_35();

	case 36: return syscall_36(reg->rbx, reg->rcx);

	case 37: return syscall_37();

	case 38: return syscall_38(reg->rbx);

	case 39: return syscall_39(reg->rbx);

	case 40: return syscall_40(reg->rbx);

	case 41: return syscall_41(reg->rbx);

	case 42: return syscall_42(reg->rbx, reg->rcx);

	case 43: return syscall_43(reg->rbx);

	case 44: return syscall_44(reg->rbx, reg->rcx);

	case 45: return syscall_45();
	}

	return 1;
}
