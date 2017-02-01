NULL=

lib_LTLIBRARIES=libeel.la

INCLUDES =						\
	-DG_LOG_DOMAIN=\"Eel\"				\
	-I$(top_srcdir)					\
	$(EEL_DEPENDENCY_CFLAGS)			\
	${GCONF_CFLAGS}					\
	$(LIBRSVG_CFLAGS)				\
	-DDATADIR=\""$(datadir)"\"			\
	-DEEL_DATADIR=\""$(datadir)/eel"\"		\
	-DSOURCE_DATADIR=\""$(top_srcdir)/data"\"	\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	$(NULL)

libeel_la_LDFLAGS =				\
	-version-info @EEL_VERSION_INFO@	\
	$(EEL_DEPENDENCY_LIBS)			\
	$(NULL)

libeel_la_SOURCES =				\
	eel-art-extensions.c			\
	eel-art-gtk-extensions.c		\
	eel-background-canvas-group.c		\
	eel-background-canvas-group.h		\
	eel-background.c			\
	eel-canvas-rect.c			\
	eel-caption-table.c			\
	eel-caption.c				\
	eel-clickable-image.c			\
	eel-clist.c				\
	eel-ctree.c				\
	eel-dateedit-extensions.c		\
	eel-debug-drawing.c			\
	eel-debug.c				\
	eel-dnd.c				\
	eel-ellipsizing-label.c			\
	eel-enumeration.c			\
	eel-features.c				\
	eel-font-manager.c			\
	eel-font-picker.c			\
	eel-gconf-extensions.c			\
	eel-gdk-extensions.c			\
	eel-gdk-font-extensions.c		\
	eel-gdk-pixbuf-extensions.c		\
	eel-generous-bin.c			\
	eel-glib-extensions.c			\
	eel-glyph.c				\
	eel-gnome-extensions.c			\
	eel-graphic-effects.c			\
	eel-gtk-container.c			\
	eel-gtk-extensions.c			\
	eel-image-chooser.c			\
	eel-image-table.c			\
	eel-image-with-background.c		\
	eel-image.c				\
	eel-label-with-background.c		\
	eel-label.c				\
	eel-labeled-image.c			\
	eel-lib-self-check-functions.c		\
	eel-list-column-title.c			\
	eel-list.c				\
	eel-password-dialog.c			\
	eel-preferences-box.c			\
	eel-preferences-group.c			\
	eel-preferences-item.c			\
	eel-preferences-pane.c			\
	eel-preferences.c			\
	eel-radio-button-group.c		\
	eel-region.c				\
	eel-scalable-font.c			\
	eel-self-checks.c			\
	eel-smooth-text-layout-cache.c		\
	eel-smooth-text-layout.c		\
	eel-smooth-widget.c			\
	eel-stock-dialogs.c			\
	eel-string-list.c			\
	eel-string-picker.c			\
	eel-string.c				\
	eel-text-caption.c			\
	eel-types.c				\
	eel-vfs-extensions.c			\
	eel-viewport.c				\
	eel-wrap-table.c			\
	eel-xml-extensions.c			\
	$(NULL)

libeelincludedir = $(includedir)/eel-1/eel

