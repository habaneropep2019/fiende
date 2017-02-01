SUBDIRS = doc glade ghtml

bin_PROGRAMS = gtt

gtt_SOURCES = 		\
	app.c		\
	ctree.c		\
	dialog.c	\
	err.c		\
	err-throw.c	\
	file-io.c	\
	ghtml.c		\
	idle-timer.c	\
	journal.c	\
	log.c		\
	main.c		\
	menucmd.c	\
	menus.c		\
	myoaf.c		\
	plug-in.c	\
	prefs.c		\
	proj.c		\
	props-invl.c	\
	props-proj.c	\
	props-task.c	\
	shorts.c	\
	timer.c		\
	toolbar.c	\
	util.c		\
	xml-read.c	\
	xml-write.c

noinst_HEADERS = 	\
	app.h		\
	ctree.h		\
	cur-proj.h	\
	dialog.h	\
	err-throw.h	\
	file-io.h	\
	ghtml.h		\
	gtt.h		\
	idle-timer.h	\
	journal.h	\
	log.h		\
	menucmd.h	\
	menus.h		\
	myoaf.h		\
	plug-in.h	\
	prefs.h		\
	proj.h		\
	proj_p.h	\
	props-invl.h	\
	props-proj.h	\
	props-task.h	\
	shorts.h	\
	timer.h		\
	toolbar.h	\
	util.h		\
	xml-gtt.h
	

CFLAGS = @CFLAGS@ 		\
	${GCONF_CFLAGS} 	\
	${GTKHTML_CFLAGS} 	\
	${GLADE_CFLAGS}		\
	${PIXBUF_CFLAGS} 

INCLUDES = 			\
	-I$(includedir)		\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	-DGTTGLADEDIR=\""$(datadir)/gtt"\" \
	-DGTTDATADIR=\""$(datadir)/gtt"\" \
	${GNOME_INCLUDEDIR}	\
	${GUILE_INCS}
						 
gtt_LDADD = \
	$(GLADE_LIBS)           \
	$(GNOME_LIBDIR)         \
	$(GNOME_XML_LIB)        \
	$(GTKHTML_LIBS)		\
	$(GNOMEUI_LIBS) 	\
	$(GCONF_LIBS)	 	\
	${GUILE_LIBS}		\
	$(INTLLIBS)

Productivitydir = $(datadir)/gnome/apps/Applications
Productivity_in_files = gtt.desktop.in
Productivity_DATA = $(Productivity_in_files:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

EXTRA_DIST = 			\
	AUTHORS			\
	COPYING			\
	ChangeLog		\
	NEWS			\
	README			\
	TODO			\
	down.xpm		\
	left.xpm		\
	design.txt		\
	$(Productivity_in_files) \
	$(Productivity_DATA) 	\
	$(top_srcdir)/sgmldocs.make

