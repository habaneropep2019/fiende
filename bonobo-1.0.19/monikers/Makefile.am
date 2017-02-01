INCLUDES =                                              \
        -DGNOMELOCALEDIR=\""$(datadir)/locale"\"        \
	-I. 						\
	-I$(srcdir) -I$(top_srcdir) 			\
	-I$(top_builddir)				\
	-I$(top_srcdir)/bonobo                          \
        -I$(includedir)                                 \
	$(BONOBOX_TEST_CFLAGS)

OAF_FILES =					\
	Bonobo_Moniker_gzip.oaf.in		\
	Bonobo_Moniker_http.oaf.in

Bonobo_Moniker_std.oaf.in : $(srcdir)/Bonobo_Moniker_std.oaf.in.in $(top_builddir)/config.status
	sed -e "s|\@MONIKER_LIBDIR\@|$(monikerdir)|" \
	 $(srcdir)/Bonobo_Moniker_std.oaf.in.in > Bonobo_Moniker_std.oaf.in

CLEANFILES+=Bonobo_Moniker_std.oaf Bonobo_Moniker_std.oaf.in

oafdir = $(datadir)/oaf
oaf_DATA = $(OAF_FILES:.oaf.in=.oaf) Bonobo_Moniker_std.oaf

@XML_I18N_MERGE_OAF_RULE@

monikerdir = $(libdir)/bonobo/monikers
moniker_LTLIBRARIES = libmoniker_std.la

bin_PROGRAMS = 				\
	bonobo-moniker-gunzip 		\
	bonobo-moniker-http		\
	moniker-test

# bonobo-moniker-untar.oaf.in

# bonobo-moniker-untar

#noinst_PROGRAMS = moniker-control-test moniker-test

moniker_ldadd = 				\
	$(top_builddir)/bonobo/libbonobo.la	\
	$(top_builddir)/bonobo/libbonobox.la	\
	$(BONOBOX_TEST_LIBS)

libmoniker_std_la_SOURCES =			\
	bonobo-moniker-extender-file.c  	\
	bonobo-moniker-extender-stream.c  	\
	bonobo-moniker-file.c			\
	bonobo-moniker-item.c			\
	bonobo-moniker-new.c			\
	bonobo-moniker-oaf.c			\
	bonobo-moniker-cache.c			\
	bonobo-stream-cache.h			\
	bonobo-stream-cache.c			\
	bonobo-moniker-query.c			\
	bonobo-moniker-query.h			\
	bonobo-moniker-std.h            	\
	bonobo-moniker-std.c			

libmoniker_std_la_LDFLAGS = -module

bonobo_moniker_gunzip_SOURCES =		\
	bonobo-moniker-gunzip.c		\
	bonobo-moniker-gunzip.h

bonobo_moniker_gunzip_LDADD =		\
	$(moniker_ldadd)

## bonobo_moniker_untar_SOURCES =          \
## 	bonobo-moniker-untar.c          \
## 	bonobo-moniker-untar.h

## bonobo_moniker_untar_LDADD =            \
## 	$(moniker_ldadd)

bonobo_moniker_http_SOURCES =		\
	bonobo-moniker-http.c		\
	bonobo-moniker-http.h

bonobo_moniker_http_LDADD =		\
	$(moniker_ldadd)

#moniker_control_test_SOURCES =		\
#	moniker-control-test.c

#moniker_control_test_LDADD =		\
#	$(moniker_ldadd) -lgtkhtml

moniker_test_SOURCES =			\
	moniker-test.c

moniker_test_LDADD =			\
	$(moniker_ldadd)

EXTRA_DIST = $(OAF_FILES)		\
	Bonobo_Moniker_std.oaf.in.in
