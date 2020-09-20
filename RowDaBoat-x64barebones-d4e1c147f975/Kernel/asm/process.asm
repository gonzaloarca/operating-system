GLOBAL _start
GLOBAL createStackFrame

;-------------------------------------------------------------
; FUNCION WRAPPER PARA LOS MAIN DE PROCESOS
;-------------------------------------------------------------
; void _start(int *(mainPtr)(int, char const **), int argc, char const *argv[])
;-------------------------------------------------------------
_start:
    mov rax, rdi    ;Necesito rdi para los param del main
    mov rdi, rsi
    mov rsi, rdx

    call rax        ;Llamo al main

    mov rbx, rax
    mov rax, 31     ;Llamo a la syscall exit
    int 80h

;--------------------------------------------------------------
; PREPARADO DEL STACK FRAME AL CREAR UN PROCESO
;--------------------------------------------------------------
;  void createStackFrame(uint64_t frame, uint64_t mainptr, int argc, int argv);
;--------------------------------------------------------------
;   rdi = *frame
;   rsi = mainptr
;   rdx = argc
;   rcx = argv
;--------------------------------------------------------------
createStackFrame:
    sub rdi, 8
    mov QWORD [rdi], 0x0          ;   SS
    sub rdi, 8
    mov [rdi], rdi              ;   RBP
    sub rdi, 8
    mov QWORD [rdi], 0x202              ;   RFLAGS
    sub rdi, 8
    mov QWORD [rdi], 0x8                ;   0x8
    sub rdi, 8
    mov QWORD [rdi], _start       ;   RIP

    sub rdi, 8                      ;   Apunto al RAX en Stack
    mov r9, 15                      ;   Cantidad de registros backupeados en Stack
.ciclo:
	dec r9
	mov QWORD[rdi+8*r9], 0				;	Seteo en cero todos los registros
	cmp r9, 0
	jnz .ciclo

    mov [rdi+8*3], rcx            ;   RDX = argv
    mov [rdi+8*5], rsi            ;   RDI = mainptr
    mov [rdi+8*6], rdx            ;   RSI = argc

    ret
