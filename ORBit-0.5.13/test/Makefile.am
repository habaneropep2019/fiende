noinst_PROGRAMS=test test-dynamic echo-client echo-server test-object \
echo-local test-any-client test-any-server test-tc-client \
test-tc-server test-ir-client test-ir-server \
empty-server empty-client test-dynany
bin_PROGRAMS=ior-decode
SUBDIRS=test-poa idl everything
BUILT_SOURCES=echo.h echo-common.c echo-stubs.c echo-skels.c \
test-any.h test-any-common.c test-any-stubs.c test-any-skels.c \
test-tc.h test-tc-common.c test-tc-stubs.c test-tc-skels.c \
test-ir.h test-ir-common.c test-ir-stubs.c test-ir-skels.c \
empty.h empty-common.c empty-stubs.c empty-skels.c

INCLUDES=-I$(top_builddir)/src/orb -I$(top_builddir)/src -I$(top_builddir)/src/ORBitutil \
	 -I$(top_srcdir)/src/orb -I$(top_srcdir)/src @GLIB_CFLAGS@
#FLAGS =
DEPS = $(top_builddir)/src/orb/libORBit.la \
	$(top_builddir)/src/ORBitutil/libORBitutil.la \
	$(top_builddir)/src/IIOP/libIIOP.la
LDADDS = \
	$(top_builddir)/src/orb/libORBit.la \
	$(top_builddir)/src/IIOP/libIIOP.la \
	$(top_builddir)/src/ORBitutil/libORBitutil.la \
	@GLIB_LIBS@ \
	-lm

test_SOURCES=test.c
test_LDFLAGS = $(FLAGS)
test_DEPENDENCIES = $(DEPS)
test_LDADD= $(LDADDS)

echo_client_SOURCES=echo-client.c echo.h echo-common.c echo-stubs.c
echo_client_LDFLAGS = $(FLAGS)
echo_client_DEPENDENCIES = $(DEPS) echo.h
echo_client_LDADD= $(LDADDS)

echo_server_SOURCES=echo-server.c echo.h echo-common.c echo-skels.c
echo_server_LDFLAGS = $(FLAGS)
echo_server_DEPENDENCIES = $(DEPS) echo.h
echo_server_LDADD= $(LDADDS)

echo_local_SOURCES=echo-local.c echo.h echo-common.c echo-stubs.c echo-skels.c
echo_local_LDFLAGS = $(FLAGS)
echo_local_DEPENDENCIES = $(DEPS) echo.h
echo_local_LDADD= $(LDADDS)

$(srcdir)/echo-local.c $(srcdir)/echo-server.c $(srcdir)/echo-client.c: echo.h
	@:

empty_client_SOURCES=empty-client.c empty.h empty-common.c empty-stubs.c
empty_client_LDFLAGS = $(FLAGS)
empty_client_DEPENDENCIES = $(DEPS) empty.h
empty_client_LDADD= $(LDADDS)

empty_server_SOURCES=empty-server.c empty.h empty-common.c empty-skels.c
empty_server_LDFLAGS = $(FLAGS)
empty_server_DEPENDENCIES = $(DEPS) empty.h
empty_server_LDADD= $(LDADDS)

$(srcdir)/empty-client.c $(srcdir)/empty-server.c: empty.h
	@:

test_dynamic_SOURCES=test.c
test_dynamic_DEPENDENCIES = $(DEPS)
test_dynamic_LDADD= $(LDADDS)

test_object_SOURCES=test-object.c
test_object_LDFLAGS = $(FLAGS)
test_object_DEPENDENCIES = $(DEPS)
test_object_LDADD= $(LDADDS)

test_any_client_SOURCES=test-any-client.c test-any-common.c test-any-stubs.c
test_any_client_LDFLAGS = $(FLAGS)
test_any_client_DEPENDENCIES = $(DEPS) test-any.h
test_any_client_LDADD= $(LDADDS)

