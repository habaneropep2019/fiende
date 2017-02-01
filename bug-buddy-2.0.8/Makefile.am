SUBDIRS = po intl pixmaps src docs

agnomedir = $(datadir)/bug-buddy/bugzilla/gnome
agnome_DATA =

aximiandir = $(datadir)/bug-buddy/bugzilla/ximian
aximian_DATA =

bugzilladir = $(datadir)/bug-buddy
bugzilla_DATA = \
	bugzilla/gnome.bugzilla	\
	bugzilla/gnome/config	\
	bugzilla/gnome/products	\
	\
	bugzilla/ximian.bugzilla	\
	bugzilla/ximian/config		\
	bugzilla/ximian/products

EXTRA_DIST = \
	bug-buddy.spec.in	\
	bug-buddy.spec		\
	xml-i18n-extract.in	\
	xml-i18n-merge.in	\
	xml-i18n-update.in	\
	COPYING.ximian-logos	\
	$(bugzilla_DATA)
