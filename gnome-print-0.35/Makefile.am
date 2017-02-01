#bin_PROGRAMS = gnome-font-install

INSTALLER_DIR = installer

if LIBGPA
LIBGPA_DIR = libgpa data
else
LIBGPA_DIR = 
endif

SUBDIRS = intl po $(LIBGPA_DIR) libgnomeprint afms profiles fonts tests  $(INSTALLER_DIR)

INCLUDES = 			\
	$(GTK_CFLAGS)		\
	-I$(top_srcdir)/intl 	\
	-I$(top_builddir)/intl	\
	$(GNOME_INCLUDEDIR)	\
	@PRINT_INCLUDEDIR@	\
	-DPROFILEDIR=\""$(profilesdir)"\"

CPPFLAGS = @CPPFLAGS@  \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" 	\
	-DBINDIR=\""$(bindir)"\"	\
	-DDATADIR=\""$(datadir)"\"

DEPS = $(top_builddir)/libgnomeprint/libgnomeprint.la
LDADDS = $(top_builddir)/libgnomeprint/libgnomeprint.la

# FIXME: I don't know how to express a shared library dependency
# to automake, but libgnomeprint.so needs to be linked with -llibart_lgpl
# so that if there are multiple versions of libart_lgpl on the system,
# the right version will be picked up.  Linking the final executable to
# libart_lgpl is not enough to ensure this.  Of course, not all OSs
# allow this, but I'm hoping that automake knows how to handle this...

tests:	testprint testtext

EXTRA_DIST =			\
	printConf.sh.in 	\
	gnome-print.spec.in	\
	gnome-print.spec	\
	run-gnome-font-install

confexecdir=$(libdir)
confexec_DATA = printConf.sh

CLEANFILES=printConf.sh

## We create printConf.sh here and not from configure because we want
## to get the paths expanded correctly.  Macros like srcdir are given
## the value NONE in configure if the user doesn't specify them (this
## is an autoconf feature, not a bug).

printConf.sh: printConf.sh.in Makefile
## Use sed and then mv to avoid problems if the user interrupts.
	sed -e 's?\@PRINT_LIBDIR\@?$(PRINT_LIBDIR)?g' \
	    -e 's?\@PRINT_INCLUDEDIR\@?$(PRINT_INCLUDEDIR)?g' \
	    -e 's?\@VERSION\@?$(VERSION)?g' \
	    -e 's?\@PRINT_LIBS\@?$(PRINT_LIBS)?g' \
	      < $(srcdir)/printConf.sh.in > printConf.tmp \
	  && mv printConf.tmp printConf.sh

#gnome_font_install_SOURCES = gnome-font-install.c
#gnome_font_install_LDADD =	\
#	$(GNOME_LIBDIR)		\
#	$(GNOME_LIBS)		\
#	$(GNOME_XML_LIB)	\
#	$(INTLLIBS)

#install-data-local: gnome-font-install
#	$(PERL) $(srcdir)/run-gnome-font-install ./gnome-font-install $(datadir) $(srcdir)
#
