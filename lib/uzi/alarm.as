;---------------------------------------------------
; UZI180 kernel interface module
;
; function 38: alarm
;---------------------------------------------------

	psect	text,global,pure
	psect	data,global
	psect	bss,global

	psect	text

	global	_alarm, __alarm, __uzi

_alarm:
__alarm:
	ld	a,38
	jp	__uzi
