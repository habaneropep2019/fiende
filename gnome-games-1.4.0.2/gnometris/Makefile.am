SUBDIRS=pix
scoredir=$(localstatedir)/games
Gamesdir=$(datadir)/gnome/apps/Games

INCLUDES=\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	$(GNOME_INCLUDEDIR)

bin_PROGRAMS=gnometris

gnometris_SOURCES=\
	main.cpp \
	blocks.cpp \
	blocks.h \
	preview.cpp \
	preview.h \
	blockops.cpp \
	blockops.h \
	field.cpp \
	field.h \
	tetris.cpp \
	tetris.h \
	scoreframe.cpp \
	scoreframe.h

gnometris_LDADD=\
	$(GNOME_LIBDIR) \
	$(GNOMEUI_LIBS) \
	$(INTLLIBS)


Games_DATA = gnometris.desktop

EXTRA_DIST = AUTHORS \
             gnometris.desktop 	

install-data-local:
	-$(mkinstalldirs) $(DESTDIR)$(scoredir)
	touch $(DESTDIR)$(scoredir)/gnometris.scores
	-chown games.games $(DESTDIR)$(scoredir)/gnometris.scores
	-chmod 664 $(DESTDIR)$(scoredir)/gnometris.scores
	-chgrp games $(DESTDIR)$(bindir)/gnometris && chmod 2111 $(DESTDIR)$(bindir)/gnometris







