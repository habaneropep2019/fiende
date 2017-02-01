# If you want to build these components, please cd to their respective
# directories and type `make'.

always_built_SUBDIRS = cddbslave

SUBDIRS = intl po macros omf-install $(always_built_SUBDIRS) \
	  @PROGRAMS_VUMETER@ @PROGRAMS_GMIX@ @PROGRAMS_TCD@ @PROGRAMS_GRECORD@

DIST_SUBDIRS = intl po macros omf-install $(always_built_SUBDIRS) \
	       vu-meter gmix tcd grecord

EXTRA_DIST = gnome-media.spec.in configure acinclude.m4 COPYING-DOCS \
		sgmldocs.make xml-i18n-extract.in xml-i18n-merge.in \
		xml-i18n-update.in

dist-hook: gnome-media.spec
	cp gnome-media.spec $(distdir)

## to automatically rebuild aclocal.m4 if any of the macros in `macros/' change
@MAINT@include macros/macros.dep
@MAINT@macros/macros.dep: macros/Makefile.am
@MAINT@	cd macros && $(MAKE) macros.dep
