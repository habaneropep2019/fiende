/*
 *  Gnome Character Map
 *  interface.h - The main window
 *
 *  Copyright (C) Hongli Lai
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef _INTERFACE_H_
#define _INTERFACE_H_

#include <gtk/gtk.h>


#define MAIN_APP_TYPE             (main_app_get_type ())
#define MAIN_APP(obj)             GTK_CHECK_CAST (obj, MAIN_APP_TYPE, MainApp)
#define MAIN_APP_CLASS(klass)     GTK_CHECK_CLASS_CAST ((klass), MAIN_APP_TYPE, MainAppClass)
#define MAIN_IS_APP(obj)          GTK_CHECK_TYPE (obj, MAIN_APP_TYPE)
#define MAIN_IS_APP_CLASS(klass)  GTK_CHECK_TYPE ((klass), MAIN_APP_TYPE)

typedef struct _MainApp
{
    GtkObject parent_struct;
    GtkWidget *window;
    GtkWidget *entry;
    GtkWidget *actionbar, *textbar;
    GtkWidget *preview_label;
    GtkWidget *fontpicker;
    GtkWidget *chartable;
    gboolean insert_at_end;
    GList *buttons;
    GtkStyle *btnstyle;
} MainApp;

typedef struct _MainAppClass
{
    GtkObjectClass parent_klass;
} MainAppClass;


extern MainApp *mainapp;

guint main_app_get_type (void);
MainApp *main_app_new (void);
void main_app_destroy (MainApp *obj);


#endif /* _INTERFACE_H_ */
