/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/* eel-canvas-rect.h - canvas rect with more-efficient update function

   Copyright (C) 2001 Ximian, Inc.
   Copyright (C) 2001 Eazel, Inc.

   The Eel Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Eel Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Authors: Chris Lahey <clahey@ximian.com>
            Darin Adler <darin@eazel.com>
*/

#ifndef EEL_CANVAS_RECT_H
#define EEL_CANVAS_RECT_H

#include <libgnomeui/gnome-canvas-rect-ellipse.h>

BEGIN_GNOME_DECLS

#define EEL_TYPE_CANVAS_RECT        (eel_canvas_rect_get_type ())
#define EEL_CANVAS_RECT(o)          (GTK_CHECK_CAST ((o), EEL_TYPE_CANVAS_RECT, EelCanvasRect))
#define EEL_CANVAS_RECT_CLASS(k)    (GTK_CHECK_CLASS_CAST((k), EEL_TYPE_CANVAS_RECT, EelCanvasRectClass))
#define EEL_IS_CANVAS_RECT(o)       (GTK_CHECK_TYPE ((o), EEL_TYPE_CANVAS_RECT))
#define EEL_IS_CANVAS_RECT_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), EEL_TYPE_CANVAS_RECT))

typedef struct EelCanvasRectDetails EelCanvasRectDetails;

typedef struct {
	GnomeCanvasRect base;
	EelCanvasRectDetails *details;
} EelCanvasRect;

typedef struct {
	GnomeCanvasRectClass parent_class;
} EelCanvasRectClass;

GtkType          eel_canvas_rect_get_type (void);
GnomeCanvasItem *eel_canvas_rect_new      (void);

END_GNOME_DECLS

#endif /* EEL_CANVAS_RECT */
