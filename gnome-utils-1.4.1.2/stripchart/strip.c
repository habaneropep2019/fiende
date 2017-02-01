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

#include "strip.h"

static void
strip_redraw(Strip *strip)
{
  GSList *list;
  GtkWidget *widget = GTK_WIDGET(strip);
  gint width = widget->allocation.width;
  gint height = widget->allocation.height;

  gdk_draw_rectangle(widget->window,
    widget->style->bg_gc[GTK_WIDGET_STATE(widget)], TRUE,
    0,0, width, height);

  for (list = CHART(strip)->param; list != NULL; list = g_slist_next(list))
    {
      gint i, x, y0 = 0, points;
      ChartDatum *datum = (ChartDatum *)list->data;
      gint h = datum->newest;
      ChartPlotStyle plot = datum->plot_style;
      ChartScaleStyle scale = datum->scale_style;

      if (datum->colors == 0)
	chart_assign_color(CHART(strip), datum);

      if (plot == chart_plot_indicator)
	continue;

      x = width - datum->idle - 1;
      points = datum->history_count;
      for (i = 0; i < points && 0 <= x; i++)
	{
	  gint y = val2gdk(datum->history[h], datum->adj, height, scale);
	  switch (plot)
	    {
	    default:
	    case chart_plot_point:
	      gdk_draw_point(widget->window, datum->gdk_gc[0], x, y);
	      break;
	    case chart_plot_line:
	      if (i)
		gdk_draw_line(widget->window, datum->gdk_gc[0], x, y, x+1, y0);
	      y0 = y;
	      break;
	    case chart_plot_solid:
	      gdk_draw_line(widget->window, datum->gdk_gc[0],
		x, y, x, val2gdk(0, datum->adj, height, scale));
	      break;
	    }
	  x--;
	  if (--h < 0)
	    h = datum->history_size - 1;
	}
    }
}

static void
strip_update_by_shifting(Strip *strip)
{
  GSList *list;
  GtkWidget *widget = GTK_WIDGET(strip);
  gint width = widget->allocation.width;
  gint height = widget->allocation.height;

  gdk_window_copy_area(widget->window,
    widget->style->fg_gc[GTK_WIDGET_STATE(widget)],
    0,0,widget->window, 1,0, width-1,height);

  gdk_draw_rectangle(widget->window,
    widget->style->bg_gc[GTK_WIDGET_STATE(widget)],
    TRUE, width-1,0, 1,height);

  for (list = CHART(strip)->param; list; list = g_slist_next(list))
    {
      gint h, y, y0;
      ChartDatum *datum = (ChartDatum *)list->data;
      ChartPlotStyle plot = datum->plot_style;
      ChartScaleStyle scale = datum->scale_style;

      if (datum->history_count == 0)
	{
	  chart_assign_color(CHART(strip), datum);
	  continue;
	}

      if (plot == chart_plot_indicator)
	continue;

      h = datum->newest;
      y = val2gdk(datum->history[h], datum->adj, height, scale);

      switch (plot)
	{
	default:
	case chart_plot_point:
	  gdk_draw_point(widget->window, datum->gdk_gc[0], width, y);
	  break;
	case chart_plot_line:
	  if (--h < 0)
	    h = datum->history_size - 1;
	  y0 = val2gdk(datum->history[h], datum->adj, height, scale);
	  gdk_draw_line(widget->window,
	    datum->gdk_gc[0], width-2,y0, width-1,y);
	  break;
	case chart_plot_solid:
	  y0 = val2gdk(0, datum->adj, height, scale);
	  gdk_draw_line(widget->window,
	    datum->gdk_gc[0], width-1,y, width-1,y0);
	  break;
	}
    }
}

static void
strip_overlay_ticks(Strip *strip)
{
  GtkWidget *widget = GTK_WIDGET(strip);
  gint width = widget->allocation.width;
  gint height = widget->allocation.height;
  gint i, x, yc = height / 2;

#ifdef DRAW_HORIZONTAL_LINE_ON_STRIP
  gdk_draw_line(widget->window, widget->style->black_gc, 0,yc, width-1,yc);
#endif
  for (i = 0, x = width - 1; x >= 0; x--, i++)
    if ((i % strip->minor_ticks) == 0)
      {
	gint dy = (i % (strip->minor_ticks * strip->major_ticks)) ? 3 : 5;
	gdk_draw_line(widget->window, widget->style->black_gc,
	  x,yc+dy, x,yc-dy);
      }
}

