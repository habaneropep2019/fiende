INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-I$(srcdir)/../../control-center		\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)		\
	-DGNOMESYSCONFDIR=\""$(sysconfdir)"\"

bin_PROGRAMS = sound-properties

sound_properties_SOURCES = sound-properties.c

sound_properties_LDADD = ../../control-center/libcapplet.la \
		     $(GNOME_LIBDIR) $(ORB_LIBS) \
		     $(GNOMEUI_LIBS) $(INTLLIBS) -lgnorba


EXTRA_DIST =					\
	sound-properties.desktop.in		\
	sound-properties.desktop

sysdir = $(datadir)/control-center/Multimedia
sys_in_files = sound-properties.desktop.in
sys_DATA = $(sys_in_files:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

install-data-local:
	$(INSTALL_DATA) $(srcdir)/sound-properties.desktop $(DESTDIR)$(datadir)/gnome/apps/Settings/Multimedia/sound-properties.desktop
