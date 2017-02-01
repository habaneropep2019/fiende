## Process this file with automake to produce Makefile.in

CFLAGS=-g

INCLUDES = \
	-I$(top_srcdir)/intl 			\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
        -DGLADEDIR=\""$(gladedir)"\"           	\
	$(GNOME_INCLUDEDIR)			\
	$(GLADE_CFLAGS)

bin_PROGRAMS = gfloppy

gladedir = $(pkgdatadir)/glade
glade_DATA = gfloppy.glade

gfloppy-strings.c: gfloppy.glade
	libglade-xgettext -c -o $(srcdir)/gfloppy-strings.c $(srcdir)/gfloppy.glade

gfloppy_SOURCES = \
	main.c \
	gfloppy.c \
	progress.c \
	fdformat.c \
	badblocks.c

gfloppy_LDADD = $(GNOME_LIBDIR) $(GNOMEUI_LIBS) $(INTLLIBS) -lxml -lglade -lglade-gnome -lext2fs

man_MANS = gfloppy.1

EXTRA_DIST = 		     \
	gfloppy.glade		\
	gfloppy.h		\
	progress.h		\
	fdformat.h		\
	badblocks.h             \
	gfloppy-i18n.h		\
	gfloppy.1


update-po: gfloppy-strings.c
	cd ../po && make update-po
