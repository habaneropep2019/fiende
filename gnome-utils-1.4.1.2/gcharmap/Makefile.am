SUBDIRS = src pixmaps doc

desktopdir = $(datadir)/gnome/apps/Utilities
desktop_in_files = gcharmap.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

EXTRA_DIST = \
	BUGS \
	$(desktop_in_files) \
	$(desktop_DATA)
