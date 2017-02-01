INCLUDES = 			\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-I$(includedir) $(GNOME_INCLUDEDIR)		\
	-DVERSION=\""$(VERSION)"\"

bin_PROGRAMS = gnomecc

CORBA_SRCLIST = \
	control-center-skels.c \
	control-center-stubs.c \
	control-center-common.c \
	control-center.h

gnomecc_SOURCES = \
	main.c			\
	tree.c			\
	tree.h			\
	callbacks.c		\
	callbacks.h		\
	corba-glue.c		\
	corba-glue.h		\
	gtk-multiview.c		\
	gtk-multiview.h		\
	splash.c		\
	splash.h		\
	capplet-manager.c	\
	capplet-manager.h

BUILT_SOURCES = $(CORBA_SRCLIST)

gnomecc_DEPS=libcapplet.la

gnomecc_LDADD = \
	libcapplet.la					\
	$(GNOME_LIBDIR)					\
	$(GNOMEUI_LIBS)					\
	$(INTLLIBS)					\
	$(GNORBA_LIBS)					\
	$(ORB_LIBS)

appicondir = $(datadir)/pixmaps
appicon_DATA = control-center.png

splashdir = $(datadir)/pixmaps
splash_DATA = ccsplash.png

EXTRA_DIST = gnomecc.desktop.in cappletConf.sh.in $(appicon_DATA) $(splash_DATA)

sysdir = $(datadir)/gnome/apps/Settings
sys_in_files = gnomecc.desktop.in
sys_DATA = $(sys_in_files:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

## Put `exec' in the name because this should be installed by
## `install-exec', not `install-data'.
confexecdir = $(libdir)
confexec_DATA = cappletConf.sh

libcapplet_la_SOURCES = \
	capplet-widget.c	\
	capplet-widget-libs.c	\
	capplet-widget-libs.h	\
	$(CORBA_SRCLIST)

libcapplet_la_LDFLAGS = -version-info 0:0:0 -rpath $(libdir)
libcapplet_la_LIBADD = $(ORB_LIBS) -lm $(DL_LIB) $(GNORBA_LIBS)

libcappletincludedir=$(includedir)/libcapplet1
libcappletinclude_HEADERS = capplet-widget.h

lib_LTLIBRARIES = libcapplet.la

test: $(test_SOURCES)
MAINTAINERCLEANFILES = $(CORBA_SRCLIST)
CLEANFILES = $(CORBA_SRCLIST) my_control_center_idl

$(CORBA_SRCLIST): my_control_center_idl

my_control_center_idl: $(top_srcdir)/idl/control-center.idl $(ORBIT_IDL)
	$(ORBIT_IDL) $(top_srcdir)/idl/control-center.idl
	touch my_control_center_idl

$(srcdir)/main.c $(srcdir)/capplet-manager.c $(srcdir)/capplet-widget.c $(srcdir)/corba-glue.c \
$(srcdir)/capplet-manager.h $(srcdir)/corba-glue.h $(srcdir)/tree.h $(srcdir)/tree.c \
capplet-manager.h corba-glue.h corba-glue.c capplet-widget.c capplet-manager.c \
tree.h tree.c: control-center.h

#
# Create the cappletConf.sh file from the sh.in file
#
cappletConf.sh: cappletConf.sh.in Makefile
	sed -e 's?\@GNOME_LIBDIR\@?$(GNOME_LIBDIR)?g' \
	    -e 's?\@GNOMEUI_LIBS\@?$(GNOMEUI_LIBS)?g' \
	    -e 's?\@CAPPLET_INCLUDEDIR\@?-I$(libcappletincludedir) $(GNOME_INCLUDEDIR)?g' \
	    -e 's?\@ORB_LIBS\@?$(ORB_LIBS)?g' \
	    -e 's?\@VERSION\@?$(VERSION)?g' \
	    < $(srcdir)/cappletConf.sh.in > cappletConf.tmp \
	  && mv cappletConf.tmp cappletConf.sh
