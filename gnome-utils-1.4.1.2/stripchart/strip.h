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

#ifndef STRIP_H
#define STRIP_H

#include "chart.h"

#define STRIP(obj) \
	(GTK_CHECK_CAST((obj), strip_get_type(), Strip))
#define IS_STRIP(obj) \
	(GTK_CHECK_TYPE((obj), strip_get_type()))
#define STRIP_CLASS(klass) \
	(GTK_CHECK_CLASS_CAST((klass), strip_get_type(), StripClass))

typedef struct _Strip		Strip;
typedef struct _StripClass	StripClass;

struct _Strip
{
  Chart chart;
  gint show_ticks, minor_ticks, major_ticks;
};

struct _StripClass
{
  ChartClass parent_class;
};

guint strip_get_type(void);

GtkWidget *strip_new(void);

void strip_set_ticks(Strip *strip, gint show, gint major, gint minor);
void strip_set_default_history_size(Strip *strip, gint size);

#endif /* STRIP_H */ 
