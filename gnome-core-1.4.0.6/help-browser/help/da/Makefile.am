## Process this file with automake to produce Makefile.in.

EXTRA_DIST = gnome-help-browser-help.html \
	     gnome-help-browser-topic.dat \
	     default-page.html

help_browser_helpdir = $(datadir)/gnome/help/help-browser/da
install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(help_browser_helpdir)
	$(INSTALL_DATA)  $(srcdir)/gnome-help-browser-help.html \
			 $(DESTDIR)$(help_browser_helpdir)/help-browser.html
	$(INSTALL_DATA)  $(srcdir)/gnome-help-browser-topic.dat \
			 $(DESTDIR)$(help_browser_helpdir)/topic.dat
	$(INSTALL_DATA)  $(srcdir)/default-page.html $(DESTDIR)$(help_browser_helpdir)/
	cd $(DESTDIR)$(help_browser_helpdir); \
	     rm -f index.html; ln -s help-browser.html index.html

