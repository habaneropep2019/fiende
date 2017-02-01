bin_PROGRAMS = loadshlib goad-browser gnome-name-service

lib_LTLIBRARIES = libgnorba.la libgnorbagtk.la

LINK = $(LIBTOOL) --mode=link $(CC) $(CFLAGS) $(LDFLAGS) -o $@

libgnorbaincludedir = $(includedir)/gnome-1.0/libgnorba

man_MANS = gnome-name-service.1	\
	goad-browser.1		\
	loadshlib.1		\
	new-object.1
	
EXTRA_DIST = yes.xpm no.xpm $(man_MANS)

INCLUDES = 						\
	-I.. 						\
	-I$(srcdir)/..					\
	$(SUPPORTINCS) 					\
	-I$(includedir) 				\
        $(GNOME_INCLUDES)  				\
	$(ORBIT_CFLAGS) 				\
	$(GLIB_INCLUDES) 				\
	$(AUDIOFILE_CFLAGS) $(ESD_CFLAGS) 		\
	$(GDK_IMLIB_CFLAGS)				\
	-DGNOMEBINDIR=$(bindir) 			\
	-DGNOMESYSCONFDIR=\"$(sysconfdir)\" 		\
	-DVERSION=\""$(VERSION)"\"

gnome_factory_built=\
	gnome-factory.h 				\
	gnome-factory-skels.c 				\
	gnome-factory-stubs.c 				\
	gnome-factory-common.c

gnome_unknown_built = \
	gnome-unknown.h					\
	gnome-unknown-skels.c				\
	gnome-unknown-stubs.c				\
	gnome-unknown-common.c

table_built=\
	Table.h						\
	Table-skels.c					\
	Table-stubs.c					\
	Table-common.c

gnorba_built = $(gnome_factory_built) \
	$(table_built) \
	$(gnome_unknown_built)

ORBIT_IDL=orbit-idl

IDL_FLAGS=-D__LIBGNORBA_COMPILATION

$(gnome_factory_built): $(top_srcdir)/idl/gnome-factory.idl
	$(ORBIT_IDL) $(IDL_FLAGS) $(top_srcdir)/idl/gnome-factory.idl

$(gnome_unknown_built): $(top_srcdir)/idl/gnome-unknown.idl
	$(ORBIT_IDL) $(IDL_FLAGS) $(top_srcdir)/idl/gnome-unknown.idl

$(table_built): $(top_srcdir)/idl/Table.idl
	$(ORBIT_IDL) $(IDL_FLAGS) $(top_srcdir)/idl/Table.idl

libgnorba_la_SOURCES = 					\
	$(gnorba_built)					\
	goad.c 						\
	orbitgtk.c					\
	orbitgnome.c					\
	orbitns.c

libgnorbagtk_la_SOURCES = 				\
	$(gnorba_built) 				\
	orbitgtk.c

libgnorbainclude_HEADERS = 				\
	gnorba.h 					\
	gnome-factory.h					\
	gnome-unknown.h					\
	Table.h

libgnorba_la_LDFLAGS = -version-info 28:8:1 -rpath $(libdir)
libgnorba_la_LIBADD = $(ORBIT_LIBS) $(GLIB_LIBS) -lm

loadshlib_SOURCES = loadshlib.c

loadshlib_LDADD = 					\
	libgnorba.la					\
	$(ORBIT_LIBS) 					\
        $(top_builddir)/libgnome/libgnome.la		\
        $(top_builddir)/libgnomeui/libgnomeui.la	\
	$(top_builddir)/libart_lgpl/libart_lgpl.la	\
        @LIBSUPPORT@					\
        $(LIBGNOMEUI_LIBS) $(LIBGNOME_LIBS)		\
	$(INTLLIBS)

goad_browser_SOURCES = 	\
	goad-browser.c

goad_browser_LDADD = 					\
	libgnorba.la					\
	$(ORBIT_LIBS) 					\
        $(top_builddir)/libgnomeui/libgnomeui.la	\
        $(top_builddir)/libgnome/libgnome.la		\
	$(top_builddir)/libart_lgpl/libart_lgpl.la	\
        @LIBSUPPORT@					\
        $(LIBGNOMEUI_LIBS) $(LIBGNOME_LIBS) 		\
	$(INTLLIBS)

gnome_name_service_SOURCES =				\
	gnome-name-server.c

gnome_name_service_LDADD = 				\
	libgnorbagtk.la					\
	-lname-server					\
	$(ORBIT_LIBS) 					\
	$(X_LIBS) -lX11					\
	$(INTLLIBS)

gbpixmapdir = $(datadir)/pixmaps
gbpixmap_DATA = yes.xpm no.xpm

install-data-hook:
	$(RM) -f $(DESTDIR)$(bindir)/new-object
	$(LN_S) goad-browser $(DESTDIR)$(bindir)/new-object

BUILT_SOURCES=$(gnorba_built)

clean-local:
	$(RM) -f $(BUILT_SOURCES)
