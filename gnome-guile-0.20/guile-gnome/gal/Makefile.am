## Process this file with automake to create Makefile.in.

INCLUDES = -I.. -I$(srcdir)/.. -I../../guile-gtk -I$(srcdir)/../../guile-gtk \
	-I$(includedir) $(GUILE_INCS) $(GNOME_INCLUDEDIR)

GAL_LIBS = -lgal 

lib_LTLIBRARIES = libguilegal.la
libguilegal_la_SOURCES = gal-glue.c 
libguilegal_la_LIBADD = $(GNOME_LIBDIR) $(GNOMEUI_LIBS) $(GAL_LIBS)
libguilegal_la_LDFLAGS = -export-dynamic


scmgnomedir = $(datadir)/guile/gnome

install-data-local:
	$(mkinstalldirs) $(scmgnomedir)
	$(INSTALL_DATA) $(srcdir)/gal.scm $(scmgnomedir)/gal.scm

EXTRA_DIST = gal.defs gal.scm gal-glue.c
            
BUILT_SOURCES = gal-glue.c 
SUFFIXES = .x
.c.x:
	guile-snarf $(DEFS) $(INCLUDES) $(CPPFLAGS) $(CFLAGS) $< >tmp \
	&& mv tmp $@

Makefile: $(BUILT_SOURCES)

defsdir = $(datadir)/guile-gnome/
defs_DATA = gal.defs

## Convenience variable.
BUILD = guile -l ../../guile-gtk/gtk/config.scm -s $(srcdir)/../../guile-gtk/build-guile-gtk

gal-glue.c: gal.defs ../../guile-gtk/build-guile-gtk
	$(BUILD) -I $(top_srcdir)/guile-gtk glue $(srcdir)/gal.defs > $<.tmpg \
        && mv $<.tmpg $@




