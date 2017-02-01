SUBDIRS = doc

INCLUDES = 			\
	-I$(includedir)		\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	$(GNOME_INCLUDEDIR)

bin_PROGRAMS = gshutdown

gshutdown_SOURCES = \
	gshutdown.c

gshutdown_LDADD = \
	$(GNOME_LIBDIR)         \
	$(GTKXMHTML_LIBS)	\
	$(GNOMEUI_LIBS) 	\
	$(INTLLIBS)

man_MANS = gshutdown.8

pixmapsdir = $(datadir)/pixmaps
pixmaps_DATA = gnome-shutdown.png

Systemdir = $(datadir)/gnome/apps/System
System_in_files = gshutdown.desktop.in
System_DATA = $(System_in_files:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

EXTRA_DIST = \
	$(man_MANS) \
	$(pixmaps_DATA) \
	$(System_in_files) \
	$(System_DATA) \
	$(top_srcdir)/sgmldocs.make \
	gshutdown-security.in \
	gshutdown-pam


install-data-local: gshutdown-security
if CONSOLE_HELPER
	$(INSTALL_DATA) -D gshutdown-pam $(DESTDIR)$(PAM_PREFIX)/pam.d/gshutdown
	$(INSTALL_DATA) -D gshutdown-security $(DESTDIR)$(PAM_PREFIX)/security/console.apps/gshutdown
	$(INSTALL_DATA) -m 755 -d $(DESTDIR)$(sbindir)
	mv -f $(DESTDIR)$(bindir)/gshutdown $(DESTDIR)$(sbindir)
	$(LN_S) @CONSOLE_HELPER@ $(DESTDIR)$(bindir)/gshutdown
endif
