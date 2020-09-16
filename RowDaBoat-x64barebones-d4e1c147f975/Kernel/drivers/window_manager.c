#include <window_manager.h>

// Funcion interna que se encarga de escribir un char en pantalla
static int printChar( char c, int rgb );

// Funcion interna que se encarga de borrar el simbolo de espera de escritura en caso de estar presente cuando se cambia de ventana
static void blockIdleSymbol();

// Funcion interna que se encarga de borrar el simbolo de espera de escritura en caso de estar presente sin validaciones
static void deleteIdleSymbol();

static Window windows[N] = {{0}};
static int activeWindow = 0;		// por default arranca en la terminal

void setWindows(){
	for(int i = 0 ; i < N ; i++){
		windows[i].xStart = i*WINDOW_WIDTH + WINDOW_MARGIN;
		windows[i].yStart = WINDOW_MARGIN;
		windows[i].firstLine = SCREEN_LINES-1;			// queremos que arranque arranque a escribir en la pantalla desde la ultima linea
		windows[i].lineCount = SCREEN_LINES-1;
		windows[i].currentLineSize = 0;
		windows[i].charColor = CHAR_COLOR;
		windows[i].flagIdle = 0;
		// Borde izquierdo
		for(int j = i*WINDOW_WIDTH; j < i*WINDOW_WIDTH+WINDOW_MARGIN ; j++)
			for(int k = WINDOW_MARGIN; k < WINDOW_HEIGHT - WINDOW_MARGIN; k++)		// no abarca nada del borde superior
				sys_writePixel( j, k, WINDOW_MARGIN_COLOUR);
		// Borde derecho
		for(int j = (i+1)*WINDOW_WIDTH- WINDOW_MARGIN; j < (i+1)*WINDOW_WIDTH ; j++)
			for(int k = WINDOW_MARGIN; k < WINDOW_HEIGHT - WINDOW_MARGIN; k++)
				sys_writePixel( j, k, WINDOW_MARGIN_COLOUR);
		// Borde superior
		for(int j = 0 ; j < WINDOW_MARGIN ; j++)
			for(int k = i*WINDOW_WIDTH; k < (i+1)*WINDOW_WIDTH ; k++)
				sys_writePixel( k, j, WINDOW_MARGIN_COLOUR);
		// Borde inferior
		for(int j = TOTAL_HEIGHT - WINDOW_MARGIN ; j < TOTAL_HEIGHT ; j++)
			for(int k = i*WINDOW_WIDTH; k < (i+1)*WINDOW_WIDTH ; k++)
				sys_writePixel( k, j, WINDOW_MARGIN_COLOUR);
	}
}

int sys_changeWindow(unsigned int newIndex){
	if(newIndex < 0 || newIndex >= N || newIndex == activeWindow)
		return 0;			// permanece en la ventana actual
	else{
		blockIdleSymbol();	// para asegurarme que no deje basura
		activeWindow = newIndex;
		return 1;
	}
}

int sys_changeWindowColor(int rgb){
	if(rgb < 0 || rgb > 0xFFFFFF )
		return 1;	// no indica un color
	else{
		windows[activeWindow].charColor = rgb;
		return 0;
	}
}

//  Funcion de uso interno que realiza la actualizacion de las lineas del buffer de la ventana activa, 
// si se llego al limite de lineas mostrada se encarga de realizar el scrolling del buffer
static void updateBuffer(){
	
	Window *currentWindow = &(windows[activeWindow]);

	currentWindow->lineCount++;						// Se agrego una linea en pantalla
	currentWindow->currentLineSize = 0;

	if( currentWindow->lineCount == SCREEN_LINES ){
		// Ahora la pantalla va a arrancar en una posicion mas alta en el buffer
		currentWindow->firstLine = (currentWindow->firstLine + 1) % BUFFER_LINES;
		currentWindow->lineCount--;
		int lastLine = (currentWindow->firstLine + SCREEN_LINES -1) % BUFFER_LINES; // Linea que ahora estara alfinal de la pantalla vacia

		// Solo borro caracteres que quedaron pendientes, no toda la linea
		for(int i = 0; i < MAX_LINE_CHARS && currentWindow->screenBuffer[lastLine][i].character != 0 ; i++ ){
			currentWindow->screenBuffer[lastLine][i].character = 0;
		}
	}
}

