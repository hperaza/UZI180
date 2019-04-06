;---------------------------------------------------
; UZI180 kernel interface module
;
; function 21: umask
;---------------------------------------------------

	psect	text,global,pure
	psect	data,global
	psect	bss,global

	psect	text

	global	_umask, __umask, __uzi

_umask:
__umask:
	ld	a,21
	jp	__uzi
