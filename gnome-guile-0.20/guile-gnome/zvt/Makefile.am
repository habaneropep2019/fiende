## Process  this file with automake to create Makefile.in.

INCLUDES = -I.. -I$(srcdir)/.. -I../../guile-gtk -I$(srcdir)/../../guile-gtk \
	-I$(includedir) $(GUILE_INCS) $(GNOME_INCLUDEDIR)

ZVT_LIBS = -lzvt -lutil 

lib_LTLIBRARIES = libguilezvt.la
libguilezvt_la_SOURCES = zvt-glue.c 
libguilezvt_la_LIBADD = $(GNOME_LIBDIR) $(GNOMEUI_LIBS) $(ZVT_LIBS)
libguilezvt_la_LDFLAGS = -export-dynamic


scmgnomedir = $(datadir)/guile/gnome

install-data-local:
	$(mkinstalldirs) $(scmgnomedir)
	$(INSTALL_DATA) $(srcdir)/zvt.scm $(scmgnomedir)/zvt.scm

EXTRA_DIST = zvt.defs zvt.scm zvt-glue.c
BUILT_SOURCES = zvt-glue.c 
SUFFIXES = .x
.c.x:
	guile-snarf $(DEFS) $(INCLUDES) $(CPPFLAGS) $(CFLAGS) $< >tmp \
	&& mv tmp $@

Makefile: $(BUILT_SOURCES)

defsdir = $(datadir)/guile-gnome/
defs_DATA = zvt.defs

## Convenience variable.
BUILD = guile -l ../../guile-gtk/gtk/config.scm -s $(srcdir)/../../guile-gtk/build-guile-gtk

zvt-glue.c: zvt.defs ../../guile-gtk/build-guile-gtk
	$(BUILD) -I $(top_srcdir)/guile-gtk glue $(srcdir)/zvt.defs > $<.tmpg \
        && mv $<.tmpg $@

