#include <std_io.h>

#define STDIN_MAXBUFFER 4096
#define NULL ((void *)0)

#define DEFAULT_CHAR_COLOR 0xFFFFFF

static char stdinBuffer[STDIN_MAXBUFFER];
static int stdinFirstPos = 0;
static int stdinLastPos = 0;

//Funcion necesaria para el funcionamiento de printf
static int vfprintf(int fd, char *format, va_list arg);

int getchar() {
	if(stdinLastPos == stdinFirstPos) { //hay que ver que pasa si stdinLastPos > STDIN_BUFFERSIZE
		stdinFirstPos = 0;
		stdinLastPos = read(0, stdinBuffer, STDIN_MAXBUFFER);
	}

	return stdinBuffer[stdinFirstPos++];
}

int fputc(char c, int fd) {
	if(fd != 1 && fd != 2) { //solo tenemos implementados stdout y stderr
		return -1;
	}

	return write(fd, &c, 1);
}

int putchar(char c) {
	return fputc(c, 1);
}

int strcmp(char *str1, char *str2) {
	if(str1 == NULL || str2 == NULL)
		return -1;

	while(*str1 == *str2) {
		if(*str1 == 0)
			return 0;
		str1++;
		str2++;
	}

	return (*str1 > *str2) ? 1 : -1;
}

int strincl(char *pre, char *str) {
	if(pre == NULL || str == NULL)
		return 0;

	while(*pre != 0 && *pre == *str) {
		pre++;
		str++;
	}

	if(*pre == 0)
		return 1;
	else
		return 0;
}

int strcopy(const char *str1, char *str2) {
	if(str1 == NULL || str2 == NULL)
		return -1;

	int i = 0;
	while(str1[i] != 0) {
		str2[i] = str1[i];
		i++;
	}
	str2[i] = 0;

	return i;
}

int strlen(char *str) {
	if(str == NULL)
		return -1;

	int i = 0;
	for(; str[i] != '\0'; i++)
		;
	return i;
}

int fputs(char *str, int fd) {
	if(str == NULL || fd < 0)
		return -1;

	return write(fd, str, strlen(str));
}

int puts(char *str) {
	return fputs(str, 1);
}

void *memset(void *destiation, int32_t c, uint64_t length) {
	uint8_t chr = (uint8_t)c;
	char *dst = (char *)destiation;

	while(length--)
		dst[length] = chr;

	return destiation;
}

static int vfprintf(int fd, char *format, va_list arg) {
	if(format == NULL || fd < 0)
		return -1;

	int i = 0;
	int count = 0;

	char numAux[32];

	while(format[i]) {
		if(format[i] != '%') {
			if(format[i] != '\\') {
				fputc(format[i++], fd);
				count++;
			} else {
				i++;
				switch(format[i]) { //puede llegar a ser un caracter especial, como un enter o un tab
				case 'n':
					fputc('\n', fd);
					i++;
					count++;
					break;
				case 't':
					fputc('\t', fd);
					i++;
					count++;
					break;
				default:
					fputc('\\', fd);
					count++;
					break;
				}
			}

		} else { // Encontre un %
			i++;
			switch(format[i]) {
			case 's':
				fputs(va_arg(arg, char *), fd);
				i++;
				break;
			case 'p': //puntero
				count += pointerToHexString(va_arg(arg, void *), numAux);
				fputs(numAux, fd);
				i++;
				break;
			case 'x':
				count += intToHexString((unsigned int)va_arg(arg, unsigned int), numAux);
				fputs(numAux, fd);
				i++;
				break;
			case 'd':
				count += intToString(va_arg(arg, int), numAux);
				fputs(numAux, fd);
				i++;
				break;
			case 'u':
				count += intToString((unsigned int)va_arg(arg, unsigned int), numAux);
				fputs(numAux, fd);
				i++;
				break;
			case 'c':
				fputc((char)va_arg(arg, int), fd); //str_arg.h promueve los chars a int
				i++;
				count++;
				break;
			case 'f': //al usar std_arg.h, cuando obtengo va_arg( arg, float ), el float se promueve a double
			case 'g': //son el mismo caso por lo tanto
				count += floatToString(va_arg(arg, double), numAux);
				fputs(numAux, fd);
				i++;
				break;
			case 'l':
				if(format[i + 1] == 'u') { //unsigned long
					count += intToString((unsigned long)va_arg(arg, unsigned long), numAux);
					fputs(numAux, fd);
					i += 2;
					break;
				}
				if(format[i + 1] == 'd') { //long
					count += intToString((long)va_arg(arg, long), numAux);
					fputs(numAux, fd);
					i += 2;
					break;
				}
				if(format[i + 1] == 'x') { //unsigned long en hexa
					count += intToHexString((unsigned long)va_arg(arg, unsigned long), numAux);
					fputs(numAux, fd);
					i += 2;
					break;
				}
				//si despues de la l no habia nada de interes, debo printear el % y lo que siga, lo cual cae en el caso default
			default:
				fputc('%', fd);
				if(format[i] == '%') { //si me pasan %%, puedo ahorrarme las comparaciones que siguen si hago que directamente el siguiente % ni entre al switch salteandolo
					i++;
				}
				count++;
			}
		}
	}

	return count;
}

