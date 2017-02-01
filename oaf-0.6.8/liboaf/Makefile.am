NULL=

lib_LTLIBRARIES=liboaf.la

INCLUDES=					\
	-I$(top_srcdir)				\
	-I$(top_builddir)			\
	-I$(top_srcdir)/liboaf			\
	-I$(top_builddir)/liboaf		\
	-DOAF_LOCALEDIR=\""$(datadir)/locale"\"	\
	@ORBIT_CFLAGS@				\
	@GLIB_CFLAGS@				\
	-DVERSION=\"$(VERSION)\"		\
	$(NULL)

LDFLAGS=					\
	@ORBIT_LIBS@				\
	@GLIB_LIBS@				\
	$(NULL)

liboafincludedir = $(includedir)/liboaf

liboafinclude_HEADERS=				\
	oaf.h					\
	liboaf.h				\
	oaf-async.h				\
	oaf-registration.h			\
	oaf-actid.h				\
	oaf-activate.h				\
	oaf-mainloop.h				\
	oaf-plugin.h				\
	oaf-servreg.h				\
	oaf-util.h				\
	oaf-async.h				\
	oaf-factory.h				\
	oaf-factory-suppress.h			\
	liboaf-version.h			\
	$(NULL)

liboaf_la_SOURCES=				\
	oaf-stubs.c				\
	oaf-skels.c				\
	oaf-common.c				\
	oaf-mainloop.c				\
	oaf-actid.c				\
	liboaf-private.h			\
	oaf-factory-stubs.c			\
	oaf-factory-skels.c			\
	oaf-factory-common.c			\
	oaf-plugin.c				\
	oaf-activate.c				\
	oaf-activate-private.h			\
	oaf-registration.c			\
	oaf-servreg.c				\
	oaf-util.c				\
	oaf-i18n.h				\
	oaf-async.c				\
	oaf-async-corba.h			\
	oaf-async-corba.c			\
	oaf-fork-server.c			\
	$(NULL)

BUILT_SOURCES=					\
	oaf-stubs.c				\
	oaf-skels.c				\
	oaf-common.c				\
	oaf-factory-stubs.c			\
	oaf-factory-skels.c			\
	oaf-factory-common.c			\
	oaf.h					\
	oaf-factory.h				\
	$(NULL)

oaf-stubs.c oaf-skels.c oaf.h oaf-common.c: stamp-idl stamp-factory-idl
oaf-factory-stubs.c oaf-factory-skels.c oaf-factory.h oaf-factory-common.c: stamp-idl stamp-factory-idl

# Work around automake brokenness.
../liboaf/oaf.h: stamp-idl
# And again.
../liboaf/oaf-factory.h: stamp-factory-idl

stamp-idl: $(top_srcdir)/idl/oaf.idl
	$(ORBIT_IDL) -I$(top_srcdir)/idl --define=OAF_COMPILATION $(top_srcdir)/idl/oaf.idl
	touch stamp-idl

stamp-factory-idl: $(top_srcdir)/idl/oaf-factory.idl
	$(ORBIT_IDL) -I$(top_srcdir)/idl --define=GNOME_FACTORY_COMPILATION $(top_srcdir)/idl/oaf-factory.idl
	touch stamp-factory-idl

CLEANFILES=					\
	stamp-idl				\
	stamp-factory-idl			\
	$(BUILT_SOURCES)			\
	$(NULL)

dist-hook:
	cd $(distdir) ; rm -f $(CLEANFILES)
