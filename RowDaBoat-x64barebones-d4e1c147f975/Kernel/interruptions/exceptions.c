#include <process_manager.h>
#include <registers.h>
#include <window_manager.h>

#define ZERO_EXCEPTION_ID 0
#define UNDEFINEDINSTRUCTION_EXCEPTION_ID 6

// Funcion de assembler que utiliza saveRegisters para guardar los registros en la estructura reg cuando se lanza una excepcion sin error code.
void saveRegistersASMexcp(RegistersType *reg);

static RegistersType reg; // Variable auxiliar para imprimir registros una vez lanzada una excepcion

static void divideByZeroException(){
	saveRegistersASMexcp(&reg);
	sys_write(2, "DIVIDE BY ZERO EXCEPTION\n", 25);
	printRegisters(&reg);
	recoverModule();
	return;
}

static void undefinedInstructionException(){
	saveRegistersASMexcp(&reg);
	sys_write(2, "UNDEFINED INSTRUCTION EXCEPTION\n", 32);
	printRegisters(&reg);
	recoverModule();
	return;
}

static void defaultException(){
	saveRegistersASMexcp(&reg);
	sys_write(2, "UNKNOWN EXCEPTION\n", 18);
	printRegisters(&reg);
	recoverModule();
	return;
}

// excpNumber indica la excepcion a la que se llamo
// Este dispatcher se encargara de ejecutar la rutina de tratamiento correspondiente a la excepcion
void exceptionDispatcher(int excpNumber) {
	switch(excpNumber)
	{
		case ZERO_EXCEPTION_ID:
			divideByZeroException();
			break;
		case UNDEFINEDINSTRUCTION_EXCEPTION_ID:
			undefinedInstructionException();
			break;

		default:
			defaultException();			// Para que ante una excepcion que no se tiene una rutina especifica no se cuelge el sistema
	}
}
