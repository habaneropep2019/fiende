if GNOMEMM
GNOME_SUBDIR=gnome--
else
GNOME_SUBDIR=
endif

SUBDIRS=build_sources $(GNOME_SUBDIR)
DISTDIRS= build_sources gnome--

EXTRA_DIST = $(GNOME_GEN_H_FILES) gnome--.cc gnome--.h

install-data-hook: 
	if test "X$(gnomemm_includedir)" != "X" && test -w "$(DESTDIR)$(gnomemm_includedir)" ; then \
	   $(INSTALL_DATA) $(srcdir)/gnome--.h $(DESTDIR)$(gnomemm_includedir); \
	else \
	   echo "WARNING : Couldn't install $(srcdir)/gnome--.h in $(DESTDIR)$(gnomemm_includedir) : can't write in $(DESTDIR)$(gnomemm_includedir)"; \
	fi ; \
        gnomedir=`gnome-config --libdir`;  \
	if [ -w "$(DESTDIR)$$gnomedir" ]; then \
	   $(INSTALL_DATA) $(srcdir)/gnomemmConf.sh $(DESTDIR)$$gnomedir; \
	else \
	   echo "WARNING : Couldn't install $(srcdir)/gnomemmConf.sh in $(DESTDIR)$$gnomedir : can't write in $(DESTDIR)$$gnomedir"; \
	fi

all-local:
	@echo "*** gnome-- finished ***"
