noinst_PROGRAMS=test-poa

INCLUDES=-I$(top_builddir)/src/orb -I$(top_builddir)/src -I$(top_builddir)/src/ORBitutil \
	-I$(top_srcdir)/src/orb -I$(top_srcdir)/src @GLIB_CFLAGS@
FLAGS = -static
DEPS = $(top_builddir)/src/orb/libORBit.la \
	$(top_builddir)/src/ORBitutil/libORBitutil.la \
	$(top_builddir)/src/IIOP/libIIOP.la
LDADDS = \
	$(top_builddir)/src/orb/libORBit.la \
	$(top_builddir)/src/IIOP/libIIOP.la \
	$(top_builddir)/src/ORBitutil/libORBitutil.la \
	@GLIB_LIBS@ \
	-lm

test_poa_SOURCES=test.c
test_poa_LDFLAGS = $(FLAGS)
test_poa_DEPENDENCIES = $(DEPS)
test_poa_LDADD= $(LDADDS)

