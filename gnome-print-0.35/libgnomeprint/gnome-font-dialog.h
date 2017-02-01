/*
 * font-dialog.h - A font selection widget and dialog.
 *
 * Authors:
 *   Chris Lahey <clahey@helixcode.com>
 *   Lauris Kaplinski <lauris@helixcode.com>
 *
 * Copyright (C) 1999-2000 Helix Code, Inc.
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

#ifndef _GNOME_FONT_SELECTION_H_
#define _GNOME_FONT_SELECTION_H_

#include <libgnomeui/gnome-dialog.h>
#include <libgnomeprint/gnome-font.h>

BEGIN_GNOME_DECLS

#define GNOME_TYPE_FONT_SELECTION	     (gnome_font_selection_get_type ())
#define GNOME_FONT_SELECTION(obj)	     (GTK_CHECK_CAST ((obj), GNOME_TYPE_FONT_SELECTION, GnomeFontSelection))
#define GNOME_FONT_SELECTION_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), GNOME_TYPE_FONT_SELECTION, GnomeFontSelectionClass))
#define GNOME_IS_FONT_SELECTION(obj)	     (GTK_CHECK_TYPE ((obj), GNOME_TYPE_FONT_SELECTION))
#define GNOME_IS_FONT_SELECTION_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GNOME_TYPE_FONT_SELECTION))

#define GNOME_TYPE_FONT_PREVIEW	             (gnome_font_preview_get_type ())
#define GNOME_FONT_PREVIEW(obj)	             (GTK_CHECK_CAST ((obj), GNOME_TYPE_FONT_PREVIEW, GnomeFontPreview))
#define GNOME_FONT_PREVIEW_CLASS(klass)      (GTK_CHECK_CLASS_CAST ((klass), GNOME_TYPE_FONT_PREVIEW, GnomeFontPreviewClass))
#define GNOME_IS_FONT_PREVIEW(obj)	     (GTK_CHECK_TYPE ((obj), GNOME_TYPE_FONT_PREVIEW))
#define GNOME_IS_FONT_PREVIEW_CLASS(klass)   (GTK_CHECK_CLASS_TYPE ((klass), GNOME_TYPE_FONT_PREVIEW))

#define GNOME_TYPE_FONT_SELECTION_DIALOG     (gnome_font_selection_dialog_get_type ())
#define GNOME_FONT_SELECTION_DIALOG(obj)     (GTK_CHECK_CAST ((obj), GNOME_TYPE_FONT_SELECTION_DIALOG, GnomeFontSelectionDialog))
#define GNOME_FONT_SELECTION_DIALOG_CLASS(klass) (GTK_CHECK_CLASS_CAST ((klass), GNOME_TYPE_FONT_SELECTION_DIALOG, GnomeFontSelectionDialogClass))
#define GNOME_IS_FONT_SELECTION_DIALOG(obj)  (GTK_CHECK_TYPE ((obj), GNOME_TYPE_FONT_SELECTION_DIALOG))
#define GNOME_IS_FONT_SELECTION_DIALOG_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GNOME_TYPE_FONT_SELECTION_DIALOG))

typedef struct _GnomeFontSelection	     GnomeFontSelection;
typedef struct _GnomeFontSelectionClass	     GnomeFontSelectionClass;

typedef struct _GnomeFontPreview	     GnomeFontPreview;
typedef struct _GnomeFontPreviewClass	     GnomeFontPreviewClass;

typedef struct _GnomeFontSelectionDialog       GnomeFontSelectionDialog;
typedef struct _GnomeFontSelectionDialogClass  GnomeFontSelectionDialogClass;

/*
 * INTERNALS ARE PRIVATE UNTIL THEY ARE STABILIZED
 *
 * We implement single signal at moment:
 *
 * void (* font_set) (GnomeFontSelection * selection, GnomeFont * font);
 *
 * You have to ref font in handler, if you want to guarantee it's persistence
 *
 */

/*****************************************************************************
 * GtkFontSelection functions.
 *   see the comments in the GtkFontSelectionDialog functions.
 *****************************************************************************/

GtkType	gnome_font_selection_get_type (void);

GtkWidget * gnome_font_selection_new (void);

/*
 * Return selected font size
 */

gdouble gnome_font_selection_get_size (GnomeFontSelection * fontsel);

/*
 * Returns GnomeFont constructed from current settings
 *
 * You need to unref that
 */

GnomeFont * gnome_font_selection_get_font (GnomeFontSelection * fontsel);

/*
 * Returns GnomeFontFace constructed from current settings
 *
 * You need to unref that
 */

GnomeFontFace * gnome_font_selection_get_face (GnomeFontSelection * fontsel);

/*
 * Sets current font
 *
 * You can unref font instantly
 */

void gnome_font_selection_set_font (GnomeFontSelection *fontsel, GnomeFont *font);

/*
 * Binds editable enter keystrokes to parent GnomeDialog
 */

void gnome_font_selection_bind_editable_enters (GnomeFontSelection * gfs, GnomeDialog * dialog);

/*
 * Binds accel group to parent window
 */

void gnome_font_selection_bind_accel_group (GnomeFontSelection * gfs, GtkWindow * window);

/*
 * GnomeFontPreview
 */

GtkType gnome_font_preview_get_type (void);

GtkWidget * gnome_font_preview_new (void);

/*
 * Sets UTF-8 demonstration phrase (NULL means font's default one)
 */

void gnome_font_preview_set_phrase (GnomeFontPreview * preview, const gchar *phrase);

void gnome_font_preview_set_font (GnomeFontPreview * preview, GnomeFont * font);
void gnome_font_preview_set_color (GnomeFontPreview * preview, guint32 color);

void gnome_font_preview_bind_editable_enters (GnomeFontPreview * gfp, GnomeDialog * dialog);
void gnome_font_preview_bind_accel_group (GnomeFontPreview * gfp, GtkWindow * window);

/*****************************************************************************
 * GnomeFontSelectionDialog functions.
 *   most of these functions simply call the corresponding function in the
 *   GnomeFontSelection.
 *****************************************************************************/

GtkType gnome_font_selection_dialog_get_type (void);

GtkWidget *gnome_font_selection_dialog_new (const gchar *title);

GtkWidget *gnome_font_selection_dialog_get_fontsel (GnomeFontSelectionDialog *gfsd);

GtkWidget *gnome_font_selection_dialog_get_preview (GnomeFontSelectionDialog *gfsd);

END_GNOME_DECLS

#endif /* __GNOME_FONTSEL_H__ */







