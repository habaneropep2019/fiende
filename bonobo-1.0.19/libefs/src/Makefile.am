#
# Makefile for libefs.
#

INCLUDES = $(GLIB_CFLAGS)

lib_LTLIBRARIES = libefs.la

bin_PROGRAMS = efstool
efstool_SOURCES = efstool.c
efstool_LDADD = libefs.la $(GLIB_LIBS) -lz $(CRYPTLIB)

noinst_PROGRAMS = efstest
efstest_SOURCES = efstest.c
efstest_LDADD = libefs.la $(GLIB_LIBS) -lz $(CRYPTLIB)

include_HEADERS = efs.h efs_error.h

noinst_HEADERS = efs_internal.h ib1_driver.h blowfish.h

libefs_la_LDFLAGS = \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) -lz

libefs_la_SOURCES = 	\
	efs.c		\
	wrapper.c		\
	lock.c		\
	ib1_cache.c	\
	ib1_alloc.c	\
	ib1_driver.c	\
	ib1_inode.c	\
	ib1_file.c	\
	blowfish.c	\
	mime.c

