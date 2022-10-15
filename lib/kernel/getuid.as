;---------------------------------------------------
; UZI180 kernel interface module
;
; function 20: getuid
;---------------------------------------------------

	psect	text,global,pure
	psect	data,global
	psect	bss,global

	psect	text

	global	_getuid, __getuid, __uzi

_getuid:
__getuid:
	ld	a,20
	jp	__uzi
