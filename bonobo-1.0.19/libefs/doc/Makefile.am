## Process this file with automake to produce Makefile.in

# The name of the module.
DOC_MODULE=libefs

# The top-level SGML file.
DOC_MAIN_SGML_FILE=libefs.sgml

# The directory containing the source code (if it contains documentation).
DOC_SOURCE_DIR=../src

TARGET_DIR=$(DESTDIR)$(HTML_DIR)/$(DOC_MODULE)

scan:
	gtkdoc-scan --module=$(DOC_MODULE) --source-dir=$(DOC_SOURCE_DIR) \
	--ignore-headers="efs_internal.h"	

templates: scan
	gtkdoc-mktmpl --module=$(DOC_MODULE)

sgml:
	gtkdoc-mkdb --module=$(DOC_MODULE) --source-dir=$(DOC_SOURCE_DIR)

html:
	if ! test -d html ; then mkdir html ; fi
	-cd html && gtkdoc-mkhtml $(DOC_MODULE) ../$(DOC_MAIN_SGML_FILE)
	-cp pictures/*gif html

print:
	-jade -t tex -d /usr/lib/sgml/stylesheet/dsssl/docbook/nwalsh/print/docbook.dsl libefs.sgml
	-mkdir print
	cd print;jadetex ../libefs.tex;jadetex ../libefs.tex;dvips libefs.dvi -o libefs.ps

clean-local:
	rm -f *~ *.bak */*.bak *.hierarchy *.signals *.args *-unused.txt *.log

maintainer-clean-local: clean
	rm -rf sgml html print $(DOC_MODULE)-decl-list.txt $(DOC_MODULE)-decl.txt libefs.tex

install-data-local:
	-install -d -m 0755 $(TARGET_DIR)
	-install -m 0644 html/*.html $(TARGET_DIR)
	-install -m 0644 html/index.sgml $(TARGET_DIR)
	-gtkdoc-fixxref --module=$(DOC_MODULE) --html-dir=$(HTML_DIR)

.PHONY : print html sgml templates scan
