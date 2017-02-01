/*  Gtk+ User Interface Builder
 *  Copyright (C) 1998  Damon Chaplin
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
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
#ifndef GLADE_GB_H
#define GLADE_GB_H

/* This header file is included by all gbwidgets in the gbwidget directory,
   so if we add any header files to Glade, we only have to change this. */

#include "gladeconfig.h"

#include "editor.h"
#include "gbwidget.h"
#include "glade_project.h"
#include "load.h"
#include "property.h"
#include "save.h"
#include "source.h"
#include "utils.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Some common functions. */
void	    gb_box_set_size			(GtkWidget *widget,
						 gint size);
void	    gb_box_create_child_properties	(GtkWidget * widget,
						 GbWidgetCreateChildArgData * data);
void	    gb_box_get_child_properties		(GtkWidget *widget,
						 GtkWidget *child,
						 GbWidgetGetArgData *data);
void	    gb_box_set_child_properties		(GtkWidget * widget,
						 GtkWidget * child,
						 GbWidgetSetArgData * data);
void	    gb_box_create_popup_menu		(GtkWidget * widget,
						 GbWidgetCreateMenuData * data);
void	    gb_box_write_add_child_source	(GtkWidget * parent,
						 const gchar * parent_name,
						 GtkWidget * child,
						 GbWidgetWriteSourceData * data);

void	    gb_button_create_child_icon_property(GtkWidget * widget,
						 GbWidgetCreateArgData * data,
						 gchar * property_name);
void	    gb_button_write_uline_accel_source	(GtkWidget * widget,
						 GbWidgetWriteSourceData *data,
						 gchar *label_text);

void	    gb_menu_item_write_accel_source	(GtkWidget * widget,
						 GbWidgetWriteSourceData *data,
						 gchar *label_text);

void	    gb_paned_create_child_properties	(GtkWidget * widget,
						 GbWidgetCreateChildArgData * data);
void	    gb_paned_get_child_properties	(GtkWidget *widget,
						 GtkWidget *child,
						 GbWidgetGetArgData *data);
void	    gb_paned_set_child_properties	(GtkWidget * widget,
						 GtkWidget * child,
						 GbWidgetSetArgData * data);
void	    gb_paned_write_add_child_source	(GtkWidget * parent,
						 const gchar *parent_name,
						 GtkWidget *child,
						 GbWidgetWriteSourceData * data);

GSList *    gb_radio_button_reset_radio_group	(GtkWidget * widget);
void	    gb_radio_button_update_radio_group	(GSList * group);

void	    gb_table_update_placeholders	(GtkWidget * table,
						 gint rows,
						 gint cols);

GtkWidget * gb_toolbar_new_toolbar_button	(GbWidgetNewData * data,
						 GtkToolbarChildType type);
void	    gb_toolbar_insert_toolbar_child	(GtkToolbar * toolbar,
						 GtkWidget * new_child,
						 gint pos);
gboolean    gb_toolbar_is_toolbar_button	(GtkWidget * widget);
gboolean    gb_toolbar_get_new_toolbar_group	(GtkWidget * toolbar,
						 GtkWidget * widget);
void	    gb_toolbar_output_child_label	(GtkWidget * widget,
						 GbWidgetGetArgData * data,
						 const gchar * Label);
void	    gb_toolbar_input_child_label	(GtkWidget * widget,
						 GbWidgetSetArgData * data,
						 const gchar * Label);
void	    gb_toolbar_output_child_icon	(GtkWidget * widget,
						 GbWidgetGetArgData * data,
						 const gchar * property_name);
void	    gb_toolbar_input_child_icon		(GtkWidget * widget,
						 GbWidgetSetArgData * data,
						 const gchar * property_name);
void	    gb_toolbar_destroy_child_icon	(GtkWidget * widget,
						 GbWidgetDestroyData * data);
void	    gb_toolbar_create_toolbar_button_popup_menu	(GtkWidget *widget,
						 GbWidgetCreateMenuData *data);
void	    gb_toolbar_write_toolbar_button_source (GtkWidget * widget,
						 GbWidgetWriteSourceData * data);

void	    gb_window_create_standard_properties(GtkWidget * widget,
						 GbWidgetCreateArgData * data,
						 gchar *title_p,
						 gchar *type_p,
						 gchar *position_p,
						 gchar *modal_p,
						 gchar *default_width_p,
						 gchar *default_height_p,
						 gchar *shrink_p,
						 gchar *grow_p,
						 gchar *auto_shrink_p,
						 gchar *wmname_p,
						 gchar *wmclass_p);
void	    gb_window_get_standard_properties	(GtkWidget * widget,
						 GbWidgetGetArgData * data,
						 gchar *title_p,
						 gchar *type_p,
						 gchar *position_p,
						 gchar *modal_p,
						 gchar *default_width_p,
						 gchar *default_height_p,
						 gchar *shrink_p,
						 gchar *grow_p,
						 gchar *auto_shrink_p,
						 gchar *wmname_p,
						 gchar *wmclass_p);
void	    gb_window_set_standard_properties	(GtkWidget * widget,
						 GbWidgetSetArgData * data,
						 gchar *title_p,
						 gchar *type_p,
						 gchar *position_p,
						 gchar *modal_p,
						 gchar *default_width_p,
						 gchar *default_height_p,
						 gchar *shrink_p,
						 gchar *grow_p,
						 gchar *auto_shrink_p,
						 gchar *wmname_p,
						 gchar *wmclass_p);
void	    gb_window_write_standard_source	(GtkWidget * widget,
						 GbWidgetWriteSourceData *data,
						 gchar *title_p,
						 gchar *type_p,
						 gchar *position_p,
						 gchar *modal_p,
						 gchar *default_width_p,
						 gchar *default_height_p,
						 gchar *shrink_p,
						 gchar *grow_p,
						 gchar *auto_shrink_p,
						 gchar *wmname_p,
						 gchar *wmclass_p);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* GLADE_GB_H */
