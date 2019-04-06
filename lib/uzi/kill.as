;---------------------------------------------------
; UZI180 kernel interface module
;
; function 39: kill
;---------------------------------------------------

	psect	text,global,pure
	psect	data,global
	psect	bss,global

	psect	text

	global	_kill, __kill, __uzi

_kill:
__kill:
	ld	a,39
	jp	__uzi
