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

#include "preview.h"
#include "blocks.h"

Preview::	Preview()
{
	gtk_widget_push_visual(gdk_imlib_get_visual());
	gtk_widget_push_colormap(gdk_imlib_get_colormap());

	w = gtk_drawing_area_new();

	gtk_widget_pop_colormap();
	gtk_widget_pop_visual();

	gtk_widget_set_events(w, gtk_widget_get_events(w) | GDK_EXPOSURE_MASK);
	gtk_signal_connect(GTK_OBJECT(w), "event", (GtkSignalFunc)eventHandler, this);
}

void 
Preview::show()
{
	gtk_widget_realize(w);

	GdkColor c;
	c.pixel = 0;
	gdk_window_set_background(w->window, &c);

	gtk_widget_show(w);
	updateSize();
}

void
Preview::updateSize()
{
	gtk_drawing_area_size(GTK_DRAWING_AREA(w), PREVIEW_SIZE * BLOCK_SIZE, PREVIEW_SIZE * BLOCK_SIZE);
}

gint 
Preview::eventHandler(GtkWidget *widget, GdkEvent *event, void *d)
{
	switch (event->type)
	{
	case GDK_EXPOSE: 
	{
		GdkEventExpose *e = (GdkEventExpose*)event;
		((Preview*)d)->paint(&e->area);
		return TRUE;
	}
	default:
		return FALSE;
	}
}

void
Preview::paint(GdkRectangle *area)
{
	gdk_window_clear_area(w->window, 0, 0, BLOCK_SIZE * PREVIEW_SIZE, BLOCK_SIZE * PREVIEW_SIZE);

	int xoffs = (PREVIEW_SIZE - sizeTable[blocknr_next][rot_next][1]) * BLOCK_SIZE / 2;
	int yoffs = (PREVIEW_SIZE - sizeTable[blocknr_next][rot_next][0]) * BLOCK_SIZE / 2;

	xoffs -= offsetTable[blocknr_next][rot_next][1] * BLOCK_SIZE;
	yoffs -= offsetTable[blocknr_next][rot_next][0] * BLOCK_SIZE;

	if (do_preview)
	{
		if (blocknr_next != -1)
		{
			for (int x = 0; x < 4; ++x)
			{
				for (int y = 0; y < 4; ++y)
				{
					if (blockTable[blocknr_next][rot_next][x][y])	
						gdk_draw_pixmap(w->window, w->style->black_gc, 
														pix, color_next * BLOCK_SIZE, 0, 
														x * BLOCK_SIZE + xoffs, y * BLOCK_SIZE + yoffs, BLOCK_SIZE, BLOCK_SIZE);
				}
			}
		}
	}
	else
	{
		gdk_draw_line(w->window, w->style->white_gc, 0, 0, PREVIEW_SIZE * BLOCK_SIZE, PREVIEW_SIZE * BLOCK_SIZE);
		gdk_draw_line(w->window, w->style->white_gc, 0, PREVIEW_SIZE * BLOCK_SIZE, PREVIEW_SIZE * BLOCK_SIZE, 0);
	}
}










