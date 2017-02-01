AUTOMAKE_OPTIONS = 1.5

SUBDIRS = macros pygtk pygnome

EXTRA_DIST = gnome-python.spec

snap:
	$(MAKE) dist distdir=$(PACKAGE)-SNAP-`date +"%Y%m%d"`