//  Funcion de uso interno que realiza el refresco de cada linea de la ventana actual, 
// modificando caracter por caracter, en caso de ser necesario, 
// teniendo en cuenta lo que se tenia escrito en pantalla(linea vieja) y lo que se esta por escribir(linea nueva)
static void refreshLine( int lineNumber ){
	// recordar que el updateBuffer me movio el firstLine una posicion

	Window *currentWindow = &(windows[activeWindow]);

	int i = 0;																					// indice para iterar sobre la linea
	int currentLineIndex = (currentWindow->firstLine + lineNumber) % BUFFER_LINES;		// indice de la linea en el buffer que debe ser impresa en lineNumber
	int previousLineIndex;																		// indice de la linea en el buffer que se encuentra antes del ciclo escrita en pantalla

	if( currentWindow->firstLine == 0 && lineNumber == 0 )
		previousLineIndex = BUFFER_LINES -1;								
	else
		previousLineIndex = (currentWindow->firstLine + lineNumber -1) % BUFFER_LINES;		// Ya me asegure que el indice sera positivo

	charWithColor *currentLine = currentWindow->screenBuffer[currentLineIndex];
	charWithColor *previousLine = currentWindow->screenBuffer[previousLineIndex];

	// En este primer ciclo sobreescribire los chars que no comparten la linea que se encuentra en pantalla y la que se debe escribir
	// Finaliza una vez terminada la linea o una de las lineas
	for( ; i < MAX_LINE_CHARS && previousLine[i].character != 0 && currentLine[i].character != 0 ; i++ )
	{
		if(previousLine[i].character != currentLine[i].character || previousLine[i].color != currentLine[i].color){
			drawChar(currentLine[i].character, currentWindow->xStart + i * FONT_WIDTH, 
				currentWindow->yStart + lineNumber * (LINE_HEIGHT) + LINE_MARGIN, currentLine[i].color, BACKGROUND_COLOR);
		}
	}

	// En caso de que la nueva linea sea mas corta que la anterior, se deben borrar en pantalla las letras que quedaron
	while( previousLine[i].character != 0 && i < MAX_LINE_CHARS) {
		drawChar(0, currentWindow->xStart + i * FONT_WIDTH, 
			currentWindow->yStart + lineNumber * (LINE_HEIGHT) + LINE_MARGIN, CHAR_COLOR, BACKGROUND_COLOR);
		i++;
	}

	// En caso de que la nueva linea sea mas larga que la anterior, se deben agregar en pantalla estas letras sobrantes
	while( currentLine[i].character != 0 && i < MAX_LINE_CHARS ) {
		drawChar(currentLine[i].character, currentWindow->xStart + i * FONT_WIDTH, 
			currentWindow->yStart + lineNumber * (LINE_HEIGHT) + LINE_MARGIN, currentLine[i].color, BACKGROUND_COLOR);
		i++;
	}
}

// Funcion interna que realiza el refresco de todas las lineas en pantalla al llegarse al maximo de lineas
static void refreshScreen() {
	for(int i = 0 ; i < SCREEN_LINES; i++) {
		refreshLine(i);
	}
}

// Funcion interna que se encarga de eliminar un caracter en la ventana actual
static void deleteChar(){
	Window *currentWindow = &(windows[activeWindow]);
	if(currentWindow->currentLineSize == 0){
		if(currentWindow->lineCount == 0)
			// No existen caracteres por borrar
			return;

		blockIdleSymbol();	// para que no quede basura, lo tengo que hacer antes del cambio de linea
		currentWindow->lineCount--;
		currentWindow->currentLineSize = MAX_LINE_CHARS;
	}else
		blockIdleSymbol();	// para que no quede basura
	
	currentWindow->currentLineSize--;	// lo borro del buffer

	int currentLine = (currentWindow->firstLine + currentWindow->lineCount) % BUFFER_LINES;
	int currentChar = currentWindow->currentLineSize;
	charWithColor aux = {0,0};
	currentWindow->screenBuffer[currentLine][currentChar] = aux;	// Borro el caracter en el buffer
	drawChar(' ', currentWindow->xStart + currentWindow->currentLineSize * FONT_WIDTH, currentWindow->yStart + currentWindow->lineCount * LINE_HEIGHT + LINE_MARGIN, BACKGROUND_COLOR, BACKGROUND_COLOR);
}

