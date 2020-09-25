GLOBAL write
GLOBAL read
GLOBAL changeWindowColor
GLOBAL emptyBuffer
GLOBAL getTime
GLOBAL getCPUTemp
GLOBAL getRegisters
GLOBAL clrScreen
GLOBAL getMemory
GLOBAL malloc
GLOBAL free
GLOBAL getMemStatus
GLOBAL startProcess
GLOBAL getpid

section .text

; Estas aclaraciones sobre las funciones tambien se encuentran en "syscalls.h"

;-------------------------------------------------------
;	SYSCALL write: RAX = 4
;			Imprime en la ventana actual los primeros count caracteres de buffer
;			si fd = 2, sera la salida de error y se imprimira en rojo, si es 1 en el color determinado por la ventana
;-------------------------------------------------------
; Llamada en C:
;	int write( unsigned int fd, char *buffer, unsigned long count )
;-------------------------------------------------------
write:
	push rbp
	mov rbp, rsp
	push rbx
	push rcx
	push rdx

	mov rax, 4			; numero de syscall sys_writePixel
	mov rbx, rdi		; primer parametro
	mov rcx, rsi
	;en rdx ya esta cargado el 3er parametro
	int 80h

	pop rdx
	pop rcx
	pop rbx
	mov rsp, rbp
	pop rbp
	ret

;-------------------------------------------------------
;	SYSCALL read: RAX = 3
;			Lee de entrada estandar en un buffer hasta que se llegue a "count" caracteres o se llegue al caracter "delim"
;-------------------------------------------------------
; Llamada en C:
;	int read( char *buffer, unsigned long count, char delim )
;-------------------------------------------------------
read:
	push rbp
	mov rbp, rsp
	push rbx
	push rcx
	push rdx

	mov rax, 3			; numero de syscall sys_read
	mov rbx, rdi		; 1er parametro 
	mov rcx, rsi		; 2do parametro
	;en rdx ya esta cargado el 3er parametro
	int 80h

	pop rdx
	pop rcx
	pop rbx
	mov rsp, rbp
	pop rbp
	ret

;-------------------------------------------------------
;	SYSCALL clrScreen: RAX = 7
;			Limpia la pantalla actual
;-------------------------------------------------------
; Llamada en C:
;	void clrScreen()
;-------------------------------------------------------
clrScreen:
	push rbp
	mov rbp, rsp
	push rax

	mov rax, 7
	int 80h

	pop rax
	mov rsp, rbp
	pop rbp
	ret

;-------------------------------------------------------
;	SYSCALL changeWindowColor: RAX = 9
;			Recibe por parametro el nuevo color de los caracteres de la ventana actual
;-------------------------------------------------------
; Llamada en C:
;	int changeWindowColor(int rgb)
;-------------------------------------------------------
changeWindowColor:
						; retorna 1 en exito, 0 caso contrario
	push rbp
	mov rbp, rsp
	push rbx

	mov rax, 9			; numero de syscall sys_changeWindowColor
	mov rbx, rdi		; 1er parametro
	int 80h

	pop rbx
	mov rsp, rbp
	pop rbp
	ret

;-------------------------------------------------------
;	SYSCALL getTime: RAX = 12
;			Rellena la estructura con informacion del tiempo actual
;-------------------------------------------------------
; Llamada en C:
;	void getTime( TimeFormat *time )
;-------------------------------------------------------
getTime:
	push rbp
	mov rbp, rsp
	push rax

	mov rax, 12
	mov rbx, rdi
	int 80h

	pop rax
	mov rsp, rbp
	pop rbp
	ret

;-------------------------------------------------------
;	SYSCALL getCPUTemp: RAX = 13
;			Retorna la temperatura del cpu
;-------------------------------------------------------
; Llamada en C:
;	int getCPUTemp()
;-------------------------------------------------------
getCPUTemp:				; int getCPUTemp()
	push rbp
	mov rbp, rsp
	
	mov rax, 13
	int 80h

	mov rsp, rbp
	pop rbp
	ret

