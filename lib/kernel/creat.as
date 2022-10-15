;---------------------------------------------------
; UZI180 kernel interface module
;
; function 3: creat
;---------------------------------------------------

	psect	text,global,pure
	psect	data,global
	psect	bss,global

	psect	text

	global	_creat, __creat, __uzi

_creat:
__creat:
	ld	a,3
	jp	__uzi
