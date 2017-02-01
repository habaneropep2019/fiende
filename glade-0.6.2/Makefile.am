## Process this file with automake to produce Makefile.in

SUBDIRS = macros intl glade po doc omf-install

Developmentdir = $(datadir)/gnome/apps/Development
Development_in_files = glade.desktop.in
Development_DATA = $(Development_in_files:.desktop.in=.desktop)

@XML_I18N_MERGE_DESKTOP_RULE@

EXTRA_DIST = \
	FAQ \
	glade.spec.in \
	glade.desktop.in \
	glade.desktop \
	glade.png \
	xml-i18n-extract.in \
	xml-i18n-merge.in \
	xml-i18n-update.in \
	examples/editor/AUTHORS \
	examples/editor/ChangeLog \
	examples/editor/Makefile.am \
	examples/editor/NEWS \
	examples/editor/README \
	examples/editor/acconfig.h \
	examples/editor/autogen.sh \
	examples/editor/configure.in \
	examples/editor/editor.glade \
	examples/editor/src/Makefile.am \
	examples/editor/src/callbacks.c \
	examples/editor/src/callbacks.h \
	examples/editor/src/interface.c \
	examples/editor/src/interface.h \
	examples/editor/src/main.c \
	examples/editor/src/support.c \
	examples/editor/src/support.h \
	examples/editor/pixmaps/new.xpm \
	examples/editor/pixmaps/open.xpm \
	examples/editor/pixmaps/save.xpm \
	examples/editor/po/ChangeLog \
	examples/editor/po/POTFILES.in

pixmapsdir = $(datadir)/pixmaps
pixmaps_DATA = glade.png

dist-hook: glade.spec
	cp glade.spec $(distdir)


# Install all the Gnome m4 macros we use to build Glade, so we can copy them
# to each project's directory (for Gnome projects).
install-data-local:
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(pkgdatadir)/gnome
	$(INSTALL_DATA) $(srcdir)/macros/Makefile.am $(DESTDIR)$(pkgdatadir)/gnome
	$(INSTALL_DATA) $(srcdir)/macros/autogen.sh $(DESTDIR)$(pkgdatadir)/gnome
	for macro_file in $(srcdir)/macros/*.m4; do \
	  $(INSTALL_DATA) $$macro_file $(DESTDIR)$(pkgdatadir)/gnome; \
	done
	$(mkinstalldirs) $(DESTDIR)$(datadir)/pixmaps/glade
	$(INSTALL_DATA) $(srcdir)/glade/graphics/glade_logo.png $(DESTDIR)$(datadir)/pixmaps/glade

