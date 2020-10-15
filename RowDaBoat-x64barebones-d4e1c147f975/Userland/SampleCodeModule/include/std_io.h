#ifndef STD_IO_H_
#define STD_IO_H_

#include <stdint.h>
#include <syscalls.h>
#include <std_num.h>
#include <c_type.h>
#include <stdarg.h>   //para tener cantidad variable de parámetros en funciones como printf y scanf

//devuelve chars casteados a int porque no hacemos uso de caracteres como EOF, ya que no hay un filesystem. Por ende no hay necesidad de operar con enteros en principio
int getchar();

char *gets(char *str);

void * memset(void * destiation, int32_t c, uint64_t length);

int fprintf(int fd, char *format, ...);

int printf(char *format, ...);

int vfprintf(int fd, char *format, va_list arg);

// Funcion que permite escribir un caracter con la opcion de elegir si se desea escribir en stdout o en stderr
int fputc(char c, int fd);

// Funcion que permite la impresion de un string terminado en null en la ventana actual
int fputs(char* str, int fd);

// Funcion que permite escribir un caracter en stdout
int putchar(char c);

// Funcion que permite la impresion de un string terminado en null en la ventana actual (sin imprimir el caracter nulo)
int puts(char * s);

int scanf(char *format, ...);

// Funcion que retorna 0 si str1 es igual o está incluido al principio de str2
// Si no, retorna -1 si str1 < str2 o 1 si str1 > str2
int strcmp(char *str1, char *str2);

// Funcion que retorna la longitud de str
int strlen(char *str);

#endif
