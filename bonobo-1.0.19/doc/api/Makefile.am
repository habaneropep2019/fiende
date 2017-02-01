## Process this file with automake to produce Makefile.in

# The name of the module.
DOC_MODULE=bonobo

# The top-level SGML file.
DOC_MAIN_SGML_FILE=bonobo-api.sgml

# The directory containing the source code (if it contains documentation).
DOC_SOURCE_DIR=$(top_srcdir)/bonobo

CFLAGS="-I../.. `gnome-config --cflags bonobo bonobox bonobox_print`"
LDFLAGS="`gnome-config --libs bonobox bonobox_print`"

TARGET_DIR=$(HTML_DIR)/$(DOC_MODULE)

tmpl_sources = 					\
        tmpl/bonobo-arg.sgml			\
        tmpl/bonobo-async.sgml			\
        tmpl/bonobo-canvas-component.sgml	\
        tmpl/bonobo-canvas-item.sgml		\
        tmpl/bonobo-client-site.sgml		\
        tmpl/bonobo-context.sgml		\
        tmpl/bonobo-control-frame.sgml		\
        tmpl/bonobo-control.sgml		\
        tmpl/bonobo-embeddable.sgml		\
        tmpl/bonobo-event-source.sgml		\
        tmpl/bonobo-exception.sgml		\
        tmpl/bonobo-generic-factory.sgml	\
        tmpl/bonobo-item-container.sgml		\
        tmpl/bonobo-item-handler.sgml		\
        tmpl/bonobo-listener.sgml		\
        tmpl/bonobo-main.sgml			\
        tmpl/bonobo-moniker.sgml		\
        tmpl/bonobo-moniker-extender.sgml	\
        tmpl/bonobo-moniker-simple.sgml		\
        tmpl/bonobo-moniker-util.sgml		\
        tmpl/bonobo-object-client.sgml		\
        tmpl/bonobo-object.sgml			\
        tmpl/bonobo-xobject.sgml		\
        tmpl/bonobo-persist-file.sgml		\
        tmpl/bonobo-persist-stream.sgml		\
        tmpl/bonobo-persist.sgml		\
        tmpl/bonobo-plug.sgml			\
        tmpl/bonobo-print-client.sgml		\
        tmpl/bonobo-print.sgml			\
        tmpl/bonobo-property-bag-client.sgml	\
        tmpl/bonobo-property-bag-xml.sgml	\
        tmpl/bonobo-property-bag.sgml		\
        tmpl/bonobo-property-control.sgml	\
        tmpl/bonobo-selector.sgml		\
        tmpl/bonobo-socket.sgml			\
        tmpl/bonobo-storage-plugin.sgml		\
        tmpl/bonobo-storage.sgml		\
        tmpl/bonobo-stream-client.sgml		\
        tmpl/bonobo-stream-memory.sgml		\
        tmpl/bonobo-stream.sgml			\
        tmpl/bonobo-transient.sgml		\
        tmpl/bonobo-ui-component.sgml		\
        tmpl/bonobo-ui-container.sgml		\
        tmpl/bonobo-ui-engine.sgml		\
        tmpl/bonobo-ui-node.sgml		\
        tmpl/bonobo-ui-sync.sgml		\
        tmpl/bonobo-ui-util.sgml		\
        tmpl/bonobo-unused.sgml			\
        tmpl/bonobo-view-frame.sgml		\
        tmpl/bonobo-view.sgml			\
        tmpl/bonobo-widget.sgml			\
        tmpl/bonobo-win.sgml			\
        tmpl/bonobo-wrapper.sgml

bonobo_docdir = $(HTML_DIR)
bonobo_doc_DATA = \
	bonobo-api.sgml		\
	bonobo-api.html		\
	bonobo.hierarchy	\
	bonobo.signals		\
	bonobo.types		\
	bonobo-decl.txt		\
	bonobo-sections.txt

EXTRA_DIST = $(bonobo_doc_DATA)

if ENABLE_GTK_DOC
bonobo-api.html: html/book1.html
	-cp $(srcdir)/html/book1.html $(srcdir)/bonobo-api.html
else
bonobo-api.html:
endif

html/book1.html: sgml/bonobo-doc.bottom
	$(MAKE) html

sgml/bonobo-doc.bottom: $(tmpl_sources)
	$(MAKE) sgml

