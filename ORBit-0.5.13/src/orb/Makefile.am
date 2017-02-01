## Process this file with automake to produce Makefile.in

INCLUDES =-I.. -I$(top_srcdir)/src @GLIB_CFLAGS@ \
	-DORBit_SYSRC=\"$(sysconfdir)/orbitrc\" \
	-I$(top_builddir)/src/ORBitutil \
	-DORBIT_MAJOR_VERSION="@ORBIT_MAJOR_VERSION@" \
	-DORBIT_MINOR_VERSION="@ORBIT_MINOR_VERSION@" \
	-DORBIT_MICRO_VERSION="@ORBIT_MICRO_VERSION@" \
	-DORBIT_VERSION=\"@ORBIT_VERSION@\" \
	-DG_LOG_DOMAIN=\"ORBit\"

lib_LTLIBRARIES = libORBit.la

libORBitincludedir = $(includedir)/orbit-1.0/orb

EXTRA_DIST=orbit.h.in

libORBit_la_SOURCES = \
	interface_repository-common.c \
	interface_repository-stubs.c \
	interface_repository.h \
	allocators.c \
	cdr.c \
	env.c \
	options.c \
	orb.c \
	orbit.c \
	poa.c \
	orbit_poa.c \
	typecode.c \
	corba_object.c \
	orbit_object.c \
	sequences.c \
	server.c \
	orbit_typecode.c \
	dii.c \
	dynany.c \
	ir.c \
	corba_any.c \
	corba_context.c \
	genrand.c genrand.h

libORBitinclude_HEADERS = \
	allocator-defs.h \
	allocators.h \
	cdr.h \
	dii.h \
	dynany.h \
	env.h \
	iop.h \
	ir.h \
	options.h \
	orb.h \
	orbit.h \
	orbit_config.h \
	orbit_types.h \
	poa.h \
	sequences.h \
	typecode.h \
	corba_any.h \
	corba_any_type.h \
	corba_any_proto.h \
	corba_basic_sequences_type.h \
	corba_context.h \
	corba_env.h \
	corba_env_type.h \
	corba_object.h \
	corba_object_type.h \
	corba_orb.h \
	corba_orb_type.h \
	corba_portableserver.h \
	corba_portableserver_type.h \
	corba_sequences.h \
	corba_sequences_type.h \
	corba_typecode.h \
	corba_typecode_type.h \
	orbit_config.h \
	orbit_object.h \
	orbit_object_type.h \
	orbit_poa.h \
	orbit_poa_type.h \
	orbit_typecode.h \
	orbit_types.h \
	interface_repository.h

IDL = $(top_builddir)/src/orbit-idl-compiler/orbit-idl
IRIDL = $(top_srcdir)/src/daemons/interface_repository/interface_repository.idl

interface_repository.h \
interface_repository-common.c \
interface_repository-stubs.c: my_ir_stuff

my_ir_stuff: $(IRIDL) $(IDL)
	$(IDL) --noskels $(IRIDL)
	touch my_ir_stuff

$(IDL):
	$(MAKE) -C $(top_builddir)/src/orbit-idl-compiler

libORBit_la_LDFLAGS = -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)
libORBit_la_LIBADD = $(GLIB_LIBS) -lm

BUILT_SOURCES = \
	orbit.h	\
	interface_repository.h \
	interface_repository-common.c \
	interface_repository-stubs.c

# Help make along by giving another name for the file
../orb/orbit.h: orbit.h
	@:

../orb/interface_repository.h: interface_repository.h
	@:

CLEANFILES=my_ir_stuff

dist-hook:
	cd $(distdir); rm -f $(BUILT_SOURCES)
