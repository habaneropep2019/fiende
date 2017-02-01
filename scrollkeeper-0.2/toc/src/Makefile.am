# copyright (C) 2000 Sun Microsystems, Inc.
## Process this file with automake to produce Makefile.in.

INCLUDES = \
	-I$(top_srcdir)/libs \
	$(XML_CFLAGS)

LDADD = \
	$(XML_LIBS) \
	$(top_builddir)/libs/libscrollkeeper.la

bin_PROGRAMS = scrollkeeper_toc 

scrollkeeper_toc_SOURCES = \
	toc.c \
	toc.h \
	toc-extract.c \
	toc-extract.h
