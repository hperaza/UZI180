LIBS = uzilibc.lib

all: loader insboot

loader: loader.asz
	zxas -n $<
	zxlink -Z -W3 -Ptext=8000h/0,boot=0C000h/ -c -oloader loader.obj

insboot: insboot.c
	zxc -o -c -v $<
	zxlink -Z -W3 -Ptext=0,data,bss -C100H -o$@ crt.obj $@.obj $(LIBS)

clean:
	rm -f loader loader.obj insboot insboot.obj core *~ *.\$$\$$\$$

rmbak:
	rm -f *~ core *.\$$\$$\$$
