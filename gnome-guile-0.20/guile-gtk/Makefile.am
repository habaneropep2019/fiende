# -*- Makefile -*-

autoconf_macros = gnome-guile-checks.m4
acinclude.m4: $(autoconf_macros)
	cd $(srcdir); \
	cat $(autoconf_macros) > acim.tmp; \
	mv acim.tmp acinclude.m4

bin_PROGRAMS    = $(GUILE_GTK_VERSION)
bin_SCRIPTS     = build-guile-gtk
lib_LTLIBRARIES = $(LIBGUILEGTK_VERSION) $(LIBGUILEDLOPENHELPER)

EXTRA_PROGRAMS = guile-gtk-1.2 guile-gtk-1.3
EXTRA_LTLIBRARIES = libguilegtk-1.2.la libguilegtk-1.3.la \
                    libguiledlopenhelper.la

guile_gtk_1_2_SOURCES = main.c
guile_gtk_1_2_LDFLAGS = -export-dynamic
guile_gtk_1_2_LDADD = libguilegtk-1.2.la $(GUILE_LIBS) $(GTK_LIBS)

guile_gtk_1_3_SOURCES = main.c
guile_gtk_1_3_LDFLAGS = -export-dynamic
guile_gtk_1_3_LDADD = libguilegtk-1.3.la $(GUILE_LIBS) $(GTK_LIBS)

INCLUDES = $(GTK_CFLAGS) -I$(includedir) $(GUILE_INCS)

MKDEP = gcc -M -DMKDEP $(DEFS) $(INCLUDES) $(CPPFLAGS) $(CFLAGS)

libguilegtk_1_2_la_SOURCES = guile-gtk.c \
	                     guile-gtk.h \
                             guile-compat.c \
                             gtk-compat.c \
			     gtk-support.c \
                             gtk-threads.c \
                             gtk-threads.h \
	                     gdk-glue.c \
			     gtk-glue.c

# Automake can't figure out the rpath for EXTRA_ libraries.  Yuck.
libguilegtk_1_2_la_LDFLAGS = -rpath $(libdir) -version-info 0:0:0 \
                             -export-dynamic
libguilegtk_1_2_la_LIBADD = $(GTK_LIBS)

libguilegtk_1_3_la_SOURCES = guile-gtk.c \
	                     guile-gtk.h \
                             guile-compat.c \
                             gtk-compat.c \
			     gtk-support.c \
                             gtk-threads.c \
                             gtk-threads.h \
	                     gdk-glue.c \
			     gtk-glue.c

libguilegtk_1_3_la_LDFLAGS = -rpath $(libdir) -version-info 0:0:0 \
                             -export-dynamic
libguilegtk_1_3_la_LIBADD = $(GTK_LIBS)

all: libguilegtk.la

libguilegtk.la:
	rm -f libguilegtk.la
	$(LN_S) $(LIBGUILEGTK_VERSION) libguilegtk.la

libguiledlopenhelper_la_SOURCES = guiledlopenhelper.c
libguiledlopenhelper_la_LDFLAGS = -rpath $(libdir) -version-info 0:0:0 \
                                  -export-dynamic
libguiledlopenhelper_la_LIBADD = $(GUILE_LIBS)

include_HEADERS = guile-gtk.h

install-exec-local:
	mkdir -p $(bindir)
	cd $(bindir) && rm -f guile-gtk && $(LN_S) $(GUILE_GTK_VERSION) guile-gtk

