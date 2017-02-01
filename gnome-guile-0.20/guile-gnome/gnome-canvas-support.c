/* -*- mode: c; c-basic-offset: 8 -*- */

/*
 *
 *     Author: Ariel Rios <ariel@arcavia.com>
 *	   Copyright Ariel Rios 2000
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
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307 USA
 */
 
#include <libgnomeui/gnome-canvas.h>

/*
  These functions are created since they are not implemented
  by libgnomeui. Hopefully thse functions will be included
  there
*/

void
gnome_canvas_item_scale_scm (GnomeCanvasItem *item, double sx, double sy)
{ 
   double scale [6];
  
  g_return_if_fail (item != NULL);
  g_return_if_fail (GNOME_IS_CANVAS_ITEM (item));
  
  art_affine_scale (scale, sx, sy);

  gnome_canvas_item_affine_relative (item, scale);
}

void
gnome_canvas_item_rotate_scm (GnomeCanvasItem *item, double angle)
{
  double rotate [6];
  
  g_return_if_fail (item != NULL);
  g_return_if_fail (GNOME_IS_CANVAS_ITEM (item));
  
  art_affine_rotate (rotate, angle);

  gnome_canvas_item_affine_relative (item, rotate);
}
