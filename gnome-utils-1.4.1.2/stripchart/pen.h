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

#ifndef PEN_H
#define PEN_H

#include "chart.h"

#define PEN(obj) \
	(GTK_CHECK_CAST((obj), pen_get_type(), Pen))
#define IS_PEN(obj) \
	(GTK_CHECK_TYPE((obj), pen_get_type()))
#define PEN_CLASS(klass) \
	(GTK_CHECK_CLASS_CAST((klass), pen_get_type(), PenClass))

typedef struct _Pen		Pen;
typedef struct _PenClass	PenClass;

struct _Pen
{
  Chart chart;
};

struct _PenClass
{
  ChartClass parent_class;
};

guint pen_get_type(void);

GtkWidget *pen_new(void);

#endif /* PEN_H */ 
