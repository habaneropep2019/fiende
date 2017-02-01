Utilitiesdir = $(datadir)/gnome/apps/Utilities
GNOMEG = $(bindir)/gnomeg

bin_SCRIPTS = cromagnon
Utilities_in_files = Cromagnon.desktop.in
Utilities_DATA = $(Utilities_in_files:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

cromagnon: cromagnon.scm Makefile
	sed -e "s#@GNOMEG@#$(GNOMEG)#" -e "s#@MYVERSION@#$(VERSION)#"<$< >$@
	chmod +x $@

EXTRA_DIST = \
	cromagnon.scm \
	$(Utilities_DATA) \
	$(Utilities_in_files)
