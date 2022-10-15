;---------------------------------------------------
; UZI180 kernel interface module
;
; function 25: setuid
;---------------------------------------------------

	psect	text,global,pure
	psect	data,global
	psect	bss,global

	psect	text

	global	_setuid, __setuid, __uzi

_setuid:
__setuid:
	ld	a,25
	jp	__uzi
