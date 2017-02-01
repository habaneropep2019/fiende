INCLUDES=-I$(top_builddir)/src -I$(top_srcdir)/src @GLIB_CFLAGS@

lib_LTLIBRARIES=libORBitutil.la

libORBitutilincludedir = $(includedir)/orbit-1.0/ORBitutil

libORBitutil_la_SOURCES = \
	trace.c \
	trace.h \
	basic_types.h \
	thread-safety.h \
	thread-safety.c \
	compat.c

libORBitutilinclude_HEADERS = \
	basic_types.h \
	thread-safety.h \
	trace.h \
	util.h \
	compat.h \
	os-specifics.h \
	os-feature-alloca.h \
	orbit-os-config.h

BUILT_SOURCES=orbit-os-config.h
EXTRA_DIST=orbit-os-config.h.in

libORBitutil_la_LDFLAGS = -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)
