SUBDIRS = stock

INCLUDES =						\
	-I$(includedir)					\
	$(GNOME_INCLUDEDIR)				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"


if BONOBO
bonobo_components = bonobo-application-ps
endif

bin_PROGRAMS = ggv $(bonobo_components)

MSG_SOURCES =                           \
        gsmessage.c                     \
        gsmessage.h

PREFS_SOURCES =                         \
        prefs.c                         \
	prefs.h

GTK_GS_SOURCES =                        \
	callbacks.h                     \
	ggvwindow.h                     \
        crop.c                          \
        crop.h                          \
	gtkgs.c                         \
	gtkgs.h                         \
        ggvutils.c                      \
	ggvutils.h                      \
	ps.c                            \
	ps.h

ggv_SOURCES =                           \
	gtkscrollpane.c                 \
	gtkscrollpane.h                 \
	gtkchecklist.c			\
	gtkchecklist.h			\
        callbacks.c                     \
        main.c                          \
        $(PREFS_SOURCES)                \
	$(MSG_SOURCES)                  \
	$(GTK_GS_SOURCES)

ggv_LDADD =                     \
	$(GNOME_LIBDIR)         \
        $(GNOMEUI_LIBS)         \
	$(INTLLIBS)

bonobo_application_ps_SOURCES =         \
        bonobo-application-ps.c         \
        $(PREFS_SOURCES)                \
	$(GTK_GS_SOURCES)

bonobo_application_ps_CFLAGS = 	\
	$(EXTRA_GNOME_CFLAGS)	\
	$(EXTRA_BONOBO_CFLAGS)

bonobo_application_ps_LDADD =   \
	$(GNOME_LIBDIR)         \
        $(GNOMEUI_LIBS)         \
	$(EXTRA_GNOME_LIBS)	\
        $(INTLLIBS)

