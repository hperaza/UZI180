;---------------------------------------------------
; UZI180 kernel interface module
;
; function 34: umount
;---------------------------------------------------

	psect	text,global,pure
	psect	data,global
	psect	bss,global

	psect	text

	global	_umount, __umount, __uzi

_umount:
__umount:
	ld	a,34
	jp	__uzi
