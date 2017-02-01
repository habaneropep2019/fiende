## Process this file with automake to produce Makefile.in.

SUBDIRS = pixmaps cursors

# This is where the high score file goes ...
scoredir = $(localstatedir)/games

INCLUDES = $(GNOME_INCLUDEDIR) -DGNOMELOCALDIR=\""$(datadir)/local"\"
bin_PROGRAMS = gnome-xbill

gnome_xbill_SOURCES = Bucket.cc Bucket.h Cable.cc Cable.h Computer.cc \
                      Computer.h Game.cc Game.h Horde.cc Horde.h Library.cc \
                      Library.h MCursor.cc MCursor.h Monster.cc Monster.h \
                      Network.cc Network.h Picture.cc Picture.h Spark.cc \
                      Spark.h Strings.h UI.cc UI.h objects.h x11-gtk.cc x11.h

gnome_xbill_LDADD = $(GNOME_LIBDIR) $(GNOMEUI_LIBS)

EXTRA_DIST = README.Ports xbill.png xbill.desktop

pixmapdir = $(datadir)/pixmaps
pixmap_DATA = xbill.png

Gamesdir=$(datadir)/gnome/apps/Games
Games_DATA = xbill.desktop

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(scoredir)
	-touch $(DESTDIR)$(scoredir)/gnome-xbill.scores
	-chown games.games $(DESTDIR)$(scoredir)/gnome-xbill.scores
	-chmod 664 $(DESTDIR)$(scoredir)/gnome-xbill.scores
	-chgrp games $(DESTDIR)$(bindir)/gnome-xbill && chmod 2111 $(DESTDIR)$(bindir)/gnome-xbill

