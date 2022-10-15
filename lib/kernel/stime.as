;---------------------------------------------------
; UZI180 kernel interface module
;
; function 28: stime
;---------------------------------------------------

	psect	text,global,pure
	psect	data,global
	psect	bss,global

	psect	text

	global	_stime, __stime, __uzi

_stime:
__stime:
	ld	a,28
	jp	__uzi
