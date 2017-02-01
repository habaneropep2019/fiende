## Process this file with automake to produce Makefile.in

INCLUDES = \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	-I$(top_srcdir)/intl \
	$(GNOME_INCLUDEDIR)	\
	-DGNOME_ICONDIR=\""$(datadir)/pixmaps"\"

bin_PROGRAMS = grecord

grecord_SOURCES = \
	main.c \
	gui.c gui.h \
	grec.c grec.h \
	preferences.c preferences.h \
	sound.c sound.h \
	prog.h

grecord_LDADD = $(GNOME_LIBDIR) $(GNOMEUI_LIBS) $(INTLLIBS)

