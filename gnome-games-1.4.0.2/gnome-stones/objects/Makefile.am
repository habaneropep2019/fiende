GSTONES_SRC_DIR = $(srcdir)/../

objectdir= $(libdir)/gnome-stones/objects

INCLUDES = 						\
	-I$(GSTONES_SRC_DIR)				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" 	\
	-I$(includedir)					\
	$(GNOME_INCLUDEDIR)

object_LTLIBRARIES = libgnome-stones.la libgnomekoban.la

libgnome_stones_la_SOURCES = gnome-stones.c
libgnomekoban_la_SOURCES   = gnomekoban.c
