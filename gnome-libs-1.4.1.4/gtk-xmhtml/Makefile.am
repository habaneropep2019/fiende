## Procesed by automake to procduce Makefile.in

libgtkxmhtmlincludedir = $(includedir)/gnome-1.0/gtk-xmhtml

INCLUDES = -I.. -I$(srcdir)/.. -I$(includedir) @GTK_CFLAGS@ \
           -DWITH_GTK -DDEBUG -DGNOME_LIB

lib_LTLIBRARIES = libgtkxmhtml.la

libgtkxmhtml_la_SOURCES =	\
	XmHTML.c		\
	gtk-error.c		\
	gtk-forms.c		\
	frames.c		\
	parse.c			\
	StringUtil.c		\
	fonts.c			\
	paint.c			\
	map.c			\
	callbacks.c		\
	format.c		\
	plc.c			\
	readXPM.c		\
	readJPEG.c		\
	readJPEGplc.c		\
	readBitmap.c		\
	readGIF.c		\
	LZWStream.c		\
	LZWStream.h		\
	readGIFplc.c		\
	readFLG.c		\
	readPNG.c		\
	colors.c		\
	images.c		\
	quantize.c		\
	layout.c		\
	events.c		\
	debug.c			\
	XmHTML-motif.h		\
	XmHTMLfuncs.h		\
	debug.h			\
	escapes.h		\
	motif.h			\
	plc.h

libgtkxmhtmlinclude_HEADERS =	\
	miniparse.h		\
	HTML.h			\
	XmHTML.h		\
	XmHTMLI.h		\
	XmHTMLP.h		\
	gtk-xmhtml-p.h		\
	gtk-xmhtml.h		\
	toolkit.h

libgtkxmhtml_la_LDFLAGS = -version-info 1:1:0 -rpath $(libdir)
libgtkxmhtml_la_LIBADD = $(LIBGTKXMHTML_LIBS)

noinst_PROGRAMS = htmltest

htmltest_SOURCES = test.c

htmltest: libgtkxmhtml.la 

htmltest_LDADD   = libgtkxmhtml.la  $(LIBGTKXMHTML_LIBS) $(INTLLIBS)

## These are included by XmHTML.c:

EXTRA_DIST = gtk-xmhtml.c XmHTML-motif.c 

dist-hook:
	mkdir $(distdir)/bitmaps
	cp $(srcdir)/bitmaps/*.xpm $(srcdir)/bitmaps/*.xbm $(distdir)/bitmaps

## If I dont do this, for some wierdo reason, it tries to compile gtk-xmhtml.o
#gtk-xmhtml:
#	echo
