#ifndef VIDEO_DRIVER_H_
#define VIDEO_DRIVER_H_

#include <stdint.h>
#include <keyboard.h>
#include <font.h>
#include <registers.h>

// Funcion que cambia el color del pixel que se encuentra en la posicion (x,y) a rgb
int sys_writePixel( int x, int y, int rgb );

// Funcion que se encarga de dibujar el caracter c de color rgb en la pantalla en la posicion y con el fondo indicados
int drawChar( char c, int x, int y, int rgb, int backgroundColour );

#endif