int fprintf(int fd, char *format, ...) {
	va_list arg;
	va_start(arg, format);

	int ret = vfprintf(fd, format, arg);

	va_end(arg);

	return ret;
}

int printf(char *format, ...) {
	va_list arg;
	va_start(arg, format);

	int ret = vfprintf(1, format, arg);

	va_end(arg);

	return ret;
}

int scanf(char *format, ...) {
	va_list args;
	va_start(args, format);

	int argsRead = 0, i = 0;
	int c = 0;
	int *argD;
	long *argLD;
	float *argF;
	double *argG;
	unsigned *argU;
	unsigned long *argLU;
	int neg = 0;
	float expF;
	double expG;
	int comma;
	char *argC;
	char *argS;

	c = getchar();
	while(format[i] != 0) { //guarda, puede ser que haya que hacer un do-while, porque hago getchar al final del ciclo, y otra vez al principio, y capaz que me coma el enter asi --> validar enter al final del ciclo si no?
		while(c == '\n') {
			c = getchar();
		}

		if(format[i] != '%' && format[i] != c) {
			va_end(args);
			return argsRead; //a la primera que no coincide el formato con la entrada, salgo de la funcion.
		}
		if(format[i] == '%') {
			i++;
			if(format[i] == '*') { //debo ignorar el campo, siempre y cuando lo que siga sea un caracter de formato valido
				i++;
				switch(format[i]) {
				case 'd':
				case 'u':
				case 'l':
					if(format[i] == 'l' && format[i + 1] != 'd' && format[i + 1] != 'u') {
						va_end(args);
						return argsRead; //no hay especificadores de formato distintos de lu y ld que empiecen con l
					}

					if(!isdigit(c) && c != '-' && c != '+') {
						va_end(args);
						return argsRead;
					}

					if(c == '-') {
						c = getchar(); //para que pase al siguiente digito
						if(!isdigit(c)) {
							va_end(args);
							return argsRead; //me pasaron "-"; no era un numero, debo salir
						}
					}

					if(c == '+') {
						c = getchar();
						if(!isdigit(c)) {
							va_end(args);
							return argsRead; //me pasaron "+" sin estar seguido de ningun digito, debo salir.
						}
					}

					while(isdigit(c = getchar()))
						; //salteo los caracteres numericos hasta toparme con algo que no sea un numero

					i++;
					break;

				case 's':
					while((c = getchar()) != ' ' && c != '\n')
						; // salteo caracteres hasta toparme con un enter o un espacio//guarda que estas obteniendo el enter aca si es que llego
					i++;
					break;

				case 'f':
				case 'g':
					if(!isdigit(c) && c != '.' && c != '-' && c != '+') {
						va_end(args);
						return argsRead;
					}
					comma = 0;

					if(c == '-') {
						c = getchar();
						if(!isdigit(c) && c != '.') {
							va_end(args);
							return argsRead; //caso en que me pasen "." como argumento
						}
					}

					if(c == '+') {
						c = getchar();
						if(!isdigit(c) && c != '.') {
							va_end(args);
							return argsRead; //caso en que me pasen "+" como argumento
						}
					}

					if(c == '.') {
						c = getchar();
						if(!isdigit(c)) {
							va_end(args);
							return argsRead; //caso en que me pasen "." como argumento
						}
						comma = 1;
					}

					do {
						if(c == '.') {
							if(comma != 0) { //no pueden pasarme 2 puntos en un float
								break;
							}
							comma = 1;
						}
					} while(isdigit(c = getchar()) || c == '.');

					i++;
					break;

				case 'c':
					c = getchar();
					i++;
					break;
				}
			} else {
				switch(format[i]) {
				case 'd':
				case 'u':
					if(!isdigit(c) && c != '-' && c != '+') {
						va_end(args);
						return argsRead;
					}
					neg = 0;
					if(c == '-') {
						c = getchar(); //para que pase al siguiente digito
						if(!isdigit(c)) {
							va_end(args);
							return argsRead; //me pasaron "-"; no era un numero, debo salir
						}
						neg = 1;
					}

					if(c == '+') {
						c = getchar();
						if(!isdigit(c)) {
							va_end(args);
							return argsRead; //me pasaron "+" sin estar seguido de ningun digito, debo salir.
						}
					}

					if(format[i] == 'd') {
						argD = va_arg(args, int *);
						*argD = 0;
						do {
							*argD = *argD * 10 + (c - '0');
						} while(isdigit(c = getchar())); //debe cortar una vez que el caracter leido no sea mas un digito

						if(neg) {
							*argD *= -1;
						}
					} else { //format[i] == 'u'
						argU = va_arg(args, unsigned int *);
						*argU = 0;
						do {
							*argU = *argU * 10 + (c - '0');
						} while(isdigit(c = getchar())); //debe cortar una vez que el caracter leido no sea mas un digito

						if(neg) {
							*argU *= -1;
						}
					}
					i++;
					argsRead++;
					break;

				case 'c':
					argC = va_arg(args, char *);
					*argC = c;
					c = getchar();
					i++;
					argsRead++;
					break; //aca evito hacer otro getchar

				case 's':
					argS = va_arg(args, char *);
					char *aux = argS;
					do {
						*(aux++) = c;
					} while((c = getchar()) != ' ' && c != '\n'); //guarda que estas obteniendo el enter aca si es que llego
					*aux = 0;
					i++;
					argsRead++;
					break;

				case 'f':
				case 'g':
					if(!isdigit(c) && c != '.' && c != '-' && c != '+') {
						va_end(args);
						return argsRead;
					}
					comma = 0;
					neg = 0;

					if(c == '-') {
						c = getchar();
						if(!isdigit(c) && c != '.') {
							va_end(args);
							return argsRead; //caso en que me pasen "." como argumento
						}
						neg = 1;
					}

					if(c == '+') {
						c = getchar();
						if(!isdigit(c) && c != '.') {
							va_end(args);
							return argsRead; //caso en que me pasen "+" como argumento
						}
					}

					if(c == '.') {
						c = getchar();
						if(!isdigit(c)) {
							va_end(args);
							return argsRead; //caso en que me pasen "." como argumento
						}
						comma = 1;
					}

					if(format[i] == 'f') {
						argF = va_arg(args, float *);
						*argF = 0;
						expF = 1.0;

						do {
							if(c == '.') {
								if(comma != 0) { //no pueden pasarme 2 puntos en un float
									break;
								}
								comma = 1;
							} else if(!comma) {
								*argF = *argF * 10 + (c - '0');
							} else {
								expF /= 10;
								*argF = *argF + ((c - '0') * expF);
							}
						} while(isdigit(c = getchar()) || c == '.');

						if(neg) {
							*argF *= -1;
						}

					} else { //format[i] == 'g'
						argG = va_arg(args, double *);
						*argG = 0;
						expG = 1.0;

						do {
							if(c == '.') {
								if(comma != 0) { //no pueden pasarme 2 puntos en un float
									break;
								}
								comma = 1;
							} else if(comma == 0) {
								*argG = *argG * 10 + (c - '0');
							} else {
								expG /= 10;
								*argG = *argG + ((c - '0') * expG);
							}
						} while(isdigit(c = getchar()) || c == '.');

						if(neg) {
							*argG *= -1;
						}
					}

					i++;
					argsRead++;
					break;

				case 'l':
					i++;
					if(format[i] == 'd' || format[i] == 'u') {
						if(!isdigit(c) && c != '-' && c != '+') {
							va_end(args);
							return argsRead;
						}

						neg = 0;
						if(c == '-') {
							c = getchar(); //para que pase al siguiente digito
							if(!isdigit(c)) {
								va_end(args);
								return argsRead; //me pasaron "-"; no era un numero, debo salir
							}
							neg = 1;
						}

						if(c == '+') {
							c = getchar();
							if(!isdigit(c)) {
								va_end(args);
								return argsRead; //me pasaron "+" sin estar seguido de ningun digito, debo salir.
							}
						}
						if(format[i] == 'd') {
							argLD = va_arg(args, long *);
							*argLD = 0;
							do {
								*argLD = *argLD * 10 + (c - '0');
							} while(isdigit(c = getchar())); //debe cortar una vez que el caracter leido no sea mas un digito

							if(neg) {
								*argLD *= -1;
							}
						} else { //format[i] == 'u'
							argLU = va_arg(args, unsigned long *);
							*argLU = 0;
							do {
								*argLU = *argLU * 10 + (c - '0');
							} while(isdigit(c = getchar())); //debe cortar una vez que el caracter leido no sea mas un digito

							if(neg) {
								*argLU *= -1;
							}
						}
						i++;
						argsRead++;
						break;
					} else {
						va_end(args);
						return argsRead; //si no era un especificador de formato valido, corto el scanf.
					}

				default:					 //si caigo aca, no habia ningun caracter valido despues del % --> a no ser que dicho caracter sea un %, corto la funcion porque no existe nada con ese formato
					if(format[i] == '%' && c == format[i]) { //si me topo con un %%, entonces quiere decir que la entrada debe tener un % explicito. No quiero que lo tome como delimitador de formato para la proxima iteracion, por ende incremento i
						i++;
						break;
					}

					va_end(args);
					return argsRead;
				}
			}
		} else {
			c = getchar();
			i++;
		}
	}

	va_end(args);
	return argsRead;
}
