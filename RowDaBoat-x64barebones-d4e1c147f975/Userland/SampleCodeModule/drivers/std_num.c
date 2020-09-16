#include <std_num.h>

int floatToString(double number, char buffer[DOUBLE_SIZE])
{
	int length = 0;

	if (number < 0)
	{
		buffer[length++] = '-';
		number *= -1;
	}

	//	Obtengo la parte entera y la agrego al buffer
	long int integer = (long int) number;
	length += intToString(integer, &(buffer[length]));

	//	Obtengo la parte decimal hasta PRECISION digitos
	//	en forma de entero para reutilizar intToString
	number -= integer;
	for (int i = 0; i < PRECISION+1; i++)
		number *= 10;
	integer = (long int) number;
	//	Hago el redondeo
	if (integer % 10 >= 5) 
		integer += 10;
	integer /= 10;

	if (integer != 0)	// Veo que la parte decimal no sea cero
	{
		buffer[length++] = '.';	//	Agrego el punto
        length += PRECISION;
        for (int i = 1; i <= PRECISION; i++)
        {
            buffer[length - i] = (integer % 10) + '0';
            integer /= 10;
        }
	}

	buffer[length] = 0;

	return length;
}

int intToString( long num, char * str ){

    if( num == 0 ){
        str[0] = '0';
        str[1] = 0;
        return 1;
    }

    int dig = 0;
    long aux = num;

    while( aux != 0 ){ //cuento digitos para saber desde donde arrancar a meter los caracteres
        aux /= 10;
        dig++;
    }
    
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

int intToHexString(unsigned long int num, char buffer[MAX_INT_DIG + 1]){

    char hexas[16] = {'0','1','2','3','4','5',
                    '6', '7', '8', '9', 'A',
                    'B', 'C', 'D', 'E', 'F'};

    char invertido[MAX_INT_DIG];
    int i;

    if (num == 0)
    {
        buffer[0] = buffer[1] = '0';
        buffer[2] = 0;
		return 2;
    }

    for (i = 0; i < MAX_INT_DIG-1 && num != 0; i ++)
    {
        invertido[i] = hexas[num%16];
        num /= 16;
    }

    if (i % 2 == 1){
        invertido[i++] = '0'; 
    }

    for (int j = 0; j < i; j++)
    {
        buffer[j] = invertido[i - 1 - j];
    }

    buffer[i] = 0;

    return i;
}

int pointerToHexString(void *pointer, char buffer[POINTER_SIZE + 1]){

    //  Obtengo la conversion a hexa con su longitud real
    int length = intToHexString((unsigned long int) pointer, buffer);

    //  Llevo el string al final (para despues agregarle '0's al principio)
    for (int i = 0; i < POINTER_SIZE && i < length; i++)
    {
        buffer[POINTER_SIZE - 1 - i] = buffer[length - 1 - i];
    }

    //  Agrego los ceros al principio
    for (int i = 0; i < POINTER_SIZE - length; i++)
    {
        buffer[i] = '0';
    }

    //  Null terminated
    buffer[POINTER_SIZE] = 0;

    return POINTER_SIZE;
}
