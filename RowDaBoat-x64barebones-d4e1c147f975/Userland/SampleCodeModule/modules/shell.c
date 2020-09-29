#include <std_io.h>
#include <comandos.h>
#include <syscalls.h>

#define INPUT_BUFFER_SIZE 100

char inputBuffer[INPUT_BUFFER_SIZE];
int indexBuffer;
static void parse();
static char *symbol = "$>";


void runShell(){
    printf("\nIngrese help y presione enter para una explicacion del programa\n");
    while(1){
        puts(symbol);
        indexBuffer = read( inputBuffer, INPUT_BUFFER_SIZE, '\n' );
        parse();
    }
}

static void parse(){
    inputBuffer[indexBuffer] = 0;         // Reemplazo con 0 el final para utilizar strcmp
    
    if(strcmp( inputBuffer, "help\n") == 0)
        help();
    else if(strcmp( inputBuffer, "inforeg\n" ) == 0)
	   printInforeg();
    else if(strcmp( inputBuffer, "clear\n" ) == 0)
        clrScreen();
    else if(strcmp( inputBuffer, "printtime\n" ) == 0){
        printTime();
        putchar('\n');
    }
    else if(strcmp( "printmem", inputBuffer ) == 0){           // solo valido que lo q este al principio del input valide con el comando,
                                                                // el resto va a ser el argumento
        if( inputBuffer[8] == ' ' && inputBuffer[9] != '\n'){       // se ingreso un parametro
            char address[INPUT_BUFFER_SIZE- 10] = {0};              // maximo tamanio posible del argumento("printmem " son 9 caracteres y tampoco cuento el \n)
            int i = 0;
            for(; inputBuffer[9 + i] != '\n' && i < indexBuffer -1; i++)   // en inputBuffer[indexBuffer] esta el \n
                address[i] = inputBuffer[9 + i];

            address[i] = 0;
            printmem(address);
        }else
            fprintf(2, "-printmem: Falta ingresar la direccion como parametro.\n");
    }
    else if(strcmp( inputBuffer, "cpuinfo\n") == 0)
        printCPUInfo();
    else if(strcmp( inputBuffer, "cputemp\n") == 0)
        printCPUTemp();
    else if(strcmp( inputBuffer, "executeZeroException\n") == 0)
        startProcess((int (*)(int, const char **))executeZeroException, 0, NULL);
    else if(strcmp( inputBuffer, "executeUIException\n") == 0)
        startProcess((int (*)(int, const char **))executeUIException, 0, NULL);
    else if(strcmp( inputBuffer, "mem\n") == 0)
        printMemStatus();
    else if(strcmp( inputBuffer, "ps\n") == 0)
        listProcess();
    else if(strcmp( "kill ", inputBuffer ) == 0){
        int pid = strToPositiveInt(inputBuffer+5, NULL);
        if(pid == -1)
            printf("Error en argumentos\n");
            
        kill(pid, KILLED);
    }
    else if(strcmp( inputBuffer, "loop\n") == 0){
        const char * loopname = "loop";
        startProcess((int (*)(int, const char **))loop, 1, &loopname);
    }
    else if(strcmp( "nice ", inputBuffer) == 0){
        int pid, priority, aux = 0;
        pid = strToPositiveInt(inputBuffer+5, &aux);
        if(pid == -1)
            printf("Error en argumentos\n");

        priority = strToPositiveInt(inputBuffer+5+aux+1, NULL);
        if(priority == -1)
            printf("Error en argumentos\n");

        if(nice(pid, priority) == 0)
            printf("Error en argumentos\n"); //puede que no haya encontrado el pid o que la prioridad no sea valida
    }
    else
        fprintf(2, "Comando no reconocido, ejecuta help para recibir informacion.\n");
}
