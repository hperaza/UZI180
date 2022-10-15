;---------------------------------------------------
; UZI180 kernel interface module
;
; function 22: getfsys
;---------------------------------------------------

	psect	text,global,pure
	psect	data,global
	psect	bss,global

	psect	text

	global	_getfsys, __getfsys, __uzi

_getfsys:
__getfsys:
	ld	a,22
	jp	__uzi
