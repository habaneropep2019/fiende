INCLUDES =                      \
        -I$(includedir) @GLIB_CFLAGS@

noinst_PROGRAMS = convertrgb

convertrgb_SOURCES = \
        convertrgb.c		\
        convertrgb.h		\
	loadpng.c

convertrgb_LDADD = \
	$(LIBGTKXMHTML_LIBS) @GLIB_LIBS@

EXTRA_DIST = README