;-------------------------------------------------------
;	SYSCALL getRegisters: RAX = 14
;			Funcion que retorna una estructura con los valores de los registros de la ultima vez que se presiono la tecla especial F1
;-------------------------------------------------------
; Llamada en C:
;	RegistersType* getRegisters()
;-------------------------------------------------------
getRegisters:
	push rbp
	mov rbp, rsp

	mov rax, 14
	int 80h

	mov rsp, rbp
	pop rbp
	ret

;-------------------------------------------------------
;	SYSCALL getMemory: RAX = 25
;			Funcion que se encarga de llamar a la syscall que escribe en la estructura indicada los 32 bytes de informacion de la memoria a partir de address
;-------------------------------------------------------
; Llamada en C:
;	void getMemory(memType* answer, char* address)
;-------------------------------------------------------
getMemory:
	push rbp
	mov rbp, rsp

	push rbx
	push rcx

	; en rdi se encuentra la estructura a retornar y en rsi la direccion
	; necesito imprimir 32 bytes

	mov rax, 25
	mov rbx, rdi
	mov rcx, rsi
	int 80h

	pop rcx
	pop rbx

	mov rsp, rbp
	pop rbp
	ret

;-------------------------------------------------------
;	SYSCALL malloc: RAX = 26
;			Funcion que se encarga de llamar a la syscall que alloca memoria suficiente para almacenar el size indicado por parametro, 
;		en caso de no haber memoria suficiente disponibleretorna NULL
;-------------------------------------------------------
; Llamada en C:
;	void *malloc(size_t size)
;-------------------------------------------------------
malloc:
	push rbp
	mov rbp, rsp

	push rbx

	mov rax, 26
	mov rbx, rdi
	int 80h

	pop rbx

	mov rsp, rbp
	pop rbp
	ret

;-------------------------------------------------------
;	SYSCALL free: RAX = 27
;			Funcion que se encarga de llamar a la syscall que libera la memoria alocada que arranca en la direccion indicada por parametro, 
;		si no existe realiza undefined behaviour
;-------------------------------------------------------
; Llamada en C:
;	void free(void *ptr)
;-------------------------------------------------------
free:
	push rbp
	mov rbp, rsp

	push rbx

	mov rax, 27
	mov rbx, rdi
	int 80h

	pop rbx

	mov rsp, rbp
	pop rbp
	ret

;-------------------------------------------------------
;	SYSCALL getMemStatus: RAX = 28
;			Funcion que se encarga de llamar a la syscall que escribe en la estructura indicada los valores que permiten saber el estado 
;		de la memoria disponible para alocar en dicho instante
;-------------------------------------------------------
; Llamada en C:
;	void getMemStatus(MemStatus *stat)
;-------------------------------------------------------
getMemStatus:
	push rbp
	mov rbp, rsp

	push rbx

	mov rax, 28
	mov rbx, rdi
	int 80h

	pop rbx

	mov rsp, rbp
	pop rbp
	ret

;-------------------------------------------------------
;	SYSCALL startProcess: RAX = 30
;			Inicia un proceso nuevo
;-------------------------------------------------------
; Llamada en C:
;	int startProcess(int (*mainptr)(int, char const **), int argc, char const *argv[]);
;-------------------------------------------------------
startProcess:
	push rbp
	mov rbp, rsp
	push rbx
	push rcx
	push rdx

	mov rax, 30
	mov rbx, rdi		; 1er parametro 
	mov rcx, rsi		; 2do parametro
	;en rdx ya esta cargado el 3er parametro
	int 80h

	pop rdx
	pop rcx
	pop rbx
	mov rsp, rbp
	pop rbp
	ret

;-------------------------------------------------------
;	SYSCALL getpid: RAX = 32
;			Funcion que se encarga de llamar a la syscall que retorna el pid del proceso actual
;-------------------------------------------------------
; Llamada en C:
;	unsigned int getpid();
;-------------------------------------------------------
getpid:
	push rbp
	mov rbp, rsp

	mov rax, 32
	int 80h

	mov rsp, rbp
	pop rbp
	ret
