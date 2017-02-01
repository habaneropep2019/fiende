## Process this file with automake to produce Makefile.in
# The name of the module.
DOC_MODULE=gnome

# The top-level SGML file.
DOC_MAIN_SGML_FILE=gnome-docs.sgml

# The directory containing the source code (if it contains documentation).
DOC_SOURCE_DIR=$(GNOME_LIBS_DIR)/libgnome

HTML_DIR=$(datadir)/gnome/html

TARGET_DIR=$(HTML_DIR)/$(DOC_MODULE)

tmpl_sources =				\
	tmpl/gnome-config.sgml		\
	tmpl/gnome-defs.sgml		\
	tmpl/gnome-dentry.sgml		\
	tmpl/gnome-exec.sgml		\
	tmpl/gnome-fileconvert.sgml	\
	tmpl/gnome-help.sgml		\
	tmpl/gnome-history.sgml		\
	tmpl/gnome-i18n.sgml		\
	tmpl/gnome-metadata.sgml	\
	tmpl/gnome-mime-info.sgml	\
	tmpl/gnome-mime.sgml		\
	tmpl/gnome-paper.sgml		\
	tmpl/gnome-popt.sgml		\
	tmpl/gnome-regex.sgml		\
	tmpl/gnome-remote.sgml		\
	tmpl/gnome-score.sgml		\
	tmpl/gnome-sound.sgml		\
	tmpl/gnome-triggers.sgml	\
	tmpl/gnome-unused.sgml		\
	tmpl/gnome-url.sgml		\
	tmpl/gnome-util.sgml		\
	tmpl/libgnome.sgml

gnome_docdir = $(HTML_DIR)
gnome_doc_DATA = 			\
	gnome.html			\
	gnome-decl.txt			\
	gnome-sections.txt

EXTRA_DIST = $(gnome_doc_DATA)

if ENABLE_GTK_DOC
gnome.html: html/book1.html
	-cd $(srcdir) && cp html/book1.html gnome.html
else
gnome.html:
endif

html/book1.html: sgml/libgnome.sgml
	$(MAKE) html

sgml/libgnome.sgml: $(tmpl_sources)
	$(MAKE) sgml	

scan:
	cd $(srcdir) \
	&& gtkdoc-scan --module=$(DOC_MODULE) \
		       --source-dir=$(DOC_SOURCE_DIR) \
	               --ignore-headers="libgnomeP.h gnome-i18nP.h \
					 gnome-triggersP.h gnome-magic.h \
					 vroot.h"

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
	  echo '-- Installing $(srcdir)/html/index.sgml' ; \
	  $(INSTALL_DATA) $(srcdir)/html/index.sgml $(DESTDIR)$(TARGET_DIR); \
	  echo '-- Fixing Crossreferences' ; \
	  gtkdoc-fixxref --module=$(DOC_MODULE) --html-dir=$(HTML_DIR) || true; \
	fi)

dist-hook:
	mkdir $(distdir)/html
	mkdir $(distdir)/sgml
	mkdir $(distdir)/tmpl
	-cp $(srcdir)/html/*.html $(srcdir)/html/*.css $(distdir)/html
	-cp $(srcdir)/tmpl/*.sgml $(distdir)/tmpl
	-cp $(srcdir)/sgml/*.sgml $(distdir)/sgml

.PHONY : html sgml templates scan
