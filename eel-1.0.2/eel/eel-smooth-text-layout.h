/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-smooth-text-layout.h - A GtkObject subclass for dealing with smooth text.

   Copyright (C) 2000 Eazel, Inc.

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

#ifndef EEL_SMOOTH_TEXT_LAYOUT_H
#define EEL_SMOOTH_TEXT_LAYOUT_H

#include <gtk/gtkobject.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <libgnome/gnome-defs.h>
#include <eel/eel-art-extensions.h>
#include <eel/eel-scalable-font.h>

BEGIN_GNOME_DECLS

#define EEL_TYPE_SMOOTH_TEXT_LAYOUT		(eel_smooth_text_layout_get_type ())
#define EEL_SMOOTH_TEXT_LAYOUT(obj)		(GTK_CHECK_CAST ((obj), EEL_TYPE_SMOOTH_TEXT_LAYOUT, EelSmoothTextLayout))
#define EEL_SMOOTH_TEXT_LAYOUT_CLASS(klass)	(GTK_CHECK_CLASS_CAST ((klass), EEL_TYPE_SMOOTH_TEXT_LAYOUT, EelSmoothTextLayoutClass))
#define EEL_IS_SMOOTH_TEXT_LAYOUT(obj)		(GTK_CHECK_TYPE ((obj), EEL_TYPE_SMOOTH_TEXT_LAYOUT))
#define EEL_IS_SMOOTH_TEXT_LAYOUT_CLASS(klass)	(GTK_CHECK_CLASS_TYPE ((klass), EEL_TYPE_SMOOTH_TEXT_LAYOUT))

typedef struct EelSmoothTextLayout        EelSmoothTextLayout;
typedef struct EelSmoothTextLayoutClass   EelSmoothTextLayoutClass;
typedef struct EelSmoothTextLayoutDetails EelSmoothTextLayoutDetails;

struct EelSmoothTextLayout
{
	/* Superclass */
	GtkObject object;

	/* Private things */
	EelSmoothTextLayoutDetails *details;
};

struct EelSmoothTextLayoutClass
{
	GtkObjectClass parent_class;
};

GtkType                      eel_smooth_text_layout_get_type                  (void);
EelSmoothTextLayout  *       eel_smooth_text_layout_new                       (const char                *text,
									       int                        text_length,
									       EelScalableFont           *font,
									       int                        font_size,
									       gboolean                   wrap);
EelDimensions                eel_smooth_text_layout_get_dimensions            (const EelSmoothTextLayout *smooth_text_layout);
void                         eel_smooth_text_layout_set_line_wrap_width       (EelSmoothTextLayout       *smooth_text_layout,
									       int                        line_wrap_width);
int                          eel_smooth_text_layout_get_width                 (const EelSmoothTextLayout *smooth_text_layout);
int                          eel_smooth_text_layout_get_height                (const EelSmoothTextLayout *smooth_text_layout);
void                         eel_smooth_text_layout_set_wrap                  (EelSmoothTextLayout       *smooth_text_layout,
									       gboolean                   wrap);
gboolean                     eel_smooth_text_layout_get_wrap                  (const EelSmoothTextLayout *smooth_text_layout);
void                         eel_smooth_text_layout_set_line_break_characters (EelSmoothTextLayout       *smooth_text_layout,
									       const char                *line_break_characters);
char *                       eel_smooth_text_layout_get_line_break_characters (const EelSmoothTextLayout *smooth_text_layout);
void                         eel_smooth_text_layout_set_font                  (EelSmoothTextLayout       *smooth_text_layout,
									       EelScalableFont           *font);
EelScalableFont             *eel_smooth_text_layout_get_font                  (const EelSmoothTextLayout *smooth_text_layout);
void                         eel_smooth_text_layout_set_font_size             (EelSmoothTextLayout       *smooth_text_layout,
									       int                        font_size);
int                          eel_smooth_text_layout_get_font_size             (const EelSmoothTextLayout *smooth_text_layout);
void                         eel_smooth_text_layout_set_line_spacing          (EelSmoothTextLayout       *smooth_text_layout,
									       int                        line_spacing);
int                          eel_smooth_text_layout_get_line_spacing          (const EelSmoothTextLayout *smooth_text_layout);
void                         eel_smooth_text_layout_set_empty_line_height     (EelSmoothTextLayout       *smooth_text_layout,
									       int                        empty_line_height);
int                          eel_smooth_text_layout_get_empty_line_height     (const EelSmoothTextLayout *smooth_text_layout);
void                         eel_smooth_text_layout_draw_to_pixbuf            (const EelSmoothTextLayout *smooth_text_layout,
									       GdkPixbuf                 *destination_pixbuf,
									       int                        source_x,
									       int                        source_y,
									       ArtIRect                   destination_area,
									       GtkJustification           justification,
									       gboolean                   underlined,
									       guint32                    color,
									       int                        opacity);
void                         eel_smooth_text_layout_draw_to_pixbuf_shadow     (const EelSmoothTextLayout *smooth_text_layout,
									       GdkPixbuf                 *destination_pixbuf,
									       int                        source_x,
									       int                        source_y,
									       ArtIRect                   destination_area,
									       int                        shadow_offset,
									       GtkJustification           justification,
									       gboolean                   underlined,
									       guint32                    color,
									       guint32                    shadow_color,
									       int                        opacity);

#if !defined (EEL_OMIT_SELF_CHECK)
gboolean                     eel_smooth_text_layout_compare                   (EelSmoothTextLayout       *x,
									       EelSmoothTextLayout       *y);
#endif /* EEL_OMIT_SELF_CHECK */

END_GNOME_DECLS

#endif /* EEL_SMOOTH_TEXT_LAYOUT_H */


