noinst_PROGRAMS=client server

EVERYTHING_BUILT=everything.h everything-stubs.c everything-skels.c everything-common.c
BUILT_SOURCES=${EVERYTHING_BUILT}

INCLUDES=-I$(top_builddir)/src/orb \
              -I$(top_builddir)/src \
              -I$(top_builddir)/src/ORBitutil \
              -I$(top_srcdir)/src/orb \
              -I$(top_srcdir)/src \
              ${GLIB_CFLAGS}

LDADDS=$(top_builddir)/src/orb/libORBit.la \
             $(top_builddir)/src/IIOP/libIIOP.la \
             $(top_builddir)/src/ORBitutil/libORBitutil.la \
             ${GLIB_LIBS}


client_SOURCES=client.c ${EVERYTHING_BUILT} everything.idl constants.h
client_DEPENDENCES=${EVERYTHING_BUILT}
client_LDFLAGS=-static
client_LDADD=${LDADDS}

server_SOURCES=server.c ${EVERYTHING_BUILT} everything.idl constants.h
server_DEPENDENCES=${EVERYTHING_BUILT}
server_LDFLAGS=-static
server_LDADD=${LDADDS}


ORBIT_IDL=$(top_builddir)/src/orbit-idl-compiler/orbit-idl

${EVERYTHING_BUILT}: everything.idl $(ORBIT_IDL)
	$(ORBIT_IDL) $(srcdir)/everything.idl

CLEANFILES=${EVERYTHING_BUILT} iorfile

TESTS=$(srcdir)/test.sh
EXTRA_DIST=test.sh basicServer.c structServer.c sequenceServer.c unionServer.c arrayServer.c anyServer.c
