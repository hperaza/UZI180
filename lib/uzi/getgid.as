;---------------------------------------------------
; UZI180 kernel interface module
;
; function 41: getgid
;---------------------------------------------------

	psect	text,global,pure
	psect	data,global
	psect	bss,global

	psect	text

	global	_getgid, __getgid, __uzi

_getgid:
__getgid:
	ld	a,41
	jp	__uzi
