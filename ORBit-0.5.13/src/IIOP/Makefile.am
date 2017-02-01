LIBWRAP_PATH=@LIBWRAP_PATH@
LIB_WRAP=@LIB_WRAP@
if HAVE_HOSTS_ACCESS
libwrap_files=$(addsuffix .lo, $(basename $(filter %.o, $(shell ar t $(LIBWRAP_PATH)))))
libwrap_deps=
else
libwrap_files=
libwrap_deps=$(LIB_WRAP)
endif

INCLUDES=-I.. -I$(top_srcdir)/src @GLIB_CFLAGS@ -I$(top_builddir)/src/ORBitutil

lib_LTLIBRARIES=libIIOP.la

libIIOPincludedir = $(includedir)/orbit-1.0/IIOP

libIIOP_la_SOURCES= \
	connection.c \
	encoders.c \
	giop-msg-buffer.c \
	iiop-endian.c \
	IIOP-config.h \
	IIOP-types.h \
	giop-msg-buffer.h \
	iiop-encoders.h \
	iiop-endian.h \
	iiop-endianP.h \
	IIOP.h \
	IIOP-private.h

libIIOPinclude_HEADERS = \
	IIOP-config.h \
	IIOP-types.h \
	IIOP.h \
	giop-msg-buffer.h \
	iiop-encoders.h \
	iiop-endian.h

libIIOP_la_LIBADD=$(libwrap_files) $(libwrap_deps)

$(libwrap_files): make_libwrap_files

make_libwrap_files: $(LIBWRAP_PATH)
	ar x $(LIBWRAP_PATH)
	for I in $(basename $(libwrap_files)); do mv $$I.o $$I.lo; done
	ar x $(LIBWRAP_PATH)
	touch make_libwrap_files

BUILT_SOURCES=$(libwrap_files)

libIIOP_la_LDFLAGS = -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)

EXTRA_DIST = IIOP-design.txt

CLEANFILES=make_libwrap_files
