SUBDIRS = doc

INCLUDES = 			\
	-I$(includedir)		\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	$(GNOME_INCLUDEDIR)

bin_PROGRAMS = idetool

idetool_SOURCES = \
	idetool.c

idetool_LDADD = \
	$(GNOME_LIBDIR)         \
	$(GTKXMHTML_LIBS)	\
	$(GNOMEUI_LIBS) 	\
	$(INTLLIBS)

man_MANS = idetool.8

pixmapdir = $(datadir)/pixmaps

BITMAPS = ide-disk-drive.png

pixmap_DATA = $(BITMAPS)

Utilitiesdir = $(datadir)/gnome/apps/Utilities
Utilities_in_files = idetool.desktop.in
Utilities_DATA = $(Utilities_in_files:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

EXTRA_DIST = \
	$(BITMAPS) \
	$(Utilities_in_files) \
	$(Utilities_DATA) \
	$(man_MANS) \
	$(top_srcdir)/sgmldocs.make

