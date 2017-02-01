SUBDIRS = po intl doc

Utilitiesdir = $(datadir)/gnome/apps/System
Utilities_DATA = gtop.desktop

pixmapsdir = $(datadir)/pixmaps
pixmaps_DATA = gnome-gtop.png

INCLUDES = 						\
	-I$(includedir)					\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" 	\
	-DLOCALEDIR=\"$(datadir)/locale\"		\
	$(GNOME_INCLUDEDIR) $(LIBGTOP_INCS)

bin_PROGRAMS = gtop

gtop_SOURCES =	\
	main.c			\
	mdi.c			\
	global.c		\
	global.h		\
	proc.c			\
	proc.h			\
	fromtop.c		\
	fromtop.h		\
	procview.c		\
	procview.h		\
	memusage.c		\
	memusage.h		\
	fsusage.c		\
	fsusage.h		\
	graph.c			\
	graph.h			\
	gtop-graph.c		\
	gtop-graph.h		\
	details.c		\
	details.h		\
	process-details.c	\
	process-details.h	\
	gtop-fsusage.c		\
	gtop-fsusage.h		\
	gtop-memusage.c		\
	gtop-memusage.h		\
	gtop-procview.c		\
	gtop-procview.h		\
	gtop-procbar.c		\
	gtop-procbar.h		\
	dummy.h			\
	properties.c		\
	properties.h		\
	gtop-page.c		\
	gtop-page.h		\
	summary.c		\
	summary.h		\
	session.c		\
	session.h		\
	libgtop.c

gtkrcdir = $(datadir)
gtkrc_DATA = gtoprc

gtop_LDADD = 			\
	$(GNOME_LIBDIR)         \
	$(GNOMEUI_LIBS)         \
	$(INTLLIBS)		\
	$(LIBGTOP_LIBS)		\
	$(LIBGTOP_EXTRA_LIBS)

EXTRA_DIST = 				\
	AUTHORS TODO NEWS BUG-REPORTING	\
	$(gtkrc_DATA)			\
	asc.xpm dsc.xpm 		\
	cpu.xpm mem.xpm swap.xpm 	\
	xcpu0.xpm xcpu1.xpm		\
	xcpu2.xpm xcpu3.xpm		\
	load.xpm			\
	tb_timer.xpm 			\
	tb_timer_stopped.xpm		\
	gtop.desktop gtop.spec.in	\
	perfview.cc perfview.h \
	gnome-gtop.png

dist-hook: gtop.spec
	cp gtop.spec $(distdir)

release:
	$(MAKE) dist distdir=$(PACKAGE)-$(VERSION)

