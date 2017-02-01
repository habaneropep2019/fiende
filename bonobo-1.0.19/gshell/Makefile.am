INCLUDES = 						\
	-I$(top_srcdir) -I$(top_srcdir)/bonobo 		\
	-I$(top_builddir) 				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-DDATADIR=\""$(datadir)"\"			\
        -I$(includedir)					\
	$(BONOBOX_TEST_CFLAGS)

bin_PROGRAMS = gshell

gshell_SOURCES = 	\
	gshell.h 	\
	gshell.c 	\
	inout.h		\
	inout.c		\
	ui.h		\
	ui.c		\
	properties.c	\
	properties.h	\
	controls.c	\
	controls.h

uidir	= $(datadir)/gnome/ui
ui_DATA	= gshell-ui.xml

gshell_LDADD = 					\
	$(top_builddir)/bonobo/libbonobo.la	\
	$(top_builddir)/bonobo/libbonobox.la	\
	$(BONOBOX_TEST_LIBS)			\
        $(INTLLIBS)

uidir = $(datadir)/gnome/ui
ui_DATA = gshell-ui.xml

EXTRA_DIST = $(ui_DATA)
