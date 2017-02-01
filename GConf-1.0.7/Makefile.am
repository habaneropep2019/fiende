
SUBDIRS = gconf backends wrappers po doc examples standard-schemas
DIST_SUBDIRS=tests $(SUBDIRS)

EXTRA_DIST=gconf-config.in gconf.m4.in gconf.spec.in gconf.spec TODO \
	gconf.pc gconfgtk.pc

install-exec-local:
	$(INSTALL_SCRIPT) gconf-config $(DESTDIR)$(bindir)/gconf-config-$(MAJOR_VERSION)
	(cd $(DESTDIR)$(bindir) && $(LN_S) -f gconf-config-$(MAJOR_VERSION) gconf-config)

install-data-local:
	-mkdir -p $(DESTDIR)$(datadir)/aclocal
	$(INSTALL_DATA) gconf.m4 $(DESTDIR)$(datadir)/aclocal/gconf-$(MAJOR_VERSION).m4
	-mkdir -p $(DESTDIR)$(sysconfdir)/gconf/gconf.xml.defaults
	-chmod 755 $(DESTDIR)$(sysconfdir)/gconf/gconf.xml.defaults
	-mkdir -p $(DESTDIR)$(sysconfdir)/gconf/gconf.xml.mandatory
	-chmod 755 $(DESTDIR)$(sysconfdir)/gconf/gconf.xml.mandatory

install-schemas:
	(cd standard-schemas && $(MAKE) $(AM_MAKEFLAGS) install-schemas)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = gconf.pc gconfgtk.pc

