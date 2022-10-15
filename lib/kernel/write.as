;---------------------------------------------------
; UZI180 kernel interface module
;
; function 8: write
;---------------------------------------------------

	psect	text,global,pure
	psect	data,global
	psect	bss,global

	psect	text

	global	_write, __write, __uzi

_write:
__write:
	ld	a,8
	jp	__uzi
