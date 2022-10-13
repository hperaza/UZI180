LIBS = uzilibc.lib

all: loader hdloader insboot

loader: loader.asz
	zxas -n $<
	zxlink -Z -W3 -Ptext=8000h/0,boot=0C000h/ -c -oloader loader.obj

hdloader: hdloader.asz
	zxas -n $<
	zxlink -Z -W3 -Ptext=8000h/0,boot=0C000h/ -c -ohdloader hdloader.obj

insboot: insboot.c
	zxc -o -c -v $<
	zxlink -Z -W3 -Ptext=0,data,bss -C100H -o$@ crt.obj $@.obj $(LIBS)

clean:
	rm -f loader loader.obj
	rm -f hdloader hdloader.obj
	rm -f insboot insboot.obj
	rm -f core *~ *.\$$\$$\$$

rmbak:
	rm -f *~ core *.\$$\$$\$$
