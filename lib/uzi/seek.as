;---------------------------------------------------
; UZI180 kernel interface module
;
; function 9: seek
;---------------------------------------------------

	psect	text,global,pure
	psect	data,global
	psect	bss,global

	psect	text

	global	_seek, __seek, __uzi

_seek:
__seek:
	ld	a,9
	jp	__uzi
