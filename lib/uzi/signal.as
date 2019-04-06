;---------------------------------------------------
; UZI180 kernel interface module
;
; function 35: signal
;---------------------------------------------------

	psect	text,global,pure
	psect	data,global
	psect	bss,global

	psect	text

	global	_signal, __signal, __uzi

_signal:
__signal:
	ld	a,35
	jp	__uzi
