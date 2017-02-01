# Process this file with automake to produce Makefile.in

SUBDIRS = pixmaps doc

Utilitiesdir = $(datadir)/gnome/apps/System
additionaldir  = $(datadir)/logview

INCLUDES = -DG_LOG_DOMAIN=\"logview\"					\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"			\
	-I$(includedir) $(GNOME_INCLUDEDIR) -I$(top_srcdir)/logview/pixmaps\
	-DLOGVIEWINSTALLPREFIX=\""$(prefix)/\""

bin_PROGRAMS = logview

logview_SOURCES = 	\
	about.c		\
	actions.c	\
	calendar.c	\
	config.c	\
	desc_db.c       \
	info.c		\
	log_repaint.c	\
	logrtns.c	\
	logrtns.h	\
	logview.c	\
	logview.h	\
	menus.c		\
	misc.c		\
	monitor.c       \
	userprefsdialog.c       \
	zoom.c

logview_LDADD = \
	$(GNOME_LIBDIR) \
	$(GNOMEUI_LIBS) \
	$(INTLLIBS)

man_MANS = logview.1

Utilities_in_files = logview.desktop.in
Utilities_DATA = $(Utilities_in_files:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

additional_DATA  = logview-descript.db \
                   logview-regexp.db \
		   logview-actions.db

EXTRA_DIST = \
	$(Utilities_in_files)	\
	$(Utilities_DATA)	\
	$(additional_DATA)	\
	logview-pam		\
	logview-security.in	\
	$(man_MANS)

install-data-local: logview-security
if CONSOLE_HELPER
	$(INSTALL_DATA) -D logview-pam $(DESTDIR)$(PAM_PREFIX)/pam.d/logview
	$(INSTALL_DATA) -D logview-security $(DESTDIR)$(PAM_PREFIX)/security/console.apps/logview
	$(INSTALL_DATA) -m 755 -d $(DESTDIR)$(sbindir)
	mv -f $(DESTDIR)$(bindir)/logview $(DESTDIR)$(sbindir)
	$(LN_S) @CONSOLE_HELPER@ $(DESTDIR)$(bindir)/logview
endif
