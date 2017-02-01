SUBDIRS  = objects graphics help

Gamesdir = $(datadir)/gnome/apps/Games
scoredir =$(localstatedir)/games
Pixmapdir= $(datadir)/pixmaps
Cavesdir = $(datadir)/gnome-stones


INCLUDES =					\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-DGNOMEPIXMAPDIR=\""$(datadir)/pixmaps"\"	\
	-DCAVESDIR=\""$(Cavesdir)"\"			\
	-I$(includedir)					\
	$(GNOME_INCLUDEDIR)


bin_PROGRAMS = gnome-stones


gnome_stones_SOURCES =				\
        main.c					\
	types.h					\
	game.c					\
	game.h					\
	cave.c					\
	cave.h					\
	object.c				\
	object.h				\
	player.c				\
	player.h				\
	status.c				\
	status.h                                \
	io.c                                    \
	io.h                                    \
	preferences.c                           \
	preferences.h                           \
	view.c                                  \
	view.h

gnome_stones_LDADD =				\
	$(GNOME_LIBDIR)				\
	$(GNOMEUI_LIBS)				\
	$(INTLLIBS)


Pixmap_DATA= gnome-stones.png gnome-stones-file.png
Games_DATA = gnome-stones.desktop
Caves_DATA = default.caves tutorial.caves gnomekoban.caves

gtkrcdir = $(datadir)
gtkrc_DATA = gnome-stonesrc gnome-stonesrc.ko

Mimedir    = $(datadir)/mime-info
Mime_DATA  = gnome-stones.keys gnome-stones.mime

EXTRA_DIST    =					\
	$(Pixmap_DATA)				\
	$(Games_DATA)				\
	$(Caves_DATA)				\
	$(gtkrc_DATA)				\
	TODO					\
	README					\
	gnome-stones.keys.in			\
	gnome-stones.mime			\
	gnome-stones-file.png

install-data-local:
	-$(mkinstalldirs) $(DESTDIR)$(scoredir)
	-touch $(DESTDIR)$(scoredir)/gnome-stones.scores
	-chown games.games $(DESTDIR)$(scoredir)/gnome-stones.scores
	-chmod 664 $(DESTDIR)$(scoredir)/gnome-stones.scores	
	-chgrp games $(DESTDIR)$(bindir)/gnome-stones && chmod 2111 $(DESTDIR)$(bindir)/gnome-stones

gnome-stones.keys: gnome-stones.keys.in Makefile
	sed -e 's^\@icondir\@^$(Pixmapdir)^g' < $(srcdir)/gnome-stones.keys.in > gnome-stones.keys.tmp \
	&& mv gnome-stones.keys.tmp gnome-stones.keys 
