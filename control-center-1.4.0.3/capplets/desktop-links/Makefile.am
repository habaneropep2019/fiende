ccenterdir = $(datadir)/control-center
Multimediadir	= $(ccenterdir)/Multimedia
Desktopdir	= $(ccenterdir)/Desktop
UIOptionsdir    = $(ccenterdir)/UIOptions
Peripheralsdir  = $(ccenterdir)/Peripherals
Sessiondir	= $(ccenterdir)/Session
Documentsdir	= $(ccenterdir)/Documents

paneldir        = $(datadir)/gnome/apps/Settings
Multimediadirp	= $(paneldir)/Multimedia
Desktopdirp	= $(paneldir)/Desktop
UIOptionsdirp   = $(paneldir)/UIOptions
Peripheralsdirp = $(paneldir)/Peripherals
Sessiondirp	= $(paneldir)/Session
Documentsdirp	= $(paneldir)/Documents



directory_in_files =				\
	Gnome.directory.in			\
	Desktop.directory.in			\
	Peripherals.directory.in		\
	Multimedia.directory.in			\
	Session.directory.in			\
	Documents.directory.in			\
	UIOptions.directory.in

noinst_DATA = $(directory_in_files:.directory.in=.directory)
@XML_I18N_MERGE_DIRECTORY_RULE@

EXTRA_DIST = $(directory_in_files) $(noinst_DATA)

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(Desktopdir)
	$(mkinstalldirs) $(DESTDIR)$(Multimediadir)
	$(mkinstalldirs) $(DESTDIR)$(UIOptionsdir)
	$(mkinstalldirs) $(DESTDIR)$(Peripheralsdir)
	$(mkinstalldirs) $(DESTDIR)$(Sessiondir)
	$(mkinstalldirs) $(DESTDIR)$(Documentsdir)
	$(INSTALL_DATA) $(srcdir)/Gnome.directory $(DESTDIR)$(ccenterdir)/.directory
	$(INSTALL_DATA) $(srcdir)/Multimedia.directory $(DESTDIR)$(Multimediadir)/.directory
	$(INSTALL_DATA) $(srcdir)/Desktop.directory $(DESTDIR)$(Desktopdir)/.directory
	$(INSTALL_DATA) $(srcdir)/UIOptions.directory $(DESTDIR)$(UIOptionsdir)/.directory
	$(INSTALL_DATA) $(srcdir)/Peripherals.directory $(DESTDIR)$(Peripheralsdir)/.directory
	$(INSTALL_DATA) $(srcdir)/Session.directory $(DESTDIR)$(Sessiondir)/.directory
	$(INSTALL_DATA) $(srcdir)/Documents.directory $(DESTDIR)$(Documentsdir)/.directory
	$(mkinstalldirs) $(DESTDIR)$(Desktopdirp)
	$(mkinstalldirs) $(DESTDIR)$(Multimediadirp)
	$(mkinstalldirs) $(DESTDIR)$(UIOptionsdirp)
	$(mkinstalldirs) $(DESTDIR)$(Peripheralsdirp)
	$(mkinstalldirs) $(DESTDIR)$(Sessiondirp)
	$(mkinstalldirs) $(DESTDIR)$(Documentsdirp)
	$(INSTALL_DATA) $(srcdir)/Multimedia.directory $(DESTDIR)$(Multimediadirp)/.directory
	$(INSTALL_DATA) $(srcdir)/Desktop.directory $(DESTDIR)$(Desktopdirp)/.directory
	$(INSTALL_DATA) $(srcdir)/UIOptions.directory $(DESTDIR)$(UIOptionsdirp)/.directory
	$(INSTALL_DATA) $(srcdir)/Peripherals.directory $(DESTDIR)$(Peripheralsdirp)/.directory
	$(INSTALL_DATA) $(srcdir)/Session.directory $(DESTDIR)$(Sessiondirp)/.directory
	$(INSTALL_DATA) $(srcdir)/Documents.directory $(DESTDIR)$(Documentsdirp)/.directory
