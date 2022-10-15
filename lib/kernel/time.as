;---------------------------------------------------
; UZI180 kernel interface module
;
; function 27: time
;---------------------------------------------------

	psect	text,global,pure
	psect	data,global
	psect	bss,global

	psect	text

	global	_time, __time, __uzi

_time:
__time:
	ld	a,27
	jp	__uzi
