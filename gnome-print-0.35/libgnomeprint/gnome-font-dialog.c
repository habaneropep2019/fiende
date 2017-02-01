/* font-dialog.c - functions that do things to fonts.
 *
 * Copyright (C) 1998 Chris Lahey.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

/* Must include these two first */
#include "config.h"
#include <libgnomeprint/gnome-print-i18n.h>

#include <stdlib.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <libgnomeprint/gnome-font-dialog.h>
#include <libgnomeui/gnome-stock.h>
#include <libgnomeui/gnome-canvas-rect-ellipse.h>
#include <libgnomeprint/gnome-canvas-hacktext.h>
#include <libgnomeprint/gnome-font-family.h>

#ifdef HAVE_STRING_H
#include <string.h>
#endif

struct _GnomeFontSelection
{
	GtkHBox hbox;
  
	GtkWidget * family;

	GtkWidget * fontbox;

	GtkWidget * stylebox;
	GtkWidget * style;

	GtkWidget * sizebox;
	GtkWidget * size;

	GtkWidget * previewframe;

	GnomeFontFamily * selectedfamily;
	GnomeFontFace * selectedface;
	GnomeFont * selectedfont;
	gdouble selectedsize;
};


struct _GnomeFontSelectionClass
{
	GtkHBoxClass parent_class;

	void (* font_set) (GnomeFontSelection * fontsel, GnomeFont * font);
};

enum {FONT_SET, LAST_SIGNAL};

/* This is the initial and maximum height of the preview entry (it expands
   when large font sizes are selected). Initial height is also the minimum. */
#define MIN_PREVIEW_HEIGHT 44
#define MAX_PREVIEW_HEIGHT 300

/* These are what we use as the standard font sizes, for the size clist.
   Note that when using points we still show these integer point values but
   we work internally in decipoints (and decipoint values can be typed in). */
static gchar * font_sizes[] = {
  "8", "9", "10", "11", "12", "13", "14", "16", "18", "20", "22", "24", "26", "28",
  "32", "36", "40", "48", "56", "64", "72"
};

static void gnome_font_selection_class_init (GnomeFontSelectionClass *klass);
static void gnome_font_selection_init (GnomeFontSelection *fontsel);
static void gnome_font_selection_destroy (GtkObject *object);

static void gnome_font_selection_select_family (GtkCList * clist, gint row, gint column, GdkEvent * event, gpointer data);
static void gnome_font_selection_select_style (GtkCList * clist, gint row, gint column, GdkEvent * event, gpointer data);
static void gnome_font_selection_select_size (GtkEditable * editable, gpointer data);
static void gnome_font_selection_fill_families (GnomeFontSelection * fontsel);
static void gnome_font_selection_fill_styles (GnomeFontSelection * fontsel);

static GtkHBoxClass *gfs_parent_class = NULL;
static guint gfs_signals[LAST_SIGNAL] = {0};

GtkType
gnome_font_selection_get_type ()
{
	static GtkType font_selection_type = 0;
	if (!font_selection_type) {
		static const GtkTypeInfo fontsel_type_info = {
			"GnomeFontSelection",
			sizeof (GnomeFontSelection),
			sizeof (GnomeFontSelectionClass),
			(GtkClassInitFunc) gnome_font_selection_class_init,
			(GtkObjectInitFunc) gnome_font_selection_init,
			NULL, NULL,
			(GtkClassInitFunc) NULL,
		};
		font_selection_type = gtk_type_unique (GTK_TYPE_HBOX, &fontsel_type_info);
	}
	return font_selection_type;
}

static void
gnome_font_selection_class_init (GnomeFontSelectionClass *klass)
{
	GtkObjectClass *object_class;
  
	object_class = (GtkObjectClass *) klass;
  
	gfs_parent_class = gtk_type_class (GTK_TYPE_HBOX);
  
	gfs_signals[FONT_SET] = gtk_signal_new ("font_set",
						GTK_RUN_LAST,
						object_class->type,
						GTK_SIGNAL_OFFSET (GnomeFontSelectionClass, font_set),
						gtk_marshal_NONE__POINTER,
						GTK_TYPE_NONE,
						1, GTK_TYPE_OBJECT);
	gtk_object_class_add_signals (object_class, gfs_signals, LAST_SIGNAL);

	object_class->destroy = gnome_font_selection_destroy;
}

