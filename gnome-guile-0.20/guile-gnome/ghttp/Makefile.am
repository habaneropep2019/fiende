## Process this file with automake to create Makefile.in.

INCLUDES = -I.. -I$(srcdir)/.. -I../../guile-gtk -I$(srcdir)/../../guile-gtk \
	-I$(includedir) $(GUILE_INCS) $(GNOME_INCLUDEDIR)

GHTTP_LIBS = -lghttp

lib_LTLIBRARIES = libguileghttp.la
libguileghttp_la_SOURCES = ghttp-glue.c 
libguileghttp_la_LIBADD = $(GNOME_LIBDIR) $(GNOMEUI_LIBS) $(GHTTP_LIBS)
libguileghttp_la_LDFLAGS = -export-dynamic


scmgnomedir = $(datadir)/guile/gnome

install-data-local:
	$(mkinstalldirs) $(scmgnomedir)
	$(INSTALL_DATA) $(srcdir)/ghttp.scm $(scmgnomedir)/ghttp.scm

EXTRA_DIST = ghttp.defs ghttp.scm ghttp-glue.c
            
BUILT_SOURCES = ghttp-glue.c 
SUFFIXES = .x
.c.x:
	guile-snarf $(DEFS) $(INCLUDES) $(CPPFLAGS) $(CFLAGS) $< >tmp \
	&& mv tmp $@

Makefile: $(BUILT_SOURCES)

defsdir = $(datadir)/guile-gnome/
defs_DATA = ghttp.defs

## Convenience variable.
BUILD = guile -l ../../guile-gtk/gtk/config.scm -s $(srcdir)/../../guile-gtk/build-guile-gtk

ghttp-glue.c: ghttp.defs ../../guile-gtk/build-guile-gtk
	$(BUILD) -I $(top_srcdir)/guile-gtk glue $(srcdir)/ghttp.defs > $<.tmpg \
        && mv $<.tmpg $@




