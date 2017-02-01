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

#ifndef CHART_H
#define CHART_H

#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <gtk/gtkdrawingarea.h>

#define CHART(obj) \
	(GTK_CHECK_CAST((obj), chart_get_type(), Chart))
#define IS_CHART(obj) \
	(GTK_CHECK_TYPE((obj), chart_get_type()))
#define CHART_CLASS(klass) \
	(GTK_CHECK_CLASS_CAST((klass), chart_get_type(), ChartClass))

typedef struct _Chart		Chart;
typedef struct _ChartClass	ChartClass;
typedef struct _ChartDatum	ChartDatum;

typedef enum
{
  chart_scale_linear,
  chart_scale_log
}
ChartScaleStyle;

typedef enum
{
  chart_plot_point,
  chart_plot_line,
  chart_plot_solid,
  chart_plot_indicator
}
ChartPlotStyle; /* FIX THIS: should be in strip.h */

struct _Chart
{
  GtkDrawingArea drawing;
  guint timer;

  gint points_in_view;
  gint default_history_size;

  ChartPlotStyle default_plot_style;
  ChartScaleStyle default_scale_style;

  GdkColormap *colormap;
  GSList *param;
};

struct _ChartClass
{
  GtkDrawingAreaClass parent_class;
  void (*chart_pre_update)  (Chart *chart);
  void (*chart_post_update) (Chart *chart);
  void (*chart_rescale)	    (Chart *chart);
};

struct _ChartDatum
{
  Chart *chart;

  gint history_size, history_count, active, idle, newest, skip;
  gfloat *history;

  gdouble max, min; /* of current window's worth of history values */
  gdouble top_max, top_min;
  gdouble bot_max, bot_min; /* Adjustment limits */
  GtkAdjustment *adj; /* Only upper and lower are to be used. */

  gdouble (*user_func)(void *user_data);
  void *user_data;

  gint (*range_func)(void *range_data);
  void *range_data;

  ChartScaleStyle scale_style;
  ChartPlotStyle plot_style; /* FIX THIS: only strips have plot_styles */

  gchar *color_names;
  gint colors;
  GdkGC **gdk_gc;
  GdkColor *gdk_color;
};

guint chart_get_type(void);

void chart_set_interval(Chart *chart, guint msec);

ChartDatum *chart_parameter_add(Chart *chart,
  gdouble (*func)(), gpointer user_data,
  gchar *color_name, GtkAdjustment *adj, int pageno,
  gdouble bot_min, gdouble bot_max, gdouble top_min, gdouble top_max);

void chart_parameter_deactivate(Chart *chart, ChartDatum *param);

gint chart_rescale_by_decade(ChartDatum *datum);
gint chart_rescale_by_125(ChartDatum *datum);

void chart_set_autorange(ChartDatum *param, gint (*func)(), void *data);

void chart_set_top_min(ChartDatum *datum, gdouble top_min);
void chart_set_top_max(ChartDatum *datum, gdouble top_max);
void chart_set_bot_min(ChartDatum *datum, gdouble bot_min);
void chart_set_bot_max(ChartDatum *datum, gdouble bot_max);

void chart_set_plot_style(ChartDatum *datum, ChartPlotStyle plot_style);
void chart_set_scale_style(ChartDatum *datum, ChartScaleStyle scale_style);
void chart_assign_color(Chart *chart, ChartDatum *parm);

gint val2gdk(gdouble val,
  GtkAdjustment *adj, gint height, ChartScaleStyle scale);

#endif /* CHART_H */ 
