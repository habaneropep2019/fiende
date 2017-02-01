/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-label.h - A widget to display a anti aliased text.

   Copyright (C) 1999, 2000 Eazel, Inc.

   The Gnome Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Authors: Ramiro Estrugo <ramiro@eazel.com>
*/

#ifndef EEL_LABEL_H
#define EEL_LABEL_H

#include <gtk/gtklabel.h>
#include <eel/eel-scalable-font.h>
#include <eel/eel-smooth-widget.h>

BEGIN_GNOME_DECLS

#define EEL_TYPE_LABEL            (eel_label_get_type ())
#define EEL_LABEL(obj)            (GTK_CHECK_CAST ((obj), EEL_TYPE_LABEL, EelLabel))
#define EEL_LABEL_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), EEL_TYPE_LABEL, EelLabelClass))
#define EEL_IS_LABEL(obj)         (GTK_CHECK_TYPE ((obj), EEL_TYPE_LABEL))
#define EEL_IS_LABEL_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), EEL_TYPE_LABEL))

typedef struct EelLabel		  EelLabel;
typedef struct EelLabelClass	  EelLabelClass;
typedef struct EelLabelDetails	  EelLabelDetails;

struct EelLabel
{
	/* Superclass */
	GtkLabel gtk_label;

	/* Private things */
	EelLabelDetails *details;
};

struct EelLabelClass
{
	GtkLabelClass parent_class;

	EelSmoothWidgetDrawBackground draw_background;
	EelSmoothWidgetSetIsSmooth set_is_smooth;
};

GtkType                 eel_label_get_type                       (void);
GtkWidget *             eel_label_new                            (const char              *text);
void                    eel_label_set_is_smooth                  (EelLabel                *label,
								  gboolean                 is_smooth);
gboolean                eel_label_get_is_smooth                  (const EelLabel          *label);
void                    eel_label_set_smooth_font                (EelLabel                *label,
								  EelScalableFont         *font);
EelScalableFont *       eel_label_get_smooth_font                (const EelLabel          *label);
void                    eel_label_set_smooth_font_size           (EelLabel                *label,
								  int                      font_size);
int                     eel_label_get_smooth_font_size           (const EelLabel          *label);
void                    eel_label_set_text_opacity               (EelLabel                *label,
								  int                      opacity);
int                     eel_label_get_text_opacity               (const EelLabel          *label);
void                    eel_label_set_background_mode            (EelLabel                *label,
								  EelSmoothBackgroundMode  background_mode);
EelSmoothBackgroundMode eel_label_get_background_mode            (const EelLabel          *label);
void                    eel_label_set_solid_background_color     (EelLabel                *label,
								  guint32                  solid_background_color);
guint32                 eel_label_get_solid_background_color     (const EelLabel          *label);
void                    eel_label_set_text_color                 (EelLabel                *label,
								  guint32                  color);
guint32                 eel_label_get_text_color                 (const EelLabel          *label);
void                    eel_label_set_smooth_drop_shadow_offset  (EelLabel                *label,
								  int                      offset);
int                     eel_label_get_smooth_drop_shadow_offset  (const EelLabel          *label);
void                    eel_label_set_smooth_drop_shadow_color   (EelLabel                *label,
								  guint32                  color);
guint32                 eel_label_get_smooth_drop_shadow_color   (const EelLabel          *label);
void                    eel_label_set_smooth_line_wrap_width     (EelLabel                *label,
								  int                      line_wrap_width);
int                     eel_label_get_smooth_line_wrap_width     (const EelLabel          *label);
gboolean                eel_label_set_text                       (EelLabel                *label,
								  const char              *text);
char*                   eel_label_get_text                       (const EelLabel          *label);
void                    eel_label_set_justify                    (EelLabel                *label,
								  GtkJustification         justification);
GtkJustification        eel_label_get_text_justify               (const EelLabel          *label);
void                    eel_label_set_wrap                       (EelLabel                *label,
								  gboolean                 line_wrap);
gboolean                eel_label_get_wrap                       (const EelLabel          *label);
GtkWidget *             eel_label_new_solid                      (const char              *text,
								  int                      drop_shadow_offset,
								  guint32                  drop_shadow_color,
								  guint32                  text_color,
								  float                    xalign,
								  float                    yalign,
								  int                      xpadding,
								  int                      ypadding,
								  guint32                  background_color,
								  GdkPixbuf               *tile_pixbuf);
void                    eel_label_make_bold                      (EelLabel                *label);
void                    eel_label_make_larger                    (EelLabel                *label,
								  guint                    num_sizes);
void                    eel_label_make_smaller                   (EelLabel                *label,
								  guint                    num_sizes);
void                    eel_label_set_tile_pixbuf                (EelLabel                *label,
								  GdkPixbuf               *pixbuf);
void                    eel_label_set_tile_width                 (EelLabel                *label,
								  int                      tile_width);
int                     eel_label_get_tile_width                 (const EelLabel          *label);
void                    eel_label_set_tile_height                (EelLabel                *label,
								  int                      tile_height);
int                     eel_label_get_tile_height                (const EelLabel          *label);
void                    eel_label_set_tile_pixbuf_from_file_name (EelLabel                *label,
								  const char              *tile_file_name);
GdkPixbuf*              eel_label_get_tile_pixbuf                (const EelLabel          *label);
void                    eel_label_set_tile_opacity               (EelLabel                *label,
								  int                      tile_opacity);
int                     eel_label_get_tile_opacity               (const EelLabel          *label);
void                    eel_label_set_tile_mode_vertical         (EelLabel                *label,
								  EelSmoothTileMode        horizontal_tile_mode);
EelSmoothTileMode       eel_label_get_tile_mode_vertical         (const EelLabel          *label);
void                    eel_label_set_tile_mode_horizontal       (EelLabel                *label,
								  EelSmoothTileMode        horizontal_tile_mode);
EelSmoothTileMode       eel_label_get_tile_mode_horizontal       (const EelLabel          *label);
void                    eel_label_set_never_smooth               (EelLabel                *label,
								  gboolean                 never_smooth);
void                    eel_label_set_adjust_wrap_on_resize      (EelLabel                *label,
								  gboolean                 adjust_wrap_on_resize);
gboolean                eel_label_get_adjust_wrap_on_resize      (const EelLabel          *label);

END_GNOME_DECLS

#endif /* EEL_LABEL_H */


