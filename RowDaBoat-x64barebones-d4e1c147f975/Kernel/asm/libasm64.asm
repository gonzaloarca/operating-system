GLOBAL getTimeRTC
GLOBAL canReadKey
GLOBAL getScanCode
GLOBAL sys_getCPUTemp
GLOBAL saveRegistersASM
GLOBAL saveRegistersASMexcp
GLOBAL sys_getMemory

section .text
	

;-------------------------------------------------------
;	Regresa el tiempo actual en un puntero a estructura
;-------------------------------------------------------
; Llamada en C:
; void getTimeRTC(TimeFormat *time);
;-------------------------------------------------------
getTimeRTC:
	push rbp
	mov rbp, rsp

	push rax
	push rcx
	push rdx

	;	Antes de poder leer la hora, debo saber si hay una actualizacion en progreso o no
.updateCheck:
	mov rax, 0Ah
	out 70h, al
	in al, 71h
	shr al, 7				; El bit de upgrade in progress es el 7mo
	cmp al, 0
	jne .updateCheck		; Si el bit era 1, debo controlar de nuevo

	;	En al se guarda el dato PERO en formato feo:
	;	Primeros 4 bits son la decena
	;	Ultimos 4 bits son la unidad
	mov rax, 0
	out 70h, al
	in al, 71h
	mov cl, al
	shr al, 4				; En "al" queda la decena
	mov edx, 10
	mul edx					; En eax esta el resultado de eax*edx
	and cl, 15				; En "cl" queda la unidad
	add al, cl				; Los sumo
	mov DWORD [rdi], eax	;	quedan los segundos en eax

	mov rax, 2
	out 70h, al
	in al, 71h
	mov cl, al
	shr al, 4				; En "al" queda la decena
	mov edx, 10
	mul edx					; En eax esta el resultado de eax*edx
	and cl, 15				; En "cl" queda la unidad
	add al, cl				; Los sumo
	mov DWORD [rdi+4], eax	;	Minutos

	mov rax, 4
	out 70h, al
	in al, 71h
	mov cl, al
	shr al, 4				; En "al" queda la decena
	mov edx, 10
	mul edx					; En eax esta el resultado de eax*edx
	and cl, 15				; En "cl" queda la unidad
	add al, cl				; Los sumo
	mov DWORD [rdi+8], eax	;	Horas

	pop rdx
	pop rcx
	pop rax

	mov rsp, rbp
	pop rbp
	ret

;-------------------------------------------------------
;	Indica si se puede leer el teclado
;	1 = Se puede
;	0 = No se puede
;-------------------------------------------------------
; Llamada en C:
; 	unsigned int canReadKey();
;-------------------------------------------------------
canReadKey:
	mov rax, 0
	in al, 64h
	and al, 1
	ret

;-------------------------------------------------------
;	Devuelve la lectura del teclado
;-------------------------------------------------------
; Llamada en C:
;	unsigned int getScanCode();
;-------------------------------------------------------
getScanCode:
	mov rax, 0
	in al, 60h
	ret

;-------------------------------------------------------
;	Devuelve la temperatura absoluta en ºC del CPU 
;
;	NOTA: Esta funcion utiliza estaticamente valores reales de registros MSR obtenidos de un CPU 
;		  Intel i7 de 7ma generacion. Utilizamos estos valores ya que QEMU siempre devuelve
;		  0 grados al intentar acceder a los registros MSR del procesador.
;		  Si se desea correr en un ambiente con los privilegios requeridos para llamar a rdmsr
;		  y para acceder a los MSR del CPU, comentar los dos mov a continuacion de los rdmsr,
;		  y descomentar los rdmsr
;
;-------------------------------------------------------
; Llamada en C:
;	int sys_getCPUTemp();
;-------------------------------------------------------
sys_getCPUTemp:			
	push rbp
	mov rbp, rsp

	push rcx
	push rdx
	push rbx

	mov rbx, 0					
	mov rax, 0			;vacio rax y rbx para tener los 32 bits mas significativos en 0
	mov ecx, 412	
	;rdmsr
	mov edx, 0
	mov eax, 0x88470800	;Obtengo el registro IA32_THERM_STATUS en edx:eax

	and eax, 0x7F0000	;en eax me quedan los bits 22:16 ()
	shr eax, 16			;ahora queda en eax el valor de la temperatura indicada por el monitor
	mov ebx, eax		;almaceno el valor en ebx

	mov rax, 0
	mov ecx, 418
	;rdmsr
	mov edx, 0		
	mov eax, 0x2640000	;Obtengo el registro MSR_TEMPERATURE_TARGET en edx:eax
	

	mov edx, eax		;Copio los 32 bits menos significativos del registro en edx
	and eax, 0xFF0000	;en eax me quedan los bits 23:16 (Temperature Target)
	shr eax, 16
	and edx, 0x3F000000	;en edx me quedan los bits 29:24 (Target Offset)
	shr edx, 24
			
	add eax, edx		;Obtengo la temperatura efectiva de alerta
	sub eax, ebx		;Obtengo la temperatura efectiva del procesador haciendo T(alerta) - T(monitor) en eax

	pop rbx
	pop rdx
	pop rcx

	mov rsp, rbp
	pop rbp
	ret

