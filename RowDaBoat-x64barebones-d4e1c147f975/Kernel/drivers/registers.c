#include <registers.h>
#include <window_manager.h>

// Respaldo de registros de la ultima vez que se presiono F1, si no se presiono nunca sus valores son cero.
static RegistersType reg;

//	Funcion de ASM
void saveRegistersASM(RegistersType *reg);

void saveRegisters(){
	saveRegistersASM(&reg);
}

RegistersType *sys_getRegisters(){
	return &reg;
}
