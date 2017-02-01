SUBDIRS = help

Gamesdir = $(datadir)/gnome/apps/Games
examplesdir = $(datadir)/gturing/examples
icondir=$(datadir)/pixmaps

INCLUDES = 			\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	-I$(includedir)				 \
	$(GNOME_INCLUDEDIR)			 \
	-DVERSION=\""$(VERSION)"\" \
	-DGTURING_EXAMPLES_DIR=\""$(examplesdir)/"\"

bin_PROGRAMS = gturing

icon_DATA = gnome-gturing.png

gturing_SOURCES =	\
	gturing.c	\
	turing.h	\
	turing.c

gturing_LDADD = \
	$(GNOME_LIBDIR) \
	$(GNOMEUI_LIBS)	\
	$(INTLLIBS)

examples_DATA = 3ones2zeroes.tur add.tur addbin.tur bin2dec.tur dec2bin.tur \
	subst.tur dashnumbers.tur bb2.tur bb3.tur bb5.tur

Games_DATA = gturing.desktop

EXTRA_DIST = play.xpm power.xpm step.xpm stop.xpm \
	README CREDITS $(examples_DATA) $(Games_DATA) $(icon_DATA)

install-data-local:
	-chgrp games $(DESTDIR)$(bindir)/gturing && chmod 2111 $(DESTDIR)$(bindir)/gturing
