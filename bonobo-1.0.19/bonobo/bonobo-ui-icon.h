/* bonobo-ui-icon.h: Icon widget for the Bonobo UI engine
 *
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Author: Federico Mena-Quintero <federico@ximian.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef _BONOBO_UI_ICON_H_
#define _BONOBO_UI_ICON_H_

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gtk/gtkwidget.h>



#define BONOBO_UI_ICON_TYPE            (bonobo_ui_icon_get_type ())
#define BONOBO_UI_ICON(obj)            (GTK_CHECK_CAST ((obj), BONOBO_UI_ICON_TYPE, BonoboUIIcon))
#define BONOBO_UI_ICON_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((obj), BONOBO_UI_ICON_TYPE, BonoboUIIcon))
#define BONOBO_IS_UI_ICON(obj)         (GTK_CHECK_TYPE ((obj), BONOBO_UI_ICON_TYPE))
#define BONOBO_IS_UI_ICON_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((obj), BONOBO_IS_UI_ICON_CLASS))

typedef struct BonoboUIIconPrivate BonoboUIIconPrivate;

typedef struct {
	GtkWidget widget;

	/* Private data */
	BonoboUIIconPrivate *priv;
} BonoboUIIcon;

typedef struct {
	GtkWidgetClass parent_class;
} BonoboUIIconClass;

GtkType bonobo_ui_icon_get_type (void);

GtkWidget *bonobo_ui_icon_new (void);

void bonobo_ui_icon_set_images (BonoboUIIcon *icon, GdkPixbuf **images);

gboolean bonobo_ui_icon_set_from_pixbuf (BonoboUIIcon *icon, GdkPixbuf *base);



#endif
