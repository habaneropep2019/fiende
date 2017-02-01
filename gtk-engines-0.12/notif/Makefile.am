SUBDIRS = Theme
INCLUDES = $(GTK_CFLAGS)

enginedir = $(libdir)/gtk/themes/engines

# replace the "notif" prefix with whatever you're using
engine_LTLIBRARIES = libnotif.la
libnotif_la_SOURCES = \
	notif_theme_draw.c	\
	notif_theme_main.c

libnotif_la_LDFLAGS = -module -avoid-version
libnotif_la_LIBADD = $(GTK_LIBS)