scan:
	-(cd $(srcdir) \
	&& env CFLAGS=$(CFLAGS) LDFLAGS=$(LDFLAGS) \
		 gtkdoc-scanobj --module=$(DOC_MODULE) \
	&& gtkdoc-scan --module=$(DOC_MODULE) 		\
		../../bonobo/bonobo-arg.h			\
		../../bonobo/bonobo-async.h			\
		../../bonobo/bonobo-canvas-component.h		\
		../../bonobo/bonobo-canvas-item.h		\
		../../bonobo/bonobo-client-site.h		\
		../../bonobo/bonobo-control-frame.h		\
		../../bonobo/bonobo-control.h			\
		../../bonobo/bonobo-context.h			\
		../../bonobo/bonobo-embeddable.h		\
		../../bonobo/bonobo-event-source.h		\
		../../bonobo/bonobo-exception.h			\
		../../bonobo/bonobo-generic-factory.h		\
		../../bonobo/bonobo-item-handler.h		\
		../../bonobo/bonobo-item-container.h		\
		../../bonobo/bonobo-listener.h			\
		../../bonobo/bonobo-main.h			\
		../../bonobo/bonobo-moniker.h			\
		../../bonobo/bonobo-moniker-extender.h		\
		../../bonobo/bonobo-moniker-simple.h		\
		../../bonobo/bonobo-moniker-util.h		\
		../../bonobo/bonobo-object-client.h		\
		../../bonobo/bonobo-object.h			\
		../../bonobo/bonobo-xobject.h			\
		../../bonobo/bonobo-persist-file.h		\
		../../bonobo/bonobo-persist-stream.h		\
		../../bonobo/bonobo-persist.h			\
		../../bonobo/bonobo-plug.h			\
		../../bonobo/bonobo-property-bag.h		\
		../../bonobo/bonobo-property-bag-client.h	\
		../../bonobo/bonobo-property-bag-xml.h		\
		../../bonobo/bonobo-property-control.h		\
		../../bonobo/bonobo-selector.h			\
		../../bonobo/bonobo-socket.h			\
		../../bonobo/bonobo-storage-plugin.h		\
		../../bonobo/bonobo-storage.h			\
		../../bonobo/bonobo-stream-client.h		\
		../../bonobo/bonobo-stream-memory.h		\
		../../bonobo/bonobo-stream.h			\
		../../bonobo/bonobo-transient.h			\
		../../bonobo/bonobo-ui-component.h            	\
		../../bonobo/bonobo-ui-container.h            	\
		../../bonobo/bonobo-ui-engine.h            	\
		../../bonobo/bonobo-ui-node.h                 	\
		../../bonobo/bonobo-ui-sync.h                 	\
		../../bonobo/bonobo-ui-util.h			\
		../../bonobo/bonobo-view-frame.h		\
		../../bonobo/bonobo-view.h			\
		../../bonobo/bonobo-widget.h 			\
		../../bonobo/bonobo-win.h			\
		../../bonobo/bonobo-wrapper.h			\
		../../bonobo/bonobo-print.h			\
		../../bonobo/bonobo-print-client.h )

templates:
	cd $(srcdir) && gtkdoc-mktmpl --module=$(DOC_MODULE)

sgml:
	cd $(srcdir) \
	&& gtkdoc-mkdb --module=$(DOC_MODULE) --source-dir=$(top_srcdir)

html:
	test -d $(srcdir)/html || mkdir $(srcdir)/html
	-cd $(srcdir)/html && gtkdoc-mkhtml $(DOC_MODULE) ../$(DOC_MAIN_SGML_FILE)

clean-local:
	rm -f *~ *.bak *-unused.txt

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
	  gtkdoc-fixxref --module=$(DOC_MODULE) --html-dir=$(HTML_DIR)|| true; \
	fi)

dist-hook:
	mkdir $(distdir)/html
	mkdir $(distdir)/sgml
	mkdir $(distdir)/tmpl
	-cp $(srcdir)/html/*.html $(srcdir)/html/*.css $(distdir)/html
	-cp $(srcdir)/tmpl/*.sgml $(distdir)/tmpl
	-cp $(srcdir)/sgml/*.sgml $(distdir)/sgml
	-cp $(srcdir)/sgml/*.bottom $(srcdir)/sgml/*.top $(distdir)/sgml

.PHONY : html sgml templates scan

