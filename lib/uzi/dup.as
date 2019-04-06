;---------------------------------------------------
; UZI180 kernel interface module
;
; function 17: dup
;---------------------------------------------------

	psect	text,global,pure
	psect	data,global
	psect	bss,global

	psect	text

	global	_dup, __dup, __uzi

_dup:
__dup:
	ld	a,17
	jp	__uzi
