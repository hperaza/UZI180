;---------------------------------------------------
; UZI180 kernel interface module
;
; function 33: mount
;---------------------------------------------------

	psect	text,global,pure
	psect	data,global
	psect	bss,global

	psect	text

	global	_mount, __mount, __uzi

_mount:
__mount:
	ld	a,33
	jp	__uzi
