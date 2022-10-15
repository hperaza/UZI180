;---------------------------------------------------
; UZI180 kernel interface module
;
; function 42: times
;---------------------------------------------------

	psect	text,global,pure
	psect	data,global
	psect	bss,global

	psect	text

	global	_times, __times, __uzi

_times:
__times:
	ld	a,42
	jp	__uzi
