;---------------------------------------------------
; UZI180 kernel interface module
;
; function 2: close
;---------------------------------------------------

	psect	text,global,pure
	psect	data,global
	psect	bss,global

	psect	text

	global	_close, __close, __uzi

_close:
__close:
	ld	a,2
	jp	__uzi
