SUBDIRS = Theme
INCLUDES = $(GTK_CFLAGS)

enginedir = $(libdir)/gtk/themes/engines

# replace the "raleigh" prefix with whatever you're using
engine_LTLIBRARIES = libraleigh.la
libraleigh_la_SOURCES = 	\
	raleigh.h		\
	raleigh_theme_draw.c	\
	raleigh_theme_main.c

libraleigh_la_LDFLAGS = -module -avoid-version
libraleigh_la_LIBADD = $(GTK_LIBS)

