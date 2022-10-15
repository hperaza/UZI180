;---------------------------------------------------
; UZI180 kernel interface module
;
; function 30: brk
;---------------------------------------------------

	psect	text,global,pure
	psect	data,global
	psect	bss,global

	psect	text

	global	_brk, __brk, __uzi

_brk:
__brk:
	ld	a,30
	jp	__uzi

