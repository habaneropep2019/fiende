/* Stripchart -- the gnome-utils stripchart plotting utility
 * Copyright (C) 2000 John Kodis <kodis@jagunet.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <stdio.h>

#include "pen.h"

guint
pen_get_type(void)
{
  static guint gc_type = 0;

  if (!gc_type)
    {
      GtkTypeInfo gc_info =
      {
	"Pen",
	sizeof(Pen),
	sizeof(PenClass),
	(GtkClassInitFunc)NULL,
	(GtkObjectInitFunc)NULL,
	(GtkArgSetFunc)NULL,
	(GtkArgGetFunc)NULL
      };
      gc_type = gtk_type_unique(chart_get_type(), &gc_info);
    }

  return gc_type;
}

static void
pen_update(Pen *pen)
{
  GSList *list;
  GtkWidget *widget = GTK_WIDGET(pen);
  gint width = widget->allocation.width;
  gint height = widget->allocation.height;

  if (widget->window == NULL)
    return;

  gdk_draw_rectangle(widget->window,
    widget->style->bg_gc[GTK_WIDGET_STATE(widget)], TRUE,
    0,0, width, height);

  for (list = CHART(pen)->param; list; list = g_slist_next(list))
    {
      gint y;
      GdkPoint tri[3];
      ChartDatum *datum = (ChartDatum *)list->data;
      ChartScaleStyle scale = datum->scale_style;

      if (datum->colors == 0)
	chart_assign_color(CHART(pen), datum);

      if (datum->plot_style == chart_plot_indicator)
	continue;

      y = val2gdk(datum->history[0], datum->adj, height, scale);
      tri[0].x = 0;     tri[0].y = y;
      tri[1].x = width; tri[1].y = y - width/2;
      tri[2].x = width; tri[2].y = y + width/2;
      gdk_draw_polygon(widget->window, datum->gdk_gc[0], TRUE, tri, 3);
    }
}

static int
pen_expose(GtkWidget *widget, GdkEventExpose *event, void *nil)
{
  pen_update(PEN(widget));
  return FALSE;
}

static void
pen_configure(GtkWidget *widget, GdkEvent *event, void *nil)
{
  pen_update(PEN(widget));
}

GtkWidget *
pen_new(void)
{
  Pen *pen = PEN(gtk_type_new(pen_get_type()));

  CHART(pen)->points_in_view = 1;

  gtk_signal_connect(GTK_OBJECT(pen),
    "expose_event", (GtkSignalFunc)pen_expose, NULL);
  gtk_signal_connect(GTK_OBJECT(pen),
    "configure_event", (GtkSignalFunc)pen_configure, NULL);
  gtk_signal_connect(GTK_OBJECT(pen),
    "chart_post_update", (GtkSignalFunc)pen_update, NULL);

  return GTK_WIDGET(pen);
}
