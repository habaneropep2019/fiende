SUBDIRS = Theme
INCLUDES = $(GTK_CFLAGS)

enginedir = $(libdir)/gtk/themes/engines

# replace the "redmond95" prefix with whatever you're using
engine_LTLIBRARIES = libredmond95.la

libredmond95_la_SOURCES = \
	redmond95_theme_draw.c	\
	redmond95_theme_main.c

libredmond95_la_LDFLAGS = -module -avoid-version
libredmond95_la_LIBADD = $(GTK_LIBS)

