INCLUDES = 			\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl     \
	-I$(srcdir)/../../control-center                \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"        \
	-I$(includedir) $(GNOME_INCLUDEDIR) $(IMLIB_CFLAGS) \
	$(PIXBUF_CFLAGS)

bin_PROGRAMS =background-properties-capplet

background_properties_capplet_SOURCES =	\
	render-background.c	\
	render-background.h	\
	property-background.c	\
	app-background.c	\
	imlib-misc.c		\
	imlib-misc.h		

background_properties_capplet_LDADD = ../../control-center/libcapplet.la \
		$(GNOME_LIBDIR) $(ORB_LIBS) $(IMLIB_LIBS) \
		$(GNOMEUI_LIBS) $(INTLLIBS) $(PIXBUF_LIBS) -lgnorba 

EXTRA_DIST = background-properties.desktop.in background-properties.desktop


sysdir = $(datadir)/control-center/Desktop
sys_in_files = background-properties.desktop.in 
sys_DATA = $(sys_in_files:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

install-data-local:
	$(INSTALL_DATA) $(srcdir)/background-properties.desktop $(DESTDIR)$(datadir)/gnome/apps/Settings/Desktop/background-properties.desktop
