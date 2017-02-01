Utilitiesdir = $(datadir)/gnome/apps/Utilities

SUBDIRS = doc

INCLUDES = \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	-DLOCALEDIR=\"$(datadir)/locale\" \
	-I$(includedir) \
	$(GNOME_INCLUDEDIR)	\
	@PIXBUF_CFLAGS@

bin_PROGRAMS = gcalc 

gcalc_SOURCES = \
	gcalc.c		\
	gnome-calc.c	\
	gnome-calc.h	\
	sr.h		\
	sr.c

gcalc_LDADD = \
	$(GNOME_LIBDIR) \
	$(GNOMEUI_LIBS) \
	$(INTLLIBS) \
	@PIXBUF_LIBS@

man_MANS = gcalc.1

Utilities_in_files = gcalc.desktop.in 
Utilities_DATA = $(Utilities_in_files:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

EXTRA_DIST = \
	$(man_MANS) \
	$(Utilities_in_files) \
	$(Utilities_DATA)
