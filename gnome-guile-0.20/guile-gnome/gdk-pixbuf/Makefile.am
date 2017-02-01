## Process this file with automake to create Makefile.in.

INCLUDES = -I.. -I$(srcdir)/.. -I../../guile-gtk -I$(srcdir)/../../guile-gtk \
	-I$(includedir) $(GUILE_INCS) $(GNOME_INCLUDEDIR)

GDKPIXBUF_LIBS = -lgdk_pixbuf

lib_LTLIBRARIES = libguilegdkpixbuf.la
libguilegdkpixbuf_la_SOURCES = gdkpixbuf-glue.c 
libguilegdkpixbuf_la_LIBADD = $(GNOME_LIBDIR) $(GNOMEUI_LIBS) $(GDKPIXBUF_LIBS)
libguilegdkpixbuf_la_LDFLAGS = -export-dynamic


scmgnomedir = $(datadir)/guile/gtk

install-data-local:
	$(mkinstalldirs) $(scmgnomedir)
	$(INSTALL_DATA) $(srcdir)/gdkpixbuf.scm $(scmgnomedir)/gdkpixbuf.scm

EXTRA_DIST = gdkpixbuf.defs gdkpixbuf.scm gdkpixbuf-glue.c
            

BUILT_SOURCES = gdkpixbuf-glue.c 
SUFFIXES = .x
.c.x:
	guile-snarf $(DEFS) $(INCLUDES) $(CPPFLAGS) $(CFLAGS) $< >tmp \
	&& mv tmp $@

Makefile: $(BUILT_SOURCES)

defsdir = $(datadir)/guile-gtk/
defs_DATA = gdkpixbuf.defs

## Convenience variable.
BUILD = guile -l ../../guile-gtk/gtk/config.scm -s $(srcdir)/../../guile-gtk/build-guile-gtk

gdkpixbuf-glue.c: gdkpixbuf.defs ../../guile-gtk/build-guile-gtk
	$(BUILD) -I $(top_srcdir)/guile-gtk glue $(srcdir)/gdkpixbuf.defs > $<.tmpg \
        && mv $<.tmpg $@




