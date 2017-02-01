SUBDIRS=backends
bin_PROGRAMS=orbit-idl

orbitidlincludedir=$(includedir)/orbit-1.0
orbitlibdir=$(libdir)/orbit-idl

INCLUDES=-I$(top_builddir) -I$(top_builddir)/src \
	 -I$(top_srcdir) -I$(top_srcdir)/src -I$(top_srcdir)/popt \
	@GLIB_CFLAGS@ @GMODULE_CFLAGS@ \
	-DORBITLIBDIR="\"$(orbitlibdir)\"" \
	-DDEBUG

orbit_idl_DEPENDENCIES = $(top_builddir)/libIDL/libIDL.la \
			 backends/c/liborbit-c-backend.a \
			 $(top_builddir)/popt/libpopt.la
orbit_idl_LDFLAGS =
orbit_idl_LDADD = \
	backends/c/liborbit-c-backend.a \
	$(top_builddir)/libIDL/libIDL.la \
	$(top_builddir)/popt/libpopt.la \
	@GLIB_LIBS@ @GMODULE_LIBS@ \
	-lm

orbit_idl_SOURCES=orbit-idl-main.c \
		   orbit-idl-driver.c \
		   orbit-idl-backends.c \
		   orbit-idl-passes.c \
		   orbit-idl-genmarshal.c \
		   orbit-idl-utils.c \
		   orbit-idl2.h
orbitidlinclude_HEADERS=orbit-idl2.h

$(top_builddir)/libIDL/libIDL.la:
	$(MAKE) -C $(top_builddir)/libIDL libIDL.la

$(top_builddir)/popt/libpopt.la:
	$(MAKE) -C $(top_builddir)/popt libpopt.la
