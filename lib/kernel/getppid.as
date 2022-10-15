;---------------------------------------------------
; UZI180 kernel interface module
;
; function 19: getppid
;---------------------------------------------------

	psect	text,global,pure
	psect	data,global
	psect	bss,global

	psect	text

	global	_getppid, __getppid, __uzi

_getppid:
__getppid:
	ld	a,19
	jp	__uzi
