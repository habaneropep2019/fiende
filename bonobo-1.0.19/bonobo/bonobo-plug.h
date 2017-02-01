/* GTK - The GIMP Toolkit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
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
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * Modified by the GTK+ Team and others 1997-1999.  See the AUTHORS
 * file for a list of people on the GTK+ Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GTK+ at ftp://ftp.gtk.org/pub/gtk/. 
 */

#ifndef __BONOBO_PLUG_H__
#define __BONOBO_PLUG_H__


#include <gdk/gdk.h>
#include <gtk/gtkwindow.h>
#include <bonobo/bonobo-control.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define BONOBO_PLUG_TYPE          (bonobo_plug_get_type ())
#define BONOBO_PLUG(obj)          (GTK_CHECK_CAST (obj, BONOBO_PLUG_TYPE, BonoboPlug))
#define BONOBO_PLUG_CLASS(klass)  (GTK_CHECK_CLASS_CAST (klass, bonobo_plug_get_type (), BonoboPlugClass))
#define BONOBO_IS_PLUG(obj)       (GTK_CHECK_TYPE (obj, bonobo_plug_get_type ()))

typedef struct _BonoboPlugPrivate BonoboPlugPrivate;

typedef struct {
	GtkWindow window;

	/* Private data */
	BonoboPlugPrivate *priv;
} BonoboPlug;

typedef struct {
	GtkWindowClass parent_class;
} BonoboPlugClass;

guint      bonobo_plug_get_type  (void);
void       bonobo_plug_construct (BonoboPlug *plug, guint32 socket_id);
GtkWidget* bonobo_plug_new       (guint32 socket_id);

void bonobo_plug_set_control (BonoboPlug *plug, BonoboControl *control);

void bonobo_plug_clear_focus_chain (BonoboPlug *plug);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __BONOBO_PLUG_H__ */
