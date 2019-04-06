;---------------------------------------------------
; UZI180 kernel interface module
;
; function 16: fstat
;---------------------------------------------------

	psect	text,global,pure
	psect	data,global
	psect	bss,global

	psect	text

	global	_fstat, __fstat, __uzi

_fstat:
__fstat:
	ld	a,16
	jp	__uzi
