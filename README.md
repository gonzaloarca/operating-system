# TP2SO

## Lista de syscalls

1. SYSCALL write: RAX = 4
		Imprime en la ventana actual los primeros count caracteres de buffer si fd = 2, sera la salida de error y se imprimira en rojo, si es 1 en el color determinado por la ventana.
        Recibe en RBX el fd, en RCX el puntero al buffer y en RDX el valor de count

2. SYSCALL read: RAX = 3
		Lee de entrada estandar en un buffer hasta que se llegue a "count" caracteres o se llegue al caracter "delim"
        Recibe en RBX el puntero al buffer, en RCX el valor de count y en RDX el caracter delim

3. SYSCALL clrScreen: RAX = 7
		Limpia la pantalla actual, no recibe parametros

4. SYSCALL changeWindowColor: RAX = 9
		Recibe en RBX el int que indica el nuevo color(RGB) de los caracteres de la ventana actual

5. SYSCALL getTime: RAX = 12
		Recibe en RBX el puntero a una estructura TimeFormat para rellenarla con informacion del tiempo actual

6. SYSCALL getCPUTemp: RAX = 13
		Retorna la temperatura del cpu, no recibe parametros

7. SYSCALL getRegisters: RAX = 14
		Retorna un puntero a una estructura RegistersType con los valores de los registros de la 
        ultima vez que se presiono la tecla especial F1, no recibe parametros

8. SYSCALL getMemory: RAX = 25
		Se encarga de llamar a la syscall que escribe en la estructura indicada los 32 bytes de informacion de la memoria a partir de address
        Recibe en RBX el puntero a la estructura memType y en RCX el char* que indica la address

7. SYSCALL malloc: RAX = 26
		Alloca memoria suficiente para almacenar el size indicado por parametro, en caso de no haber memoria suficiente disponible retorna NULL
        Recibe en RBX el size_t

8. SYSCALL free: RAX = 27
		Libera la memoria alocada que arranca en la direccion indicada por parametro, si no existe realiza undefined behaviour
        Recibe en RBX el void* que indica la direccion

9. SYSCALL getMemStatus: RAX = 28
		Escribe en la estructura indicada los valores que permiten saber el estado de la memoria disponible para alocar en dicho instante
        Recibe en RBX el puntero a la estructura MemStatus

10. SYSCALL startProcess: RAX = 30
		Iniciar un proceso que corre la funcion indicada en RAX(int (*)(int, char const **)) con RBX argumentos que recibe en RCX(char const *)

11.	SYSCALL exit: RAX = 31
		Se encarga de setear el estado como KILLED del proceso que la llama para que luego sea eliminado, no recibe parametros

12.	SYSCALL getpid: RAX = 32
		Syscall que retorna en RAX el pid del proceso actual, no recibe parametros

13.	SYSCALL listProcess: RAX = 33
		Syscall que imprime en salida estandar el listado de los procesos actuales, no recibe parametros

14.	SYSCALL kill: RAX = 34
		Syscall para cambiar el estado de un proceso al indicado en RCX segun un pid dado en RBX

15. SYSCALL runNext: RAX = 35
		Syscall para que el proceso corriendo en el momento renuncie al CPU y se corra el siguiente proceso, no recibe parametros

16. SYSCALL nice: RAX = 36
		Syscall para cambiar la prioridad de un proceso segun un pid dado, retorna 0 en caso de error