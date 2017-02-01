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

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "chart.h"

enum { PRE_UPDATE, POST_UPDATE, RESCALE, SIGNAL_COUNT };
static gint chart_signals[SIGNAL_COUNT] = { 0 };

static void 
chart_class_init(ChartClass *klass)
{
  GtkObjectClass *object_class = (GtkObjectClass *)klass;

  chart_signals[PRE_UPDATE] = 
    gtk_signal_new("chart_pre_update",
      GTK_RUN_FIRST, object_class->type,
      GTK_SIGNAL_OFFSET(ChartClass, chart_pre_update),
      gtk_signal_default_marshaller, GTK_TYPE_NONE, 0);

  chart_signals[POST_UPDATE] = 
    gtk_signal_new("chart_post_update",
      GTK_RUN_FIRST, object_class->type,
      GTK_SIGNAL_OFFSET(ChartClass, chart_post_update),
      gtk_signal_default_marshaller, GTK_TYPE_NONE, 0);

  chart_signals[RESCALE] = 
    gtk_signal_new("chart_rescale",
      GTK_RUN_FIRST, object_class->type,
      GTK_SIGNAL_OFFSET(ChartClass, chart_rescale),
      gtk_signal_default_marshaller, GTK_TYPE_NONE, 0);

  gtk_object_class_add_signals(object_class, chart_signals, SIGNAL_COUNT);
  klass->chart_rescale = NULL;
}

static void
chart_configure(GtkWidget *widget, GdkEvent *event, void *nil)
{
  CHART(widget)->points_in_view = widget->allocation.width;

  if (!CHART(widget)->colormap)
    CHART(widget)->colormap = gdk_window_get_colormap(widget->window);
}

static void
chart_object_init(Chart *chart)
{
  chart->param = NULL;
  chart->default_history_size = 1;
  chart->default_plot_style = chart_plot_line;
  chart->default_scale_style = chart_scale_linear;

  chart_set_interval(chart, 1000);

  gtk_signal_connect(GTK_OBJECT(chart),
    "configure_event", (GtkSignalFunc)chart_configure, NULL);
}

guint
chart_get_type(void)
{
  static guint gc_type = 0;

  if (!gc_type)
    {
      GtkTypeInfo gc_info =
      {
	"Chart",
	sizeof(Chart),
	sizeof(ChartClass),
	(GtkClassInitFunc)chart_class_init,
	(GtkObjectInitFunc)chart_object_init,
	(GtkArgSetFunc)NULL,
	(GtkArgGetFunc)NULL
      };
      gc_type = gtk_type_unique(gtk_drawing_area_get_type(), &gc_info);
    }
  return gc_type;
}

typedef struct
{
  gint top_index, bot_index, ranges, step;
  gdouble *range;
}
Range;

static gint
chart_rescale_by_table(ChartDatum *datum, gdouble *table, gint nels, gint step)
{
  gint j, changed = 0;
  gdouble top, bot, last_upper;
  Range *r = (Range *)datum->range_data;

  if (r == NULL)
    {
      r = g_malloc(sizeof(Range));
      r->range = table;
      r->ranges = nels;
      r->step = step;
      r->top_index = r->bot_index = r->ranges / 2;
      datum->range_data = r;
    }

  /* Put top above max and top_min, but not above top_max or end of
     table. */
  j = r->top_index;
  last_upper = datum->adj->upper;
  top = MAX(datum->max, datum->top_min);
  while (table[r->top_index] <= top
    && r->top_index + step < r->ranges
    && table[r->top_index + step] <= datum->top_max)
    {
      r->top_index += step;
      changed++;
    }

  /* Put top to lowest value above max, but above top_min and start of
     table. */
  bot = MIN(datum->max, datum->top_max);
  while (step <= r->top_index
    && bot < table[r->top_index - step]
    && datum->top_min <= table[r->top_index - step])
    {
      r->top_index -= step;
      changed++;
    }

  /* Set the new upper adjustment value.  The new value will be on of
     table[r->top_index], top_min, or top_max.  If a user-supplied
     top_max is available, we use that as the outer limit.  */
  datum->adj->upper = table[r->top_index];
  if (top > table[r->top_index] && table[r->top_index] < datum->top_max)
    datum->adj->upper = datum->top_max;
  else if (step <= r->top_index
    && bot < table[r->top_index-step]
    && table[r->top_index-step] < datum->top_min)
    {
      datum->adj->upper = datum->top_min;
      if (!changed && last_upper != datum->adj->upper)
	changed = -1;
    }

#ifdef DEBUG
  if (changed)
    printf("changed: %p=%2dx%d, %g..%g; %g(%d) -> %g(%d): %g\n",
      datum, changed, step, datum->min, datum->max,
      table[j], j, table[r->top_index], r->top_index, datum->adj->upper);
#endif

  return changed;
}

