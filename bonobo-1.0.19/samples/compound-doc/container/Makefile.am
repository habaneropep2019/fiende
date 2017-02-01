GPRINT_LIBS = $(top_builddir)/bonobo/libbonobo-print.la

INCLUDES =	\
        -DGNOMELOCALEDIR=\""$(datadir)/locale"\"        \
	-I$(srcdir) -I$(top_srcdir) 			\
	-I. -I$(top_srcdir)/bonobo			\
        -I$(includedir)                                 \
	$(BONOBOX_TEST_CFLAGS)

bin_PROGRAMS = sample-container

sample_container_SOURCES =	\
	container.c		\
	container.h		\
	component.c		\
	component.h		\
	container-io.h		\
	container-io.c		\
	container-filesel.h	\
	container-filesel.c	\
	container-menu.h	\
	container-menu.c	\
	container-print.h	\
	container-print.c	\
	embeddable-io.c		\
	embeddable-io.h

sample_container_LDADD =				\
	$(top_builddir)/bonobo/libbonobo.la		\
	$(top_builddir)/bonobo/libbonobox.la		\
	$(top_builddir)/bonobo/libbonobo-print.la	\
	$(BONOBOX_TEST_LIBS)				\
	$(INTLLIBS)
