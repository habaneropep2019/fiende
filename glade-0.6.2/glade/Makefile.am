## Process this file with automake to produce Makefile.in

@SET_MAKE@

bin_PROGRAMS = glade

INCLUDES = -I$(top_srcdir)/intl -I$(top_builddir)/intl

DIST_SUBDIRS = gbwidgets gnome gnome-db data

SUBDIRS = gbwidgets $(GLADE_GNOME_DIR) $(GLADE_GNOME_DB_DIR) data

glade_LDADD = \
	gbwidgets/libgbwidgets.a \
	$(GLADE_GNOME_LIB) \
	$(GLADE_GNOME_DB_LIB) \
	$(INTLLIBS)

glade_DEPENDENCIES = \
	gbwidgets/libgbwidgets.a \
	$(GLADE_GNOME_LIB) \
	$(GLADE_GNOME_DB_LIB) \
	$(INTLLIBS)

glade_SOURCES =	\
	debug.c			\
	editor.c		\
	gbwidget.c		\
	glade_widget_data.c	\
	gbsource.c		\
	glade.c			\
	glade_clipboard.c	\
	glade_gnome.c		\
	glade_gnomelib.c	\
	glade_gnomedblib.c	\
	glade_gtk12lib.c	\
	glade_keys_dialog.c	\
	glade_menu_editor.c	\
	glade_palette.c		\
	glade_plugin.c		\
	glade_project.c		\
	glade_project_options.c	\
	glade_project_view.c	\
	glade_project_window.c	\
	keys.c			\
	load.c			\
	main.c			\
	palette.c		\
	property.c		\
	save.c			\
	source.c		\
	source_os2.c		\
	styles.c		\
	tree.c			\
	utils.c

noinst_HEADERS = \
	debug.h			\
	editor.h		\
	gb.h			\
	gbwidget.h		\
	glade_widget_data.h	\
	gbsource.h		\
	glade.h			\
	glade_clipboard.h	\
	glade_gnome.h		\
	glade_gnomelib.h	\
	glade_gnomedblib.h	\
	glade_gtk12lib.h	\
	glade_keys_dialog.h	\
	glade_menu_editor.h	\
	glade_palette.h		\
	glade_plugin.h		\
	glade_project.h		\
	glade_project_options.h	\
	glade_project_view.h	\
	glade_project_window.h	\
	gladeconfig.h		\
	keys.h			\
	load.h			\
	palette.h		\
	property.h		\
	save.h			\
	source.h		\
	source_os2.h		\
	styles.h		\
	tree.h			\
	utils.h


dist-hook:
	mkdir $(distdir)/graphics
	cp -p graphics/*.xpm $(distdir)/graphics
	cp -p graphics/*.png $(distdir)/graphics