static gdouble
chart_decade_125[] =
{
  1e-30, 2e-30, 5e-30, 1e-29, 2e-29, 5e-29, 1e-28, 2e-28, 5e-28,
  1e-27, 2e-27, 5e-27, 1e-26, 2e-26, 5e-26, 1e-25, 2e-25, 5e-25,
  1e-24, 2e-24, 5e-24, 1e-23, 2e-23, 5e-23, 1e-22, 2e-22, 5e-22,
  1e-21, 2e-21, 5e-21, 1e-20, 2e-20, 5e-20, 1e-19, 2e-19, 5e-19,
  1e-18, 2e-18, 5e-18, 1e-17, 2e-17, 5e-17, 1e-16, 2e-16, 5e-16,
  1e-15, 2e-15, 5e-15, 1e-14, 2e-14, 5e-14, 1e-13, 2e-13, 5e-13,
  1e-12, 2e-12, 5e-12, 1e-11, 2e-11, 5e-11, 1e-10, 2e-10, 5e-10,
  1e-09, 2e-09, 5e-09, 1e-08, 2e-08, 5e-08, 1e-07, 2e-07, 5e-07,
  1e-06, 2e-06, 5e-06, 1e-05, 2e-05, 5e-05, 1e-04, 2e-04, 5e-04,
  1e-03, 2e-03, 5e-03, 1e-02, 2e-02, 5e-02, 1e-01, 2e-01, 5e-01,
  1e+00, 2e+00, 5e+00, 1e+01, 2e+01, 5e+01, 1e+02, 2e+02, 5e+02,
  1e+03, 2e+03, 5e+03, 1e+04, 2e+04, 5e+04, 1e+05, 2e+05, 5e+05,
  1e+06, 2e+06, 5e+06, 1e+07, 2e+07, 5e+07, 1e+08, 2e+08, 5e+08,
  1e+09, 2e+09, 5e+09, 1e+10, 2e+10, 5e+10, 1e+11, 2e+11, 5e+11,
  1e+12, 2e+12, 5e+12, 1e+13, 2e+13, 5e+13, 1e+14, 2e+14, 5e+14,
  1e+15, 2e+15, 5e+15, 1e+16, 2e+16, 5e+16, 1e+17, 2e+17, 5e+17,
  1e+18, 2e+18, 5e+18, 1e+19, 2e+19, 5e+19, 1e+20, 2e+20, 5e+20,
  1e+21, 2e+21, 5e+21, 1e+22, 2e+22, 5e+22, 1e+23, 2e+23, 5e+23,
  1e+24, 2e+24, 5e+24, 1e+25, 2e+25, 5e+25, 1e+26, 2e+26, 5e+26,
  1e+27, 2e+27, 5e+27, 1e+28, 2e+28, 5e+28, 1e+29, 2e+29, 5e+29, 1e+30, 
};

gint
chart_rescale_by_125(ChartDatum *datum)
{
  return chart_rescale_by_table(datum,
    chart_decade_125, sizeof(chart_decade_125) / sizeof(*chart_decade_125), 1);
}

gint
chart_rescale_by_decade(ChartDatum *datum)
{
  return chart_rescale_by_table(datum,
    chart_decade_125, sizeof(chart_decade_125) / sizeof(*chart_decade_125), 3);
}

void
chart_parameter_deactivate(Chart *chart, ChartDatum *param)
{
  if (param)
    param->active = FALSE;
}

static gint
chart_timer(Chart *chart)
{
  gint rescale = 0;
  GSList *list, *next;

  gtk_signal_emit_by_name(GTK_OBJECT(chart), "chart_pre_update", NULL);

  for (list = chart->param; list != NULL; list = g_slist_next(list))
    {
      gdouble val;
      ChartDatum *datum = list->data;

      if (!datum->active)
	{
	  datum->idle++;
	  if (datum->history_size <= datum->history_count + datum->idle)
	    datum->history_count--;
	  if (datum->history_count == 0)
	    {
	      #ifdef DEBUG
	      printf("timer: deleting: chart %p, param %p, datum %p\n",
		chart, chart->param, datum);
	      #endif
	      next = g_slist_next(list);
	      chart->param = g_slist_remove(chart->param, datum);
	      g_free(datum);
	      list = next;
	    }
	  continue;
	}

      val = datum->user_func(datum->user_data);

      if (datum->skip)
	{
	  datum->skip--;
	  continue;
	}

      if (datum->history_count < datum->history_size)
	datum->history_count++;
      if (++datum->newest >= datum->history_size)
	datum->newest = 0;

      datum->history[datum->newest] = val;

      if (datum->range_func)
	{
	  gint view = chart->points_in_view;
	  gint idle = datum->idle;
	  gint data = datum->history_count;
	  gint n = MIN(data, view - idle);

	  gint h = datum->newest;
	  datum->min = datum->max = datum->history[h];

	  while (n-- > 0)
	    {
	      gfloat x = datum->history[h];
	      if (x < datum->min)
		datum->min = x;
	      else if (datum->max < x)
		datum->max = x;
	      if (--h < 0)
		h = datum->history_size - 1;
	    }
	  rescale += datum->range_func(datum);
	}
    }

  if (rescale)
    gtk_signal_emit_by_name(GTK_OBJECT(chart), "chart_rescale", NULL);
  gtk_signal_emit_by_name(GTK_OBJECT(chart), "chart_post_update", NULL);

  return TRUE;
}

