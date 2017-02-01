## Process  this file with automake to create Makefile.in.

INCLUDES = -I.. -I$(srcdir)/.. -I../../guile-gtk -I$(srcdir)/../../guile-gtk \
	-I$(includedir) $(GUILE_INCS) $(GNOME_INCLUDEDIR)

GTKHTML_LIBS = -lgtkhtml -lgdk_pixbuf

lib_LTLIBRARIES = libguilegtkhtml.la
libguilegtkhtml_la_SOURCES = gtkhtml-glue.c gtkhtml-support.c
libguilegtkhtml_la_LIBADD = $(GNOME_LIBDIR) $(GNOMEUI_LIBS) $(GTKHTML_LIBS)
libguilegtkhtml_la_LDFLAGS = -export-dynamic


scmgnomedir = $(datadir)/guile/gtk

install-data-local:
	$(mkinstalldirs) $(scmgnomedir)
	$(INSTALL_DATA) $(srcdir)/gtkhtml.scm $(scmgnomedir)/gtkhtml.scm

EXTRA_DIST = gtkhtml.defs gtkhtml.scm gtkhtml-glue.c \
	     gtkhtml-support.c gtkhtml-support.h
            
BUILT_SOURCES = gtkhtml-glue.c 
SUFFIXES = .x
.c.x:
	guile-snarf $(DEFS) $(INCLUDES) $(CPPFLAGS) $(CFLAGS) $< >tmp \
	&& mv tmp $@

Makefile: $(BUILT_SOURCES)

defsdir = $(datadir)/guile-gnome/
defs_DATA = gtkhtml.defs

## Convenience variable.
BUILD = guile -l ../../guile-gtk/gtk/config.scm -s $(srcdir)/../../guile-gtk/build-guile-gtk

gtkhtml-glue.c: gtkhtml.defs ../../guile-gtk/build-guile-gtk
	$(BUILD) -I $(top_srcdir)/guile-gtk glue $(srcdir)/gtkhtml.defs > $<.tmpg \
        && mv $<.tmpg $@

