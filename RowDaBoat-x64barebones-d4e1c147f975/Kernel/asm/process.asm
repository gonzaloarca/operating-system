GLOBAL _start

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

    mov rax, 35
    int 80h         ;Llamo a la syscall para runNext forzar un cambiar de proceso

