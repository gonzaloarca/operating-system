#include <keyboard.h>
#include <interrupts.h>
#include <window_manager.h>

#define LEFT_SHIFT 42
#define RIGHT_SHIFT 54
#define CAPS_LOCK 58
#define LSHIFT_RELEASED (LEFT_SHIFT+0x80)
#define RSHIFT_RELEASED (RIGHT_SHIFT+0x80)
#define F1 59
#define F2 60
#define F3 61
#define BUFFER_SIZE 200

//	Tengo que guardarme si el shift se encuentra presionado
static int lshift = 0;
static int rshift = 0;
static int caps = 0;

const char asccode[58][2] = { {0,0}, {0,0}, {'1', '!'}, {'2', '@'}, {'3', '#'},{'4', '$'},
						{'5','%'},{'6','^'},{'7','&'},{'8','*'},{'9','('},{'0',')'},
						{'-','_'},{'=','+'},{'\b', '\b'},{'\t','\t'},{'q','Q'}, 
						{'w','W'}, {'e','E'},{'r','R'},{'t','T'},{'y','Y'},{'u','U'},
						{'i','I'},{'o','O'},{'p','P'},{'[','{'},{']','}'},
					  	{'\n','\n'},{0,0},{'a','A'},{'s','S'},{'d','D'},{'f','F'},
					  	{'g','G'},{'h','H'},{'j','J'},{'k','K'},{'l','L'}, 
					  	{';',':'},{'\'', '\"'},{'`','~'},{0,0},{'\\','|'},
					  	{'z','Z'},{'x','X'},{'c','C'},{'v','V'},{'b','B'},
					  	{'n','N'},{'m','M'}, {',', '<'},{'.','>'},{'/','?'},
					  	{0,0},{0,0},{0,0},{' ',' '}};

char asciiMap(int code);

static int buffer[BUFFER_SIZE];
static int lastPos = 0;
static int firstPos = 0;

void keyboard_handler()
{
	//	Si no habia nada para leer, no hago nada
	if (canReadKey() == 0) return;
	// Asigno los codigos de make y break de las teclas al buffer
	buffer[lastPos] = getScanCode();

	if(buffer[lastPos] == F1)
		saveRegisters();

	if(buffer[lastPos] == F3)
		switchProcess();

	// Actualizo el indice del buffer circular
	lastPos = (lastPos + 1) % BUFFER_SIZE;
}

char asciiMap(int code)
{
	//	Seteo el estado de los shift
	switch(code)
	{
		case LEFT_SHIFT: 
			lshift = 1; return 0;
		case RIGHT_SHIFT: 
			rshift = 1; return 0;
		case CAPS_LOCK:
			caps = !caps; return 0;
		case F2:
			return 19;
		case LSHIFT_RELEASED:
			lshift = 0; return 0;
		case RSHIFT_RELEASED:
			rshift = 0; return 0;
		case 0:
			return 0;
	}

	if (code >= 0x80) return 0;	//	No me interesan los exit codes

	//	Convierto el codigo a char fijandome el estado de los shifts y el caps lock
	if (lshift + rshift >= 1){
		if( caps && scanCodetoChar(code, 0) >= 97 && scanCodetoChar(code, 0) <= 122 ){ 
			return scanCodetoChar(code, 0); //si shift = 1 y caps = 1, si tipeo una letra debe aparecer minuscula
		} else{
			return scanCodetoChar(code, 1); //shift = 1, el caps no importa porque el codigo no es una letra, imprimo el caracter modificado por el shift
		}
	} else if( caps && scanCodetoChar(code, 0) >= 97 && scanCodetoChar(code, 0) <= 122 ){ 
			return scanCodetoChar(code, 1); //shift = 0 y caps = 1, la letra debe aparecer como mayuscula
	} else{
			return scanCodetoChar(code, 0); //shift = 0, y el caps no importa porque no es una letra el codigo, imprimo el caracter sin modificacion alguna
	}
}

void sys_emptyBuffer(){
	firstPos = 0;
	lastPos = 0;
}

int readBuffer(){
	if (firstPos == lastPos) return 0; //si el buffer esta vacio no debe levantar teclas

	int ret = buffer[firstPos];
	firstPos = (firstPos + 1) % BUFFER_SIZE;
	return ret;
}

char scanCodetoChar (unsigned int scan_code, unsigned int shift){
	if ((shift != 0 && shift != 1) || scan_code >= 58) 
		return 0;

	return asccode[scan_code][shift];
}

uint64_t sys_read(char* out_buffer, unsigned long int count, char delim)
{
	sys_emptyBuffer();
	int i = 0;
	char c = 0;
	char bspace = '\b', space = ' ';
	while( c != delim && i < count ){
		while( (c = asciiMap(readBuffer())) == 0 ){
			_hlt();
		} //levanto una tecla valida del buffer del teclado

		switch( c ){
			case '\b':
				if( i == 0 ){
					break;
				}
				i--;
				sys_write(1, &c, 1);
				break;
				
			case 19:		//codigo ASCII asginado al make code de la tecla F2
				// Borro la linea actual
				
				while(i > 0){
					sys_write(1, &bspace, 1);
					i--;
				}
				break;
			
			case '\t':
				for( int j = 0; j < 4 && (i + j < count - 1); j++ ){
					sys_write(1, &space, 1);	//que tipee 4 espacios al entrar a un tab
					out_buffer[i++] = ' ';
				}
				break;

			case '\n':
				if (delim != '\n')
					break;	// para que no haga saltos de linea en un programa donde el delimitador que se usa no es el enter
				// si no es entra en el siguiente caso

			default:
				if (c == delim || i < count - 1)
				{
					out_buffer[i++] = c;
					sys_write(1, &c, 1);
					if (c == delim) {
						return i;
					}
				}	
		}		
	}
	return i;
}
