Utilitiesdir = $(datadir)/gnome/apps/Utilities

SUBDIRS = help

INCLUDES = 						\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" 	\
	-DLOCALEDIR=\"$(datadir)/locale\"		\
	-I$(includedir)					\
	$(GNOME_INCLUDEDIR)

bin_PROGRAMS = gsearchtool 

gsearchtool_SOURCES =	\
	gsearchtool.c	\
	gsearchtool.h	\
        outdlg.c	\
	outdlg.h

gsearchtool_LDADD = \
	$(GNOME_LIBDIR) \
	$(GNOMEUI_LIBS) \
	$(INTLLIBS)

man_MANS = gsearchtool.1

pixmapdir = $(datadir)/pixmaps
pixmap_DATA = gnome-searchtool.png

Utilities_in_files = gsearchtool.desktop.in 
Utilities_DATA = $(Utilities_in_files:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

EXTRA_DIST = \
	$(pixmap_DATA) \
	$(Utilities_in_files) \
	$(Utilities_DATA) \
	$(man_MANS)
