;---------------------------------------------------
; UZI180 kernel interface module
;
; function 43: utime
;---------------------------------------------------

	psect	text,global,pure
	psect	data,global
	psect	bss,global

	psect	text

	global	_utime, __utime, __uzi

_utime:
__utime:
	ld	a,43
	jp	__uzi
