#ifndef STD_IO_H_
#define STD_IO_H_

#include <c_type.h>
#include <std_num.h>
#include <stdarg.h> //para tener cantidad variable de parámetros en funciones como printf y scanf
#include <stdint.h>
#include <syscalls.h>

// Funcion analoga al getchar de stdio.h
int getchar();

// Funcion analoga a gets de stdio.h
char *gets(char *str);

void *memset(void *destiation, int32_t c, uint64_t length);

// Funcion que dado un string de formateo y una serie de argumentos imprime
// a pantalla. Cumple las mismas funciones que el printf del verdadero stdio.h
int printf(char *format, ...);

// Funcion que cumple la misma funcion que el printf normal pero imprime a un
// filedescriptor en especifico
int fprintf(int fd, char *format, ...);

// Funcion que permite escribir un caracter con la opcion de elegir el fd
int fputc(char c, int fd);

// Funcion que permite la impresion de un string terminado en null en el fd indicado
int fputs(char *str, int fd);

// Funcion que permite escribir un caracter en stdout
int putchar(char c);

// Funcion que permite la impresion de un string terminado en null en stdout
int puts(char *s);

// El famoso scanf, funciona de forma analoga a la implementacion de la verdadera stdio.h
int scanf(char *format, ...);

// Funcion que retorna 0 si str1 es igual a str2
// Si no, retorna -1 si str1 < str2 o 1 si str1 > str2
int strcmp(char *str1, char *str2);

//  Funcion booleana que retorna 1 si pre es prefijo de str
//  Si no, retorna 0
int strincl(char *pre, char *str);

//  Copia str1 en str2, ambos null-terminated
//  Devuelve la cantidad de chars copiados
// En caso de error devuelve -1
int strcopy(const char *str1, char *str2);

// Funcion que retorna la longitud de str
// En caso de error retorna -1
int strlen(char *str);

#endif
