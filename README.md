# Sistemas Operativos
----------------------
## Trabajo Práctico Nro. 2
#### Construcción del Núcleo de un Sistema Operativo y estructuras de administración de recursos
----------------------
## Hecho hasta ahora
----------------------
#### Memory Managers
- Implementado MM1: buddy system con liberacion de los bloques alocados
- Implementado MM2: malloc con liberación de los bloques alocados al estilo del libro 'El lenguaje de programación C' de Kernighan & Ritchie.
- En ambos casos se permite consultar la memoria con la syscall getMemStatus
- El test_mm parece funcionar correctamente

**Para compilar el proyecto utilizando el MM2, ejecutar desde la carpeta RowDaBoat:**

```
make all
```

**Para compilarlo utilizando el MM1 (buddy system), ejecutar desde la carpeta RowDaBoat:**

```
make buddy
```
----------------
#### Procesos, Context Switching y Scheduling
- Implementada creación de procesos
- Implementado scheduling utilizando round-robin como algoritmo
- Implementado sistema de prioridades en donde se le permite a los procesos correr quantums más largos/cortos, dependiendo del caso
- Implementado sistema de foreground y background, en donde el proceso corriendo en foreground está siempre al principio de la lista de procesos (falta implementar el uso de comandos como & desde la shell)
- Implementado sistema en donde las interrupciones de teclado hacen que el siguiente proceso a correr sea el que esté en foreground, si es que éste estaba bloqueado esperando por entrada
- Implementadas todas las syscalls solicitadas en la sección de mismo nombre en el enunciado
- Los tests test_processes y test_priority parecen funcionar bien
- 
------------------------
#### Nota: Para ejecutar los tests desde la shell, o por cualquier otra cosa que quiera saberse sobre los comandos disponibles, ejecute 'help' desde ésta para obtener la información correspondiente.
-----------------------

## Lista de syscalls

A continuación se muestran las llamadas al sistema disponibles, indicando los parametros que reciben, y su respectivo número para su invocacion desde la interrupcion 80h

| Syscall | RAX | Descripción |
| ------- | ------- | ------- |
| int write(unsigned int fd, char *buffer, unsigned long count) | 4 | Imprime en la ventana actual los primeros count caracteres de buffer si fd = 2, sera la salida de error y se imprimira en rojo, si es 1 en el color determinado por la ventana. Recibe en RBX el fd, en RCX el puntero al buffer y en RDX el valor de count |
| int read(char *buffer, unsigned long int count, char delim) | 3 | Lee de entrada estandar en un buffer hasta que se llegue a "count" caracteres o se llegue al caracter "delim". Recibe en RBX el puntero al buffer, en RCX el valor de count y en RDX el caracter delim |
| void clrScreen() | 7 | Limpia la pantalla actual, no recibe parametros |
| int changeWindowColor(int rgb) | 9 | Recibe en RBX el int que indica el nuevo color(RGB) de los caracteres de la ventana actual |
| void getTime(TimeFormat *time) | 12 | Recibe en RBX el puntero a una estructura formada por 3 unsigned ints de 32 bits, secs (segundos), mins (minutos), hours (horas) para rellenarla con informacion del tiempo actual. |
| int getCPUTemp() | 13 | Retorna la temperatura del cpu, no recibe parametros |
| RegistersType* getRegisters() | 14 | Retorna un puntero a una estructura con 17 unsigned longs de 64 bits (llamados rax, rbx, rcx, rbp, rdi, rsi, r8, r9, r10, r11, r12, r13, r14, r15, rsp, rip) con los valores de los registros de la ultima vez que se presiono la tecla especial F1, no recibe parametros |
| void getMemory(memType* answer, char * address) | 25 | Se encarga de llamar a la syscall que escribe en la estructura indicada los 32 bytes de informacion de la memoria a partir de address. Recibe en RBX el puntero a la estructura memType y en RCX el char* que indica la address |
| void *malloc(size_t size) | 26 | Aloca memoria suficiente para almacenar el size indicado por parametro, en caso de no haber memoria suficiente disponible retorna NULL. Recibe en RBX el size_t |
| void free(void *ptr) | 27 | Libera la memoria alocada que arranca en la direccion indicada por parametro, si no existe realiza undefined behavior. Recibe en RBX el void* que indica la direccion |
| void getMemStatus(MemStatus *stat) | 28 | Escribe en la estructura indicada (la cual con tiene 3 campos de tipo size_t, totalMem (memoria total), occMem (memoria ocupada), freeMem (memoria libre)) los valores que permiten saber el estado de la memoria disponible para alocar en dicho instante en bytes. Recibe en RBX el puntero a la estructura MemStatus |
| unsigned int startProcessFg(int (*mainptr)(int, char const **), int argc, char const *argv[]) | 29 | Iniciar un proceso en foreground que corre la funcion indicada en RAX(int (*)(int, char const **)) con RBX argumentos que recibe en RCX(char const *) y retorna el PID |		
| unsigned int startProcessBg(int (*mainptr)(int, char const **), int argc, char const *argv[]) | 30 | Análogo a la syscall 29 pero corre el proceso en background |
| void exit() | 31 | Se encarga de setear el estado como KILLED del proceso que la llama para que luego sea eliminado, no recibe parametros |
| unsigned int getpid() | 32 | Syscall que retorna en RAX el pid del proceso actual, no recibe parametros |
| void listProcess() | 33 | Syscall que imprime en salida estandar el listado de los procesos actuales, no recibe parametros |
| int kill(unsigned int pid, char state) | 34 | Syscall para cambiar el estado de un proceso al indicado en RCX segun un pid dado en RBX | 
| void runNext() | 35 | Syscall para que el proceso corriendo en el momento renuncie al CPU y se corra el siguiente proceso, no recibe parametros |
| int nice(unsigned int pid, unsigned int priority) | 36 | Syscall para cambiar la prioridad de un proceso segun un pid dado, retorna 0 en caso de error |