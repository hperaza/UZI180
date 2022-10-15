;---------------------------------------------------
; UZI180 kernel interface module
;
; function 14: chown
;---------------------------------------------------

	psect	text,global,pure
	psect	data,global
	psect	bss,global

	psect	text

	global	_chown, __chown, __uzi

_chown:
__chown:
	ld	a,14
	jp	__uzi
