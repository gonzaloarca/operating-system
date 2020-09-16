GLOBAL getMemoryASM
GLOBAL executeZeroException
GLOBAL executeUIException
GLOBAL cpuModel
GLOBAL cpuVendor
GLOBAL cpuBrand
section .text

;-------------------------------------------------------
;	Indica el modelo y la familia del CPU
;-------------------------------------------------------
; Llamada en C:
;	void cpumodel(int buffer[2]);
;-------------------------------------------------------
cpuModel:
	push rbp
	mov rbp, rsp
	push rbx

	mov eax, 1
	cpuid

	; Me guardo el valor base de model
	mov ebx, eax
	shr ebx, 4
	and ebx, 0xF
	mov [rdi+4], ebx

	; Obtengo el FamilyID
	mov ebx, eax
	shr ebx, 8
	and ebx, 0xF
	; Me guardo su valor base
	mov [rdi], ebx

	; Veo si es 15 o 6
	cmp ebx, 15
	je .quince
	cmp ebx, 6
	je .seis
	jmp .return

	; Si es 15, necesito el Model y Family extendido
.quince:
	mov ebx, eax
	shr ebx, 20
	and ebx, 0xFF
	add DWORD [rdi], ebx
	; Si es 6, solo necesito el Model extendido
.seis:
	mov ebx, eax
	shr ebx, 16
	and ebx, 0xF
	shl ebx, 4
	add [rdi+4], ebx

.return:
	pop rbx
	mov rsp, rbp
	pop rbp
	ret

;-------------------------------------------------------
;	Indica el fabricante del CPU
;-------------------------------------------------------
; Llamada en C:
;	char *cpuVendor(char buffer[13]);
;-------------------------------------------------------
cpuVendor:
	push rbp
	mov rbp, rsp

	push rbx

	mov rax, 0
	cpuid

	mov [rdi], ebx
	mov [rdi + 4], edx
	mov [rdi + 8], ecx
	mov byte [rdi + 12], 0

	mov rax, rdi

	pop rbx

	mov rsp, rbp
	pop rbp
	ret

;-------------------------------------------------------
;	Indica la marca del CPU
;-------------------------------------------------------
; Llamada en C:
;	char *cpuBrand(char buffer[48]);
;-------------------------------------------------------
cpuBrand:
	push rbp
	mov rbp, rsp
	push rbx

	; Esta instruccion es para checkear si existe la info que queremos
	mov eax, 80000000h
	cpuid
	cmp eax, 80000004h
	jb .return

	mov rsi, rdi

	; Con las siguientes llamadas obtengo el string en partes
	mov eax, 80000002h
	cpuid
	mov [rdi], eax
	mov [rdi+4], ebx
	mov [rdi+8], ecx
	mov [rdi+12], edx
	add rdi, 16

	mov eax, 80000003h
	cpuid
	mov [rdi], eax
	mov [rdi+4], ebx
	mov [rdi+8], ecx
	mov [rdi+12], edx
	add rdi, 16

	mov eax, 80000004h
	cpuid
	mov [rdi], eax
	mov [rdi+4], ebx
	mov [rdi+8], ecx
	mov [rdi+12], edx

	mov rax, rsi

.return:
	pop rbx
	mov rsp, rbp
	pop rbp
	ret

;-------------------------------------------------------
; Lanza la excepcion de dividir por cero
;-------------------------------------------------------
executeZeroException:
	mov rax, 0
	div rax
	ret

;-------------------------------------------------------
; Lanza la excepcion de codigo invalido(undefined instruction)
;-------------------------------------------------------
executeUIException:
	UD2
	ret