// Funcion interna que inicia una nueva linea
static void setNewLine(){
	blockIdleSymbol();	// para que no quede basura
	if(windows[activeWindow].lineCount == (SCREEN_LINES -1)) {
		// Se llego alfinal de las lineas en pantalla, se debe subir una linea para que la ultima quede libre
		updateBuffer();
		refreshScreen();
	}else
		updateBuffer();
}


// Funcion interna que se encarga de la impresion de un caracter
static int printChar( char c, int rgb ){
	if ( c == '\n'){
		setNewLine();
		return 0;
	}else if( c == '\b'){
		deleteChar();
		return 0;
	}else if( c == '\t' ){
		for( int i = 0; i < 4; i++ ){	// tab = 4 espacios
			printChar( 32, rgb );
		}
		return 0;
	}

	Window *currentWindow = &(windows[activeWindow]);

	if( currentWindow->currentLineSize == MAX_LINE_CHARS ){
		// Se llego al tope de caracteres que entran en una linea, se debe pasar a la siguiente
		setNewLine();
	}

	// Escribo en pantalla el nuevo caracter en la linea y posicion actual, y luego incremento la posicion para el proximo caracter
	drawChar( c, currentWindow->xStart + currentWindow->currentLineSize * FONT_WIDTH, 
		currentWindow->yStart + currentWindow->lineCount * (LINE_HEIGHT) + LINE_MARGIN, rgb, BACKGROUND_COLOR);
	
	// Cambio el caracter en el buffer
	//*(windows[activeWindow].screenBuffer[ (windows[activeWindow].firstLine + windows[activeWindow].lineCount) % BUFFER_LINES ] +
	//windows[activeWindow].currentLineSize) = c;

	int currentLine = (currentWindow->firstLine + currentWindow->lineCount) % BUFFER_LINES;
	int currentChar = currentWindow->currentLineSize;

	charWithColor newChar = {c, rgb};
	currentWindow->screenBuffer[currentLine][currentChar] = newChar;
	
	// Incremento contador de caracteres en linea
	currentWindow->currentLineSize++;

	return 0;
}

int sys_write(unsigned int fd, const char * str, unsigned long count){
	
	int color = windows[activeWindow].charColor;

	if (fd == 2)	//	STDERR
		color = 0xFF0000;
	else if (fd != 1)	//	Solo tenemos STDOUT y STDERR
		return 0;

	for( int i = 0; i < count; i++ ){
		printChar( str[i], color );
	}

	return count;
}

// Funcion de uso interno para obtener la cantidad de caracteres de un long
static int intLength(long num){
	int dig = 0;
	while( num != 0 ){ //cuento digitos para saber desde donde arrancar a meter los caracteres
        num /= 10;
        dig++;
    }

    return dig;
}

// Funcion que transforma un int en un String, retorna la longitud
static int uint64_tToString( uint64_t num, char * str ){

    if( num == 0 ){
        str[0] = '0';
        str[1] = 0;
        return 1;
    }

    int dig = intLength(num);
    
    if( num < 0 ){
        dig++;
        str[0] = '-';
        num *= -1;      //porque num % n da negativo si num < 0
    }

    int i = dig;
    str[i--] = 0;

    while( num != 0 ){
        str[i--] = (num % 10) + '0';
        num /= 10;
    }

    return dig;
}

void sys_clrScreen()
{
	Window *currentWindow = &(windows[activeWindow]);

	setNewLine();

	for (int i = 0; i < BUFFER_LINES -1 ; i++)
	{
		for (int j = 0; j < MAX_LINE_CHARS && currentWindow->screenBuffer[(currentWindow->firstLine + i) % BUFFER_LINES][j].character != 0 ; j++)
		{
			//	Vacío la parte del buffer que aparece en pantalla(que es la que tiene contenido, la otra linea se encuentra en cero) y la pantalla
			currentWindow->screenBuffer[(currentWindow->firstLine + i) % BUFFER_LINES][j].character = 0;
			drawChar(' ', currentWindow->xStart + j * FONT_WIDTH,
					currentWindow->yStart + i * LINE_HEIGHT + LINE_MARGIN,
					CHAR_COLOR, BACKGROUND_COLOR);
		}
	}
}

