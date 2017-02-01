GPRINT_LIBS = $(top_builddir)/bonobo/libbonobo-print.la

INCLUDES =						\
        -DGNOMELOCALEDIR=\""$(datadir)/locale"\"        \
	-I$(srcdir) -I$(top_srcdir) 			\
	-I. -I$(top_srcdir)/bonobo			\
        -I$(includedir)                                 \
	$(BONOBOX_TEST_CFLAGS)

bin_PROGRAMS = bonobo-sample-hello

bonobo_sample_hello_SOURCES =	\
	bonobo-hello.c		\
	hello-embeddable.c	\
	hello-embeddable.h	\
	hello-object-io.h	\
	hello-object-io.c	\
	hello-view.c		\
	hello-view.h		\
	hello-object-print.h	\
	hello-object-print.c

bonobo_sample_hello_LDADD =				\
	$(top_builddir)/bonobo/libbonobo.la		\
	$(top_builddir)/bonobo/libbonobox.la		\
	$(top_builddir)/bonobo/libbonobo-print.la	\
	$(BONOBOX_TEST_LIBS)				\
	$(INTLLIBS)

oafdir = $(datadir)/oaf
oaf_DATA = Bonobo_Sample_Hello.oafinfo

EXTRA_DIST = $(oaf_DATA)
