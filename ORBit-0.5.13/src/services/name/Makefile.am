libORBitservicesincludedir = $(includedir)/orbit-1.0/ORBitservices

lib_LTLIBRARIES=libORBitCosNaming.la

bin_PROGRAMS = name-client orbit-name-server old-name-server

lib_LIBRARIES = libname-server.a

libORBitCosNaming_la_SOURCES = 						\
	CosNaming-common.c 						\
	CosNaming-stubs.c						\
	LName.c	

libORBitservicesinclude_HEADERS = 					\
	CosNaming.h							\
	CosNaming_impl.h

libORBitCosNaming_la_LDFLAGS = \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)

INCLUDES = -I. -I$(top_builddir)/src -I$(top_builddir)/src/ORBitutil -I$(top_srcdir)/src @GLIB_CFLAGS@

IDL = $(top_builddir)/src/orbit-idl-compiler/orbit-idl

CosNaming_built = 							\
	CosNaming.h 							\
	CosNaming-common.c 						\
	CosNaming-stubs.c 						\
	CosNaming-skels.c

$(CosNaming_built): my_cosnaming_built
	@:

my_cosnaming_built: CosNaming.idl $(srcdir)/LName.h $(IDL)
	$(IDL) $(srcdir)/CosNaming.idl
	cat $(srcdir)/LName.h >>CosNaming.h
	touch my_cosnaming_built

CLEANFILES=my_cosnaming_built

DEPS = 	$(top_builddir)/src/orb/libORBit.la			\
	$(top_builddir)/src/ORBitutil/libORBitutil.la 		\
	$(top_builddir)/src/IIOP/libIIOP.la			\
	libORBitCosNaming.la

LDADDS = \
	$(top_builddir)/src/orb/libORBit.la			\
	$(top_builddir)/src/IIOP/libIIOP.la			\
	$(top_builddir)/src/ORBitutil/libORBitutil.la 		\
	libORBitCosNaming.la					\
	@GLIB_LIBS@ 						\
	-lm

$(srcdir)/name-client.c $(srcdir)/orbit-name-server.c \
$(srcdir)/new-name-server.c $(srcdir)/boot.c: CosNaming.h

#name-client.c boot.c orbit-name-server.c new-name-server.c: CosNaming.h

name_client_SOURCES = name-client.c
name_client_LDFLAGS = $(FLAGS)
name_client_DEPENDENCIES = $(DEPS) CosNaming.h
name_client_LDADD = $(LDADDS)

old_name_server_SOURCES = orbit-name-server.c CosNaming-skels.c
old_name_server_LDFLAGS = $(FLAGS)
old_name_server_DEPENDENCIES = $(DEPS) CosNaming.h
old_name_server_LDADD = $(LDADDS)

libname_server_a_SOURCES = new-name-server.c CosNaming-skels.c
libname_server_a_DEPENDENCIES = $(DEPS) CosNaming.h

orbit_name_server_SOURCES = boot.c
orbit_name_server_LDFLAGS = $(FLAGS)
orbit_name_server_DEPENDENCIES = $(DEPS) CosNaming.h
orbit_name_server_LDADD = libname-server.a $(LDADDS)

EXTRA_DIST = $(idl_DATA) LName.h

idldir   = $(datadir)/idl/orbit-1.0
idl_DATA = CosNaming.idl LName.pidl

BUILT_SOURCES = $(CosNaming_built)

dist-hook:
	cd $(distdir); rm -f $(BUILT_SOURCES)
