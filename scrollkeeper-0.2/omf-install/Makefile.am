omf_dest_dir=$(datadir)/omf/scrollkeeper
scrollkeeper_localstate_dir = $(localstatedir)/$(SCROLLKEEPER_PARTIAL_DB_DIR)

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(omf_dest_dir)
	-for file in $(srcdir)/*.omf; do \
		$(INSTALL_DATA) $(srcdir)/$$file $(DESTDIR)$(omf_dest_dir); \
	done
	rm -rf $(DESTDIR)$(scrollkeeper_localstate_dir)	
	$(mkinstalldirs) $(DESTDIR)$(scrollkeeper_localstate_dir)
	-scrollkeeper-update -p $(scrollkeeper_localstate_dir) 

uninstall-local:
	-for file in $(srcdir)/*.omf; do \
		basefile=`basename $$file`; \
		rm -f $(omf_dest_dir)/$$basefile; \
        done
	rm -rf $(DESTDIR)$(scrollkeeper_localstate_dir)
