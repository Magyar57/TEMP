bits 16

section _TEXT class=CODE

; void _cdecl x86_div64_32(uint64_t dividend, uint32_t divisor, uint64_t* quotientOut, uint32_t* remainderOut);
global _x86_div64_32
_x86_div64_32:
	; make new call frame
	push bp
	mov bp, sp
	; save registers that we'll be using
	push bx

	; divide upper 32 bits
	mov eax, [bp + 8] ; eax <- upper 32 bits of dividend
	mov ecx, [bp + 12] ; exc <- divisor
	xor edx, edx ; edx = 0
	div ecx	; eax - quot, edx - remainder

	; store upper 32 bits of quotient
	mov bx, [bp + 16]
	mov [bx + 4], eax

	; divide lower 32 bits
	mov eax, [bp + 4] ; eax <- lower 32 bits of dividend
	; edx elready contains the old remainder, nothing to assign
	div ecx

	; store results
	mov [bx], eax
	mov bx, [bp + 18]
	mov [bx], edx

	pop bx ; restore used register
	; restore call frame
	mov sp, bp
	pop bp
	ret

;
; int 10h ah=0eh
; args: character, page
;
global _x86_Video_WriteCharTeletype
_x86_Video_WriteCharTeletype:
	; make new call frame
	push bp			; save old call frame
	mov bp, sp		; init new call frame

	; save bx (we're using it later in the function)
	push bx

	; [bp + 0] - old call frame
	; [bp + 2] - return address (small memory model => 2 bytes)
	; [bp + 4] - first function argument (a char) ; bytes are converted to words, you can't push a byte on the stack. We're in 16 bits mode, a word is 2 bytes
	; [bp + 6] - second arg (page)
	
	; 0eh interruption : print a character to the screen
	mov ah, 0eh
	mov al, [bp + 4]
	mov bh, [bp + 6]
	int 10h

	; restore used registers
	pop bx

	; restore old call frame
	mov sp, bp
	pop bp

	ret
