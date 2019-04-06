;	memmove(void *dst, const void *src, size_t n);

;	Copies n bytes from memory area src to memory area dst.
;	The memory areas may overlap.


	global	_memmove
	psect	text

_memmove:
	pop	hl		; return address
	exx
	pop	de		; dst
	pop	hl		; src
	pop	bc		; n

	ld	a,b
	or	c
	jr	z,1f

	ld	a,l
	sub	e
	ld	a,h
	sbc	a,d
	jr	c,2f
	ldir
	jr	1f

2:
	add	hl,bc
	ex	de,hl
	add	hl,bc
	ex	de,hl
	dec	hl
	dec	de
	lddr

1:
	push	bc		; stack is as it was
	push	de
	push	hl
	exx
	jp	(hl)
