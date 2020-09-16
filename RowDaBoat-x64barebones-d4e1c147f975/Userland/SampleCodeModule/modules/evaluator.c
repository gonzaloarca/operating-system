#include <evaluator.h>

#define ANSWER 'A'
#define ERROR_MSG "\nExpresion erronea, pruebe de vuelta\n"
#define BUFFER_SIZE 100

//	Funciones internas del programa
static int main_op(char *expression, int length);
static double strtonum(char *string, int length);
static int is_operand(char c);
static int balance (char s[]);
static double recursive_evaluation(char *expression, int length);
static int validChar(char c);
static void printHelp();

static char *symbol = "Exp: ";

static double last_result = 0;
static int error = 0;

void calculator()
{
	char buffer[BUFFER_SIZE+1];
	int length;
	double result;
	printf("\nIngrese help= para una explicacion del programa\n");

	while(1){
		puts(symbol);
		if ( (length = read(buffer, BUFFER_SIZE+1, '=')) != 0 )
		{
			if (length > 1)
			{
				buffer[--length] = 0;		//	Quito el '=' del final

				if (strcmp(buffer,"help") == 0)
					printHelp();
				else if (strcmp(buffer,"clear") == 0)
					clrScreen();
				else
				{
					if (balance(buffer) == 0)
					{
						result = recursive_evaluation(buffer, length);
						if (error == 0){
							printf("\b = %g", result);
							last_result = result;
						} else
							error = 0;
					} else
						fprintf(2, ERROR_MSG);
				}
			} else
				fprintf(2, ERROR_MSG);
				
			puts("\n");
		}
	}
}

static void printHelp()
{
	printf("\n\t\t\t-----CALCULADORA-----\n");
	printf("\tEscriba una expresion inorder sin espacios\n");
	printf("\tSoportado: + - * /\n");
	printf("\tLa letra A denota el resultado de la ultima expresion\n");
	printf("\tApriete '=' para calcular el resultado\n");
	printf("\tApriete F2 para borrar la expresion actual\n");
	printf("\tApriete F3 para ejecutar el cambio de proceso\n");
	printf("\tComandos: help, clear\n");
}

/*	Esta funcion funciona recursivamente.
**	Encuentra el ultimo operador que debe evaluar
**		(o sea, el primero de menor precedencia empezando 
**		por el final de la expresion) y luego evalua
**		las expresiones a su izquierda y derecha.
**	Finalmente, evalua utilizando su operador.
*/
static double recursive_evaluation(char *expression, int length)
{
	//	Busco la posicion del operador a evaluar
	int operator = main_op(expression, length);

	//	Si no habia operador principal, tengo que convertir a numero
	//	Nota: tambien entra en este if las expresion con parentesis
	if (operator == -1)
	{
		//	Si era una expresion con parentesis, tengo que evaluarla
		if (expression[0] == '(' && expression[length-1] == ')')
			return recursive_evaluation(expression+1, length-2);
		//	Si no, era un numero y la tengo que convertir
		return strtonum(expression, length);
	}

	//	Evaluo expresiones de la izquierda y la derecha del operador
	double izq, der;
	izq = recursive_evaluation(expression, operator);
	der = recursive_evaluation(expression+operator+1, length-operator-1);

	//	Segun cual operador era, devuelvo su resultado correspondiente
	switch(expression[operator])
	{
		case '+':
			return izq + der;
		case '-':
			return izq - der;
		case '*':
			return izq * der;
		case '/':
			if (der == 0)
			{
				fprintf(2, ERROR_MSG);
				error = 1;
				return 0;
			}
			return izq / der;
	}

	//	Si llega hasta aca, debe haber algun error, retorno 0 por las dudas
	return 0;
}

/*	La funcion main_op devuelve la posicion del 
**		operador "principal" en la expresion
**		(el ultimo de menor precedencia)
**	Este se debe encontrar fuera de un parentesis
**		si no, se lo ignora
*/
static int main_op(char *expression, int length)
{
	//	brackets indica si hay parentesis abierto o cerrado
	int brackets = 0;
	//	posicion del ultimo operador encontrado
	int last_op = -1;

	for (int i = length-1; i >= 0; i--)
	{
		switch(expression[i])
		{
			case '+': case '-':
				if (brackets==0)
				{
					if (last_op == -1 || expression[last_op] == '*' 
						|| expression[last_op] == '/')
					{
						return i;
					}
				}
				break;

			case '*': case '/':
				if (brackets==0 && last_op==-1)
				{
					last_op = i;
				}
				break;			

			case '(':
				brackets++;
				break;

			case ')':
				brackets--;
				break;
		}
	}

	return last_op;
}

static double strtonum(char *string, int length)
{
	if (length == 1 && string[0] == ANSWER)
		return last_result;

	//	El numero acumulado hasta ahora
	double number = 0.0,
		exp = 1.0; 	//Exponente para la parte real del numero
	//	El char actual
	char c;
	//	Indica si ya se encontro el punto decimal
	int decimal = 0;	

	for (int i=0; i<length; i++)
	{
		c = string[i];
		if (c >= '0' && c <= '9')
		{
			//	Si todavia no llegue al punto, es la parte entera
			if (!decimal)
			{
				number *= 10;
				number += c - '0';
			}
			//	En cambio, si ya llegué tengo que realizar otro calculo
			else
			{
				exp /= 10;
				number += exp * (c - '0');
			}
		}
		else if (c == '.')
			if (decimal)
			{
				fprintf(2, ERROR_MSG);
				error = 1;
				return 0;
			}
			else
				decimal = 1;
		// Si hay un operador, hubo algun error, devuelvo 0
		else if (is_operand(c))
			return 0;
	}

	return number;
}

static int is_operand(char c)
{
	switch(c)
	{
		case '+': case '-': case '/': case '*':
		return 1;
	}
	return 0;
}

//	Funcion para checkear si los parentesis son utilizados correctamente
//	Devuelve 0 si la expresion tiene todos sus parentesis apareados
//	Copiada de la guia de PI
static int balance (char s[])
{
	int b;
	char c = *s;
	if ( c == 0 )
		return 0;
	
	b = balance(s+1);
	
	if ( c == '(' )
	{
		if ( b < 0 )
			b++;
		else if ( b == 0 )
			b--;
	}
	
	else if ( c == ')' )
		b--;

	else if (!validChar(c))
		b--;
	
	return b;
}

static int validChar(char c)
{
	return is_operand(c) || (c >= '0' && c <= '9')
		||	c == ANSWER || c == '.';
}

