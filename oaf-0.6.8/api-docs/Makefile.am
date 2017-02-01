NULL=

# The name of the module.
DOC_MODULE=liboaf

# The top-level SGML file.
DOC_MAIN_SGML_FILE=liboaf.sgml

# The directory containing the source code (if it contains documentation).
DOC_SOURCE_DIR=$(top_srcdir)/liboaf

tmpl_sources =				\
	tmpl/utilities.sgml		\
	tmpl/initialization.sgml	\
	tmpl/query.sgml			\
	tmpl/registration.sgml		\
	tmpl/debugging.sgml		\
	$(NULL)

.PHONY : templates manual

MANUAL_FILES = html/api-reference.html			\
               html/architecture.html			\
               html/async-activation.html		\
               html/attribute-tag.html			\
               html/book1.html				\
               html/checklist.html			\
               html/configuring.html			\
               html/fdl.html				\
               html/fdl-section10.html			\
               html/fdl-section1.html			\
               html/fdl-section2.html			\
               html/fdl-section3.html			\
               html/fdl-section4.html			\
               html/fdl-section5.html			\
               html/fdl-section6.html			\
               html/fdl-section7.html			\
               html/fdl-section8.html			\
               html/fdl-section9.html			\
               html/fdl-using.html			\
               html/index.sgml				\
               html/liboaf-debugging.html		\
               html/liboaf-initialization.html		\
               html/liboaf-query-and-activation.html	\
               html/liboaf-registration.html		\
               html/liboaf-utilities.html		\
               html/migrating.html			\
               html/oafinfo-ref.html			\
               html/oaf.png				\
               html/query-langage.html			\
               html/query-syntax.html			\
               html/server-arch.html			\
               html/tutorial.html

if ENABLE_GTK_DOC
$(MANUAL_FILES): manual

manual:
	if test ! -d $(top_builddir)/api-docs/tmpl; then			\
		mkdir $(top_builddir)/api-docs/tmpl; 				\
		cp $(srcdir)/tmpl/*.sgml $(top_builddir)/api-docs/tmpl/; 	\
		cp $(srcdir)/*.sgml $(srcdir)/*.txt  $(top_builddir)/api-docs/; \
	fi
	cd $(top_builddir)/api-docs/ \
	&& gtkdoc-mkdb --module=$(DOC_MODULE) --source-dir=$(DOC_SOURCE_DIR)
	if test ! -d $(top_builddir)/api-docs/html; then	\
		mkdir $(top_builddir)/api-docs/html;		\
	fi
	-cd $(top_builddir)/api-docs/html && \
	gtkdoc-mkhtml $(DOC_MODULE) ../$(DOC_MAIN_SGML_FILE)
	-cd $(top_builddir)/api-docs/ && \
	cp $(srcdir)/oaf.png $(top_builddir)/api-docs/html

noinst_DATA=$(MANUAL_FILES)

endif


my_sgml_files = 			\
	tutorial.sgml			\
	migrating.sgml			\
	oafinfo-ref.sgml		\
	configuring.sgml		\
	query-langage.sgml		\
	architecture.sgml		\
	liboaf.sgml			\
	fdl.sgml			\
	$(NULL)

EXTRA_DIST =					\
	liboaf-decl.txt				\
	liboaf-sections.txt			\
	$(my_sgml_files)			\
	$(tmpl_sources)				\
	$(MANUAL_FILES)				\
	$(NULL)


templates:
	cd $(srcdir) && gtkdoc-mktmpl --module=$(DOC_MODULE)


clean-local:
	rm -f *~ *.bak *.signals *-unused.txt

maintainer-clean-local: clean
	cd $(top_builddir)/api-docs/ && rm -rf sgml html $(DOC_MODULE)-decl-list.txt $(DOC_MODULE)-decl.txt



