/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Copyright (C) 1997, 1998, 1999, 2000 Free Software Foundation
 * All rights reserved.
 *
 * This file is part of the Gnome Library.
 *
 * The Gnome Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * The Gnome Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with the Gnome Library; see the file COPYING.LIB.  If not,
 * write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
/*
  @NOTATION@
 */

/* GnomeCalculator - double precision simple calculator widget
 *
 * Author: George Lebl <jirka@5z.com>
 */

#ifndef __GNOME_CALC_H__
#define __GNOME_CALC_H__

#include <gdk/gdk.h>
#include <gtk/gtkvbox.h>

#include <libgnome/gnome-defs.h>

BEGIN_GNOME_DECLS


#define GNOME_TYPE_CALC            (gnome_calc_get_type ())
#define GNOME_CALC(obj)            (GTK_CHECK_CAST ((obj), GNOME_TYPE_CALC, GnomeCalc))
#define GNOME_CALC_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), GNOME_TYPE_CALC, GnomeCalcClass))
#define GNOME_IS_CALC(obj)         (GTK_CHECK_TYPE ((obj), GNOME_TYPE_CALC))
#define GNOME_IS_CALC_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GNOME_TYPE_CALC))
#define GNOME_CALC_GET_CLASS(obj)  (GTK_CHECK_GET_CLASS ((obj), GNOME_TYPE_CALC, GnomeCalcClass))


typedef struct _GnomeCalc        GnomeCalc;
typedef struct _GnomeCalcPrivate GnomeCalcPrivate;
typedef struct _GnomeCalcClass   GnomeCalcClass;

typedef enum {
	GNOME_CALC_DEG,
	GNOME_CALC_RAD,
	GNOME_CALC_GRAD
} GnomeCalcMode;

struct _GnomeCalc {
	GtkVBox vbox;

	/*< private >*/
	GnomeCalcPrivate *_priv;
};

struct _GnomeCalcClass {
	GtkVBoxClass parent_class;

	void (* result_changed)(GnomeCalc *gc,
				gdouble result);
};


GtkType		 gnome_calc_get_type	(void) G_GNUC_CONST;
GtkWidget	*gnome_calc_new		(void);

void		 gnome_calc_clear	(GnomeCalc *gc,
					 const gboolean reset);
void		 gnome_calc_set		(GnomeCalc *gc,
					 gdouble result);

gdouble		 gnome_calc_get_result	      (GnomeCalc *gc);
GtkAccelGroup   *gnome_calc_get_accel_group   (GnomeCalc *gc);
const char	*gnome_calc_get_result_string (GnomeCalc *gc);

void             gnome_calc_bind_extra_keys (GnomeCalc *gc,
					     GtkWidget *widget);

END_GNOME_DECLS

#endif
