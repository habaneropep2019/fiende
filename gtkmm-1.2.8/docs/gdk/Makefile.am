
all-local: header html

header:	headers
	cp $(top_srcdir)/gdk--/gdk--/*.h headers/
	touch header

headers:
	@mkdir $@

html:
	@PERL@ $(top_srcdir)/docgen/pce2 --plgdir $(top_srcdir)/docgen/ headers/*.h
	touch html

DISTCLEANFILES = headers/*.h *.html
CLEANFILES = headers/*.h *.html html header

DEST=$$USER@gtkmm.sourceforge.net:/home/groups/gtkmm/htdocs

tarball:
	(cd ..; tar cvzf gdk/gdkmm-ref.tar.gz `find gdk -name "*.html"`)

sf_install: tarball
	scp -1 *.html *.tar.gz $(DEST)/docs/gdk
