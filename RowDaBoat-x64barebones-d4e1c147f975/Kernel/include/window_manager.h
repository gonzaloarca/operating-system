#ifndef _WINDOW_MANAGER_H
#define _WINDOW_MANAGER_H

#include <registers.h>
#include <screenInfo.h>
#include <video_driver.h>
#include <stdint.h>

// Estructura que utiliza el buffer de la ventana para saber el caracter y color de cierta posicion
typedef struct{

	char character;
	int color;

}charWithColor;

// Estructura que maneja el comportamiento visual de una ventana en la pantalla
typedef struct{

	// Donde arranca el canvas de la ventana
	int xStart;
	int yStart;

	// Buffer de la pantalla para permitir scrolling
	charWithColor screenBuffer[BUFFER_LINES][MAX_LINE_CHARS]; // necesito una linea mas para tener la anterior

	// Cantidad de caracteres en la linea actual
	int currentLineSize;

	// Cantidad de lineas escritas actualmente
	int lineCount;

	// Linea en la cual arranca la pantalla
	int firstLine;

	// Color a utilizar para imprimir los nuevos caracteres
	int charColor;

	// Si el simbolo de espera de escritura fue escrito se encuentra en 1, caso contrario 0
	int flagIdle;

} Window;

// Funcion de booteo, establece parametros default de cada ventana a mostrar en la pantalla
void setWindows();

// Funcion para cambiar la ventana activa
int sys_changeWindow(unsigned int newIndex);

// Funcion que permite el cambio del color de los caracteres a escribir en la ventana actual
int sys_changeWindowColor(int rgb);

// Funcion que permite la impresion de un caracter en la ventana actual
int sys_write(unsigned int fd, const char * str, unsigned long count);

//	Syscall que limpia la pantalla activa actualmente
void sys_clrScreen();

// Funcion que imprime los registros pasados por parametro
void printRegisters(RegistersType *reg);

// Funcion que muestra la espera de escritura en pantalla
void idleSymbol();

#endif