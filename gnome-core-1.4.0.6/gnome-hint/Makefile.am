INCLUDES = 			\
	-I$(includedir)		\
	-I$(top_srcdir)/intl	\
	-I$(top_builddir)/intl	\
	$(GNOME_INCLUDEDIR)	\
	-DGNOMELOCALEDIR=\""$(datadir)/locale\""

LDADD = \
	$(GNOME_LIBDIR)					\
	$(GNOMEUI_LIBS)					\
	$(GNOME_XML_LIB)				\
	$(INTLLIBS)

bin_PROGRAMS = gnome-hint gnome-hint-properties-capplet

gnome_hint_SOURCES = 		\
	gnome-hint.c

gnome_hint_properties_capplet_SOURCES = gnome-hint-properties.c

man_MANS = gnome-hint-properties-capplet.1	\
	gnome-hint.1
	
gnome_hint_properties_capplet_LDADD = \
	$(GNOME_LIBDIR)		\
	$(GNOME_CAPPLET_LIBS)	\
	$(INTLLIBS)

utildir = $(datadir)/gnome/apps/Utilities
util_in_files = gnome-hint.desktop.in
util_DATA = $(util_in_files:.desktop.in=.desktop)

sysdir = $(datadir)/control-center/Session/
sys_in_files = gnome-hint-properties.desktop.in
sys_DATA = $(sys_in_files:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

pixmapsdir = $(datadir)/pixmaps
pixmaps_DATA = gnome-hint.png

EXTRA_DIST = \
	$(util_in_files)	\
	$(util_DATA)		\
	$(sys_in_files)		\
	$(sys_DATA)		\
	gnome-hint.png		\
	$(man_MANS)

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(datadir)/gnome/apps/Settings/Session/
	$(INSTALL_DATA) gnome-hint-properties.desktop $(DESTDIR)$(datadir)/gnome/apps/Settings/Session/gnome-hint-properties.desktop

