INCLUDES=-I$(top_srcdir)/src/orbit-idl-compiler -I$(top_srcdir) \
	-I$(top_srcdir)/src -I$(top_builddir) -I$(top_builddir)/src \
	@GLIB_CFLAGS@

lib_LIBRARIES=liborbit-c-backend.a

liborbit_c_backend_a_SOURCES=\
	orbit-idl-c-backend.h \
	orbit-idl-c-backend.c \
	orbit-idl-c-stubs.c \
	orbit-idl-c-skels.c \
	orbit-idl-c-headers.c \
	orbit-idl-c-common.c \
	orbit-idl-c-skelimpl.c \
	orbit-idl-c-utils.c \
	orbit-idl-c-typecode.c \
	orbit-idl-c-marshal.c \
	orbit-idl-c-demarshal.c
