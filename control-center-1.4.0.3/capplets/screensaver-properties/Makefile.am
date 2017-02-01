INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-I$(srcdir)/../../control-center		\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR) $(IMLIB_CFLAGS)

bin_PROGRAMS = screensaver-properties-capplet


screensaver_properties_capplet_SOURCES = \
	screensaver-properties-capplet.c	\
	callbacks.c				\
	callbacks.h				\
	screensaver-dialog.c			\
	screensaver-dialog.h			\
	parser.c				\
	parser.h

screensaver_properties_capplet_LDADD = \
../../control-center/libcapplet.la \
$(GNOME_LIBDIR) $(ORB_LIBS) $(GNOMEUI_LIBS) $(INTLLIBS) \
-lgnorba $(IMLIB_LIBS)

EXTRA_DIST = screensaver-properties.desktop.in screensaver-properties.desktop

SUBDIRS=screensaver-desktops

sysdir = $(datadir)/control-center/Desktop
sys_in_files = screensaver-properties.desktop.in
sys_DATA = $(sys_in_files:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

install-data-local:
	$(INSTALL_DATA) $(srcdir)/screensaver-properties.desktop $(DESTDIR)$(datadir)/gnome/apps/Settings/Desktop/screensaver-properties.desktop
