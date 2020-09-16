#ifndef _COMANDOS_H
#define _COMANDOS_H

#include <stdint.h>
#include <std_io.h>
#include <syscalls.h>

//	Programa que imprime la hora actual
void printTime();

//	Programa que imprime informacion del CPU
void printCPUInfo();

//  Programa que imprime la temperatura actual del CPU
void printCPUTemp();

// Programa que imprime el valor de 4 direcciones de memoria a partir de la indicada por parametro
void printmem(char* address);

// Programa que muestra informacion sobre los posibles usos de la terminal
void help();

// Programa que imprime los valores de los registros que se tenian la ultima vez que se presiono f1
void printInforeg();

//	Indica el fabricante del CPU
char *cpuVendor(char buffer[13]);

//	Indica la marca del CPU
char *cpuBrand(char buffer[48]);

//	Indica el modelo y la familia del CPU
void cpuModel(int buffer[2]);

// Estructura en la que se retorna informacion del cpu
typedef struct{
	char vendor[13];
	char brand[48];
	int family;
	int model;
} CpuInfo;

// Programa para ejecutar excepcion de dividir por cero
void executeZeroException();

// Programa para ejecutar excepcion de instruccion indefinida
void executeUIException();

#endif
