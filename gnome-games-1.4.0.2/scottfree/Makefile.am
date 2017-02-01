INCLUDES = 			\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	-I$(includedir)		\
	$(GNOME_INCLUDEDIR)

bin_PROGRAMS = GnomeScott

GnomeScott_SOURCES = \
	GnomeScott.c	\
	Scott.h

GnomeScott_LDADD = \
	$(GNOME_LIBDIR) $(ZVT_LIBS) $(GNOMEUI_LIBS) $(INTLLIBS)

EXTRA_DIST = 	\
	down.xpm	\
	east.xpm	\
	inventory.xpm	\
	north.xpm	\
	south.xpm	\
	up.xpm		\
	west.xpm	\
	Definition
