Audiodir = $(datadir)/gnome/apps/Multimedia
Pixmapdir = $(datadir)/pixmaps

EXTRA_DIST = README TODO NEWS gtcd.desktop.in gtcd.png gtcd.goad gtcd.idl

# Should make sure that tcd will work with all flavors of curses.
# Make sure it can support curses w/o color and old versions.
if HAS_CURSES
CURSES_TCD=tcd
endif

bin_PROGRAMS = gtcd $(CURSES_TCD)

SUBDIRS = icons doc

GTCD_GENERATED = \
	gtcd.h \
	gtcd-stubs.c \
	gtcd-skels.c \
	gtcd-common.c

gtcd_SOURCES = 		\
	gtcd.c 		\
	cddb.c 		\
	linux-cdrom.c 	\
	gtracked.c 	\
	cddb.h 		\
	gtracked.h 	\
	prefs.c		\
	prefs.h		\
	led.h		\
	led.c		\
	led.xpm		\
	linux-cdrom.h	\
	callbacks.c	\
	callbacks.h	\
	popup.c		\
	keybindings.c	\
	keybindings.h	\
	gtcd_public.h	\
	cddb_status.c	\
	cddb_props.c	\
	cddb_props.h    \
	gtcd-skels.c    \
	gtcd-stubs.c    \
	gtcd-common.c   \
	gtcd.h

tcd_SOURCES = 		\
	tcd.c 		\
	cddb.c 		\
	linux-cdrom.c 	\
	cddb.h 		\
	linux-cdrom.h	\
	tcd.h		\
	tracked.h	\
	tracked.c

gtcd_LDADD = 	$(GNOME_LIBDIR) \
		$(GNOMEUI_LIBS) \
		$(GNOMEGNORBA_LIBS) \
		$(INTLLIBS)

tcd_LDADD = $(GNOME_LIBDIR) -lglib -lm $(CURSES_LIBS) $(INTLLIBS)

CLEANFILES = *~ $(GTCD_GENERATED) my-gtcd-idl

INCLUDES = -I$(includedir) $(GNOME_INCLUDEDIR) $(CURSES_INCLUDEDIR) \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	-DGNOME_ICONDIR=\""$(Pixmapdir)"\"

Audio_in_files = gtcd.desktop.in
Audio_DATA = $(Audio_in_files:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@
Pixmap_DATA = gtcd.png

idldir = $(datadir)/idl
idl_DATA = gtcd.idl

goaddir = $(sysconfdir)/CORBA/servers
goad_DATA = gtcd.goad

BUILT_SOURCES = $(GTCD_GENERATED)

gtcd.h: my-gtcd-idl
gtcd-stubs.c: my-gtcd-idl
gtcd-skels.c: my-gtcd-idl
gtcd-common.c: my-gtcd-idl

my-gtcd-idl:
	orbit-idl $(GNOME_IDLFLAGS) $(GNOME_INCLUDEDIR) gtcd.idl
	touch my-gtcd-idl
