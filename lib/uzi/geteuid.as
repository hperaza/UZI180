;---------------------------------------------------
; UZI180 kernel interface module
;
; function 44: geteuid
;---------------------------------------------------

	psect	text,global,pure
	psect	data,global
	psect	bss,global

	psect	text

	global	_geteuid, __geteuid, __uzi

_geteuid:
__geteuid:
	ld	a,44
	jp	__uzi