void printRegisters(RegistersType *reg){
	char aux[10];	// maxima longitud de un longint
	int longitud;
	
	longitud = uint64_tToString(reg->rax, aux);
	sys_write(2, "RAX: ", 5);
	sys_write(2, aux, longitud);

	longitud = uint64_tToString(reg->rbx, aux);
	sys_write(2, "    RBX: ", 9);
	sys_write(2, aux, longitud);

	longitud = uint64_tToString(reg->rcx, aux);
	sys_write(2, "    RCX: ", 9);
	sys_write(2, aux, longitud);
	sys_write(2, "\n", 1);

	longitud = uint64_tToString(reg->rdx, aux);
	sys_write(2, "RDX: ", 5);
	sys_write(2, aux, longitud);

	longitud = uint64_tToString(reg->rbp, aux);
	sys_write(2, "    RBP: ", 9);
	sys_write(2, aux, longitud);

	longitud = uint64_tToString(reg->rdi, aux);
	sys_write(2, "    RDI: ", 9);
	sys_write(2, aux, longitud);
	sys_write(2, "\n", 1);

	longitud = uint64_tToString(reg->rsi, aux);
	sys_write(2, "RSI: ", 5);
	sys_write(2, aux, longitud);

	longitud = uint64_tToString(reg->r8, aux);
	sys_write(2, "    R8: ", 8);
	sys_write(2, aux, longitud);

	longitud = uint64_tToString(reg->r9, aux);
	sys_write(2, "    R9: ", 8);
	sys_write(2, aux, longitud);
	sys_write(2, "\n", 1);

	longitud = uint64_tToString(reg->r10, aux);
	sys_write(2, "R10: ", 5);
	sys_write(2, aux, longitud);

	longitud = uint64_tToString(reg->r11, aux);
	sys_write(2, "    R11: ", 9);
	sys_write(2, aux, longitud);

	longitud = uint64_tToString(reg->r12, aux);
	sys_write(2, "    R12: ", 9);
	sys_write(2, aux, longitud);
	sys_write(2, "\n", 1);

	longitud = uint64_tToString(reg->r13, aux);
	sys_write(2, "R13: ", 5);
	sys_write(2, aux, longitud);

	longitud = uint64_tToString(reg->r14, aux);
	sys_write(2, "    R14: ", 9);
	sys_write(2, aux, longitud);

	longitud = uint64_tToString(reg->r15, aux);
	sys_write(2, "    R15: ", 9);
	sys_write(2, aux, longitud);
	sys_write(2, "\n", 1);

	longitud = uint64_tToString(reg->rsp, aux);
	sys_write(2, "RSP: ", 5);
	sys_write(2, aux, longitud);

	longitud = uint64_tToString(reg->rip, aux);
	sys_write(2, "    RIP: ", 9);
	sys_write(2, aux, longitud);
	sys_write(2, "\n", 1);
}

void idleSymbol(){
	if(windows[activeWindow].currentLineSize != MAX_LINE_CHARS)
	{
		if(windows[activeWindow].flagIdle == 0){
			drawChar( IDLE_SYMBOL, windows[activeWindow].xStart + windows[activeWindow].currentLineSize * FONT_WIDTH, 
				windows[activeWindow].yStart + windows[activeWindow].lineCount * (LINE_HEIGHT) + LINE_MARGIN, windows[activeWindow].charColor, BACKGROUND_COLOR);
			windows[activeWindow].flagIdle = 1;
		}
		else{
			deleteIdleSymbol();
		}
	}
}

static void deleteIdleSymbol(){
	drawChar( 0, windows[activeWindow].xStart + windows[activeWindow].currentLineSize * FONT_WIDTH, 
	windows[activeWindow].yStart + windows[activeWindow].lineCount * (LINE_HEIGHT) + LINE_MARGIN, windows[activeWindow].charColor, BACKGROUND_COLOR);
	windows[activeWindow].flagIdle = 0;
}

static void blockIdleSymbol(){
	if(windows[activeWindow].currentLineSize != MAX_LINE_CHARS && windows[activeWindow].flagIdle == 1)
		deleteIdleSymbol();
}