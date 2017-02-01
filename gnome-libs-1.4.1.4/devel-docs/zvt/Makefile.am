## Process this file with automake to produce Makefile.in
# The name of the module.
DOC_MODULE=zvt

# The top-level SGML file.
DOC_MAIN_SGML_FILE=zvt-docs.sgml

# The directory containing the source code (if it contains documentation).
DOC_SOURCE_DIR=$(GNOME_LIBS_DIR)/zvt

HTML_DIR=$(datadir)/gnome/html

TARGET_DIR=$(HTML_DIR)/$(DOC_MODULE)

tmpl_sources =				\
	tmpl/zvtterm.sgml

gnome_docdir = $(HTML_DIR)
gnome_doc_DATA = 			\
	zvtterm.html			\
	zvt-decl.txt			\
	zvt-sections.txt

EXTRA_DIST = $(gnome_doc_DATA)

if ENABLE_GTK_DOC
zvtterm.html: html/book1.html
	-cd $(srcdir) && cp html/book1.html zvtterm.html
else
zvtterm.html:
endif

html/book1.html: sgml/zvtterm.sgml
	$(MAKE) html

sgml/zvtterm.sgml: $(tmpl_sources)
	$(MAKE) sgml	

scan:
	cd $(srcdir) \
	&& gtkdoc-scan --module=$(DOC_MODULE) \
		       --source-dir=$(DOC_SOURCE_DIR)

templates: scan
	cd $(srcdir) && gtkdoc-mktmpl --module=$(DOC_MODULE)

sgml:
	cd $(srcdir) \
	&& gtkdoc-mkdb --module=$(DOC_MODULE) --source-dir=$(DOC_SOURCE_DIR)

html:
	test -d $(srcdir)/html || mkdir $(srcdir)/html
	-cd $(srcdir)/html && gtkdoc-mkhtml $(DOC_MODULE) ../$(DOC_MAIN_SGML_FILE)

clean-local:
	rm -f *~ *.bak *.hierarchy *.signals *-unused.txt

maintainer-clean-local: clean
	cd $(srcdir) && rm -rf sgml html $(DOC_MODULE)-decl-list.txt $(DOC_MODULE)-decl.txt 

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
	  echo '-- Installing $(srcdir)/html/book1.html' ; \
	  $(INSTALL_DATA) $(srcdir)/html/book1.html $(DESTDIR)$(TARGET_DIR); \
	  echo '-- Fixing Crossreferences' ; \
	  gtkdoc-fixxref --module=$(DOC_MODULE) --html-dir=$(HTML_DIR)|| true; \
	fi)

dist-hook:
	mkdir $(distdir)/html
	mkdir $(distdir)/sgml
	mkdir $(distdir)/tmpl
	-cp $(srcdir)/html/*.html $(srcdir)/html/*.css $(distdir)/html
	-cp $(srcdir)/tmpl/*.sgml $(distdir)/tmpl
	-cp $(srcdir)/sgml/*.sgml $(distdir)/sgml

.PHONY : html sgml templates scan
