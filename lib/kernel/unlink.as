;---------------------------------------------------
; UZI180 kernel interface module
;
; function 6: unlink
;---------------------------------------------------

	psect	text,global,pure
	psect	data,global
	psect	bss,global

	psect	text

	global	_unlink, __unlink, __uzi

_unlink:
__unlink:
	ld	a,6
	jp	__uzi
