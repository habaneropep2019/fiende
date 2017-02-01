NULL=

INCLUDES =\
	-I$(top_srcdir)/test \
	-I$(top_srcdir) \
	$(EEL_DEPENDENCY_CFLAGS) \
	-DVERSION="\"$(VERSION)\"" \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	-DEEL_DATADIR=\""$(datadir)/eel"\" \
	-DDATADIR=\""$(datadir)"\" \
	$(NULL)

LDADD =\
	$(top_builddir)/eel/libeel.la \
	$(NULL)

noinst_PROGRAMS =\
	test-eel-background \
	test-eel-clickable-image \
	test-eel-font \
	test-eel-font-manager \
	test-eel-font-picker \
	test-eel-font-simple \
	test-eel-glyph \
	test-eel-glyph-simple \
	test-eel-gtk-style \
	test-eel-image \
	test-eel-image-background \
	test-eel-image-chooser \
	test-eel-image-scrolled \
	test-eel-image-simple \
	test-eel-image-table \
	test-eel-image-tile \
	test-eel-label \
	test-eel-label-background \
	test-eel-label-flavorful \
	test-eel-label-offset \
	test-eel-label-scrolled \
	test-eel-label-simple \
	test-eel-label-wrapped \
	test-eel-labeled-image \
	test-eel-password-dialog \
	test-eel-pixbuf-tile \
	test-eel-smooth-text-layout \
	test-eel-viewport-constraint \
	test-eel-widgets \
	$(NULL)

test_eel_background_SOURCES = test-eel-background.c
test_eel_clickable_image_SOURCES = test-eel-clickable-image.c test.c
test_eel_font_SOURCES = test-eel-font.c test.c
test_eel_font_manager_SOURCES = test-eel-font-manager.c test.c
test_eel_font_picker_SOURCES = test-eel-font-picker.c test.c
test_eel_font_simple_SOURCES = test-eel-font-simple.c test.c
test_eel_glyph_SOURCES = test-eel-glyph.c test.c
test_eel_glyph_simple_SOURCES = test-eel-glyph-simple.c test.c
test_eel_gtk_style_SOURCES = test-eel-gtk-style.c test.c dumb-box.c dumb-box.h
test_eel_image_SOURCES = test-eel-image.c
test_eel_image_background_SOURCES = test-eel-image-background.c test.c
test_eel_image_chooser_SOURCES = test-eel-image-chooser.c test.c
test_eel_image_scrolled_SOURCES = test-eel-image-scrolled.c test.c
test_eel_image_simple_SOURCES = test-eel-image-simple.c
test_eel_image_table_SOURCES = test-eel-image-table.c test.c
test_eel_image_tile_SOURCES = test-eel-image-tile.c test.c
test_eel_label_SOURCES = test-eel-label.c
test_eel_label_background_SOURCES = test-eel-label-background.c test.c
test_eel_label_flavorful_SOURCES = test-eel-label-flavorful.c
test_eel_label_offset_SOURCES = test-eel-label-offset.c test.c
test_eel_label_scrolled_SOURCES = test-eel-label-scrolled.c test.c
test_eel_label_simple_SOURCES = test-eel-label-simple.c test.c
test_eel_label_wrapped_SOURCES = test-eel-label-wrapped.c
test_eel_labeled_image_SOURCES = test-eel-labeled-image.c test.c
test_eel_password_dialog_SOURCES = test-eel-password-dialog.c
test_eel_pixbuf_tile_SOURCES = test-eel-pixbuf-tile.c test.c
test_eel_smooth_text_layout_SOURCES = test-eel-smooth-text-layout.c test.c
test_eel_widgets_SOURCES = test-eel-widgets.c

EXTRA_DIST = \
	test.h \
	$(NULL)
