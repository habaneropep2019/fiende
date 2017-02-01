## Process this file with automake to produce Makefile.in

SUBDIRS = doc

INCLUDES = -DG_LOG_DOMAIN=\"GDiskFree\"			\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR) $(LIBGTOP_INCS)

bin_PROGRAMS = gdiskfree 

gdiskfree_SOURCES = \
	gdiskfree_app.c		gdiskfree_app.h		\
	gdiskfree_menus.c	gdiskfree_menus.h	\
	gdiskfree_options.c	gdiskfree_options.h	\
	gdiskfree.c


gdiskfree_LDADD = $(GNOME_LIBDIR) $(GTKXMHTML_LIBS) \
		  $(GNOMEUI_LIBS) $(INTLLIBS) \
		  $(LIBGTOP_LIBS) $(LIBGTOP_EXTRA_LIBS)

man_MANS = gdiskfree.1

Systemdir = $(datadir)/gnome/apps/System
System_in_files = gdiskfree.desktop.in
System_DATA = $(System_in_files:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

Pixmapsdir = $(datadir)/pixmaps
Pixmaps_DATA = gdiskfree.png gdiskfree-splash.png

EXTRA_DIST = \
	$(man_MANS) \
	$(System_in_files) \
	$(System_DATA) \
	$(Pixmaps_DATA) \
	$(top_srcdir)/sgmldocs.make
