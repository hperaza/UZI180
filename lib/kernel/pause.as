;---------------------------------------------------
; UZI180 kernel interface module
;
; function 37: pause
;---------------------------------------------------

	psect	text,global,pure
	psect	data,global
	psect	bss,global

	psect	text

	global	_pause, __pause, __uzi

_pause:
__pause:
	ld	a,37
	jp	__uzi
