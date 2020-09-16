GLOBAL _cli
GLOBAL _sti
GLOBAL picMasterMask
GLOBAL picSlaveMask
GLOBAL haltcpu
GLOBAL _hlt

GLOBAL _irq00Handler
GLOBAL _irq01Handler
GLOBAL _irq02Handler
GLOBAL _irq03Handler
GLOBAL _irq04Handler
GLOBAL _irq05Handler

GLOBAL _sysCallHandler

GLOBAL _exception0Handler
GLOBAL _exception6Handler

EXTERN irqDispatcher
EXTERN exceptionDispatcher
EXTERN sysCallDispatcher

SECTION .text

%macro pushStateNoA 0
	push rbx
	push rcx
	push rdx
	push rbp
	push rdi
	push rsi
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
%endmacro

%macro popStateNoA 0
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rsi
	pop rdi
	pop rbp
	pop rdx
	pop rcx
	pop rbx
%endmacro

%macro pushState 0
	push rax
	pushStateNoA
%endmacro

%macro popState 0
	popStateNoA
	pop rax
%endmacro

%macro irqHandlerMaster 1
	pushState

	mov rdi, %1 ; pasaje de parametro
	call irqDispatcher

	; signal pic EOI (End of Interrupt)
	mov al, 20h
	out 20h, al

	popState
	iretq
%endmacro


%macro exceptionHandler 1
	pushState

	mov rdi, %1 ; pasaje de parametro
	call exceptionDispatcher

	popState
	iretq
%endmacro


_hlt:
	sti
	hlt
	ret

_cli:
	cli
	ret


_sti:
	sti
	ret

picMasterMask:
	push rbp
    mov rbp, rsp
    mov ax, di
    out	21h,al
    pop rbp
    retn

picSlaveMask:
	push    rbp
    mov     rbp, rsp
    mov     ax, di  ; ax = mascara de 16 bits
    out	0A1h,al
    pop     rbp
    retn


;8254 Timer (Timer Tick)
_irq00Handler:
	irqHandlerMaster 0

;Keyboard
_irq01Handler:
	irqHandlerMaster 1

;Cascade pic never called
_irq02Handler:
	irqHandlerMaster 2

;Serial Port 2 and 4
_irq03Handler:
	irqHandlerMaster 3

;Serial Port 1 and 3
_irq04Handler:
	irqHandlerMaster 4

;USB
_irq05Handler:
	irqHandlerMaster 5


;Zero Division Exception
_exception0Handler:
	exceptionHandler 0

;Invalid Opcode Exception
_exception6Handler:
	exceptionHandler 6

;---------------------------------------------------
;	SysCall Handler
;---------------------------------------------------
; El rax representa a cual SysCall se llama.
; Los registros rbx, rcx, rdx, rsi y rdi contienen
; 	a los distintos parametros de la SysCall.
; Se llama a sysCallDispatcher con un puntero
; 	a la estructura de los parametros
;---------------------------------------------------
_sysCallHandler:
	pushStateNoA

	sub rsp, 40		; Los otros parametros estan en una estructura
	mov [rsp], rbx
	mov [rsp+8], rcx
	mov [rsp+16], rdx
	mov [rsp+24], rsi
	mov [rsp+32], rdi
	mov rdi, rax	; Se pasa el eax como primer parametro
	mov rsi, rsp	; El segundo argumento es el puntero a los parametros
	call sysCallDispatcher
	;	Lo que devuelve esta en RAX y permanece ahi para el usuario

	add rsp, 40
	popStateNoA
	iretq

haltcpu:
	cli
	hlt
	ret

SECTION .bss
	aux resq 1