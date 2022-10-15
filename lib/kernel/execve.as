;---------------------------------------------------
; UZI180 kernel interface module
;
; function 23: execve
;---------------------------------------------------

	psect	text,global,pure
	psect	data,global
	psect	bss,global

	psect	text

	global	_execve, __execve, __uzi

_execve:
__execve:
	ld	a,23
	jp	__uzi
