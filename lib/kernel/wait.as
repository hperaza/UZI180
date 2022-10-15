;---------------------------------------------------
; UZI180 kernel interface module
;
; function 24: wait
;---------------------------------------------------

	psect	text,global,pure
	psect	data,global
	psect	bss,global

	psect	text

	global	_wait, __wait, __uzi

_wait:
__wait:
	ld	a,24
	jp	__uzi
