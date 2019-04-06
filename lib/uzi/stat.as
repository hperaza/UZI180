;---------------------------------------------------
; UZI180 kernel interface module
;
; function 15: stat
;---------------------------------------------------

	psect	text,global,pure
	psect	data,global
	psect	bss,global

	psect	text

	global	_stat, __stat, __uzi

_stat:
__stat:
	ld	a,15
	jp	__uzi
