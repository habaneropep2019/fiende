if LIBGPA
LIBGPA_TEST_CFLAGS = -I$(top_srcdir)/libgpa
LIBGPA_TEST_LIBS = $(top_builddir)/libgpa/libgpa.la
else
LIBGPA_TEST_CFLAGS = 
LIBGPA_TEST_LIBS =
endif

CPPFLAGS = @CPPFLAGS@ \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	-DBINDIR=\""$(bindir)"\" \
	-DDATADIR=\""$(datadir)"\" \
	-DSYSCONFDIR=\""$(sysconfdir)"\" \
	-DLIBDIR=\""$(libdir)"\" \
	-DFONTMAPDIR_STATIC=\""$(datadir)/gnome/fonts"\" \
	-DFONTMAPDIR_DYNAMIC=\""$(sysconfdir)/gnome/fonts"\" \
	-DVERSION=\""$(VERSION)"\"

INCLUDES = \
	-I$(top_srcdir) \
	-I$(top_builddir) \
	$(LIBGNOMEPRINT_CFLAGS)			

bin_PROGRAMS = gnome-font-install

gnome_font_install_SOURCES = \
	gnome-font-install.c \
	gf-pfb.c gf-pfb.h \
	gf-ttf.c gf-ttf.h \
	gf-fontmap.c gf-fontmap.h \
	parseAFM.c parseAFM.h

gnome_font_install_LDADD = \
	$(LIBGNOMEPRINT_LIBS) \
	$(INTLLIBS)

EXTRA_DIST = README.installer README.binaries README

readmedir = $(datadir)/fonts
readme_DATA = README.installer README.binaries

install-data-local: gnome-font-install
	$(PERL) $(top_srcdir)/run-gnome-font-install ./gnome-font-install $(datadir) $(top_srcdir) $(sysconfdir)
