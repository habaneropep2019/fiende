bin_PROGRAMS=orbit-ird

INCLUDES=-I$(top_builddir) -I$(top_builddir)/src -I$(top_builddir)/src/ORBitutil -I$(top_builddir)/src/orb \
	 -I$(top_srcdir) -I$(top_srcdir)/src @GLIB_CFLAGS@

orbit_ird_DEPENDENCIES = $(top_builddir)/src/orb/libORBit.la \
			 $(top_builddir)/src/IIOP/libIIOP.la \
			 $(top_builddir)/src/ORBitutil/libORBitutil.la

orbit_ird_LDFLAGS = -static
orbit_ird_LDADD = \
	$(top_builddir)/src/orb/libORBit.la \
	$(top_builddir)/src/IIOP/libIIOP.la \
	$(top_builddir)/src/ORBitutil/libORBitutil.la \
	@GLIB_LIBS@ \
	-lm

orbit_ird_SOURCES= \
	interface_repository-skels.c \
	interface_repository-impl.c

#IDL = $(top_builddir)/src/idl-compiler/orbit-idl
IDL = $(top_builddir)/src/orbit-idl-compiler/orbit-idl
IRIDL = $(top_srcdir)/src/daemons/interface_repository/interface_repository.idl

orbit_ird_built = interface_repository-skels.c

$(orbit_ird_built): $(IRIDL) $(IDL)
	$(IDL) --noheaders --nostubs --nocommon $(IRIDL)

EXTRA_DIST = interface_repository.idl
BUILT_SOURCES = $(orbit_ird_built)

dist-hook:
	cd $(distdir); rm -f $(BUILT_SOURCES)