test_any_server_SOURCES=test-any-server.c test-any-common.c test-any-skels.c
test_any_server_LDFLAGS = $(FLAGS)
test_any_server_DEPENDENCIES = $(DEPS) test-any.h
test_any_server_LDADD= $(LDADDS)

test_dynany_SOURCES=test-dynany.c
test_dynany_LDFLAGS = $(FLAGS)
test_dynany_DEPENDENCIES = $(DEPS)
test_dynany_LDADD= $(LDADDS)

$(srcdir)/test-any-client.c $(srcdir)/test-any-server.c: test-any.h

test_tc_client_SOURCES=test-tc-client.c test-tc-common.c test-tc-stubs.c
test_tc_client_LDFLAGS = $(FLAGS)
test_tc_client_DEPENDENCIES = $(DEPS) test-tc.h
test_tc_client_LDADD= $(LDADDS)

test_tc_server_SOURCES=test-tc-server.c test-tc-common.c test-tc-skels.c
test_tc_server_LDFLAGS = $(FLAGS)
test_tc_server_DEPENDENCIES = $(DEPS) test-tc.h
test_tc_server_LDADD= $(LDADDS)

$(srcdir)/test-tc-server.c $(srcdir)/test-tc-client.c: test-tc.h

test_ir_client_SOURCES=test-ir-client.c test-ir-common.c test-ir-stubs.c
test_ir_client_LDFLAGS = $(FLAGS)
test_ir_client_DEPENDENCIES = $(DEPS) test-ir.h
test_ir_client_LDADD= $(LDADDS)

test_ir_server_SOURCES=test-ir-server.c test-ir-common.c test-ir-skels.c
test_ir_server_LDFLAGS = $(FLAGS)
test_ir_server_DEPENDENCIES = $(DEPS) test-ir.h
test_ir_server_LDADD= $(LDADDS)

$(srcdir)/test-ir-client.c $(srcdir)/test-ir-server.c: test-ir.h

ior_decode_SOURCES=ior-decode.c
ior_decode_LDFLAGS = $(FLAGS)
ior_decode_DEPENDENCIES = $(DEPS)
ior_decode_LDADD= $(LDADDS)

#test_ird_SOURCES=test-ird.c
#test_ird_LDFLAGS = $(FLAGS)
#test_ird_DEPENDENCIES = $(DEPS)
#test_ird_LDADD= $(LDADDS)

ORBIT_IDL=$(top_builddir)/src/orbit-idl-compiler/orbit-idl

EXTRA_DIST = echo.idl test-args.idl inheritance-test.idl \
	test-any.idl test-ir.idl test-tc.idl empty.idl

echo.h echo-common.c echo-stubs.c echo-skels.c: echo_built

echo_built: echo.idl $(ORBIT_IDL)
	$(ORBIT_IDL) $(srcdir)/echo.idl
	touch echo_built

empty.h empty-common.c empty-stubs.c empty-skels.c: empty_built

empty_built: empty.idl $(ORBIT_IDL)
	$(ORBIT_IDL) $(srcdir)/empty.idl
	touch empty_built

test-any.h test-any-common.c test-any-stubs.c test-any-skels.c: test_any_built

test_any_built: test-any.idl $(ORBIT_IDL)
	$(ORBIT_IDL) $(srcdir)/test-any.idl
	touch test_any_built

test-tc.h test-tc-common.c test-tc-stubs.c test-tc-skels.c: test_tc_built

test_tc_built: test-tc.idl $(ORBIT_IDL)
	$(ORBIT_IDL) $(srcdir)/test-tc.idl
	touch test_tc_built

test-ir.h test-ir-common.c test-ir-stubs.c test-ir-skels.c: test_ir_built

test_ir_built: test-ir.idl $(ORBIT_IDL)
	$(ORBIT_IDL) $(srcdir)/test-ir.idl
	touch test_ir_built

dist-hook:
	cd $(distdir); rm -f $(BUILT_SOURCES)

CLEANFILES=echo_built empty_built test_any_built test_tc_built test_ir_built

TESTS = test-dynany
