SUBDIRS = doc

INCLUDES = -DG_LOG_DOMAIN=\"stripchart\" \
 -DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
 -DLOCALEDIR=\"$(datadir)/locale\" \
 -DCONFDIR=\"$(datadir)/stripchart\" \
 -I$(includedir) $(GNOME_INCLUDEDIR) $(LIBGTOP_INCS)

bin_PROGRAMS = stripchart stripchart-applet

stripchart_SOURCES = \
 stripchart.c \
 chart-app.c chart.c eval.c params.c pen.c prefs.c strip.c utils.c \
 chart-app.h chart.h eval.h params.h pen.h prefs.h strip.h utils.h

stripchart_LDADD = \
 $(GNOME_LIBDIR) $(GNOMEUI_LIBS) $(INTLLIBS) \
 $(LIBGTOP_LIBS) $(LIBGTOP_EXTRA_LIBS) $(GNOME_XML_LIB) 

stripchart_applet_SOURCES = \
 stripchart-applet.c \
 chart-app.c chart.c eval.c params.c pen.c prefs.c strip.c utils.c \
 chart-app.h chart.h eval.h params.h pen.h prefs.h strip.h utils.h

stripchart_applet_LDADD = \
 $(GNOME_LIBDIR) $(GNOMEUI_LIBS) $(INTLLIBS) $(GNOME_APPLETS_LIBS) \
 $(LIBGTOP_LIBS) $(LIBGTOP_EXTRA_LIBS) $(GNOME_XML_LIB) 

man_MANS = stripchart.1 stripchart-applet.1

pixmapdir = $(datadir)/pixmaps
pixmap_DATA = gnome-stripchart.png

confdir = $(datadir)/stripchart
conf_DATA = stripchart.conf stripchart.params

Utilitiesdir = $(datadir)/gnome/apps/Utilities
Utilities_in_files = stripchart.desktop.in
Utilities_DATA = $(Utilities_in_files:.desktop.in=.desktop)

Appletdir = $(datadir)/applets/Monitors
Applet_in_files = stripchart-applet.desktop.in
Applet_DATA = $(Applet_in_files:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

gnorbadir  = $(sysconfdir)/CORBA/servers
gnorba_DATA = stripchart-applet.gnorba

EXTRA_DIST = \
	$(man_MANS) \
	$(pixmap_DATA) \
	$(conf_DATA) \
	$(Utilities_DATA) \
	$(Utilities_in_files) \
	$(Applet_DATA) \
	$(Applet_in_files) \
	$(gnorba_DATA) \
	$(top_srcdir)/sgmldocs.make

sanitize:	clean
	$(RM) core *~ doc/C/*.html doc/C/*~
	$(RM) -r .deps doc/C/stripchart doc/C/stripchart.junk
