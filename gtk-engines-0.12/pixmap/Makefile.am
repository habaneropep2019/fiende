SUBDIRS = Theme
INCLUDES = $(GDK_IMLIB_CFLAGS)

enginedir = $(libdir)/gtk/themes/engines

# replace the "pixmap" prefix with whatever you're using
engine_LTLIBRARIES = libpixmap.la

libpixmap_la_SOURCES = \
	pixmap_theme_draw.c	\
	pixmap_theme_main.c	\
	pixmap_theme.h

libpixmap_la_LDFLAGS = -module -avoid-version
libpixmap_la_LIBADD = $(GDK_IMLIB_LIBS)
