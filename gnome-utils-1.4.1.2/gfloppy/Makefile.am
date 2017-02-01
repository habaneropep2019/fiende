## Process this file with automake to produce Makefile.in

appsdir = $(datadir)/gnome/apps
Utilitiesdir = $(appsdir)/Utilities
Mimedir = $(datadir)/mime-info

SUBDIRS = doc src

EXTRA_DIST = \
	gfloppy.desktop \
	gfloppy.desktop.in \
	gfloppy.keys \
	gfloppy.keys.in \
	gfloppy.mime

@XML_I18N_MERGE_DESKTOP_RULE@
@XML_I18N_MERGE_KEYS_RULE@

install-data-local: gfloppy.desktop gfloppy.keys
	@$(NORMAL_INSTALL)
	if test -d $(srcdir)/pixmaps; then \
	  $(mkinstalldirs) $(DESTDIR)@PACKAGE_PIXMAPS_DIR@; \
	  for pixmap in $(srcdir)/pixmaps/*; do \
	    if test -f $$pixmap; then \
	      $(INSTALL_DATA) $$pixmap $(DESTDIR)@PACKAGE_PIXMAPS_DIR@; \
	    fi \
	  done \
	fi

	$(mkinstalldirs) $(DESTDIR)$(Utilitiesdir)
	$(mkinstalldirs) $(DESTDIR)$(Mimedir)
	$(INSTALL_DATA) $(srcdir)/gfloppy.desktop $(DESTDIR)$(Utilitiesdir)/
	$(INSTALL_DATA) $(srcdir)/gfloppy.mime $(DESTDIR)$(Mimedir)/
	$(INSTALL_DATA) $(srcdir)/gfloppy.keys $(DESTDIR)$(Mimedir)/

dist-hook:
	if test -d pixmaps; then \
	  mkdir $(distdir)/pixmaps; \
	  for pixmap in pixmaps/*; do \
	    if test -f $$pixmap; then \
	      cp -p $$pixmap $(distdir)/pixmaps; \
	    fi \
	  done \
	fi

