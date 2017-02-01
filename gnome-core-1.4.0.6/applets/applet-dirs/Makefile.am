appletdir = $(datadir)/applets

Utilitydir       = $(appletdir)/Utility
Monitorsdir      = $(appletdir)/Monitors
Multimediadir    = $(appletdir)/Multimedia
Networkdir       = $(appletdir)/Network
Amusementsdir    = $(appletdir)/Amusements
Clocksdir	 = $(appletdir)/Clocks

directory_in_files = \
	Utility.directory.in	\
	Monitors.directory.in	\
	Multimedia.directory.in	\
	Amusements.directory.in	\
	Network.directory.in	\
	Clocks.directory.in

noinst_DATA = $(directory_in_files:.directory.in=.directory)
@XML_I18N_MERGE_DIRECTORY_RULE@

Directory_files = \
	Utility.directory \
	Monitors.directory \
	Multimedia.directory \
	Amusements.directory \
	Network.directory \
	Clocks.directory

## We explicitly define these so that the appropriate directories will
## be made at install time.
Utility_DATA =
Monitors_DATA =
Multimedia_DATA =
Network_DATA =
Amusements_DATA =
Clocks_DATA =

EXTRA_DIST = $(Utility_DATA)		\
	     $(Monitors_DATA)		\
	     $(Multimedia_DATA)		\
	     $(Network_DATA)		\
	     $(Amusements_DATA)		\
	     $(Clocks_DATA)		\
	     $(Directory_files)		\
	     $(directory_in_files) 	\
	     $(noinst_DATA)

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(Utilitydir)
	$(mkinstalldirs) $(DESTDIR)$(Monitorsdir)
	$(mkinstalldirs) $(DESTDIR)$(Multimediadir)
	$(mkinstalldirs) $(DESTDIR)$(Networkdir)
	$(mkinstalldirs) $(DESTDIR)$(Amusementsdir)
	$(mkinstalldirs) $(DESTDIR)$(Clocksdir)
	$(INSTALL_DATA) Utility.directory $(DESTDIR)$(Utilitydir)/.directory
	$(INSTALL_DATA) Monitors.directory $(DESTDIR)$(Monitorsdir)/.directory
	$(INSTALL_DATA) Multimedia.directory $(DESTDIR)$(Multimediadir)/.directory
	$(INSTALL_DATA) Network.directory $(DESTDIR)$(Networkdir)/.directory
	$(INSTALL_DATA) Amusements.directory $(DESTDIR)$(Amusementsdir)/.directory
	$(INSTALL_DATA) Clocks.directory $(DESTDIR)$(Clocksdir)/.directory
