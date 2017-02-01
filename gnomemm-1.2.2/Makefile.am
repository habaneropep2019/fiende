# hmm, I removed macros dir from distribution - people cannot build
# gtk-- from scratch anyway, because of no autogen.sh in distribution.
# the problem of course is that we only have now only few complete
# "backups" which can be used to build distributions :(
SUBDIRS =  src docs docgen
# gtk--draw not distributed anymore.
DIST_SUBDIRS = src examples docs docgen scripts

#bin_SCRIPTS = 

EXTRA_DIST = Makefile.am Makefile.in gnome--.m4
#	Gtk--FAQ
#patch-libtool.sh

dist-hook: gnome--.spec
	cp gnome--.spec $(distdir)

# Install the m4 macro to be used by autoconf in apps
# that use Gtk-- and want to check for the correct version
m4datadir = $(datadir)/aclocal
m4data_DATA = gnome--.m4

# Install compiler specific headers
#includedir = $(pkglibdir)/include
#include_HEADERS = gdk--config.h gtk--config.h


all-local:
	@echo "*** Everything completed ***"


