# Process this file with automake to produce Makefile.in

SUBDIRS = doc

if GUNAME_CAPPLET
GUNAME_CAPPLET_LIBS=-lcapplet -lgnorba
CFLAGS+=-DGUNAME_CAPPLET
endif

INCLUDES = 			\
	-I$(includedir)		\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	$(GNOME_INCLUDEDIR) $(LIBGTOP_NAMES_INCS)

bin_PROGRAMS = guname

if HAVE_LIBGTOP
moreinfo_c		= moreinfo.c
moreinfo_h		= moreinfo.h
endif

guname_SOURCES = \
	guname.c	\
	info.c		\
	list.c		\
	$(moreinfo_c)	\
	AUTHORS.h

noinst_HEADERS = \
	info.h		\
	list.h		\
	$(moreinfo_h)

guname_LDADD = \
	$(GNOME_LIBDIR)         \
	$(GNOMEUI_LIBS) 	\
	$(INTLLIBS)		\
	$(LIBGTOP_NAMES_LIBS)	\
	$(LIBGTOP_EXTRA_LIBS)	\
	$(GUNAME_CAPPLET_LIBS)

man_MANS = guname.1

Systemdir = $(datadir)/gnome/apps/System
System_in_files = guname.desktop.in
System_DATA = $(System_in_files:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

if GUNAME_CAPPLET
install-data-local: guname-capplet.desktop
	$(INSTALL_DATA) $(srcdir)/guname-capplet.desktop $(DESTDIR)$(datadir)/control-center/guname-capplet.desktop
endif

EXTRA_DIST = 		\
	$(man_MANS)	\
	$(System_in_files) \
	$(System_DATA) \
	moreinfo.c	\
	moreinfo.h	\
	guname-capplet.desktop.in \
	guname-capplet.desktop \
	$(top_srcdir)/sgmldocs.make

