
# Before making a release, the LTVERSION string should be modified.
# The string is of the form C:R:A.
# - If interfaces have been changed or added, but binary compatibility has
#   been preserved, change to C+1:0:A+1
# - If binary compatibility has been broken (eg removed or changed interfaces)
#   change to C+1:0:0
# - If the interface is the same as the previous version, change to C:R+1:A

LTVERSION = 4:2:4

INCLUDES = -I$(srcdir)/.. $(XML_CFLAGS) $(GTK_CFLAGS) $(GNOME_INCLUDEDIR) \
  $(BONOBO_CFLAGS) $(GNOMEDB_CFLAGS)

the_sources =		\
	glade-init.c 	\
	glade-sax.c	\
	glade-xml.c 	\
	glade-gtk.c 	\
	glade-tree.c 	\
	glade-private.h

the_headers =		\
	glade.h 	\
	glade-xml.h 	\
	glade-build.h	\
	glade-widget-tree.h

if GNOME_SUPPORT
build_glade_gnome = libglade-gnome.la
endif
if BONOBO_SUPPORT
build_glade_bonobo = libglade-bonobo.la
endif
if GNOMEDB_SUPPORT
build_glade_gnomedb = libglade-gnomedb.la
endif

lib_LTLIBRARIES = \
	libglade.la \
	$(build_glade_gnome) \
	$(build_glade_bonobo) \
	$(build_glade_gnomedb)

libglade_la_SOURCES  = $(the_sources)
libglade_la_LIBADD   = $(GTK_LIBS) $(XML_LIBS)
libglade_la_LDFLAGS  = -version-info $(LTVERSION)

libglade_gnome_la_SOURCES  = glade-gnome.c glade-build.h glade-xml.h
libglade_gnome_la_LIBADD   = $(GNOME_LIBDIR) $(GNOMEUI_LIBS) $(XML_LIBS)
libglade_gnome_la_LDFLAGS  = -version-info $(LTVERSION)

libglade_bonobo_la_SOURCES  = glade-bonobo.c glade-build.h glade-xml.h
libglade_bonobo_la_LIBADD   = $(BONOBO_LIBS) $(XML_LIBS)
libglade_bonobo_la_LDFLAGS  = -version-info $(LTVERSION)

libglade_gnomedb_la_SOURCES = glade-gnomedb.c glade-build.h glade-xml.h
libglade_gnomedb_la_LIBADD = $(GNOMEDB_LIBS) $(XML_LIBS)
libglade_gnomedb_la_LDFLAGS = -version-info $(LTVERSION)

gladeincludedir      = $(includedir)/libglade-1.0/glade
gladeinclude_HEADERS = $(the_headers)

EXTRA_DIST = $(the_headers) \
	makefile.mingw	\
	glade.def
