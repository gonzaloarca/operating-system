# Sistemas Operativos
## Trabajo Práctico Nro. 2
#### Construcción del Núcleo de un Sistema Operativo y estructuras de administración de recursos
## Autores

* Arca, Gonzalo - Legajo: 60303
* Parma, Manuel - Legajo: 60425
* Rodriguez, Manuel Joaquin - Legajo: 60258

### Dependencias del proyecto

* GNU Binutils (probado en version 2.25)
* NASM (probado en version 2.11.05)
* QEMU (probado en versiones 4.2.1 y 3.1.0)
* GCC (probado en version 4.9.2)
* Make (probado en version 4.0)

### Como compilar

1- Situarse dentro de la carpeta del proyecto y ejecute los siguientes comandos
NOTA: si utiliza Docker, siga el instructivo incluido para poder acceder a la carpeta del proyecto desde Docker

```
cd RowDaBoat-x64barebones-d4e1c147f975
cd Toolchain
make all
cd ..
```

Para compilar el proyecto utilizando el Memory Manager de Kernighan&Ritchie, ejecutar desde la carpeta RowDaBoat:

```
make all
```

Para compilarlo utilizando el Memory Manager basado en Buddy system, ejecutar desde la carpeta RowDaBoat:

```
make buddy
```

### Como ejecutar el proyecto

Ejecute el siguiente comando

```
./run.sh
```

#### Nota: Para ejecutar los tests desde la shell, o por cualquier otra cosa que quiera saberse sobre los comandos disponibles, ejecute 'help' desde ésta para obtener la información correspondiente.
-----------------------

## Lista de syscalls

A continuación se muestran las llamadas al sistema disponibles, indicando los parametros que reciben, y su respectivo número para su invocacion desde la interrupcion 80h

