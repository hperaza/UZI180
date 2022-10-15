SHELL=/bin/sh

subdirs = crt curses gen stdio termcap unistd uzi utils

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
	@echo Cleaning all in biglib
	@(cd biglib ; rm -f *.obj uzilibc.lib uzilibc.map uzilibc.mod checklib sortlib)

big-lib:
	@cp utils/checklib biglib
	@cp utils/sortlib biglib
	@rm -f biglib/*.obj
	@cp -i unistd/*.obj biglib
	@cp -i stdio/*.obj biglib
	@cp -i uzi/*.obj biglib
	@cp -i gen/*.obj biglib
	@( \
		cd biglib ; \
		rm -f libc.lib uzilibc.lib ; \
		echo "Making uzilibc.lib..." ; \
		zxcc libr -"<" +mkbiglib ; \
		zxcc libr -s uzilibc.lib > uzilibc.map ; \
		zxcc libr -m uzilibc.lib > uzilibc.mod ; \
		ls -al *.lib ; \
		./checklib uzilibc.map ; \
	)
