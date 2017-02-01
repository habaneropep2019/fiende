## Process this file with automake to create Makefile.in.

if WITH_BONOBO
BONOBO_DIR = bonobo
else
BONOBO_DIR = 
endif

if WITH_GTKHTML
GTKHTML_DIR = gtkhtml
else
GTKHTML_DIR = 
endif

if WITH_LIBGLADE
LIBGLADE_DIR = libglade
else
LIBGLADE_DIR = 
endif

if HAVE_GDK_PIXBUF
GDK_PIXBUF_DIR = gdk-pixbuf
else 
GDK_PIXBUF_DIR = 
endif

if HAVE_GHTTP
GHTTP_DIR = ghttp
else 
GHTTP_DIR =
endif

if HAVE_GAL 
GAL_DIR = gal
else
GAL_DIR =
endif

SUBDIRS = zvt $(BONOBO_DIR) $(GTKHTML_DIR) $(LIBGLADE_DIR) $(GDK_PIXBUF_DIR) $(GHTTP_DIR) $(GAL_DIR)

INCLUDES = -I.. -I$(srcdir)/.. -I../guile-gtk -I$(srcdir)/../guile-gtk \
	-I$(includedir) $(GUILE_INCS) $(GNOME_INCLUDEDIR)

bin_PROGRAMS = gnomeg

lib_LTLIBRARIES = libguilegnome.la libguilegnorba.la 
libguilegnome_la_SOURCES = gnome-glue.c gnomeg.c gnome-ui-infos.h gnome-canvas-support.c gnome-canvas-support.h
libguilegnome_la_LIBADD = $(GNOME_LIBDIR) $(GNOMEUI_LIBS)
libguilegnome_la_LDFLAGS = -export-dynamic

libguilegnorba_la_SOURCES = gnorba-glue.c gnorba-main.c gnorba-main.h
libguilegnorba_la_LIBADD = $(GNOME_LIBDIR) $(GNOMEUI_LIBS) $(GNORBA_LIBS)
libguilegnorba_la_LDFLAGS = -export-dynamic

gnomeg_SOURCES = main.c gnomeg.c client.c gnome-glue.c gnome-ui-infos.h gnome-canvas-support.c gnome-canvas-support.h
gnomeg_LDADD = \
	$(top_builddir)/guile-gtk/libguilegtk.la     \
	@GUILE_LIBS@ 				     \
	$(GNOME_LIBDIR)                              \
	$(GNOMEUI_LIBS)

scmgnomedir = $(datadir)/guile/gnome

install-data-local:
	$(mkinstalldirs) $(scmgnomedir)
	$(INSTALL_DATA) $(srcdir)/gnome.scm $(scmgnomedir)/gnome.scm
	$(INSTALL_DATA) $(srcdir)/gnorba.scm $(scmgnomedir)/gnorba.scm

EXTRA_DIST = gnome.scm gnome.defs gnome.defs.guile gnome-glue.c main.c \
             examples/uiinfo_demo.scm gnorba.defs gnorba.defs.guile \
	     gnorba.scm

CLEANFILES = client.x

BUILT_SOURCES = main.c client.x gnome-glue.c gnorba-glue.c
SUFFIXES = .x
.c.x:
	guile-snarf $(DEFS) $(INCLUDES) $(CPPFLAGS) $(CFLAGS) $< >tmp \
	&& mv tmp $@

client.x: client.c

Makefile: $(BUILT_SOURCES)

defsdir = $(datadir)/guile-gnome/
defs_DATA = gnome.defs gnorba.defs

## Convenience variable.
BUILD = guile -l ../guile-gtk/gtk/config.scm -s $(srcdir)/../guile-gtk/build-guile-gtk

gnome-glue.c: gnome.defs ../guile-gtk/build-guile-gtk
	$(BUILD) -I $(top_srcdir)/guile-gtk glue $(srcdir)/gnome.defs > $<.tmpg \
        && mv $<.tmpg $@

gnorba-glue.c: gnorba.defs ../guile-gtk/build-guile-gtk
	$(BUILD) -I $(top_srcdir)/guile-gtk glue $(srcdir)/gnorba.defs > $<.tmpg \
	&& mv $<.tmpg $@

main.c: gnome.defs ../guile-gtk/build-guile-gtk
	$(BUILD) -I $(top_srcdir)/guile-gtk main $(srcdir)/gnome.defs > $<.tmpm \
        && mv $<.tmpm $@
