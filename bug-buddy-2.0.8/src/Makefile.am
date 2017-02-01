buddydir = $(datadir)/bug-buddy

INCLUDES = \
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	$(EXTRA_GNOME_CFLAGS) -I$(includedir)		\
	-DBUDDY_DATADIR=\""$(buddydir)"\"		\
        -DBUDDY_ICONDIR=\""$(datadir)/pixmaps"\"	\
	-DCLIST_HEIGHT=20				\
	-D_GNU_SOURCE


bin_PROGRAMS = bug-buddy

bug_buddy_SOURCES = \
	bug-buddy.c		\
	bug-buddy.h		\
	bugzilla.c		\
	bugzilla.h		\
	config.c		\
	distro.h		\
	distro-debian.c		\
	distro-irix.c		\
	distro-redhat.c		\
	gdb-buddy.c		\
	libglade-buddy.h	\
	util.c			\
	util.h			\
	united-states-of-bug-buddy.c

bug_buddy_LDFLAGS = \
	-export-dynamic

bug_buddy_LDADD = \
	$(EXTRA_GNOME_LIBS)	\
	$(INTLLIBS)

noinst_PROGRAMS = gnome-crash

gnome_crash_SOURCES = gnome-crash.c

gnome_crash_LDADD = \
	$(GNOME_LIBDIR) \
	$(GNOMEUI_LIBS)

# taken from gnumeric/Makefile.am
bug-buddy.keys: bug-buddy.keys.in Makefile
	sed -e 's^\@icondir\@^$(datadir)/pixmaps^g' < $(srcdir)/bug-buddy.keys.in > bug-buddy.keys.tmp \
	&& mv bug-buddy.keys.tmp bug-buddy.keys 

mimedir = $(datadir)/mime-info
mime_DATA = \
	bug-buddy.keys	\
	bug-buddy.mime

debian-packages: Makefile.am
	grep -h ^Package $(DEBIAN_ROOT)/var/state/apt/lists/*_Packages | \
	sort | awk '{ print $$2 }' > debian-packages

gnome-packages: Makefile.am
	sort $(top_srcdir)/debbugs/Maintainers | \
	awk '{ print $$1 }' > gnome-packages

buddy_DATA = \
	bug-buddy.glade	\
	gdb-cmd

#	gnome.appmap	\
#	gnome-packages	\
#	debian-packages	\
#	kde-packages	\
#	helix-packages

utilsdir = $(datadir)/gnome/apps/Utilities
utils_in_files = bug-buddy.desktop.in
utils_DATA = $(utils_in_files:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

#xmldir = $(datadir)/bug-buddy/xml
#xml_DATA = \
#	GNOME.bts	\
#	Debian.bts	\
#	KDE.bts		\
#	Helix_Code.bts	\
#	Independent_Project.bts

EXTRA_DIST = \
	$(buddy_DATA)	\
	$(utils_DATA)	\
	bug-buddy.mime	\
	bug-buddy.keys.in

#	$(xml_DATA)	\
