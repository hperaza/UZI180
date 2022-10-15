;---------------------------------------------------
; UZI180 kernel interface module
;
; function 10: chdir
;---------------------------------------------------

	psect	text,global,pure
	psect	data,global
	psect	bss,global

	psect	text

	global	_chdir, __chdir, __uzi

_chdir:
__chdir:
	ld	a,10
	jp	__uzi
