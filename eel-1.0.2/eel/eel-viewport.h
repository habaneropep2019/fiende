/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-viewport.h - A subclass of GtkViewport with non broken drawing.

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

#ifndef EEL_VIEWPORT_H
#define EEL_VIEWPORT_H

#include <gtk/gtkviewport.h>
#include <libgnome/gnome-defs.h>
#include <eel/eel-smooth-widget.h>
#include <gtk/gtkscrolledwindow.h>

BEGIN_GNOME_DECLS

#define EEL_TYPE_VIEWPORT            (eel_viewport_get_type ())
#define EEL_VIEWPORT(obj)            (GTK_CHECK_CAST ((obj), EEL_TYPE_VIEWPORT, EelViewport))
#define EEL_VIEWPORT_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), EEL_TYPE_VIEWPORT, EelViewportClass))
#define EEL_IS_VIEWPORT(obj)         (GTK_CHECK_TYPE ((obj), EEL_TYPE_VIEWPORT))
#define EEL_IS_VIEWPORT_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), EEL_TYPE_VIEWPORT))

typedef struct EelViewport	     EelViewport;
typedef struct EelViewportClass	     EelViewportClass;
typedef struct EelViewportDetails    EelViewportDetails;

struct EelViewport
{
	/* Superclass */
	GtkViewport viewport;

	/* Private things */
	EelViewportDetails *details;
};

struct EelViewportClass
{
	GtkViewportClass parent_class;
	EelSmoothWidgetSetIsSmooth set_is_smooth;
};

GtkType      eel_viewport_get_type                     (void);
GtkWidget *  eel_viewport_new                          (GtkAdjustment     *hadjustment,
							GtkAdjustment     *vadjustment);
void         eel_viewport_set_is_smooth                (EelViewport       *eel_viewport,
							gboolean           is_smooth);
gboolean     eel_viewport_get_is_smooth                (const EelViewport *eel_viewport);
void         eel_viewport_set_never_smooth             (EelViewport       *eel_viewport,
							gboolean           never_smooth);
EelArtIPoint eel_viewport_get_scroll_offset            (const EelViewport *eel_viewport);
void         eel_viewport_set_constrain_width          (EelViewport       *eel_viewport,
							gboolean           constrain_width);
gboolean     eel_viewport_get_constrain_width          (const EelViewport *eel_viewport);
void         eel_viewport_set_constrain_height         (EelViewport       *eel_viewport,
							gboolean           constrain_height);
gboolean     eel_viewport_get_constrain_height         (const EelViewport *eel_viewport);
void         eel_gtk_scrolled_window_add_with_viewport (GtkScrolledWindow *scrolled_window,
							GtkWidget         *child);

END_GNOME_DECLS

#endif /* EEL_VIEWPORT_H */
