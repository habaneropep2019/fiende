INCLUDES = -I. -I$(srcdir) 				\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-I$(srcdir)/../../control-center		\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)		\
	-DGNOMESYSCONFDIR=\""$(sysconfdir)"\"

bin_PROGRAMS = ui-properties

ui_properties_SOURCES = ui-properties.c

ui_properties_LDADD = ../../control-center/libcapplet.la \
		     $(GNOME_LIBDIR) $(ORB_LIBS) \
		     $(GNOMEUI_LIBS) $(INTLLIBS) -lgnorba


sysdir = $(datadir)/control-center/UIOptions
sys_in_files =					\
	application.desktop.in			\
	dialogs.desktop.in			\
	mdi.desktop.in 
sys_DATA = $(sys_in_files:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

directory_in_files = ui-properties.directory.in
noinst_DATA = $(directory_in_files:.directory.in=.directory)

EXTRA_DIST = $(sys_in_files) $(sys_DATA) $(directory_in_files) $(noinst_DATA)

@XML_I18N_MERGE_DIRECTORY_RULE@

install-data-local:
	$(INSTALL_DATA) $(srcdir)/ui-properties.directory $(DESTDIR)$(sysdir)/.directory