;-------------------------------------------------------
;	Guarda en la estructura indicada por parametro los registros al tocar f1
;-------------------------------------------------------
; Llamada en C:
;	void saveRegistersASM(registersType reg)
;-------------------------------------------------------
saveRegistersASM:
	push rbp
	mov rbp, rsp

	mov rax, [rbp]		; rbp de saveRegisters de C

	mov rax, [rax]		; rbp de keyboardHandler

	mov rax, [rax]		; rbp de int_21

	mov rax, [rax]		; rbp de irqDispatcher

	; en rax tengo la posicion donde esta guardando el rbp viejo
	; en rax+8 retorno a irq Master
	; en rax+16 tengo el ultimo registro pusheado q es r15

	add rax, 16

	call fillRegisters

	mov rsp, rbp
	pop rbp
	ret


;-------------------------------------------------------
;	Guarda en la estructura indicada por parametro los registros al lanzarse una excepcion
;-------------------------------------------------------
; Llamada en C:
;	void saveRegistersASMexcp(registersType reg)
;-------------------------------------------------------
saveRegistersASMexcp:
	push rbp
	mov rbp, rsp

	mov rax, [rbp]		; rbp de zero_division

	mov rax, [rax]		; rbp de exceptionDispatcher

	; en rax tengo la posicion donde esta guardando el rbp viejo
	; en rax+8 retorno a exception Handler
	; en rax+16 tengo el ultimo registro pusheado q es r15

	add rax, 16

	call fillRegisters

	mov rsp, rbp
	pop rbp
	ret



;-------------------------------------------------------
;	Funcion interna para guardar en la estructura recibida en rax los 16 registros
;-------------------------------------------------------
; 	fillRegisters		
;			recibe en rax el puntero a r15 y en rdi el puntero a la estructura de tipo RegistersType
;			(sigue el comportamiento de como se pushean los registros en una interrupcion)
;-------------------------------------------------------
fillRegisters:
	push rbp
	mov rbp, rsp
	push rbx
	push rcx

	mov rcx, 15

.ciclo:
	dec rcx
	mov rbx, [rax]
	mov QWORD[rdi + 8*rcx], rbx
	add rax, 8
	cmp rcx, 0
	jnz .ciclo

	; ahora tengo RIP, CS, EFLAGS, RSP y SS que fueron guardados por la excepcion(no hay error code)

	mov rbx, [rax]
	mov QWORD[rdi + 8*16], rbx				;rip

	mov rbx, [rax+24]
	mov QWORD[rdi + 8*15], rbx				;rsp

	pop rcx
	pop rbx
	mov rsp, rbp
	pop rbp
	ret
	

;-------------------------------------------------------
;	Funcion que se encarga de volcar en la estructura recibida en rbx los 32 bytes de informacion encontrados a partir de la direccion recibida en rdx
;-------------------------------------------------------
; Llamada en C:
;	void sys_getMemory(memType* answer, char* address)
;-------------------------------------------------------

sys_getMemory:

	push rbp
	mov rbp, rsp

	push rbx; lo uso como auxiliar
	
	; en rdi se encuentra la estructura a retornar y en rsi la direccion
	; necesito imprimir 32 bytes

	mov rax, [rsi]
	mov QWORD[rdi], rax
	mov rax, [rsi + 8]		; avanzo 8 bytes
	mov QWORD[rdi + 8], rax
	mov rax, [rsi + 16]
	mov QWORD[rdi + 16], rax
	mov rax, [rsi + 24]
	mov QWORD[rdi + 24], rax

	pop rbx

	mov rsp, rbp
	pop rbp
	ret