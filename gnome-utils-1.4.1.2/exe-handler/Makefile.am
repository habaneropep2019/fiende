bin_PROGRAMS = gnome-exe-handler

SUBDIRS = help

INCLUDES = \
	-DG_LOG_DOMAIN=\"ExeHandler\" 			\
	-I$(top_srcdir) 				\
	$(GNOME_INCLUDEDIR)				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"

gnome_exe_handler_SOURCES = 	\
	gnome-exe-handler.c

gnome_exe_handler_LDADD = 	\
	$(GNOME_LIBDIR)		\
	$(GNOMEUI_LIBS)

man_MANS = gnome-exe-handler.1

#
# FIXME: use xml-i18n-tools for this thing
#
Mimedir = $(datadir)/mime-info
Mime_in_files = gnome-exe.keys.in
Mime_DATA = $(Mime_in_files:.keys.in=.keys)
@XML_I18N_MERGE_KEYS_RULE@
Appregdir = $(datadir)/application-registry
Appreg_DATA = gnome-exe.applications

EXTRA_DIST = gnome-exe-handler.1 $(Mime_DATA) $(Appreg_DATA)
