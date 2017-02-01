## Process this file with automake to produce Makefile.in
## borrowed from gdk-pixbuf

# The name of the module.
DOC_MODULE=libole2

# The top-level SGML file.
DOC_MAIN_SGML_FILE=libole2.sgml

# The directory containing the source code (if it contains documentation).
DOC_SOURCE_DIR=$(LIBOLE2_DIR)/libole2

HTML_DIR=$(datadir)/libole2/html

TARGET_DIR=$(HTML_DIR)/$(DOC_MODULE)

tmpl_sources =					\
	tmpl/ms-ole.sgml			\
	tmpl/ms-ole-summary.sgml		\
	tmpl/ms-ole-miscellaneous.sgml

libole2_docdir = $(HTML_DIR)
libole2_doc_DATA =			\
	libole2.html			\
	libole2-decl.txt		\
	libole2-sections.txt

EXTRA_DIST = $(libole2_doc_DATA) $(DOC_MAIN_SGML_FILE)


if ENABLE_GTK_DOC
libole2.html: html/index.html
	-cd $(srcdir) && cp html/index.html libole2.html
else
libole2.html:
endif

html/index.html: sgml/libole2-doc.bottom
	$(MAKE) html

sgml/libole2-doc.bottom: $(tmpl_sources)
	$(MAKE) sgml

libole2-decl.txt:
	$(MAKE) templates

scan:
	-(cd $(srcdir) \
	&& gtkdoc-scan --module=$(DOC_MODULE) --source-dir=$(DOC_SOURCE_DIR) )

templates: scan
	cd $(srcdir) && gtkdoc-mktmpl --module=$(DOC_MODULE)

sgml: libole2-decl.txt
	cd $(srcdir) \
	&& gtkdoc-mkdb --module=$(DOC_MODULE) --source-dir=$(DOC_SOURCE_DIR)

html:
	test -d $(srcdir)/html || mkdir $(srcdir)/html
	-cd $(srcdir)/html && gtkdoc-mkhtml $(DOC_MODULE) ../$(DOC_MAIN_SGML_FILE)


clean-local:
	rm -f *~ *.bak *.signals *-unused.txt

maintainer-clean-local: clean
	cd $(srcdir) && rm -rf sgml html $(DOC_MODULE)-decl-list.txt libole2.html

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(TARGET_DIR)
	(installfiles=`echo $(srcdir)/html/*.html`; \
	if test "$$installfiles" = '$(srcdir)/html/*.html'; \
	then echo '-- Nothing to install' ; \
	else \
	  for i in $$installfiles; do \
	    echo '-- Installing '$$i ; \
	    $(INSTALL_DATA) $$i $(DESTDIR)$(TARGET_DIR); \
	  done; \
	  echo '-- Installing $(srcdir)/html/index.sgml' ; \
	  $(INSTALL_DATA) $(srcdir)/html/index.sgml $(DESTDIR)$(TARGET_DIR); \
	  echo '-- Fixing Crossreferences' ; \
	  gtkdoc-fixxref --module=$(DOC_MODULE) --html-dir=$(HTML_DIR)|| true; \
	fi)

dist-hook:
	mkdir $(distdir)/html
	mkdir $(distdir)/sgml
	mkdir $(distdir)/tmpl
	cp $(srcdir)/html/*.html $(distdir)/html
	-cp $(srcdir)/html/*.css $(distdir)/html
	cp $(srcdir)/tmpl/*.sgml $(distdir)/tmpl
	cp $(srcdir)/sgml/*.sgml $(distdir)/sgml
	cp $(srcdir)/sgml/*.bottom $(srcdir)/sgml/*.top $(distdir)/sgml


.PHONY : html sgml templates scan
