if GNOME_SUPPORT
THE_FLAGS = $(GNOME_INCLUDEDIR)
THE_LIBS  = glade/libglade-gnome.la glade/libglade.la $(GNOME_LIBDIR) $(GNOMEUI_LIBS)
libglade_gnome_pc = libglade-gnome.pc
else
THE_FLAGS = $(GTK_CFLAGS)
THE_LIBS  = glade/libglade.la $(GTK_LIBS)
libglade_gnome_pc =
endif

if BONOBO_SUPPORT
libglade_bonobo_pc = libglade-bonobo.pc
else
libglade_bonobo_pc =
endif

if GNOMEDB_SUPPORT
libglade_gnomedb_pc = libglade-gnomedb.pc
else
libglade_gnomedb_pc =
endif

INCLUDES = $(THE_FLAGS) $(XML_CFLAGS)

EXTRA_DIST =			\
	libgladeConf.sh		\
	libglade.spec.in	\
	libglade.spec		\
	test.glade		\
	simple.glade		\
	example.glade		\
	custom.glade		\
	gnome-widgets.glade	\
	dialogs.glade		\
	libglade.m4		\
	libglade-xgettext	\
	libglade.pc.in		\
	libglade-gnome.pc.in	\
	libglade-bonobo.pc.in	\
	config.h.win32		\
	makefile.mingw

bin_SCRIPTS = libglade-config libglade-xgettext
noinst_PROGRAMS = test-libglade
test_libglade_LDADD = $(THE_LIBS) $(XML_LIBS)
SUBDIRS = intl macros glade doc

m4datadir = $(datadir)/aclocal
m4data_DATA = libglade.m4

confexecdir = $(libdir)
confexec_DATA = libgladeConf.sh

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libglade.pc $(libglade_gnome_pc) $(libglade_bonobo_pc) \
  $(libglade_gnomedb_pc)

test_libglade_SOURCES = test-libglade.c
