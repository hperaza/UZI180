;---------------------------------------------------
; UZI180 kernel interface module
;
; function 31: sbrk
;---------------------------------------------------

	psect	text,global,pure
	psect	data,global
	psect	bss,global

	psect	text

	global	_sbrk, __sbrk, __uzi

_sbrk:
__sbrk:
	ld	a,31
	jp	__uzi
