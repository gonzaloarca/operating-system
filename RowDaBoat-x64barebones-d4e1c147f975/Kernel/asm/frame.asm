GLOBAL getBackupINT
GLOBAL setBackupINT
GLOBAL startRunning
GLOBAL startRunningEXC
GLOBAL restart

;-------------------------------------------------------
;	Guarda en la estructura indicada por parametro los registros
;	pusheados por interrupcion de teclado
;-------------------------------------------------------
; Llamada en C:
;	void getBackupINT(RegistersType *reg);
;-------------------------------------------------------
getBackupINT:
	push rbp
	mov rbp, rsp
	push rbx

	mov rax, rbp
	mov rcx, 4					;	cantidad de saltos para llegar al stack frame del inicio de la interrupcion

.jump:
	mov rax, [rax]	
	dec rcx	
	jnz .jump

	; en rax tengo la posicion donde esta guardando el rbp viejo
	; en rax+8 retorno a irq Master
	; en rax+16 tengo el ultimo registro pusheado q es r15
	; empiezo guardando ese y luego con un contador voy guardando el resto

	mov rcx, 15
	add rax, 16

.ciclo:
	dec rcx
	mov rbx, [rax]
	mov QWORD[rdi + 8*rcx], rbx
	add rax, 8
	cmp rcx, 0
	jnz .ciclo

	; ahora tengo RIP, CS, EFLAGS, RSP y SS que fueron guardados por la interrupcion

	mov rbx, [rax]
	mov QWORD[rdi + 8*16], rbx				;rip

	mov rbx, [rax+16]
	mov QWORD[rdi + 8*17], rbx				;eflags

	mov rbx, [rax+24]
	mov QWORD[rdi + 8*15], rbx				;rsp

	pop rbx
	mov rsp, rbp
	pop rbp
	ret


;-------------------------------------------------------
;	Restaura de la estructura indicada por parametro los registros
;	pusheados por interrupcion de teclado
;-------------------------------------------------------
; Llamada en C:
;	void setBackupINT(RegistersType *reg);
;-------------------------------------------------------
setBackupINT:
	push rbp
	mov rbp, rsp
	push rbx

	mov rax, rbp
	mov rcx, 4					;	cantidad de saltos para llegar al stack frame del inicio de la interrupcion

.jump:
	mov rax, [rax]	
	dec rcx
	jnz .jump

	; en rax tengo la posicion donde esta guardando el rbp viejo
	; en rax+8 retorno a irq Master
	; en rax+16 tengo el ultimo registro pusheado q es r15
	; empiezo guardando ese y luego con un contador voy guardando el resto

	mov rcx, 15
	add rax, 16

.ciclo:
	dec rcx
	mov rbx, QWORD[rdi + 8*rcx]
	mov [rax], rbx
	add rax, 8
	cmp rcx, 0
	jnz .ciclo

	; ahora tengo RIP, CS, EFLAGS, RSP y SS que fueron guardados por la interrupcion

	mov rbx, QWORD[rdi+8*16]
	mov [rax], rbx						;rip

	mov rbx, QWORD[rdi+8*17]
	mov [rax+16], rbx					;eflags

	mov rbx, QWORD[rdi+8*15]
	mov [rax+24], rbx					;rsp

	pop rbx
	mov rsp, rbp
	pop rbp
	ret

;-------------------------------------------------------
;	Empieza a ejecutar el programa indicado
;-------------------------------------------------------
; Llamada en C:
;	void startRunning(uint64_t rip, uint64_t rsp);
;-------------------------------------------------------
startRunning:
	push rbp
	mov rbp, rsp
	push rbx

	mov rax, rbp
	mov rcx, 3				;	cantidad de saltos para llegar al stack frame del inicio de la syscall

.jump:
	mov rax, [rax]	
	dec rcx	
	jnz .jump

	; en rax tengo la posicion donde esta guardando el rbp viejo
	; en rax+8 retorno a irq Master
	; en rax+16 tengo el ultimo registro pusheado q es r15
	; empiezo guardando ese y luego con un contador voy guardando el resto

	mov rcx, 15
	
	add rax, 16

	add rax, 40			; 	Parametros de una syscall

	mov rcx, 14

.ciclo:
	dec rcx
	mov QWORD[rax], 0				;	Seteo en cero todos los otros registros
	add rax, 8
	cmp rcx, 0
	jnz .ciclo

	; ahora tengo RIP, CS, EFLAGS, RSP y SS que fueron guardados por la interrupcion

	mov [rax], rdi

	mov [rax+24], rsi

	pop rbx
	mov rsp, rbp
	pop rbp
	ret

;-------------------------------------------------------
;	Empieza a ejecutar el programa indicado luego de
;	que haya ocurrido una excepcion
;-------------------------------------------------------
; Llamada en C:
;	void startRunning(uint64_t rip, uint64_t rsp);
;-------------------------------------------------------
startRunningEXC:
	push rbp
	mov rbp, rsp
	push rbx

	mov rax, rbp
	mov rcx, 3					;	cantidad de saltos para llegar al stack frame del inicio de la excepcion

.jump:
	mov rax, [rax]	
	dec rcx
	jnz .jump

	mov rcx, 15
	add rax, 16

.ciclo:
	dec rcx
	mov QWORD[rax], 0				;	Seteo en cero todos los otros registros
	add rax, 8
	cmp rcx, 0
	jnz .ciclo

	mov [rax], rdi					;rip
	mov [rax+24], rsi				;rsp

	pop rbx
	mov rsp, rbp
	pop rbp
	ret

;-------------------------------------------------------
;	Ejecuta el programa indicado por los parametros
;	CUIDADO: NUNCA EJECUTAR DENTRO DE UNA INTERRUPCION
;-------------------------------------------------------
;	void restart(uint64_t rip, uint64_t rsp);
;-------------------------------------------------------
restart:
	mov rsp, rsi
	push rdi
	ret
