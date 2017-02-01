SUBDIRS = po idl macros \
	 libversit gncal gnomecard

EXTRA_DIST = gnome-pim.spec.in gnome-pim.keys.in gnome-pim.spec


appicondir = $(datadir)/pixmaps

Mimedir = $(datadir)/mime-info
Mime_DATA = gnome-pim.keys

gnome-pim.keys: gnome-pim.keys.in
	sed -e 's^\@icondir\@^$(appicondir)^g' < $(srcdir)/gnome-pim.keys.in > gnome-pim.keys.tmp \
	&& mv gnome-pim.keys.tmp gnome-pim.keys 


## to automatically rebuild aclocal.m4 if any of the macros in
## `macros/' change
@MAINT@include macros/macros.dep
@MAINT@macros/macros.dep: macros/Makefile.am
@MAINT@	cd macros && $(MAKE) macros.dep
