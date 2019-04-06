;---------------------------------------------------
; UZI180 kernel interface module
;
; function 1: open
;---------------------------------------------------

	psect	text,global,pure
	psect	data,global
	psect	bss,global

	psect	text

	global	_open, __open, __uzi

_open:
__open:
	ld	a,1
	jp	__uzi
