;---------------------------------------------------
; UZI180 kernel interface module
;
; function 7: read
;---------------------------------------------------

	psect	text,global,pure
	psect	data,global
	psect	bss,global

	psect	text

	global	_read, __read, __uzi

_read:
__read:
	ld	a,7
	jp	__uzi
