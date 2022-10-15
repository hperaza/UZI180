;---------------------------------------------------
; UZI180 kernel interface module
;
; function 26: setgid
;---------------------------------------------------

	psect	text,global,pure
	psect	data,global
	psect	bss,global

	psect	text

	global	_setgid, __setgid, __uzi

_setgid:
__setgid:
	ld	a,26
	jp	__uzi