static void
strip_overlay_indicators(Strip *strip)
{
  GSList *list;
  GtkWidget *widget = GTK_WIDGET(strip);
  gint indicator_x = 0, indicator_y = 0, indicator_step = 10;

  for (list = CHART(strip)->param; list; list = g_slist_next(list))
    if (((ChartDatum *)list->data)->plot_style == chart_plot_indicator)
      indicator_x += indicator_step;

  for (list = CHART(strip)->param; list; list = g_slist_next(list))
    {
      ChartDatum *datum = (ChartDatum *)list->data;
      ChartPlotStyle plot = datum->plot_style;

      if (plot == chart_plot_indicator)
	{
	  gint c = datum->history[datum->newest] + 0.5;
	  indicator_x -= indicator_step;
	  if (c > 0)
	    {
	      if (c > datum->colors)
		c = datum->colors;
	      gdk_draw_rectangle(widget->window,
		widget->style->bg_gc[GTK_WIDGET_STATE(widget)], TRUE,
		indicator_x, indicator_y, 1, indicator_step);
	      gdk_draw_rectangle(
		widget->window, datum->gdk_gc[c - 1], TRUE,
		indicator_x + 1, indicator_y + 1,
		indicator_step - 1, indicator_step - 1);
	    }
	}
    }
}

static void
strip_update(Strip *strip)
{
  static int show_ticks = 1;

  if (!show_ticks)
    strip_update_by_shifting(strip);
  else
    {
      strip_redraw(strip);
      if (strip->show_ticks)
	strip_overlay_ticks(strip);
    }
  strip_overlay_indicators(strip);
  show_ticks = strip->show_ticks;
}

static gint
strip_expose(GtkWidget *widget, GdkEventExpose *event, void *nil)
{
  Strip *strip = STRIP(widget);
  strip_redraw(strip);
  if (strip->show_ticks)
    strip_overlay_ticks(strip);
  strip_overlay_indicators(strip);
  return FALSE;
}

static void
strip_configure(GtkWidget *widget, GdkEvent *event, void *nil)
{
  Strip *strip = STRIP(widget);

  strip_redraw(strip);
  if (strip->show_ticks)
    strip_overlay_ticks(strip);
}

void
strip_set_ticks(Strip *strip, gint show, gint major, gint minor)
{
  strip->show_ticks = show;
  if (major)
    strip->major_ticks = major;
  if (minor)
    strip->minor_ticks = minor;
}

void
strip_set_default_history_size(Strip *strip, gint size)
{
  CHART(strip)->default_history_size = size;
}

static void
strip_init(Chart *chart)
{
  chart->default_history_size = 100;
  strip_set_ticks(STRIP(chart), FALSE, 5, 12);
}

guint
strip_get_type(void)
{
  static guint gc_type = 0;
  if (!gc_type)
    {
      GtkTypeInfo gc_info =
      {
	"Strip",
	sizeof(Strip),
	sizeof(StripClass),
	(GtkClassInitFunc)NULL,
	(GtkObjectInitFunc)strip_init,
	(GtkArgSetFunc)NULL,
	(GtkArgGetFunc)NULL
      };
      gc_type = gtk_type_unique(chart_get_type(), &gc_info);
    }
  return gc_type;
}

GtkWidget *
strip_new(void)
{
  Strip *strip = STRIP(gtk_type_new(strip_get_type()));

  gtk_signal_connect(GTK_OBJECT(strip),
    "expose_event", (GtkSignalFunc)strip_expose, NULL);
  gtk_signal_connect(GTK_OBJECT(strip),
    "configure_event", (GtkSignalFunc)strip_configure, NULL);
  gtk_signal_connect(GTK_OBJECT(strip),
    "chart_post_update", (GtkSignalFunc)strip_update, NULL);

  return GTK_WIDGET(strip);
}
