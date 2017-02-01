INCLUDES =			\
	-I$(top_srcdir)/	\
	-I$(srcdir)/		\
	$(GLIB_CFLAGS)

libole2_sources =		\
	ms-ole.c		\
	ms-ole.h		\
	ms-ole-summary.c	\
	ms-ole-summary.h	\
	ms-ole-vba.c		\
	ms-ole-vba.h

lib_LTLIBRARIES = libgnomeole2.la

libgnomeole2_la_LIBADD =        \
	$(GLIB_LIBS)

libgnomeole2_la_SOURCES =	\
	$(libole2_sources)	\
	version.c		\
	libole2.h

libgnomeole2includedir      = $(includedir)/libole2
libgnomeole2include_HEADERS =	\
	libole2.h		\
	ms-ole.h		\
	ms-ole-summary.h	\
	ms-ole-vba.h

