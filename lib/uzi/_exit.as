;---------------------------------------------------
; UZI180 kernel interface module
;
; function 0: exit
;---------------------------------------------------

	psect	text,global,pure
	psect	data,global
	psect	bss,global

	psect	text

	global	__exit, __sync, __uzi

__exit:				; return code is already on stack
;	call	__sync
	ld	a,0
	jp	__uzi
