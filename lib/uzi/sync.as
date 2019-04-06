;---------------------------------------------------
; UZI180 kernel interface module
;
; function 11: sync
;---------------------------------------------------

	psect	text,global,pure
	psect	data,global
	psect	bss,global

	psect	text

	global	_sync, __sync, __uzi

_sync:
__sync:
	ld	a,11
	jp	__uzi
