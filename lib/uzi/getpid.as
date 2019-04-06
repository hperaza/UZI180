;---------------------------------------------------
; UZI180 kernel interface module
;
; function 18: getpid
;---------------------------------------------------

	psect	text,global,pure
	psect	data,global
	psect	bss,global

	psect	text

	global	_getpid, __getpid, __uzi

_getpid:
__getpid:
	ld	a,18
	jp	__uzi
