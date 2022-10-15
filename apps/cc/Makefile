.SUFFIXES: .c .obj

SRC = c.c
OBJ = $(SRC:.c=.obj)

.c.obj:
	zxc -I. -o -c -v $<

all: cc.com

cc.com: c.com patch-cpm.c
	@cc -o patch-cpm patch-cpm.c
	./patch-cpm c.com cc.com

c.com: $(OBJ)
	zxlink -Z -W3 -Ptext=0,data,bss -C100h -o$@ crtcpm.obj $< libc.lib

clean:
	rm -f $(OBJ) c.com cc.com patch-cpm core *~ $.\$$\$$\$$
