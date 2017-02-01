scoredir=$(localstatedir)/games
pixmapdir = $(datadir)/pixmaps/mahjongg
bgdir = $(datadir)/pixmaps/mahjongg/bg
Gamesdir = $(datadir)/gnome/apps/Games
appicondir=$(datadir)/pixmaps

INCLUDES = -I.. -I$(srcdir)/..		\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	-I$(includedir)			\
	$(GNOME_INCLUDEDIR)

bin_PROGRAMS = mahjongg

mahjongg_SOURCES = 	\
	mahjongg.c	\
	solubility.c	\
	solubility.h	\
	mahjongg.h	\
	button-images.h

mahjongg_LDADD = \
	$(GNOME_LIBDIR) \
	$(GNOMEUI_LIBS)	\
	$(INTLLIBS)

pixmap_DATA = default.png seta.png

bg_DATA = bg1.png bg2.png bg3.png bg4.png

appicon_DATA = gnome-mahjongg.png

EXTRA_DIST = mahjongg.desktop $(pixmap_DATA) $(appicon_DATA) $(bg_DATA)

Games_DATA = mahjongg.desktop

SUBDIRS = help

install-data-local:
	-$(mkinstalldirs) $(DESTDIR)$(scoredir)
	$(mkinstalldirs) $(DESTDIR)$(bgdir)
	touch $(DESTDIR)$(scoredir)/mahjongg.{easy,difficult}.scores
	-chown games.games $(DESTDIR)$(scoredir)/mahjongg.{easy,difficult}.scores
	-chmod 664 $(DESTDIR)$(scoredir)/mahjongg.{easy,difficult}.scores
	-chgrp games $(DESTDIR)$(bindir)/mahjongg && chmod 2111 $(DESTDIR)$(bindir)/mahjongg
	$(mkinstalldirs) $(DESTDIR)$(Gamesdir)



