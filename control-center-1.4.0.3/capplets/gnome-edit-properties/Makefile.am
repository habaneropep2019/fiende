INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-I$(srcdir)/../../control-center		\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)		\
	-DGNOMESYSCONFDIR=\""$(sysconfdir)"\"

bin_PROGRAMS = gnome-edit-properties-capplet

gnome_edit_properties_capplet_SOURCES = gnome-edit-properties.c

gnome_edit_properties_capplet_LDADD = ../../control-center/libcapplet.la \
		     $(GNOME_LIBDIR) $(ORB_LIBS) \
		     $(GNOMEUI_LIBS) $(INTLLIBS) -lgnorba


EXTRA_DIST =					\
	gnome-edit-properties.desktop.in	\
	gnome-edit-properties.desktop

sysdir = $(datadir)/control-center/Documents
sys_in_files = gnome-edit-properties.desktop.in
sys_DATA = $(sys_in_files:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

install-data-local:
	$(INSTALL_DATA) $(srcdir)/gnome-edit-properties.desktop $(DESTDIR)$(datadir)/gnome/apps/Settings/Documents/gnome-edit-properties.desktop
