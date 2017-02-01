
EXTRA_DIST = \
	autogen.sh

# Install the autogen.sh script for GTK+ apps.
install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(pkgdatadir)/gtk
	$(INSTALL_DATA) $(srcdir)/autogen.sh $(DESTDIR)$(pkgdatadir)/gtk
