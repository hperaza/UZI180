;---------------------------------------------------
; UZI180 kernel interface module
;
; function 32: fork
;---------------------------------------------------

	psect	text,global,pure
	psect	data,global
	psect	bss,global

	psect	text

	global	_fork, __fork, __uzi

_fork:
__fork:
	ld	a,32
	jp	__uzi
