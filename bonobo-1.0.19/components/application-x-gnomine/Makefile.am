scoredir=$(localstatedir)/games
Gamesdir = $(datadir)/gnome/apps/Games

INCLUDES = -I$(top_srcdir)				\
	-I$(top_builddir)				\
	-I$(top_srcdir)/bonobo				\
	-I$(top_builddir)/bonobo			\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" 	\
	-DDATADIR=\""$(datadir)"\"			\
	-I$(includedir)				 	\
	$(BONOBOX_TEST_CFLAGS)

#bin_PROGRAMS = gnomine bonobo-application-x-mines
bin_PROGRAMS = bonobo-application-x-mines

#gnomine_SOURCES =	\
#	gnomine.c	\
#	minefield.h	\
#	minefield.c
#
#gnomine_LDADD = \
#	$(GNOME_LIBDIR) \
#	$(GNOMEUI_LIBS) \
#	$(INTLLIBS)

bonobo_application_x_mines_SOURCES = 	\
	minefield.h			\
	minefield.c			\
	bonobo-application-x-mines.c

bonobo_application_x_mines_LDADD =		\
	$(top_builddir)/bonobo/libbonobo.la	\
	$(top_builddir)/bonobo/libbonobox.la	\
	$(BONOBOX_TEST_LIBS)

#Games_DATA = gnomine.desktop

#install-data-local:
#	-$(mkinstalldirs) $(DESTDIR)$(scoredir)
#	-touch $(DESTDIR)$(scoredir)/gnomine.Tiny.scores
#	-chown games.games $(DESTDIR)$(scoredir)/gnomine.Tiny.scores
#	-chmod 664 $(DESTDIR)$(scoredir)/gnomine.Tiny.scores	
#	-touch $(DESTDIR)$(scoredir)/gnomine.Medium.scores
#	-chown games.games $(DESTDIR)$(scoredir)/gnomine.Medium.scores
#	-chmod 664 $(DESTDIR)$(scoredir)/gnomine.Medium.scores	
#	-touch $(DESTDIR)$(scoredir)/gnomine.Biiiig.scores
#	-chown games.games $(DESTDIR)$(scoredir)/gnomine.Biiiig.scores
#	-chmod 664 $(DESTDIR)$(scoredir)/gnomine.Biiiig.scores	
#	-touch $(DESTDIR)$(scoredir)/gnomine.Custom.scores
#	-chown games.games $(DESTDIR)$(scoredir)/gnomine.Custom.scores
#	-chmod 664 $(DESTDIR)$(scoredir)/gnomine.Custom.scores	
#	-chgrp games $(DESTDIR)$(bindir)/gnomine && chmod 2111 $(DESTDIR)$(bindir)/gnomine


oaf_in_files = Bonobo_Sample_Mines.oaf.in 
oafdir = $(datadir)/oaf
oaf_DATA = $(oaf_in_files:.oaf.in=.oaf)

@XML_I18N_MERGE_OAF_RULE@

EXTRA_DIST = flag.xpm mine.xpm face-cool.xpm face-sad.xpm \
	face-smile.xpm face-win.xpm face-worried.xpm \
	gnomine.desktop README AUTHORS $(oaf_in_files) $(oaf_DATA) \
	bonobo-application-x-mines-ui.xml