| Syscall | RAX | Descripción |
| ------- | ------- | ------- |
| int read(int fd, char *buffer, unsigned long count) | 3 | Syscall que lee de el file descriptor indicado, escribiendolo en buffer hasta que se llegue a "count" caracteres o se llegue al caracter '\n' o a EOF. Retorna la cantidad de caracteres leidos. Recibe en RBX el file descriptor RCX el puntero al buffer y en RDX el valor de count |
| int write(unsigned int fd, char *buffer, unsigned long count) | 4 | Syscall que imprime en el file descriptor indicado los primeros count caracteres de buffer. Por defecto, fd = 1 es STDOUT y fd = 2 es STDERR. Retorna la cantidad de caracteres escritos. Recibe en RBX el fd, en RCX el puntero al buffer y en RDX el valor de count |
| void clrScreen() | 7 | Syscall que limpia la pantalla actual, no retorna ni recibe parametros |
| int changeWindowColor(int rgb) | 9 | Syscall que cambia el color de los caracteres de la ventana actual. Retorna -1 en error y 0 caso contrario. Recibe en RBX el int que indica el nuevo color(RGB) |
| void getTime(TimeFormat *time) | 12 | Syscall que rellena una estructura formada por 3 unsigned ints de 32 bits, secs (segundos), mins (minutos), hours (horas) para rellenarla con informacion del tiempo actual. No retorna nada y recibe en RBX el puntero a la estructura |
| int getCPUTemp() | 13 | Syscall que retorna la temperatura del cpu, no recibe parametros |
| RegistersType* getRegisters() | 14 | Syscall que retorna un puntero a una estructura con 17 unsigned longs de 64 bits (llamados rax, rbx, rcx, rbp, rdi, rsi, r8, r9, r10, r11, r12, r13, r14, r15, rsp, rip) con los valores de los registros de la ultima vez que se presiono la tecla especial F1, no recibe parametros |
| void getMemory(memType* answer, char * address) | 25 | Syscall que escribe en la estructura indicada los 32 bytes de informacion de la memoria a partir de address. No retorna nada y recibe en RBX el puntero a la estructura memType y en RCX el char* que indica la address |
| void *malloc(size_t size) | 26 | Syscall que aloca memoria suficiente para almacenar el size indicado por parametro, en caso de no haber memoria suficiente disponible retorna NULL caso contrario retorna el puntero al espacio alocado. Recibe en RBX el size_t |
| void free(void *ptr) | 27 | Syscall que libera la memoria alocada que arranca en la direccion indicada por parametro, si no existe realiza undefined behavior. No retorna nada y recibe en RBX el void* que indica la direccion |
| void getMemStatus(MemStatus *stat) | 28 | Syscall que escribe en la estructura indicada (la cual con tiene 3 campos de tipo size_t, totalMem (memoria total), occMem (memoria ocupada), freeMem (memoria libre)) los valores que permiten saber el estado de la memoria disponible para alocar en dicho instante en bytes. No retorna nada y recibe en RBX el puntero a la estructura MemStatus |
| unsigned int startProcessFg(int (*mainptr)(int, char const **), int argc, char const *argv[]) | 29 | Syscall que inicia un proceso en foreground que corre la funcion mainptr con los argumentos indicados. Retorna el PID y recibe en RAX la funcion, en RBX la cantidad de argumentos y en RCX los argumentos |		
| unsigned int startProcessBg(int (*mainptr)(int, char const **), int argc, char const *argv[]) | 30 | Syscall analoga a la 29 pero corre el proceso en background |
| void exit() | 31 | Syscall que se encarga de setear el estado como KILLED del proceso que la llama para que luego sea eliminado, no retorna ni recibe parametros |
| unsigned int getpid() | 32 | Syscall que retorna el pid del proceso actual, no recibe parametros |
| void listProcess() | 33 | Syscall que imprime en salida estandar el listado de los procesos actuales, no recibe parametros |
| int kill(unsigned int pid, char state) | 34 | Syscall para cambiar el estado de un proceso al indicado en RCX segun un pid dado en RBX | 
| void runNext() | 35 | Syscall para que el proceso corriendo en el momento renuncie al CPU y se corra el siguiente proceso, no retorna ni recibe parametros |
| int nice(unsigned int pid, unsigned int priority) | 36 | Syscall para cambiar la prioridad de un proceso segun un pid dado, retorna 0 en caso de error. Recibe en RBX el pid y en RCX la prioridad |
| int createChannel() | 37 | Syscall que crea un canal de comunicacion para señales de sleep y wakeup, devuelve el ID del canal, y en caso de error devuelve -1. No recibe parametros |
| int deleteChannel(unsigned int id) | 38 | Syscall que destruye un canal de comunicacion para señales de sleep y wakeup dado su ID. Si el ID no se corresponde con ningun canal existente, devuelve -1. Sino devuelve 0. Recibe el ID del canal por RBX |
| int sleep(unsigned int id) | 39 | Syscall que manda a dormir al proceso actual en el caso que corresponda. Si ya habian señales pendientes, retorna 1. En caso contrario, devuelve 0. En caso de error, devuelve -1. Recibe el id del canal correspondiente por RBX |
| int wakeup(unsigned int id) | 40 | Syscall que despierta a los procesos esperando en el canal pasado como argumento por su ID. Si no habia nadie durmiendo, incrementa el contador de señales. En este caso, retorna 1. Si habia alguien durmiendo, lo despierta y retorna 0. En caso de error retorna -1. Recibe el id del canal correspondiente por RBX |
| void printChannelPIDs(unsigned int channelId) | 41 | Syscall que imprime los pids bloqueados por el canal indicado en RBX |
| int pipeOpen(unsigned int pipeId, int pipefd[2]) | 42 | Syscall que abre un pipe de comunicacion, devuelve un puntero a un vector que en la primer posicion contiene el indice del pipe de lectura y en el segundo el de escritura, y en caso de error devuelve -1. Recibe en RBX el pipeId y en RCX el vector de indices|
| int pipeClose(unsigned int index) | 43 | Syscall 43 cierra para el proceso actual el acceso al pipe que se encuentra en el indice indicado por paramtro dentro de su vector de pipes |
| int sys_dup2(int oldfd, int newfd) | 44 | Syscall que pisa oldfd con newfd en el proceso actual. Recibe en RBX oldfd y en RCX newfd |
| void listPipes() | 45 | Syscall que imprime los pipes. No recibe parametros |