#
# Makefile for the sample Echo server/client Bonobo components.
#

bin_PROGRAMS     = bonobo-echo echo-client
noinst_LIBRARIES = libEcho.a

bonobo_echo_SOURCES =		\
	main.c			\
	echo.c			\
	echo.h

bonobo_echo_LDADD =				\
	$(top_builddir)/bonobo/libbonobo.la	\
	$(top_builddir)/bonobo/libbonobox.la	\
	$(BONOBOX_TEST_LIBS)			\
	libEcho.a				\
	$(INTLLIBS)

echo_client_LDADD =				\
	$(top_builddir)/bonobo/libbonobo.la	\
	$(BONOBOX_TEST_LIBS)			\
	libEcho.a				\
	$(INTLLIBS)

ECHO_CORBA_GENERATED =				\
	Bonobo_Sample_Echo-common.c		\
	Bonobo_Sample_Echo-skels.c		\
	Bonobo_Sample_Echo-stubs.c		\
	Bonobo_Sample_Echo.h

libEcho_a_SOURCES = $(ECHO_CORBA_GENERATED)

$(ECHO_CORBA_GENERATED): Bonobo_Sample_Echo.idl 
	orbit-idl -I$(top_srcdir)/idl -I`$(GNOME_CONFIG) --datadir`/idl $(srcdir)/Bonobo_Sample_Echo.idl
	touch my_echo_idl


oafdir = $(datadir)/oaf
oaf_in_files = Bonobo_Sample_Echo.oaf.in
oaf_DATA = $(oaf_in_files:.oaf.in=.oaf)

@XML_I18N_MERGE_OAF_RULE@

idldir    = $(datadir)/idl
idl_DATA  = Bonobo_Sample_Echo.idl

EXTRA_DIST = $(oaf_in_files) $(oaf_DATA) $(idl_DATA)

INCLUDES = 						\
        -DGNOMELOCALEDIR=\""$(datadir)/locale"\"        \
	-DG_LOG_DOMAIN=\"Echo\"				\
	-I. -I$(top_srcdir)/bonobo			\
	-I$(srcdir) -I$(top_srcdir) 			\
	-I$(top_builddir)				\
        -I$(includedir)                                 \
	$(BONOBOX_TEST_CFLAGS)

MYHACKAROUNDAUTOMAKE=$(shell echo $(ECHO_CORBA_GENERATED))

BUILT_SOURCES = $(ECHO_CORBA_GENERATED)
CLEANFILES += $(ECHO_CORBA_GENERATED)

dist-hook:
	cd $(distdir); rm -f $(BUILT_SOURCES)
