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

#include "chart-app.h"
#include <ctype.h>
#include <string.h>

/*
 * streq -- case-blind string comparison returning true on equality.
 */
int
streq(const char *s1, const char *s2)
{
  return s1 && s2 && strcasecmp(s1, s2) == 0;
}

/*
 * error -- error handler used primarily by eval.
 */
const char *
error(char *msg, ...)
{
  int len;
  va_list args;
  static char err_msg[1000];

  fflush(stdout);
  va_start(args, msg);
  len = sprintf(err_msg, "%s: ", prog_name);
  len += vsprintf(err_msg + len, msg, args);
  va_end(args);

  fprintf(stderr, "%s\n", err_msg);

  gnome_dialog_run(GNOME_DIALOG(gnome_error_dialog(err_msg)));

  return err_msg;
}

ChartPlotStyle
str_to_plot_style(const char *style_name)
{
  if (streq(style_name, "indicator"))
    return chart_plot_indicator;
  if (streq(style_name, "point"))
    return chart_plot_point;
  if (streq(style_name, "line"))
    return chart_plot_line;
  if (streq(style_name, "solid"))
    return chart_plot_solid;
  return chart_plot_line;
}

ChartScaleStyle
str_to_scale_style(const char *style_name)
{
  if (streq(style_name, "linear"))
    return chart_scale_linear;
  if (streq(style_name, "log"))
    return chart_scale_log;
  return chart_scale_linear;
}

gdouble
str_to_gdouble(const char *str, gdouble fallback)
{
  gdouble value;

  if (str && sscanf(str, "%lf", &value) == 1)
    return value;

  return fallback;
}
