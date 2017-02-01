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

#ifndef UTILS_H
#define UTILS_H

int streq(const char *s1, const char *s2);

#ifdef __GNUC__
const char *error(char *msg, ...) __attribute__((format (printf, 1, 2)));
#else 
const char *error(char *msg, ...);
#endif

ChartPlotStyle str_to_plot_style(const char *style_name);
ChartScaleStyle str_to_scale_style(const char *style_name);

gdouble str_to_gdouble(const char *double_string, gdouble fallback);

#endif /* UTILS_H */
