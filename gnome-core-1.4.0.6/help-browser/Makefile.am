## Process this file with automake to produce Makefile.in.

SUBDIRS = help gnome-man2html gnome-info2html

if HELP_USE_GTKHTML
htmlcflags=-DHELP_USE_GTKHTML
endif

INCLUDES = 							\
	-I$(includedir)						\
	-I$(top_srcdir)/intl -I$(top_builddir)/intl		\
	$(GNOME_INCLUDEDIR)					\
	-DGNOME_PREFIX=\""$(prefix)\"" 	\
	-DGNOMELOCALEDIR=\""$(datadir)/locale\""	\
	$(htmlcflags)

if HELP_USE_GTKHTML
htmllibs = $(GNOME_XML_LIB) @PIXBUF_LIBS@ \
	$(GTK_HTML_PREFIX)/lib/libgtkhtml.a	\
	$(GNOME_PRINT_PREFIX)/lib/libgnomeprint.a
else
htmllibs = $(GTKXMHTML_LIBS)
endif

LDADD = \
	$(GNOMEUI_LIBS)		\
	$(GNORBA_LIBS)		\
	$(GNOME_LIBDIR)		\
	$(ORBIT_LIBS)		\
        $(INTLLIBS)

bin_PROGRAMS = gnome-help-browser  gnome-help-caller

gnome_help_caller_SOURCES = \
				$(ORBIT_HELP_CLIENT_SOURCES)   \
				help-caller.c

gnome_help_browser_SOURCES = gnome-helpwin.c gnome-helpwin.h \
			     gnome-help-browser.c \
			     toc2.c toc2-man.c toc2-ghelp.c toc2-info.c \
			     toc2.h toc2-man.h toc2-ghelp.h toc2-info.h \
	  		     docobj.c docobj.h mime.c mime.h \
			     transport.c transport.h \
			     HTParse.c HTParse.h cache.c cache.h \
			     parseUrl.c parseUrl.h \
			     queue.c queue.h history.c history.h \
                             misc.c misc.h window.c window.h \
			     visit.c visit.h magic.c magic.h \
			     bookmarks.c bookmarks.h \
			     help-browser-impl.c \
				htmlurl.c htmlurl.h \
			     $(ORBIT_HELP_BROWSER_SOURCES)


ORBIT_HELP_COMMON_SOURCES   = help-browser.h \
			      help-browser-common.c

ORBIT_HELP_BROWSER_SOURCES  = $(ORBIT_HELP_COMMON_SOURCES) \
			      help-browser-skels.c

ORBIT_HELP_CLIENT_SOURCES   = $(ORBIT_HELP_COMMON_SOURCES) \
			      help-browser-stubs.c

ORBIT_GENERATED_SOURCEFILES      = $(ORBIT_HELP_COMMON_SOURCES) \
		               help-browser-skels.c help-browser-stubs.c

man_MANS = gnome-convert.1	\
	gnome-download.1
	
MAINTAINERCLEANFILES = $(ORBIT_GENERATED_SOURCEFILES)
CLEANFILES = $(ORBIT_GENERATED_SOURCEFILES) my_help_browser_idl

$(ORBIT_GENERATED_SOURCEFILES): my_help_browser_idl

BUILT_SOURCES = $(ORBIT_GENERATED_SOURCEFILES)

my_help_browser_idl: $(top_srcdir)/idl/help-browser.idl $(ORBIT_IDL)
	$(ORBIT_IDL) $<
	touch my_help_browser_idl

help-caller.o gnome-help-browser.o: help-browser.h

gnome_help_browser_LDADD = $(LDADD) $(htmllibs) $(BZ_LIBS)

PIXMAPS = close.xpm contents.xpm help.xpm left_arrow.xpm right_arrow.xpm 

EXTRA_DIST = toc.c toc-man.c toc-ghelp.c toc-info.c \
	     toc.h toc-man.h toc-ghelp.h toc-info.h \
	     gnome-convert gnome-download sample.c \
	     $(PIXMAPS) gnome-help.desktop.in 	\
	     help-browser.gnorba $(man_MANS)

gnorbadir = $(sysconfdir)/CORBA/servers
gnorba_DATA = help-browser.gnorba

sysdir = $(datadir)/gnome/apps
sys_in_files = gnome-help.desktop.in
sys_DATA = $(sys_in_files:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

install-exec-local:
	$(INSTALL_PROGRAM) $(top_srcdir)/help-browser/gnome-download \
		$(top_srcdir)/help-browser/gnome-convert $(DESTDIR)$(bindir)/
