INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-I$(srcdir)/../../control-center		\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = mouse-properties-capplet

mouse_properties_capplet_SOURCES = mouse-properties.c

mouse_properties_capplet_LDADD = ../../control-center/libcapplet.la \
		     $(GNOME_LIBDIR) $(ORB_LIBS) \
		     $(GNOMEUI_LIBS) $(INTLLIBS) -lgnorba

EXTRA_DIST =					\
	mouse-properties.desktop.in		\
	mouse-properties.desktop


sysdir = $(datadir)/control-center/Peripherals
sys_in_files = mouse-properties.desktop.in 
sys_DATA = $(sys_in_files:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

install-data-local:
	$(INSTALL_DATA) $(srcdir)/mouse-properties.desktop $(DESTDIR)$(datadir)/gnome/apps/Settings/Peripherals/mouse-properties.desktop
