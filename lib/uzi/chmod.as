;---------------------------------------------------
; UZI180 kernel interface module
;
; function 13: chmod
;---------------------------------------------------

	psect	text,global,pure
	psect	data,global
	psect	bss,global

	psect	text

	global	_chmod, __chmod, __uzi

_chmod:
__chmod:
	ld	a,13
	jp	__uzi