void
chart_set_interval(Chart *chart, guint msec)
{
  if (chart->timer)
    gtk_timeout_remove(chart->timer);
  chart->timer = gtk_timeout_add(msec, (GtkFunction)chart_timer, chart);
}

ChartDatum *
chart_parameter_add(Chart *chart,
  gdouble (*user_func)(), gpointer user_data,
  gchar *color_names, GtkAdjustment *adj, int pageno,
  gdouble bot_min, gdouble bot_max, gdouble top_min, gdouble top_max)
{
  ChartDatum *datum = g_malloc(sizeof(*datum));

  datum->chart = chart;
  datum->user_func = user_func;
  datum->user_data = user_data;
  datum->range_func = NULL;
  datum->range_data = NULL;
  datum->active = TRUE;
  datum->idle = datum->newest = datum->history_count = 0;

  if (adj == NULL)
    adj = GTK_ADJUSTMENT(gtk_adjustment_new(0,0,0,0,0,0));
  datum->adj = adj;
  datum->adj->upper = datum->top_min;
  datum->adj->lower = datum->bot_min;

  datum->top_max = top_max;
  datum->bot_max = bot_max;
  datum->top_min = top_min;
  datum->bot_min = bot_max;
#ifdef DEBUG
  printf("Initial bounds %d: %lf - %lf | %lf - %lf\n", pageno,
    datum->bot_min, datum->bot_max, datum->top_min, datum->top_max);
#endif

  datum->colors = 0;
  datum->color_names = g_strdup(color_names);
  datum->gdk_gc = NULL;
  datum->gdk_color = NULL;
  chart->param = g_slist_insert(chart->param, datum, pageno);

  datum->plot_style = chart->default_plot_style;
  datum->scale_style = chart->default_scale_style;

  datum->skip = 2;
  datum->history_size = chart->default_history_size;
  datum->history = g_malloc(datum->history_size * sizeof(*datum->history));

  return datum;
}

void
chart_set_top_max(ChartDatum *datum, double top_max)
{
  datum->top_max = top_max;
}

void
chart_set_top_min(ChartDatum *datum, double top_min)
{
  datum->top_min = top_min;
}

void
chart_set_bot_max(ChartDatum *datum, double bot_max)
{
  datum->bot_max = bot_max;
}

void
chart_set_bot_min(ChartDatum *datum, double bot_min)
{
  datum->bot_min = bot_min;
}

void
chart_set_autorange(ChartDatum *datum, int (*range_func)(), void *range_data)
{
  datum->range_func = range_func;
  datum->range_data = range_data;
}

void
chart_set_scale_style(ChartDatum *datum, ChartScaleStyle scale_style)
{
  datum->scale_style = scale_style;
}

void
chart_set_plot_style(ChartDatum *datum, ChartPlotStyle plot_style)
{
  datum->plot_style = plot_style;
}

void
chart_assign_color(Chart *chart, ChartDatum *datum)
{
  gchar *names, *color, *whitespace = " \t\r\n";
  GdkColor default_fg = GTK_WIDGET(chart)->style->fg[GTK_WIDGET_STATE(chart)];

  datum->colors = 0;
  datum->gdk_gc = NULL;
  datum->gdk_color = NULL;

  names = g_strdup(datum->color_names);
  color = strtok(names, whitespace);
  while (color != NULL)
    {
      gint cnum = datum->colors++;
      datum->gdk_color = g_realloc(datum->gdk_color,
	datum->colors * sizeof(*datum->gdk_color));
      datum->gdk_gc = g_realloc(datum->gdk_gc,
	datum->colors * sizeof(*datum->gdk_gc));

      if (!gdk_color_parse(color, &datum->gdk_color[cnum]))
	{
	  g_warning("Unknown color: (%s); Using default fg color.", color);
	  datum->gdk_color[cnum] = default_fg;
	}
      gdk_color_alloc(chart->colormap, &datum->gdk_color[cnum]);
      datum->gdk_gc[cnum] = gdk_gc_new(GTK_WIDGET(chart)->window);
      gdk_gc_set_foreground(datum->gdk_gc[cnum], &datum->gdk_color[cnum]);
      color = strtok(NULL, whitespace);
    }
  g_free(names);
}

/*
 * val2y -- scales a parameter value into a y coordinate value.
 */
gint
val2gdk(gdouble val, GtkAdjustment *adj, gint height, ChartScaleStyle scale)
{
  gdouble y, delta = adj->upper - adj->lower;

#ifndef HAVE_LOGF
#define logf(x) ((float)log(x))
#endif

  height--;
  if (scale == chart_scale_log)
    y = height - (height * logf(1 + val - adj->lower) / logf(1 + delta));
  else
    y = height - height * (val - adj->lower) / delta;

  if (isnan(y))
    y = height;
  else if (y < 0)
    y = 0;
  else if (height < y)
    y = height;

  return y + 0.5;
}

