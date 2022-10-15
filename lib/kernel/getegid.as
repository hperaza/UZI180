;---------------------------------------------------
; UZI180 kernel interface module
;
; function 45: getegid
;---------------------------------------------------

	psect	text,global,pure
	psect	data,global
	psect	bss,global

	psect	text

	global	_getegid, __getegid, __uzi

_getegid:
__getegid:
	ld	a,45
	jp	__uzi
