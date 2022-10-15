;-----------------------------------------------------------------------
; UZI180 kernel interface module
;
; Execute kernel function
; Copyright (C) 1999, 2000, 2001 Hector Peraza
;-----------------------------------------------------------------------

	psect   text,global,pure
	psect   data,global
	psect   bss,global

	psect   text

	global  __uzi, _errno

__uzi:
	pop     de		; pop return address from stack and
				; save it in DE, so parameters are passed
				; correctly to the uzi kernel call
	ld      l,a
	ld      h,0
	push    hl		; push function number onto stack
	rst     30h		; note that the kernel function call saves
				; and restores all the registers, including
				; alternates, except HL (which contains the
				; return code), and AF

	pop     bc		; remove function number from stack
	push    de		; and push return address back
	ret	nc
	ld      (_errno),hl
	ld      hl,-1
	ret

	psect	bss

_errno:	defs	2
