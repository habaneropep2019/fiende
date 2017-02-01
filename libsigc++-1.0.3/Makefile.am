SUBDIRS = sigc++ tests 
DIST_SUBDIRS = sigc++ examples tests scripts

bin_SCRIPTS = sigc-config

m4datadir = $(datadir)/aclocal
m4data_DATA = sigc++.m4

EXTRA_DIST =  \
  FEATURES 		acconfig.h \
  IDEAS 		autogen.sh 		ltdll.c \
  libsigc++.spec.in 	sigc-config.in 		sigc++.m4 \
  win32/README		win32/libsigc.dsp  	win32/wintest.dsp \
  win32/libsigc.dsw  	win32/wintest.cpp  \
  djgpp/Makefile.dj  	djgpp/README  		djgpp/sigc++config.h \
  riscos/riscos.s 	riscos/sprites		riscos/sprites22 \
  riscos/sigc++config	riscos/README \
  doc/API		doc/UML			doc/diagrams \
  doc/signals		doc/conventions		doc/powerusers \
  doc/requirements	doc/win32 		doc/FAQ  \
  doc/riscos 		doc/marshal \
  borland/README        borland/Makefile        borland/wintest.cpp
  
dist-hook:
	cp libsigc++.spec $(distdir)
	cp sigc++/config/sigc++config.h win32

vc-dist:
	./win32/makevc.sh

riscos-dist:
	tar xzvf libsigc++-@VERSION@.tar.gz
	(cd libsigc++-@VERSION@/riscos; \
        ./riscos.s ; \
	find ./\!LibSig | zip -@ libsigc++-riscos-@VERSION@.zip stype,feb ; \
        cp libsigc++-riscos-@VERSION@.zip ../.. )
	rm -Rf libsigc++-@VERSION@