gtkmoduledir = $(datadir)/guile/gtk
vgtkmoduledir = $(datadir)/guile/gtk-$(GTK_VERSION)
vscmsrcdir = $(srcdir)/gtk-$(GTK_VERSION)
install-data-local:
	$(mkinstalldirs) $(vgtkmoduledir) $(gtkmoduledir)
	$(INSTALL_DATA) $(vscmsrcdir)/gtk.scm $(vgtkmoduledir)/gtk.scm
	$(INSTALL_DATA) $(vscmsrcdir)/gdk.scm $(vgtkmoduledir)/gdk.scm
	$(INSTALL_DATA) $(srcdir)/gtk/gtk.scm  $(gtkmoduledir)/gtk.scm
	$(INSTALL_DATA) $(srcdir)/gtk/gdk.scm  $(gtkmoduledir)/gdk.scm
	$(INSTALL_DATA) $(srcdir)/gtk/dynlink.scm  $(gtkmoduledir)/dynlink.scm
	$(INSTALL_DATA) $(srcdir)/gtk/dynlink-old.scm  $(gtkmoduledir)/dynlink-old.scm
	$(INSTALL_DATA) ./gtk/config.scm  $(gtkmoduledir)/config.scm
	$(INSTALL_DATA) $(srcdir)/gtk/repl.scm $(gtkmoduledir)/repl.scm
	$(INSTALL_DATA) $(srcdir)/gtk/threads.scm $(gtkmoduledir)/threads.scm

gtkdefs = gtk-$(GTK_VERSION).defs
gdkdefs = gdk-$(GTK_VERSION).defs

defsdir = $(datadir)/guile-gtk/
defs_DATA = $(gtkdefs) $(gdkdefs) 

BUILT_SOURCES = gdk-glue.c gtk-glue.c guile-gtk.x gtk-support.x main.c

CLEANFILES = gdk-glue.c gtk-glue.c guile-gtk.x gtk-support.x main.c

SUFFIXES = .x
.c.x:
	guile-snarf $(DEFS) $(INCLUDES) $(CPPFLAGS) $(CFLAGS) $< >$<.tmp && mv $<.tmp $@

BUILD = guile -l ./gtk/config.scm -s $(srcdir)/build-guile-gtk

gtk-glue.c: $(gtkdefs) $(gdkdefs) build-guile-gtk
	$(BUILD) -I $(srcdir) glue $(gtkdefs) >tmpt && mv tmpt $@

gdk-glue.c: $(gdkdefs) build-guile-gtk
	$(BUILD) -I $(srcdir) glue $(gdkdefs) >tmpd && mv tmpd $@

main.c:  $(gtkdefs) $(gdkdefs) build-guile-gtk
	$(BUILD) -I $(srcdir) main $(gtkdefs) $(gdkdefs) >tmpm && mv tmpm $@

Makefile: $(BUILT_SOURCES)

EXTRA_DIST = \
             gdk-1.2.defs gtk-1.2.defs \
             gdk-1.3.defs gtk-1.3.defs \
             gtk/gtk.scm gtk/gdk.scm gtk/dynlink.scm \
	     gtk/dynlink-old.scm gtk/event-repl.scm \
	     gtk/repl.scm \
             gtk/config.scm.in gtk/threads.scm \
             gtk-1.2/gtk.scm gtk-1.2/gdk.scm \
             gtk-1.3/gtk.scm gtk-1.3/gdk.scm \
             examples/test-gtk.scm examples/test.xpm examples/calc.scm \
	     examples/calcrc examples/hello-world.scm examples/test-gdk.scm \
	     examples/simple.scm examples/continuations.scm \
             examples/Makefile.am examples/Makefile.in \
             examples/configure.in examples/configure \
             examples/foo.c examples/foo.h \
             examples/foo.defs examples/foo.defs.guile \
             examples/foo.scm examples/test-foo.scm \
             examples/tictactoe.scm examples/metaspline.scm \
             examples/clist.scm examples/calendar.scm \
             README.gnome-guile gnome-guile-checks.m4 \
             guile-gtk.spec \
             build-guile-gtk \
	     toolkits/translate/ChangeLog \
	     toolkits/translate/case.scm \
             toolkits/translate/support.scm \
             toolkits/translate/translate.scm


# We don't want to distribute pre-generated glue code because it
# depends on the version of Gtk on the users system.  We could
# distribute pre-generated glue code for both 1.2 and 1.3 (as
# gtk-glue-1.2.c, etc.) but that seems overkill to me.

dist-hook:
	rm -f $(distdir)/gtk-glue.c $(distdir)/gdk-glue.c
