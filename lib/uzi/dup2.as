;---------------------------------------------------
; UZI180 kernel interface module
;
; function 36: dup2
;---------------------------------------------------

	psect	text,global,pure
	psect	data,global
	psect	bss,global

	psect	text

	global	_dup2, __dup2, __uzi

_dup2:
__dup2:
	ld	a,36
	jp	__uzi
