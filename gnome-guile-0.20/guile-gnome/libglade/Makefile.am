## Process this file with automake to create Makefile.in.

INCLUDES = -I.. -I$(srcdir)/.. -I../../guile-gtk -I$(srcdir)/../../guile-gtk \
	-I$(includedir) $(GUILE_INCS) $(GNOME_INCLUDEDIR)

GLADE_LIBS = -lglade -lglade-gnome 

lib_LTLIBRARIES = libguileglade.la
libguileglade_la_SOURCES = libglade-glue.c 
libguileglade_la_LIBADD = $(GNOME_LIBDIR) $(GNOMEUI_LIBS) $(GLADE_LIBS)
libguileglade_la_LDFLAGS = -export-dynamic


scmgnomedir = $(datadir)/guile/gtk

install-data-local:
	$(mkinstalldirs) $(scmgnomedir)
	$(INSTALL_DATA) $(srcdir)/libglade.scm $(scmgnomedir)/libglade.scm

EXTRA_DIST = libglade.defs libglade.scm libglade-glue.c
            
BUILT_SOURCES = libglade-glue.c 
SUFFIXES = .x
.c.x:
	guile-snarf $(DEFS) $(INCLUDES) $(CPPFLAGS) $(CFLAGS) $< >tmp \
	&& mv tmp $@

Makefile: $(BUILT_SOURCES)

defsdir = $(datadir)/guile-gtk/
defs_DATA = libglade.defs

## Convenience variable.
BUILD = guile -l ../../guile-gtk/gtk/config.scm -s $(srcdir)/../../guile-gtk/build-guile-gtk

libglade-glue.c: libglade.defs ../../guile-gtk/build-guile-gtk
	$(BUILD) -I $(top_srcdir)/guile-gtk glue $(srcdir)/libglade.defs > $<.tmpg \
        && mv $<.tmpg $@


