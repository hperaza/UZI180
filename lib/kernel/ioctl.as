;---------------------------------------------------
; UZI180 kernel interface module
;
; function 29: ioctl
;---------------------------------------------------

	psect	text,global,pure
	psect	data,global
	psect	bss,global

	psect	text

	global	_ioctl, __ioctl, __uzi

_ioctl:
__ioctl:
	ld	a,29
	jp	__uzi
