;---------------------------------------------------
; UZI180 kernel interface module
;
; function 5: link
;---------------------------------------------------

	psect	text,global,pure
	psect	data,global
	psect	bss,global

	psect	text

	global	_link, __link, __uzi

_link:
__link:
	ld	a,5
	jp	__uzi
