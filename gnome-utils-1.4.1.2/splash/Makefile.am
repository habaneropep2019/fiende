## Process this file with automake to produce makefile.in

INCLUDES = -I$(includedir) 				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" 	\
	-DLOCALEDIR=\"$(datadir)/locale\"		\
	$(GNOME_INCLUDEDIR)

bin_PROGRAMS = splash splac

splash_SOURCES = \
	main.c	\
	splash.h

splac_SOURCES = \
	splac.c

splash_LDADD = $(GNOME_LIBDIR) $(GNOMEUI_LIBS) $(INTLLIBS)

man_MANS = splash.1 splac.1

EXTRA_DIST = splash.1 splac.1
