# Process this file with automake to produce Makefile.am

SUBDIRS = doc

INCLUDES = 			\
	-I$(includedir)		\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	$(GNOME_INCLUDEDIR)

bin_PROGRAMS = gw

gw_SOURCES = \
	gw.c

gw_LDADD = \
	$(GNOME_LIBDIR)         \
	$(GNOMEUI_LIBS) 	\
	$(INTLLIBS)

man_MANS = gw.1

Systemdir = $(datadir)/gnome/apps/System
System_in_files = gw.desktop.in
System_DATA = $(System_in_files:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

pixmapdir = $(datadir)/pixmaps
pixmap_DATA = gnome-who.png

EXTRA_DIST = \
	$(man_MANS) \
	$(System_in_files) \
	$(System_DATA) \
	$(pixmap_DATA) \
	$(top_srcdir)/sgmldocs.make
