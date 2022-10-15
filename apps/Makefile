SHELL=/bin/sh

subdirs = cc curses find insboot make shells simple

all:
	@for i in ${subdirs}; do \
		echo Making all in $$i ; \
		(cd $$i; ${MAKE} all) ; \
	done

clean:
	@for i in ${subdirs}; do \
		echo Cleaning all in $$i ; \
		(cd $$i; ${MAKE} clean) ; \
	done