static void
gnome_font_selection_init (GnomeFontSelection * fontsel)
{
	static GList * sizelist = NULL;
	GtkWidget * f, * sw, * cl, * vb, * hb, * c, * l;

	gtk_box_set_homogeneous ((GtkBox *) fontsel, TRUE);
	gtk_box_set_spacing ((GtkBox *) fontsel, 4);

	/* Family frame */

	f = gtk_frame_new (_("Font family"));
	gtk_widget_show (f);
	gtk_box_pack_start ((GtkBox *) fontsel, f, TRUE, TRUE, 0);

	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_container_border_width ((GtkContainer *) sw, 4);
	gtk_scrolled_window_set_policy ((GtkScrolledWindow *) sw, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_widget_show (sw);
	gtk_container_add ((GtkContainer *) f, sw);

	cl = gtk_clist_new (1);
	gtk_clist_set_selection_mode ((GtkCList *) cl, GTK_SELECTION_SINGLE);
	gtk_clist_column_titles_hide ((GtkCList *) cl);
#if 0
	gtk_widget_set_usize (cl, 64, 128);
#endif
	gtk_clist_set_column_auto_resize ((GtkCList *) cl, 0, TRUE);
	gtk_widget_show (cl);
	gtk_signal_connect (GTK_OBJECT (cl), "select_row",
			    GTK_SIGNAL_FUNC (gnome_font_selection_select_family), fontsel);
	gtk_container_add ((GtkContainer *) sw, cl);
	fontsel->family = cl;
	fontsel->selectedfamily = NULL;

	/* Fontbox */

	vb = gtk_vbox_new (FALSE, 4);
	gtk_widget_show (vb);
	gtk_box_pack_start ((GtkBox *) fontsel, vb, TRUE, TRUE, 0);
	fontsel->fontbox = vb;

	/* Style frame */

	f = gtk_frame_new (_("Style"));
	gtk_widget_show (f);
	gtk_box_pack_start ((GtkBox *) vb, f, TRUE, TRUE, 0);

	/* Stylebox */

	vb = gtk_vbox_new (FALSE, 4);
	gtk_container_border_width ((GtkContainer *) vb, 4);
	gtk_widget_show (vb);
	gtk_container_add ((GtkContainer *) f, vb);
	fontsel->stylebox = vb;

	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy ((GtkScrolledWindow *) sw, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_widget_show (sw);
	gtk_box_pack_start ((GtkBox *) vb, sw, TRUE, TRUE, 0);

	cl = gtk_clist_new (1);
	gtk_clist_set_selection_mode ((GtkCList *) cl, GTK_SELECTION_SINGLE);
	gtk_clist_column_titles_hide ((GtkCList *) cl);
#if 0
	gtk_widget_set_usize (cl, 64, 64);
#endif
	gtk_clist_set_column_auto_resize ((GtkCList *) cl, 0, TRUE);
	gtk_widget_show (cl);
	gtk_signal_connect (GTK_OBJECT (cl), "select_row",
			    GTK_SIGNAL_FUNC (gnome_font_selection_select_style), fontsel);
	gtk_container_add ((GtkContainer *) sw, cl);
	fontsel->style = cl;
	fontsel->selectedface = NULL;

	/* Sizebox */

	hb = gtk_hbox_new (FALSE, 4);
	gtk_widget_show (hb);
	gtk_box_pack_start ((GtkBox *) vb, hb, FALSE, FALSE, 0);
	fontsel->sizebox = hb;

	c = gtk_combo_new ();
	gtk_widget_set_usize (c, 64, -1);
	gtk_combo_set_value_in_list ((GtkCombo *) c, FALSE, FALSE);
	gtk_combo_set_use_arrows ((GtkCombo *) c, TRUE);
	gtk_combo_set_use_arrows_always ((GtkCombo *) c, TRUE);
	gtk_widget_show (c);
	gtk_signal_connect (GTK_OBJECT (((GtkCombo *) c)->entry), "changed",
			    GTK_SIGNAL_FUNC (gnome_font_selection_select_size), fontsel);
	gtk_box_pack_end ((GtkBox *) hb, c, FALSE, FALSE, 0);
	fontsel->size = c;

	if (!sizelist) {
		gint i;
		for (i = 0; i < (sizeof (font_sizes) / sizeof (font_sizes[0])); i++) {
			sizelist = g_list_prepend (sizelist, font_sizes[i]);
		}
		sizelist = g_list_reverse (sizelist);
	}

	gtk_combo_set_popdown_strings ((GtkCombo *) c, sizelist);

	gtk_entry_set_text ((GtkEntry *) ((GtkCombo *) c)->entry, "12");
	fontsel->selectedsize = 12.0;

	l = gtk_label_new (_("Font size:"));
	gtk_widget_show (l);
	gtk_box_pack_end ((GtkBox *) hb, l, FALSE, FALSE, 0);
}

static void
gnome_font_selection_destroy (GtkObject *object)
{
	GnomeFontSelection *fontsel;
  
	g_return_if_fail (object != NULL);
	g_return_if_fail (GNOME_IS_FONT_SELECTION (object));
  
	fontsel = GNOME_FONT_SELECTION (object);

	if (fontsel->selectedfont) gnome_font_unref (fontsel->selectedfont);
	if (fontsel->selectedface) gnome_font_face_unref (fontsel->selectedface);
	if (fontsel->selectedfamily) gnome_font_family_unref (fontsel->selectedfamily);

  	if (GTK_OBJECT_CLASS (gfs_parent_class)->destroy)
		(* GTK_OBJECT_CLASS (gfs_parent_class)->destroy) (object);
}

GtkWidget *
gnome_font_selection_new()
{
	GnomeFontSelection * fontsel;
  
	fontsel = gtk_type_new (GNOME_TYPE_FONT_SELECTION);
  
	gnome_font_selection_fill_families (fontsel);

	/* Select first font in family list */

	gtk_clist_select_row ((GtkCList *) fontsel->family, 0, 0);

	return GTK_WIDGET (fontsel);
}

static void
gnome_font_selection_select_family (GtkCList * clist, gint row, gint column, GdkEvent * event, gpointer data)
{
	GnomeFontSelection * fontsel;
	gchar * familyname;
	GnomeFontFamily * family;

	fontsel = GNOME_FONT_SELECTION (data);

	gtk_clist_get_text (clist, row, column, &familyname);

	if (fontsel->selectedfamily) {
		gnome_font_family_unref (fontsel->selectedfamily);
	}

	if (familyname) {
		family = gnome_font_family_new (familyname);
		fontsel->selectedfamily = family;
	} else {
		fontsel->selectedfamily = NULL;
	}

	gnome_font_selection_fill_styles (fontsel);
}

static void
gnome_font_selection_select_style (GtkCList * clist, gint row, gint column, GdkEvent * event, gpointer data)
{
	GnomeFontSelection * fontsel;
	gchar * style;

	fontsel = GNOME_FONT_SELECTION (data);

	if (!fontsel->selectedfamily) return;

	gtk_clist_get_text (clist, row, column, &style);

	if (fontsel->selectedface) gnome_font_face_unref (fontsel->selectedface);
	fontsel->selectedface = gnome_font_family_get_face_by_stylename (fontsel->selectedfamily, style);

	if (fontsel->selectedfont) gnome_font_unref (fontsel->selectedfont);
	fontsel->selectedfont = gnome_font_face_get_font_default (fontsel->selectedface, fontsel->selectedsize);

	gtk_signal_emit (GTK_OBJECT (fontsel), gfs_signals[FONT_SET], fontsel->selectedfont);
}

static void
gnome_font_selection_select_size (GtkEditable * editable, gpointer data)
{
	GnomeFontSelection * fontsel;
	gchar * sizestr;

	fontsel = GNOME_FONT_SELECTION (data);

	if (!fontsel->selectedface) return;

	sizestr = gtk_editable_get_chars (GTK_EDITABLE (GTK_COMBO (fontsel->size)->entry), 0, -1);

	fontsel->selectedsize = MAX (atoi (sizestr), 1.0);

	g_free (sizestr);

	if (fontsel->selectedfont) gnome_font_unref (fontsel->selectedfont);
	fontsel->selectedfont = gnome_font_face_get_font_default (fontsel->selectedface, fontsel->selectedsize);

	gtk_signal_emit (GTK_OBJECT (fontsel), gfs_signals[FONT_SET], fontsel->selectedfont);
}

static void
gnome_font_selection_fill_families (GnomeFontSelection * fontsel)
{
	GList * families, * l;

	families = gnome_font_family_list ();
	g_return_if_fail (families != NULL);

	gtk_clist_freeze ((GtkCList *) fontsel->family);
	gtk_clist_clear ((GtkCList *) fontsel->family);

	for (l = families; l != NULL; l = l->next) {
		gtk_clist_append ((GtkCList *) fontsel->family, (gchar **) &l->data);
	}

	gtk_clist_thaw ((GtkCList *) fontsel->family);

	gnome_font_family_list_free (families);
}

static void
gnome_font_selection_fill_styles (GnomeFontSelection * fontsel)
{
	GList * styles, * l;

	gtk_clist_freeze ((GtkCList *) fontsel->style);
	gtk_clist_clear ((GtkCList *) fontsel->style);

	if (fontsel->selectedfamily) {
		styles = gnome_font_family_style_list (fontsel->selectedfamily);
		for (l = styles; l != NULL; l = l->next) {
			gtk_clist_append ((GtkCList *) fontsel->style, (gchar **) &l->data);
		}
		gnome_font_family_style_list_free (styles);
	}

	gtk_clist_thaw ((GtkCList *) fontsel->style);

	/* Select first font in style list */

	gtk_clist_select_row ((GtkCList *) fontsel->style, 0, 0);
}


/*****************************************************************************
 * These functions are the main public interface for getting/setting the font.
 *****************************************************************************/

gdouble
gnome_font_selection_get_size (GnomeFontSelection * fontsel)
{
	return fontsel->selectedsize;
}

GnomeFont*
gnome_font_selection_get_font (GnomeFontSelection * fontsel)
{
	g_return_val_if_fail (fontsel != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_SELECTION (fontsel), NULL);

	if (!fontsel->selectedface) return NULL;

	return gnome_font_face_get_font_default (fontsel->selectedface, fontsel->selectedsize);
}

void
gnome_font_selection_set_font (GnomeFontSelection * fontsel, GnomeFont * font)
{
	const GnomeFontFace * face;
	const gchar * familyname, * stylename;
	gdouble size;
	gchar b[32];
	gint rows, row;

	g_return_if_fail (fontsel != NULL);
	g_return_if_fail (GNOME_IS_FONT_SELECTION (fontsel));
	g_return_if_fail (font != NULL);
	g_return_if_fail (GNOME_IS_FONT (font));

	face = gnome_font_get_face (font);
	familyname = gnome_font_face_get_family_name (face);
	stylename = gnome_font_face_get_species_name (face);
	size = gnome_font_get_size (font);

	rows = ((GtkCList *) fontsel->family)->rows;
	for (row = 0; row < rows; row++) {
		gchar * text;
		gtk_clist_get_text ((GtkCList *) fontsel->family, row, 0, &text);
		if (strcmp (text, familyname) == 0) break;
	}
	gtk_clist_select_row ((GtkCList *) fontsel->family, row, 0);

	rows = ((GtkCList *) fontsel->style)->rows;
	for (row = 0; row < rows; row++) {
		gchar * text;
		gtk_clist_get_text ((GtkCList *) fontsel->style, row, 0, &text);
		if (strcmp (text, stylename) == 0) break;
	}
	gtk_clist_select_row ((GtkCList *) fontsel->style, row, 0);

	g_snprintf (b, 32, "%2.1f", size);
	b[31] = '\0';
	gtk_entry_set_text ((GtkEntry *) ((GtkCombo *) fontsel->size)->entry, b);
	fontsel->selectedsize = size;
}

void
gnome_font_selection_bind_editable_enters (GnomeFontSelection * gfs, GnomeDialog * dialog)
{
/* fixme: implement */
}

void
gnome_font_selection_bind_accel_group (GnomeFontSelection * gfs, GtkWindow * window)
{
/* fixme: implement */
}

/********************************************
 * GnomeFontPreview
 ********************************************/

struct _GnomeFontPreview
{
	GnomeCanvas canvas;

	GtkObject * phrase;

	gchar * text;
	GnomeFont * font;
	guint32 color;
};


struct _GnomeFontPreviewClass
{
	GnomeCanvasClass parent_class;
};

static void gnome_font_preview_class_init (GnomeFontPreviewClass *klass);
static void gnome_font_preview_init (GnomeFontPreview * preview);
static void gnome_font_preview_destroy (GtkObject *object);
static void gnome_font_preview_update (GnomeFontPreview * preview);

static GnomeCanvasClass *gfp_parent_class = NULL;

GtkType
gnome_font_preview_get_type()
{
	static GtkType font_preview_type = 0;
	if (!font_preview_type) {
		static const GtkTypeInfo gfp_type_info = {
			"GnomeFontPreview",
			sizeof (GnomeFontPreview),
			sizeof (GnomeFontPreviewClass),
			(GtkClassInitFunc) gnome_font_preview_class_init,
			(GtkObjectInitFunc) gnome_font_preview_init,
			NULL, NULL,
			(GtkClassInitFunc) NULL,
		};
		font_preview_type = gtk_type_unique (GNOME_TYPE_CANVAS, &gfp_type_info);
	}
	return font_preview_type;
}

static void
gnome_font_preview_class_init (GnomeFontPreviewClass *klass)
{
	GtkObjectClass *object_class;
  
	object_class = (GtkObjectClass *) klass;
  
	gfp_parent_class = gtk_type_class (GNOME_TYPE_CANVAS);
  
	object_class->destroy = gnome_font_preview_destroy;
}

static void
gnome_font_preview_init (GnomeFontPreview * preview)
{
	GtkWidget * w;
	GtkStyle * style;
	GnomeCanvasItem * phrase;

	w = (GtkWidget *) preview;

	preview->text = NULL;
	preview->font = NULL;
	preview->color = 0x000000ff;

	style = gtk_style_copy (w->style);
	style->bg[GTK_STATE_NORMAL] = style->white;
	gtk_widget_set_style (w, style);

	gtk_widget_set_usize (w, 64, MIN_PREVIEW_HEIGHT);

	phrase = gnome_canvas_item_new (gnome_canvas_root ((GnomeCanvas *) preview),
					GNOME_TYPE_CANVAS_HACKTEXT,
					"fill_color_rgba", preview->color,
					NULL);

	preview->phrase = (GtkObject *) phrase;
}

static void
gnome_font_preview_destroy (GtkObject *object)
{
	GnomeFontPreview *preview;
  
	preview = (GnomeFontPreview *) object;

	if (preview->text) {
		g_free (preview->text);
		preview->text = NULL;
	}

	if (preview->font) {
		gnome_font_unref (preview->font);
		preview->font = NULL;
	}

	preview->phrase = NULL;

  	if (GTK_OBJECT_CLASS (gfp_parent_class)->destroy)
		(* GTK_OBJECT_CLASS (gfp_parent_class)->destroy) (object);
}

GtkWidget *
gnome_font_preview_new (void)
{
	GnomeFontPreview * preview;
  
	preview = gtk_type_new (GNOME_TYPE_FONT_PREVIEW);

	return GTK_WIDGET (preview);
}

void
gnome_font_preview_set_phrase (GnomeFontPreview * preview, const gchar *phrase)
{
	g_return_if_fail (preview != NULL);
	g_return_if_fail (GNOME_IS_FONT_PREVIEW (preview));

	if (preview->text) g_free (preview->text);

	if (phrase) {
		preview->text = g_strdup (phrase);
	} else {
		preview->text = NULL;
	}

	gnome_font_preview_update (preview);
}


void
gnome_font_preview_set_font (GnomeFontPreview * preview, GnomeFont * font)
{
	g_return_if_fail (preview != NULL);
	g_return_if_fail (GNOME_IS_FONT_PREVIEW (preview));
	g_return_if_fail (font != NULL);
	g_return_if_fail (GNOME_IS_FONT (font));

	gnome_font_ref (font);

	if (preview->font) gnome_font_unref (preview->font);

	preview->font = font;

	gnome_font_preview_update (preview);
}

void
gnome_font_preview_set_color (GnomeFontPreview * preview, guint32 color)
{
	g_return_if_fail (preview != NULL);
	g_return_if_fail (GNOME_IS_FONT_PREVIEW (preview));

	preview->color = color;

	gnome_font_preview_update (preview);
}

void
gnome_font_preview_bind_editable_enters (GnomeFontPreview * gfp, GnomeDialog * dialog)
{
}

void
gnome_font_preview_bind_accel_group (GnomeFontPreview * gfp, GtkWindow * window)
{
}

static void
gnome_font_preview_update (GnomeFontPreview * preview)
{
	const gchar * sample;
	gdouble ascender, descender, width;

	if (!preview->font) return;

	if (preview->text) {
		sample = preview->text;
	} else {
		sample = gnome_font_face_get_sample (gnome_font_get_face (preview->font));
	}

	ascender = gnome_font_get_ascender (preview->font);
	descender = gnome_font_get_descender (preview->font);
	width = gnome_font_get_width_string (preview->font, sample);

	gnome_canvas_set_scroll_region (GNOME_CANVAS (preview),
					-16.0, -ascender, width + 16.0, descender);

	gnome_canvas_item_set ((GnomeCanvasItem *) preview->phrase,
			       "font", preview->font,
			       "text", sample,
			       "fill_color_rgba", preview->color,
			       NULL);
}

/*****************************************************************************
 * GtkFontSelectionDialog
 *****************************************************************************/

struct _GnomeFontSelectionDialog {
	GnomeDialog dialog;

	GtkWidget *fontsel;
	GtkWidget *preview;
};

struct _GnomeFontSelectionDialogClass {
	GnomeDialogClass parent_class;
};

static void gnome_font_selection_dialog_class_init (GnomeFontSelectionDialogClass *klass);
static void gnome_font_selection_dialog_init (GnomeFontSelectionDialog *fontseldiag);

static void gfsd_update_preview (GnomeFontSelection * fs, GnomeFont * font, GnomeFontSelectionDialog * gfsd);

static GnomeDialogClass *gfsd_parent_class = NULL;

guint
gnome_font_selection_dialog_get_type (void)
{
  static guint font_selection_dialog_type = 0;
  
  if (!font_selection_dialog_type)
    {
      GtkTypeInfo fontsel_diag_info =
      {
	"GnomeFontSelectionDialog",
	sizeof (GnomeFontSelectionDialog),
	sizeof (GnomeFontSelectionDialogClass),
	(GtkClassInitFunc) gnome_font_selection_dialog_class_init,
	(GtkObjectInitFunc) gnome_font_selection_dialog_init,
	/* reserved_1 */ NULL,
	/* reserved_2 */ NULL,
        (GtkClassInitFunc) NULL,
      };
      
      font_selection_dialog_type = gtk_type_unique (gnome_dialog_get_type(), &fontsel_diag_info);
    }
  
  return font_selection_dialog_type;
}

static void
gnome_font_selection_dialog_class_init (GnomeFontSelectionDialogClass *klass)
{
  GtkObjectClass *object_class;
  
  object_class = (GtkObjectClass*) klass;
  
  gfsd_parent_class = gtk_type_class (gnome_dialog_get_type());
}

static void
gnome_font_selection_dialog_init (GnomeFontSelectionDialog *fontseldiag)
{
	GnomeFont * font;

	gtk_window_set_default_size (GTK_WINDOW (fontseldiag), 500, 300);
	
	gnome_dialog_append_button (GNOME_DIALOG (fontseldiag), GNOME_STOCK_BUTTON_OK);
	gnome_dialog_append_button (GNOME_DIALOG (fontseldiag), GNOME_STOCK_BUTTON_CANCEL);

	gnome_dialog_set_default (GNOME_DIALOG (fontseldiag), 0);

	gtk_container_set_border_width (GTK_CONTAINER (fontseldiag), 4);
	gtk_window_set_policy (GTK_WINDOW (fontseldiag), FALSE, TRUE, TRUE);

	fontseldiag->fontsel = gnome_font_selection_new ();
	gtk_widget_show (fontseldiag->fontsel);
	gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (fontseldiag)->vbox), fontseldiag->fontsel, TRUE, TRUE, 0);

	fontseldiag->preview = gnome_font_preview_new ();
	gtk_widget_show (fontseldiag->preview);
	gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (fontseldiag)->vbox), fontseldiag->preview, TRUE, TRUE, 0);

	font = gnome_font_selection_get_font ((GnomeFontSelection *) fontseldiag->fontsel);
	gnome_font_preview_set_font ((GnomeFontPreview *) fontseldiag->preview, font);

	gtk_signal_connect (GTK_OBJECT (fontseldiag->fontsel), "font_set",
			    GTK_SIGNAL_FUNC (gfsd_update_preview), fontseldiag);
}

GtkWidget*
gnome_font_selection_dialog_new	(const gchar	  *title)
{
	GnomeFontSelectionDialog *fontseldiag;
  
	fontseldiag = gtk_type_new (GNOME_TYPE_FONT_SELECTION_DIALOG);
	gtk_window_set_title (GTK_WINDOW (fontseldiag), title ? title : _("Font Selection"));

	return GTK_WIDGET (fontseldiag);
}

GtkWidget *
gnome_font_selection_dialog_get_fontsel (GnomeFontSelectionDialog *gfsd)
{
	g_return_val_if_fail (gfsd != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_SELECTION_DIALOG (gfsd), NULL);

	return gfsd->fontsel;
}

GtkWidget *
gnome_font_selection_dialog_get_preview (GnomeFontSelectionDialog *gfsd)
{
	g_return_val_if_fail (gfsd != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_FONT_SELECTION_DIALOG (gfsd), NULL);

	return gfsd->preview;
}

static void
gfsd_update_preview (GnomeFontSelection * fs, GnomeFont * font, GnomeFontSelectionDialog * gfsd)
{
	gnome_font_preview_set_font ((GnomeFontPreview *) gfsd->preview, font);
}

