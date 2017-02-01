INCLUDES = $(GTK_CFLAGS) 	\
	$(GNOME_INCLUDEDIR)

bin_PROGRAMS = gnome-edit

gnome_edit_SOURCES = 		\
	gnome-edit.c

man_MANS = gnome-edit.1
	
gnome_edit_LDADD =             	\
	$(GNOME_LIBDIR) 	\
	$(GNOMEUI_LIBS)		\
	$(GTK_LIBS)		\
	$(INTLLIBS)

EXTRA_DIST = $(man_MANS)
