## Process this file with automake to produce Makefile.in

SUBDIRS = doc

INCLUDES = -DG_LOG_DOMAIN=\"gdict\"			\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-I$(includedir) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = gdict

gdict_SOURCES = \
	gdict-about.c gdict-about.h \
	gdict-pref.c gdict-pref.h \
        gdict-pref-dialog.c gdict-pref-dialog.h \
	gdict-app.c gdict-app.h \
	gdict-applet.c gdict-applet.h \
	gdict-defbox.c gdict-defbox.h \
	gdict-speller.c gdict-speller.h \
	dict.c dict.h \
	main.c

gdict_LDADD = $(GNOME_LIBDIR) $(GNOMEUI_LIBS) $(GNOME_APPLETS_LIBS) $(GNOME_PRINT_LIBS) $(INTLLIBS)

man_MANS = gdict.1

pixmapsdir = $(datadir)/pixmaps
pixmaps_DATA = gdict.png

gdictgnorbadir  = $(sysconfdir)/CORBA/servers
gdictgnorba_DATA = gdict.gnorba

gdictappletdir  = $(datadir)/applets/Utility
gdictapplet_in_files = gdict.desktop.in
gdictapplet_DATA = $(gdictapplet_in_files:.desktop.in=.desktop)

gdictappdir  = $(datadir)/gnome/apps/Utilities
gdictapp_in_files = GDict.desktop.in
gdictapp_DATA = $(gdictapp_in_files:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

EXTRA_DIST = \
	$(gdictapp_in_files) \
	$(gdictapp_DATA) \
	$(gdictapplet_in_files) \
	$(gdictapplet_DATA) \
	$(gdictgnorba_DATA) \
	$(pixmaps_DATA) \
	$(man_MANS) \
	$(top_srcdir)/sgmldocs.make

