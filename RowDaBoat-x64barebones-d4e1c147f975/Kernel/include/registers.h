#ifndef _REGISTERS_H
#define _REGISTERS_H

#include <stdint.h>

typedef struct RegistersType {
	uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rbp;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t rsp;
    uint64_t rip;
    uint64_t eflags;
} RegistersType;

// Funcion que se encarga de guardar los valores de los registros en la estructura estatica.
void saveRegisters();

// Funcion de assembler que utiliza saveRegisters para guardar los registros en la estructura reg cuando se preisona F1.
void saveRegistersASMf1(RegistersType *reg);

// Syscall que retorna la estructura con los registros de la ultima vez que se presiono la tecla especial F1.
RegistersType* sys_getRegisters();

#endif
