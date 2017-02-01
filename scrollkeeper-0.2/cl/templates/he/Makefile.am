# copyright (C) 2001 Sun Microsystems, Inc.

scrollkeeper_data_dir=$(datadir)/scrollkeeper/Templates/he
EXTRA_DIST=scrollkeeper_cl.xml

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(scrollkeeper_data_dir)
	$(INSTALL_DATA) $(srcdir)/scrollkeeper_cl.xml $(DESTDIR)$(scrollkeeper_data_dir)

