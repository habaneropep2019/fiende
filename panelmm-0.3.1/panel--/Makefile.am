## $Id: Makefile.am,v 1.6 2002/01/17 21:04:08 cactus Exp $

SUBDIRS = . private 

MM_GEN_FILES	= applet-main.gen_h applet.gen_h
MM_CC_FILES	= $(MM_GEN_FILES:.gen_h=.cc)
MM_H_FILES	= $(MM_GEN_FILES:.gen_h=.h)

BUILT_SOURCES	= $(MM_CC_FILES) $(MM_H_FILES)

%.cc %.h private/%_p.h: %.gen_h $(GTKMM_PROC)
	$(GTKMM_PROC) --local --gnome $$(basename $< .gen_h) $(srcdir) $(top_builddir)/panel--

libpanelmm_includedir		= $(includedir)/panel--
libpanelmm_include_HEADERS	= $(MM_H_FILES)

include_HEADERS			= panel--.h

lib_LTLIBRARIES			= libpanelmm.la

libpanelmm_la_SOURCES		= $(MM_CC_FILES) $(MM_H_FILES)
libpanelmm_la_LDFLAGS		= -version-info $(PANELMM_LIBRARY_VERSION)
INCLUDES			= -I$(top_builddir)

DISTCLEANFILES			= $(BUILT_SOURCES)
CLEANFILES    			= $(BUILT_SOURCES)
EXTRA_DIST			= $(MM_GEN_FILES)

