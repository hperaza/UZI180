;---------------------------------------------------
; UZI180 kernel interface module
;
; function 4: mknod
;---------------------------------------------------

	psect	text,global,pure
	psect	data,global
	psect	bss,global

	psect	text

	global	_mknod, __mknod, __uzi

_mknod:
__mknod:
	ld	a,4
	jp	__uzi
