## Process this file with automake to produce Makefile.in.

# Actually the earliest release that should be OK should be 1.2e, I think.
# Still, 1.3 is the first "released" automake that is OK.
AUTOMAKE_OPTIONS = 1.3

bin_SCRIPTS = gnome-config gnome-bug

if COMPILE_TEST_GNOME
test_gnome = test-gnome test-suite
else
test_gnome =
endif

SUBDIRS = po intl support macros \
	tools \
	images \
	libart_lgpl \
	libgnome libgnomeui gtk-xmhtml zvt \
	libgnorba idl $(test_gnome) devel-docs gnome-data man

gnomeincludedir=$(includedir)/gnome-1.0
gnomeinclude_HEADERS = gnome.h

EXTRA_DIST =	HACKING gnomeConf.sh.in gnome-bug.in gnome-config.in \
		gnome-libs.spec.in

dist-hook: gnome-libs.spec
	cp gnome-libs.spec $(distdir)

release:
	$(MAKE) dist distdir=$(PACKAGE)$(VERSION)

## Put `exec' in the name because this should be installed by
## `install-exec', not `install-data'.
confexecdir=$(libdir)
confexec_DATA=gnomeConf.sh

## to automatically rebuild aclocal.m4 if any of the macros in
## `macros/' change
@MAINT@include macros/macros.dep
@MAINT@macros/macros.dep: macros/Makefile.am
@MAINT@	cd macros && $(MAKE) macros.dep

## We create gnomeConf.sh here and not from configure because we want
## to get the paths expanded correctly.  Macros like srcdir are given
## the value NONE in configure if the user doesn't specify them (this
## is an autoconf feature, not a bug).
gnomeConf.sh: gnomeConf.sh.in Makefile
## Use sed and then mv to avoid problems if the user interrupts.
	sed -e 's^\@GNOME_LIBDIR\@^$(GNOME_LIBDIR)^g' \
	    -e 's^\@GNOME_INCLUDEDIR\@^$(GNOME_INCLUDEDIR)^g' \
	    -e 's^\@GNOME_LIBS\@^$(GNOME_LIBS)^g' \
	    -e 's^\@GNOMEUI_LIBS\@^$(GNOMEUI_LIBS)^g' \
	    -e 's^\@GTKXMHTML_LIBS\@^$(GTKXMHTML_LIBS)^g' \
            -e 's^\@GTK_CFLAGS\@^$(GTK_CFLAGS)^g' \
	    -e 's^\@need_gnome_support\@^$(need_gnome_support)^g' \
	      < $(srcdir)/gnomeConf.sh.in > gnomeConf.tmp \
	  && mv gnomeConf.tmp gnomeConf.sh

CLEANFILES=gnomeConf.sh
