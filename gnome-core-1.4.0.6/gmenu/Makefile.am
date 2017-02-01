INCLUDES = 			\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	-I$(top_srcdir)/intl -I$(top_builddir)/intl	\
	-I$(includedir) $(GNOME_INCLUDEDIR)		\
	-DGNOME_DATADIR=\""$(datadir)"\"

bin_PROGRAMS = gmenu

icon_entry_hack_dir = $(top_srcdir)/panel

ICON_ENTRY_HACK_SRCLIST = \
	icon-entry-hack.c		\
	icon-entry-hack.h

$(ICON_ENTRY_HACK_SRCLIST):
	for file in $(ICON_ENTRY_HACK_SRCLIST); do \
		echo $$file ; \
		ln -sf $(icon_entry_hack_dir)/$$file . ; \
	done

gmenu_SOURCES =	\
	$(ICON_ENTRY_HACK_SRCLIST)	\
	desktop.c		\
	edit.c			\
	gmenu.h			\
	main.c			\
	order.c			\
	save.c			\
	tree.c			\
	treedel.c		\
	treednd.c		\
	treenew.c		\
	treeutil.c		\
	utils.c

man_MANS = gmenu.1

gmenu_LDADD = \
	$(GNOME_LIBDIR)					\
	$(GNOMEUI_LIBS)					\
	$(INTLLIBS)

BUILT_SOURCES = $(ICON_ENTRY_HACK_SRCLIST)

EXTRA_DIST = gmenu.desktop.in top.xpm unknown.xpm $(man_MANS)

sysdir = $(datadir)/gnome/apps/Settings
sys_in_files = gmenu.desktop.in
sys_DATA = $(sys_in_files:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@
