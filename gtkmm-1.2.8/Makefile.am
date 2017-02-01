# hmm, I removed macros dir from distribution - people cannot build
# gtk-- from scratch anyway, because of no autogen.sh in distribution.
# the problem of course is that we only have now only few complete
# "backups" which can be used to build distributions :(
SUBDIRS = gdk-- src docs docgen
#gnome--

DIST_SUBDIRS = gdk-- src examples docs docgen scripts
#gnome--


bin_SCRIPTS = gtkmm-config gtkmmconvert

EXTRA_DIST = \
gtk--.m4 \
gtk--.spec.in \
Makefile.am \
Makefile.in \
gtkmmconvert \
win32/README \
win32/gdkmm_static.dsp \
win32/gtkmm_static.dsp \
win32/gtkmm.dsw \
win32/makevc.sh

#	Gtk--FAQ
#patch-libtool.sh

dist-hook: gtk--.spec
	cp gtk--.spec $(distdir)

# Install the m4 macro to be used by autoconf in apps
# that use Gtk-- and want to check for the correct version
m4datadir = $(datadir)/aclocal
m4data_DATA = gtk--.m4

# Install compiler specific headers
includedir = $(pkglibdir)/include
include_HEADERS = gdk--config.h gtk--config.h

vc-dist:
	sh ./win32/makevc.sh

all-local:
	@echo "*** Everything completed ***"

