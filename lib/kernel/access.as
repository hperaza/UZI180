;---------------------------------------------------
; UZI180 kernel interface module
;
; function 12: access
;---------------------------------------------------

	psect	text,global,pure
	psect	data,global
	psect	bss,global

	psect	text

	global	_access, __access, __uzi

_access:
__access:
	ld	a,12
	jp	__uzi

