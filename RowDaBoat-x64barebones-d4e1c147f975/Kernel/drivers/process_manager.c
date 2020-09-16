#include <process_manager.h>
#include <window_manager.h>
#include <screenInfo.h>

#define MB	0x100000
#define PROGRAM_MEMORY (MB/10)

//	Estructura que representa a los modulos (programas) "activos" 
typedef struct 
{
	void (*program)();					//puntero al inicio del programa
	unsigned int pid;					//process ID del programa
	RegistersType backup;				//Backup de los registros
} Module;

static void newStackFrame(Module *module);
static void restartModule();

//	Lista de modulos/programas
static Module modules[N];
static int windowMap[N] = {0}; //mapa que recibe el pid de un programa y devuelve el indice de la ventana que le corresponde
static unsigned int activeModule = 0;	
static unsigned int numberOfModules = 0;

//	Reservo espacio para los stack frames
static char reserve[N][PROGRAM_MEMORY];

int sys_initModule(void (*program)())
{
	//	Si ya no tengo mas ventanas, no puedo agregar nada
	if (numberOfModules >= N)
		return 1;

	//	Busco la direccion del nuevo modulo en la lista
	Module *newModule = &(modules[numberOfModules]);

	//	Le asigno al modulo el puntero al programa recibido
	newModule->program = program;

	//	Le asigno el numero de process ID segun la cantidad de procesos corriendo hasta el momento
	newModule->pid = numberOfModules;

	//	Se le asigna al mapa de pid-window la ventana que ocupara el programa (tomamos la funcion identidad por convencion)
	windowMap[newModule->pid] = newModule->pid;

	newStackFrame(newModule);

	numberOfModules++;

	return 0;
}

int switchProcess(){
	if( numberOfModules <= 1 ){
		return 1;
	}
	int nextPid = (activeModule + 1) % numberOfModules;
	//	Llamo a la syscall para cambiar de ventana graficamente
	int success = sys_changeWindow( windowMap[nextPid] );
	if(success)
	{
		//	Tengo que guardar el stack frame actual
		getBackupINT(&(modules[activeModule].backup));
		//	Cambio el modulo activado
		activeModule = nextPid;
		//	Seteo el stack frame
		setBackupINT(&(modules[activeModule].backup));
	}
	return 0;
}

void sys_runFirstProcess(){
	if (numberOfModules == 0)
		return;

	else
	{
		//	Empieza el primer programa
		startRunning(modules[activeModule].backup.rip, modules[activeModule].backup.rsp);
	}
}

static void newStackFrame(Module *module)
{
	//	Voy a asignarle espacio para el stack frame:
	// 	Tengo que moverme al final de la memoria guardada ya que
	//	el stack comienza en direcciones alta y va disminuyendo
	uint64_t *last_address = (uint64_t*) (reserve[module->pid + 1] - 8);

	//	Seteo el programa de reseteo (en caso de terminar de ejecutar el modulo)
	*last_address = (uint64_t) restartModule;

	//	Pongo la entrada a mi programa
	module->backup.rip = (uint64_t) module->program;

	//	rsp al inicio del stack frame
	module->backup.rsp = (uint64_t) last_address;
}

void recoverModule()
{
	// Obtengo la direccion al modulo actual
	Module *module = &(modules[activeModule]);

	newStackFrame(module);

	//	El programa de antes de la interrupcion se reinicia
	startRunningEXC(module->backup.rip, module->backup.rsp);
}

static void restartModule()
{
	// Obtengo la direccion al modulo actual
	Module *module = &(modules[activeModule]);

	newStackFrame(module);

	restart(module->backup.rip, module->backup.rsp);
}
