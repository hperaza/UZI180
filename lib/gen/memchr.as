	psect	text
	global	rcsv, cret, _memchr

_memchr:
	call	rcsv

1:
	ld	a,b
	or	c
	jr	z,2f
	ld	a,(hl)
	cp	e
	jp	z,cret
	inc	hl
	dec	bc
	jr	1b

2:	ld	hl,0
	jp	cret
