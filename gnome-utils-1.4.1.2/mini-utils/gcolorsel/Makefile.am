# Process this file with automake to produce Makefile.in

SUBDIRS = glade doc

gladedir = $(datadir)/gcolorsel/glade
Utilitiesdir = $(datadir)/gnome/apps/Utilities

CFLAGS += -DGCOLORSEL_GLADEDIR=\""$(gladedir)/"\" -g
CFLAGS += -DXLIBDIR=\""$(XLIBDIR)/"\"

INCLUDES = 						\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" 	\
	-DLOCALEDIR=\"$(datadir)/locale\"		\
	-I$(includedir)					\
	$(GNOME_INCLUDEDIR) $(GLADE_CFLAGS)

bin_PROGRAMS = gcolorsel

gcolorsel_SOURCES = 	dialog-new-doc.c		\
			dialog-new-view.c		\
			dialog-prefs.c			\
			dialogs.h			\
			gcolorsel.c			\
			gcolorsel.h			\
			idle.c				\
			idle.h				\
			menus.c				\
			menus.h				\
			mdi-color-generic.c		\
			mdi-color-generic.h		\
			mdi-color.c			\
			mdi-color.h			\
			mdi-color-file.c		\
			mdi-color-file.h		\
			mdi-color-virtual.c		\
			mdi-color-virtual.h		\
			mdi-color-virtual-monitor.c	\
			mdi-color-virtual-monitor.h	\
			mdi-color-virtual-rgb.c		\
			mdi-color-virtual-rgb.h		\
			session.c			\
			session.h			\
			utils.c				\
			utils.h				\
			view-color-edit.c		\
			view-color-edit.h		\
			view-color-generic.c		\
			view-color-generic.h		\
			view-color-list.c		\
			view-color-list.h		\
			view-color-grid.c		\
			view-color-grid.h		\
			widget-color-grid.c		\
			widget-color-grid.h		\
			widget-control-generic.c	\
			widget-control-generic.h	\
			widget-control-virtual-rgb.c	\
			widget-control-virtual-rgb.h

gcolorsel_LDADD = \
	$(GNOME_LIBDIR)  				\
	$(GNOMEUI_LIBS) 				\
	$(INTLLIBS)					\
	@GLADE_LIBS@					\
	$(GNOME_XML_LIB)

man_MANS = gcolorsel.1

Utilities_in_files = gcolorsel.desktop.in
Utilities_DATA = $(Utilities_in_files:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

EXTRA_DIST = \
	$(man_MANS) \
	$(Utilities_in_files) \
	$(Utilities_DATA) \
	$(top_srcdir)/sgmldocs.make


CLEANFILES=*~
