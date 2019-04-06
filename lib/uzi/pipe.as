;---------------------------------------------------
; UZI180 kernel interface module
;
; function 40: pipe
;---------------------------------------------------

	psect	text,global,pure
	psect	data,global
	psect	bss,global

	psect	text

	global	_pipe, __pipe, __uzi

_pipe:
__pipe:
	ld	a,40
	jp	__uzi
