NULL=

bin_PROGRAMS=oafd oaf-client

INCLUDES=					\
	-I$(top_srcdir) 			\
	-I$(top_builddir) 			\
	-I$(top_builddir)/liboaf		\
	-I$(top_srcdir)/liboaf			\
	-DOAFINFODIR=\"$(pkgdatadir)\" 		\
	-DOAF_LOCALEDIR=\""$(datadir)/locale"\" \
	@ORBIT_CFLAGS@ 				\
	@XML_CFLAGS@ 				\
	@GLIB_CFLAGS@				\
	-DOAF_CONFDIR=\"$(sysconfdir)\"		\
	$(NULL)

oafd_SOURCES=					\
	oafd-corba-extensions.c			\
	oafd-corba-extensions.h			\
	qsort_ex.c				\
	qsort_ex.h				\
	oaf-skels.c				\
	ac-query-lex.c				\
	ac-query-parse.c			\
	ac-query-parse.h			\
	ac-query-expr.h				\
	ac-corba.c				\
	ac-query-expr.c				\
	od-corba.c				\
	od-load.c				\
	od-utils.c				\
	od-activate.c				\
	main.c					\
	od-utils.h				\
	oafd.h

LDADD=../liboaf/liboaf.la \
	@ORBIT_LIBS@ -lORBitCosNaming -lname-server \
	@XML_LIBS@ \
	@INTLLIBS@ \
	@GLIB_LIBS@

oaf_client_SOURCES=client.c

oafinfodir=$(pkgdatadir)

oaf_in_files = OAF_naming-service.oaf.in
oafinfo_DATA = $(oaf_in_files:.oaf.in=.oaf)

@XML_I18N_MERGE_OAF_RULE@

EXTRA_DIST=ac-query-lex.l ac-query-parse.y $(oaf_in_files) $(oafinfo_DATA)



YFLAGS                  = -d -v

ac-query-lex.c: $(srcdir)/ac-query-lex.l ac-query-parse.h
	@rm -f $@
	$(LEX) -t $(srcdir)/ac-query-lex.l >$@

stamp-parser: $(srcdir)/ac-query-parse.y
	$(YACC) $(YFLAGS) $(srcdir)/ac-query-parse.y
	touch $@

ac-query-parse.c: stamp-parser
	test -f y.tab.c && mv -f y.tab.c ac-query-parse.c || touch $@

ac-query-parse.h: stamp-parser
	test -f y.tab.h && mv -f y.tab.h ac-query-parse.h || touch $@

BUILT_SOURCES=ac-query-lex.c ac-query-parse.c ac-query-parse.h oaf-skels.c OAF_naming-service.oaf

CLEANFILES=stamp-parser stamp-idl

oaf-skels.c: $(top_srcdir)/idl/oaf.idl $(ORBIT_IDL)
	$(ORBIT_IDL) --noheaders $(top_srcdir)/idl/oaf.idl

dist-hook:
	cd $(distdir) ; rm -f $(CLEANFILES)
