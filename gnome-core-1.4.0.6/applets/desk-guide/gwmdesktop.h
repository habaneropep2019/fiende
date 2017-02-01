/* GwmDesktop - desktop widget
 * Copyright (C) 1999 Tim Janik
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 *
 * this code is loosely based on the original gnomepager_applet
 * implementation of The Rasterman (Carsten Haitzler) <raster@rasterman.com>
 */
#ifndef __GWM_DESKTOP_H__
#define __GWM_DESKTOP_H__

#include        "gwmh.h"


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/* --- Gtk+ type macros --- */
#define GWM_TYPE_DESKTOP            (gwm_desktop_get_type ())
#define GWM_DESKTOP(object)         (GTK_CHECK_CAST ((object), GWM_TYPE_DESKTOP, GwmDesktop))
#define GWM_DESKTOP_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), GWM_TYPE_DESKTOP, GwmDesktopClass))
#define GWM_IS_DESKTOP(object)      (GTK_CHECK_TYPE ((object), GWM_TYPE_DESKTOP))
#define GWM_IS_DESKTOP_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GWM_TYPE_DESKTOP))
#define GWM_DESKTOP_GET_CLASS(obj)  ((GwmDesktopClass*) (((GtkObject*) (obj))->klass))


/* --- structures & typedefs --- */
typedef struct  _GwmDesktop             GwmDesktop;
typedef struct  _GwmDesktopClass        GwmDesktopClass;
struct _GwmDesktop
{
  GtkDrawingArea parent_object;
  
  guint          index;
  guint          harea, varea;
  guint          last_desktop;
  GList         *task_list;

  gfloat	 area_width;
  gfloat	 area_height;
  
  GdkBitmap     *bitmap;
  GdkPixmap     *pixmap;
  GtkTooltips   *tooltips;

  /* grab motion */
  gint16         x_spixels;
  gint16         y_spixels;
  gint16         x_origin;
  gint16         y_origin;
  gint16         x_comp;
  gint16         y_comp;
  GwmhTask      *grab_task;
};
struct _GwmDesktopClass
{
  GtkDrawingAreaClass  parent_class;

  GtkOrientation orientation;
  guint          area_size;
  guint		 thumb_timeout;
  guint          raised_grid : 1;
  guint          move_to_frame_offset : 1;

  GSList        *objects;

  gboolean      (*check_task)   (GwmDesktop     *desktop,
                                 GwmhTask       *task);
};


/* --- prototypes --- */
GtkType         gwm_desktop_get_type     (void);
GtkWidget*      gwm_desktop_new          (guint        index,
                                          GtkTooltips *tooltips);
void            gwm_desktop_set_index    (GwmDesktop  *desktop,
                                          guint        index);

void            gwm_desktop_class_config (GwmDesktopClass *klass,
                                          guint		   thumb_timeout,
                                          GtkOrientation   orientation,
                                          guint            area_size,
                                          gboolean         raised_grid,
					  gboolean	   move_to_frame_offset);
void		gwm_desktop_class_reload_thumbs (void);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GWM_DESKTOP_H__ */
