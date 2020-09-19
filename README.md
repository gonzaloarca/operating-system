# TP2SO

## Lista de syscalls

;--------------------------------------------------------------------------------------------------------------
;	SYSCALL write: RAX = 4
;			Imprime en la ventana actual los primeros count caracteres de buffer
;			si fd = 2, sera la salida de error y se imprimira en rojo, si es 1 en el color determinado 
;           por la ventana
;           recibe en RBX el fd, en RCX el puntero al buffer y en RDX el valor de count
;--------------------------------------------------------------------------------------------------------------

;--------------------------------------------------------------------------------------------------------------
;	SYSCALL read: RAX = 3
;			Lee de entrada estandar en un buffer hasta que se llegue a "count" caracteres o se llegue al 
;           caracter "delim"
;           recibe en RBX el puntero al buffer, en RCX el valor de count y en RDX el caracter delim
;--------------------------------------------------------------------------------------------------------------

;--------------------------------------------------------------------------------------------------------------
;	SYSCALL clrScreen: RAX = 7
;			Limpia la pantalla actual
;--------------------------------------------------------------------------------------------------------------

;--------------------------------------------------------------------------------------------------------------
;	SYSCALL changeWindowColor: RAX = 9
;			Recibe en RBX el int que indica el nuevo color(RGB) de los caracteres de la ventana actual
;--------------------------------------------------------------------------------------------------------------

;--------------------------------------------------------------------------------------------------------------
;	SYSCALL getTime: RAX = 12
;			Recibe en RBX el puntero a una estructura TimeFormat para rellenarla con informacion 
;           del tiempo actual
;--------------------------------------------------------------------------------------------------------------

;--------------------------------------------------------------------------------------------------------------
;	SYSCALL getCPUTemp: RAX = 13
;			Retorna la temperatura del cpu
;--------------------------------------------------------------------------------------------------------------

;--------------------------------------------------------------------------------------------------------------
;	SYSCALL getRegisters: RAX = 14
;			Retorna un puntero a una estructura RegistersType con los valores de los registros de la 
;           ultima vez que se presiono la tecla especial F1
;--------------------------------------------------------------------------------------------------------------

;--------------------------------------------------------------------------------------------------------------
;	SYSCALL initProcess: RAX = 21
;			Agregar el modulo indicado en RBX(como void*) a la lista de modulos
;--------------------------------------------------------------------------------------------------------------

;--------------------------------------------------------------------------------------------------------------
;	SYSCALL runFirstProcess: RAX = 23
;			Se encarga de correr el primer proceso en la cola, en caso de existir
;--------------------------------------------------------------------------------------------------------------

;--------------------------------------------------------------------------------------------------------------
;	SYSCALL getMemory: RAX = 25
;			Se encarga de llamar a la syscall que escribe en la estructura indicada los 32 bytes de informacion 
;           de la memoria a partir de address
;           Recibe en RBX el puntero a la estructura memType y en RCX el char* que indica la address
;--------------------------------------------------------------------------------------------------------------

;--------------------------------------------------------------------------------------------------------------
;	SYSCALL malloc: RAX = 26
;			Alloca memoria suficiente para almacenar el size indicado por parametro, en caso de no haber 
;           memoria suficiente disponible retorna NULL
;           Recibe en RBX el size_t
;--------------------------------------------------------------------------------------------------------------

;--------------------------------------------------------------------------------------------------------------
;	SYSCALL free: RAX = 27
;			Libera la memoria alocada que arranca en la direccion indicada por parametro, si no existe 
;           realiza undefined behaviour
;           Recibe en RBX el void* que indica la direccion
;--------------------------------------------------------------------------------------------------------------

;--------------------------------------------------------------------------------------------------------------
;	SYSCALL getMemStatus: RAX = 28
;			Escribe en la estructura indicada los valores que permiten saber el estado de la memoria disponible 
;           para alocar en dicho instante
;           Recibe en RBX el puntero a la estructura MemStatus
;--------------------------------------------------------------------------------------------------------------
