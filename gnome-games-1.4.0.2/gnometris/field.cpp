/*
 * written by J. Marcin Gorycki <marcin.gorycki@intel.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * For more details see the file COPYING.
 */

#include "field.h"
#include "blocks.h"

Field::	Field()
{
	bg = 0;
	gtk_widget_push_visual (gdk_imlib_get_visual ());
	gtk_widget_push_colormap (gdk_imlib_get_colormap ());
	w = gnome_canvas_new();
	gtk_widget_pop_colormap ();
	gtk_widget_pop_visual ();
}

void 
Field::show()
{
	gtk_widget_realize(w);

	gtk_widget_show(w);
	updateSize(0);
}

void
Field::updateSize(GdkImlibImage * bgImage)
{
	gtk_widget_set_usize(w, COLUMNS * BLOCK_SIZE, LINES * BLOCK_SIZE);
	gnome_canvas_set_scroll_region(GNOME_CANVAS(w), 0.0, 0.0, COLUMNS * BLOCK_SIZE, LINES * BLOCK_SIZE);

	if (bg)
		gtk_object_destroy(GTK_OBJECT(bg));
	
  	if (bgImage)
  		bg = gnome_canvas_item_new(
  			gnome_canvas_root(GNOME_CANVAS(w)),
  			gnome_canvas_image_get_type(),
  			"image", bgImage,
  			"x", (double) 0,
  			"y", (double) 0,
  			"width", (double) COLUMNS * BLOCK_SIZE,
  			"height", (double) LINES * BLOCK_SIZE,
  			"anchor", GTK_ANCHOR_NW,
  			0);
		else
			bg = gnome_canvas_item_new(
  			gnome_canvas_root(GNOME_CANVAS(w)),
				gnome_canvas_rect_get_type(),
  			"x1", (double) 0,
  			"y1", (double) 0,
  			"x2", (double) COLUMNS * BLOCK_SIZE,
  			"y2", (double) LINES * BLOCK_SIZE,
				"fill_color", "black",
				"outline_color", "black",
				"width_units", 1.0,
  			0);
}
