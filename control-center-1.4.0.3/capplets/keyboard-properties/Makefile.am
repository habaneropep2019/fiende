INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-I$(srcdir)/../../control-center		\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = keyboard-properties

keyboard_properties_SOURCES = keyboard-properties.c

keyboard_properties_LDADD = ../../control-center/libcapplet.la \
		     $(GNOME_LIBDIR) $(ORB_LIBS) \
		     $(GNOMEUI_LIBS) $(INTLLIBS) @XF86MISC_LIBS@ -lgnorba

EXTRA_DIST =					\
	keyboard-properties.desktop.in		\
	keyboard-properties.desktop

sysdir = $(datadir)/control-center/Peripherals
sys_in_files = keyboard-properties.desktop.in
sys_DATA = $(sys_in_files:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

install-data-local:
	$(INSTALL_DATA) $(srcdir)/keyboard-properties.desktop $(DESTDIR)$(datadir)/gnome/apps/Settings/Peripherals/keyboard-properties.desktop
