Audiodir = $(datadir)/gnome/apps/Multimedia

pixmapdir = $(datadir)/pixmaps

INCLUDES = \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	$(GNOME_INCLUDEDIR)			\
	-DGNOME_ICONDIR=\""$(pixmapdir)"\"

bin_PROGRAMS = vumeter

vumeter_SOURCES = \
	gtkled.c	\
	gtkled.h	\
	gtkledbar.c	\
	gtkledbar.h	\
	main.c

vumeter_LDADD = \
	$(GNOME_LIBDIR) $(GNOMEUI_LIBS) $(INTLLIBS)

BITMAPS = gnome-vumeter.png

pixmap_DATA = $(BITMAPS)

EXTRA_DIST = $(BITMAPS) vumeter.desktop.in README TODO

Audio_in_files = vumeter.desktop.in
Audio_DATA = $(Audio_in_files:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@
