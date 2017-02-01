SUBDIRS = help macros po intl src pixmaps omf-install

## we don't install these just yet
Graphicsdir = $(datadir)/gnome/apps/Graphics
Graphics_in_files = ggv.desktop.in
Graphics_DATA = $(Graphics_in_files:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

oafdir = $(datadir)/oaf

oaf_DATA = Bonobo_GGV.oafinfo


EXTRA_DIST = 		\
	ggv.desktop.in 	\
	ggv.spec.in	\
	app-docs.make   \
	xml-i18n-extract.in \
	xml-i18n-update.in  \
	xml-i18n-merge.in   \
	COPYING-DOCS    \
	$(oaf_DATA)

dist-hook: ggv.spec
	cp ggv.spec $(distdir)

## to automatically rebuild aclocal.m4 if any of the macros in `macros/' change
@MAINT@include macros/macros.dep
@MAINT@macros/macros.dep: macros/Makefile.am
@MAINT@	cd macros && $(MAKE) macros.dep
