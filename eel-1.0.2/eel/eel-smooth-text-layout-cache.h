/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-smooth-text-layout-cache.h - A GtkObject subclass for efficiently rendering smooth text.

   Copyright (C) 2001 Eazel, Inc.

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

   Authors: John Harper <jsh@eazel.com>
*/

#ifndef EEL_SMOOTH_TEXT_LAYOUT_CACHE_H
#define EEL_SMOOTH_TEXT_LAYOUT_CACHE_H

#include <gtk/gtkobject.h>
#include <libgnome/gnome-defs.h>
#include <eel/eel-smooth-text-layout.h>

BEGIN_GNOME_DECLS

#define EEL_TYPE_SMOOTH_TEXT_LAYOUT_CACHE	     (eel_smooth_text_layout_cache_get_type ())
#define EEL_SMOOTH_TEXT_LAYOUT_CACHE(obj)	     (GTK_CHECK_CAST ((obj), EEL_TYPE_SMOOTH_TEXT_LAYOUT_CACHE, EelSmoothTextLayoutCache))
#define EEL_SMOOTH_TEXT_LAYOUT_CACHE_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), EEL_TYPE_SMOOTH_TEXT_LAYOUT_CACHE, EelSmoothTextLayoutCacheClass))
#define EEL_IS_SMOOTH_TEXT_LAYOUT_CACHE(obj)	     (GTK_CHECK_TYPE ((obj), EEL_TYPE_SMOOTH_TEXT_LAYOUT_CACHE))
#define EEL_IS_SMOOTH_TEXT_LAYOUT_CACHE_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), EEL_TYPE_SMOOTH_TEXT_LAYOUT_CACHE))

typedef struct EelSmoothTextLayoutCache        EelSmoothTextLayoutCache;
typedef struct EelSmoothTextLayoutCacheClass   EelSmoothTextLayoutCacheClass;
typedef struct EelSmoothTextLayoutCacheDetails EelSmoothTextLayoutCacheDetails;

struct EelSmoothTextLayoutCache
{
	/* Superclass */
	GtkObject object;

	/* Private things */
	EelSmoothTextLayoutCacheDetails *details;
};

struct EelSmoothTextLayoutCacheClass
{
	GtkObjectClass parent_class;
};
GtkType                      eel_smooth_text_layout_cache_get_type (void);
EelSmoothTextLayoutCache    *eel_smooth_text_layout_cache_new      (void);
EelSmoothTextLayout	    *eel_smooth_text_layout_cache_render   (EelSmoothTextLayoutCache *cache,
								    const char               *text,
								    int                       text_length,
								    EelScalableFont          *font,
								    int                       font_size,
								    gboolean                  wrap,
								    int                       line_spacing,
								    int                       max_text_width);

#if !defined (EEL_OMIT_SELF_CHECK)
void			     eel_self_check_smooth_text_layout_cache (void);
#endif /* EEL_OMIT_SELF_CHECK */

END_GNOME_DECLS

#endif /* EEL_SMOOTH_TEXT_LAYOUT_CACHE_H */
