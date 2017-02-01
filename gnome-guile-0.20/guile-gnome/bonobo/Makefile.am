## Process  this file with automake to create Makefile.in.

INCLUDES = -I.. -I$(srcdir)/.. -I../../guile-gtk -I$(srcdir)/../../guile-gtk \
	-I$(includedir) $(GUILE_INCS) $(GNOME_INCLUDEDIR)

BONOBO_LIBS = -lbonobo 

lib_LTLIBRARIES = libguilebonobo.la
libguilebonobo_la_SOURCES = bonobo-glue.c 
libguilebonobo_la_LIBADD = $(GNOME_LIBDIR) $(GNOMEUI_LIBS) $(BONOBO_LIBS)
libguilebonobo_la_LDFLAGS = -export-dynamic


scmgnomedir = $(datadir)/guile/gnome

install-data-local:
	$(mkinstalldirs) $(scmgnomedir)
	$(INSTALL_DATA) $(srcdir)/bonobo.scm $(scmgnomedir)/bonobo.scm

EXTRA_DIST = bonobo.defs bonobo.scm bonobo-glue.c
            
BUILT_SOURCES = bonobo-glue.c 
SUFFIXES = .x
.c.x:
	guile-snarf $(DEFS) $(INCLUDES) $(CPPFLAGS) $(CFLAGS) $< >tmp \
	&& mv tmp $@

Makefile: $(BUILT_SOURCES)

defsdir = $(datadir)/guile-gnome/
defs_DATA = bonobo.defs

## Convenience variable.
BUILD = guile -l ../../guile-gtk/gtk/config.scm -s $(srcdir)/../../guile-gtk/build-guile-gtk

bonobo-glue.c: bonobo.defs ../../guile-gtk/build-guile-gtk
	$(BUILD) -I $(top_srcdir)/guile-gtk glue $(srcdir)/bonobo.defs > $<.tmpg \
        && mv $<.tmpg $@

