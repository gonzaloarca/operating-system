#ifndef VIDEO_DRIVER_H_
#define VIDEO_DRIVER_H_

#include <font.h>
#include <keyboard.h>
#include <registers.h>
#include <stdint.h>

// Funcion que cambia el color del pixel que se encuentra en la posicion (x,y) a rgb
void sys_writePixel(int x, int y, int rgb);

// Funcion que se encarga de dibujar el caracter c de color rgb en la pantalla en la posicion y con el fondo indicados
void drawChar(char c, int x, int y, int rgb, int backgroundColour);

#endif