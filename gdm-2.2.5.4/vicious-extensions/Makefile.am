SUBDIRS=.

INCLUDES = \
 	-I.					\
	-I..					\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	-I$(includedir)				\
	$(LIBGLADE_CFLAGS)			\
	$(GNOME_INCLUDEDIR)

noinst_LIBRARIES = libvicious.a libviciousui.a

libvicious_a_SOURCES =	\
	ve-misc.c	\
	ve-misc.h	\
	vicious.h

libviciousui_a_SOURCES = \
	glade-helper.c	\
	glade-helper.h	\
	icon-entry-hack.c	\
	icon-entry-hack.h	\
	ve-miscui.c	\
	ve-miscui.h	\
	viciousui.h
