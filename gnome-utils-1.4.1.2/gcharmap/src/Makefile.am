bin_PROGRAMS = gcharmap

INCLUDES =\
	-DG_LOG_DOMAIN=\"gcharmap\"\
	-I$(srcdir)/../pixmaps\
	$(GNOME_INCLUDEDIR)\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"

gcharmap_SOURCES = \
        main.c\
        interface.c interface.h\
        menus.c menus.h\
        callbacks.c callbacks.h\
        asciiselect.c asciiselect.h

gcharmap_LDADD = $(GNOME_LIBDIR) $(GNOME_LIBS) $(GNOMEUI_LIBS) $(INTLLIBS)

man_MANS = gcharmap.1

EXTRA_DIST = $(gcharmap_SOURCES) gcharmap.1
