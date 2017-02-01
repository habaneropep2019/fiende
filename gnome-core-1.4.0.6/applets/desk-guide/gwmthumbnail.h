/* GwmThumbNail - DeskGuide ThumbNail maintenance
 * Copyright (C) 2000 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#ifndef __GWM_THUMB_NAIL_H__
#define __GWM_THUMB_NAIL_H__

#include	<gdk/gdk.h>
#include	<gdk-pixbuf/gdk-pixbuf.h>

#ifdef __cplusplus
extern "C" {
#pragma }
#endif /* __cplusplus */


/* --- typedefs & structures --- */
typedef struct _GwmThumbNail GwmThumbNail;
typedef void (*GwmThumbNailDestroy) (gpointer      user_data,
				     GwmThumbNail *thumb_nail);
struct _GwmThumbNail
{
  GdkPixbuf *pixbuf;
  gint       width;
  gint	     height;
  gint	     thumb_row;	/* incremental mark */
  guint      default_color;
  gpointer   size_list;
  gpointer   user_data;
  GwmThumbNailDestroy dtor;
};


/* --- fucntions --- */
void		gwm_thumb_nails_set_active	(gboolean	 thumb_nails_enabled);
GwmThumbNail*	gwm_thumb_nail_new		(guint		 default_color,
						 guint		 width,
						 guint		 height,
						 glong		 grow_request_id,
						 gpointer	 user_data,
						 GwmThumbNailDestroy dtor);
void		gwm_thumb_nail_destroy		(GwmThumbNail	*nail);
void		gwm_thumb_nail_grow		(GwmThumbNail	*nail,
						 guint		 width,
						 guint		 height,
						 glong		 request_id);
void		gwm_thumb_nail_ungrow		(GwmThumbNail	*nail,
						 glong		 request_id);
gboolean	gwm_thumb_nail_update_drawable	(GwmThumbNail	*nail,
						 GdkDrawable    *drawable,
						 gint	         drawable_x,
						 gint	         drawable_y);
void		gwm_thumb_nail_flag_reload	(GwmThumbNail	*nail);


#ifdef __cplusplus
#pragma {
}
#endif /* __cplusplus */

#endif /* __GWM_THUMB_NAIL_H__ */
