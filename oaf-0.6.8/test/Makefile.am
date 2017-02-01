NULL=

noinst_PROGRAMS=		\
	oaf-test-client		\
	oaf-test-async		\
	$(NULL)

bin_PROGRAMS=			\
	oaf-empty-server 	\
	oaf-run-query		\
	$(NULL)

oaf_test_async_SOURCES=		\
	oaf-test-async.c 	\
	$(NULL)

oaf_test_client_SOURCES=	\
	oaf-test-client.c 	\
	empty-common.c 		\
	empty.h 		\
	empty-stubs.c		\
	$(NULL)

oaf_run_query_SOURCES=		\
	oaf-run-query.c 	\
	empty-common.c 		\
	empty.h 		\
	empty-stubs.c		\
	$(NULL)

oaf_empty_server_SOURCES=	\
	empty-server.c 		\
	empty.h 		\
	empty-common.c 		\
	empty-skels.c		\
	$(NULL)

BUILT_SOURCES=			\
	empty.h 		\
	empty-common.c 		\
	empty-stubs.c 		\
	empty-skels.c		\
	$(NULL)

empty.h empty-common.c empty-stubs.c empty-skels.c: empty_built

empty_built: empty.idl $(ORBIT_IDL)
	$(ORBIT_IDL) $(srcdir)/empty.idl
	touch empty_built

CLEANFILES=empty_built

INCLUDES=-I$(top_srcdir) -I$(top_builddir) -I$(top_builddir)/liboaf -DOAFINFODIR=\"$(pkgdatadir)\" \
	@ORBIT_CFLAGS@ @XML_CFLAGS@ @GLIB_CFLAGS@
LDADD=../liboaf/liboaf.la @ORBIT_LIBS@ @GLIB_LIBS@ @INTLLIBS@

oaffiles=empty.oafinfo

emptydatadir=$(datadir)/oaf
emptydata_DATA=$(oaffiles)

bin_SCRIPTS=$(OAF_SLAY)
EXTRA_SCRIPTS=oaf-slay.in

EXTRA_DIST=empty.idl $(oaffiles) broken.oafinfo

check:
	OAF_INFO_PATH="$(top_srcdir)/test:$$OAF_INFO_PATH" PATH="$(top_builddir)/test:$$PATH" $(top_builddir)/test/oaf-test-client
