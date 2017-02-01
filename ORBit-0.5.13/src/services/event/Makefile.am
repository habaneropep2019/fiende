INCLUDES = -I$(top_srcdir)/src -I. -I$(top_builddir)/src -I$(top_builddir)/src/ORBitutil \
	-I$(top_srcdir)/src/orb -I$(top_builddir)/src/orb @GLIB_CFLAGS@

IDL = $(top_builddir)/src/orbit-idl-compiler/orbit-idl

CosEvent_built = 							\
	CosEventComm.h							\
	CosEventChannel.h						\
	CosEventComm-skels.c	 					\
	CosEventComm-common.c 						\
	CosEventComm-stubs.c						\
	CosEventChannel-skels.c 					\
	CosEventChannel-common.c 					\
	CosEventChannel-stubs.c

$(CosEvent_built): my_cosevent_built

my_cosevent_built: CosEventComm.idl CosEventChannel.idl $(IDL)
	$(IDL) $(srcdir)/CosEventComm.idl
	$(IDL) $(srcdir)/CosEventChannel.idl
	touch my_cosevent_built

CLEANFILES=my_cosevent_built

bin_PROGRAMS=orbit-event-server
noinst_PROGRAMS=event-consumer #event-supplier

DEPS = 	$(top_builddir)/src/orb/libORBit.la				\
	$(top_builddir)/src/ORBitutil/libORBitutil.la			\
	$(top_builddir)/src/IIOP/libIIOP.la

LDADDS = \
	$(top_builddir)/src/orb/libORBit.la				\
	$(top_builddir)/src/IIOP/libIIOP.la				\
	$(top_builddir)/src/ORBitutil/libORBitutil.la			\
	@GLIB_LIBS@ 							\
	-lm

event_consumer_SOURCES = event-consumer.c				\
	CosEventComm-skels.c	 					\
	CosEventChannel-common.c 					\
	CosEventChannel-stubs.c

$(srcdir)/event-consumer.c $(srcdir)/event-supplier.c $(srcdir)/orbit-event-server.c: $(CosEvent_built)

event_consumer_LDFLAGS = $(FLAGS)
event_consumer_DEPENDENCIES = $(DEPS) CosEventChannel.h
event_consumer_LDADD = $(LDADDS)

#event_supplier_SOURCES = event-supplier.c				\
#	CosEventComm-skels.c	 					\
#	CosEventChannel-common.c 					\
#	CosEventChannel-stubs.c
#
#event_supplier_LDFLAGS = $(FLAGS)
#event_supplier_DEPENDENCIES = $(DEPS) CosEventChannel.h
#event_supplier_LDADD = $(LDADDS)

orbit_event_server_SOURCES = orbit-event-server.c			\
	CosEventChannel-skels.c 					\
	CosEventChannel-common.c 					\
	CosEventChannel-stubs.c

orbit_event_server_LDFLAGS = $(FLAGS)
orbit_event_server_DEPENDENCIES = $(DEPS) CosEventChannel.h
orbit_event_server_LDADD = $(LDADDS)

EXTRA_DIST = $(idl_DATA)

idldir   = $(datadir)/idl/orbit-1.0
idl_DATA =				\
	CosEventComm.idl		\
	CosEventChannel.idl		\
	CosTypedEventComm.idl		\
	CosTypedEventChannel.idl

BUILT_SOURCES = $(CosEvent_built)

dist-hook:
	cd $(distdir); rm -f $(BUILT_SOURCES)