eel_headers =					\
	eel-art-extensions.h			\
	eel-art-gtk-extensions.h		\
	eel-background.h			\
	eel-canvas-rect.h			\
	eel-caption-table.h			\
	eel-caption.h				\
	eel-clickable-image.h			\
	eel-clist.h				\
	eel-ctree.h				\
	eel-dateedit-extensions.h		\
	eel-debug-drawing.h			\
	eel-debug.h				\
	eel-ellipsizing-label.h			\
	eel-enumeration.h			\
	eel-features.h				\
	eel-font-manager.h			\
	eel-font-picker.h			\
	eel-gconf-extensions.h			\
	eel-gdk-extensions.h			\
	eel-gdk-font-extensions.h		\
	eel-gdk-pixbuf-extensions.h		\
	eel-generous-bin.h			\
	eel-glib-extensions.h			\
	eel-glyph.h				\
	eel-gnome-extensions.h			\
	eel-graphic-effects.h			\
	eel-gtk-container.h			\
	eel-gtk-extensions.h			\
	eel-gtk-macros.h			\
	eel-image-chooser.h			\
	eel-image-table.h			\
	eel-image-with-background.h		\
	eel-image.h				\
	eel-label-with-background.h		\
	eel-label.h				\
	eel-labeled-image.h			\
	eel-list-column-title.h			\
	eel-list.h				\
	eel-password-dialog.h			\
	eel-preferences-box.h			\
	eel-preferences-group.h			\
	eel-preferences-item.h			\
	eel-preferences-pane.h			\
	eel-preferences.h			\
	eel-radio-button-group.h		\
	eel-region.h				\
	eel-scalable-font-private.h		\
	eel-scalable-font.h			\
	eel-self-checks.h			\
	eel-smooth-text-layout-cache.h		\
	eel-smooth-text-layout.h		\
	eel-smooth-widget.h			\
	eel-stock-dialogs.h			\
	eel-string-list.h			\
	eel-string-picker.h			\
	eel-string.h				\
	eel-text-caption.h			\
	eel-types.h				\
	eel-vfs-extensions.h			\
	eel-viewport.h				\
	eel-wrap-table.h			\
	eel-xml-extensions.h			\
	eel.h					\
	$(NULL)

noinst_HEADERS = \
	eel-lib-self-check-functions.h		\
	$(NULL)

libeelinclude_HEADERS =				\
	$(eel_headers)				\
	eel-dnd.h				\
	eel-type-builtins.h			\
	$(NULL)

BUILT_SOURCES =					\
	eel-enums.defs				\
	eel-type-builtins.h			\
	eel-type-builtins-evals.c		\
	eel-type-builtins-ids.c			\
	eel-type-builtins-vars.c		\
	$(NULL)

CLEANFILES = \
	$(BUILT_SOURCES) \
	$(NULL)

# generate eel-enums.defs file
eel-enums.defs: @REBUILD@ makeenums.pl $(eel_headers)
	cd $(srcdir)						\
	&& $(PERL) makeenums.pl defs $(eel_headers) > ed.tmp	\
	&& mv ed.tmp eel-enums.defs

# generate type identifier header (GTK_TYPE_WIDGET_FLAGS)
eel-type-builtins.h: @REBUILD@ eel-enums.defs maketypes.awk
	cd $(srcdir)								\
	&& LC_COLLATE=C $(AWK) -f maketypes.awk eel-enums.defs macros > etb.tmp	\
	&& mv etb.tmp eel-type-builtins.h

# generate type identifier variables (GTK_TYPE_WIDGET_FLAGS)
eel-type-builtins-vars.c: @REBUILD@ eel-enums.defs maketypes.awk
	cd $(srcdir)								\
	&& LC_COLLATE=C $(AWK) -f maketypes.awk eel-enums.defs variables > etbv.tmp	\
	&& mv etbv.tmp eel-type-builtins-vars.c

# generate type entries for type-id registration
eel-type-builtins-ids.c: @REBUILD@ eel-enums.defs maketypes.awk
	cd $(srcdir)								\
	&& LC_COLLATE=C $(AWK) -f maketypes.awk eel-enums.defs entries > etbi.tmp	\
	&& mv etbi.tmp eel-type-builtins-ids.c

# generate enum value arrays
eel-type-builtins-evals.c: @REBUILD@ makeenums.pl eel-enums.defs
	cd $(srcdir)							\
	&& $(PERL) makeenums.pl arrays $(eel_headers) > etbe.tmp	\
	&& mv etbe.tmp eel-type-builtins-evals.c


noinst_PROGRAMS = check-program

DEPS = $(top_builddir)/eel/libeel.la

check_program_SOURCES=check-program.c
check_program_DEPENDENCIES = $(DEPS)
check_program_LDADD =
check_program_LDFLAGS =				\
	$(DEPS)					\
	-lm

TESTS=check-eel

EXTRA_DIST =					\
	eel-enums.defs				\
	maketypes.awk				\
	makeenums.pl				\
	check-eel				\
	eel-type-builtins-evals.c		\
	eel-type-builtins-ids.c			\
	eel-type-builtins-vars.c		\
	$(NULL)
