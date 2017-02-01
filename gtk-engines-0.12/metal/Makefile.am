SUBDIRS = Theme
INCLUDES = $(GTK_CFLAGS)

enginedir = $(libdir)/gtk/themes/engines

# replace the "metal" prefix with whatever you're using
engine_LTLIBRARIES = libmetal.la

libmetal_la_SOURCES = \
	metal_theme.h      	\
	metal_theme_draw.c	\
	metal_theme_main.c

libmetal_la_LDFLAGS = -module -avoid-version
libmetal_la_LIBADD = $(GTK_LIBS)

