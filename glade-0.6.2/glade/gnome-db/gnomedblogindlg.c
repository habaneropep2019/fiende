/*  Gtk+ User Interface Builder
 *  Copyright (C) 1999  Damon Chaplin
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

#include <config.h>

#include <gnome-db.h>
#include "../gb.h"

/* Include the 21x21 icon pixmap for this widget, to be used in the palette */
#include "../graphics/gnome-db-login-dlg.xpm"

/*
 * This is the GbWidget struct for this widget (see ../gbwidget.h).
 * It is initialized in the init() function at the end of this file
 */
static GbWidget gbwidget;

static gchar *Title = "GnomeDbLoginDlg|GtkWindow::title";
static gchar *Type = "GnomeDbLoginDlg|GtkWindow::type";
static gchar *Position = "GnomeDbLoginDlg|GtkWindow::position";
static gchar *Modal = "GnomeDbLoginDlg|GtkWindow::modal";
static gchar *DefaultWidth = "GnomeDbLoginDlg|GtkWindow::default_width";
static gchar *DefaultHeight = "GnomeDbLoginDlg|GtkWindow::default_height";
static gchar *Shrink = "GnomeDbLoginDlg|GtkWindow::allow_shrink";
static gchar *Grow = "GnomeDbLoginDlg|GtkWindow::allow_grow";
static gchar *AutoShrink = "GnomeDbLoginDlg|GtkWindow::auto_shrink";
static gchar *WMName = "GnomeDbLoginDlg|GtkWindow::wmclass_name";
static gchar *WMClass = "GnomeDbLoginDlg|GtkWindow::wmclass_class";


/******
 * NOTE: To use these functions you need to uncomment them AND add a pointer
 * to the function in the GbWidget struct at the end of this file.
 ******/

/*
 * Creates a new GtkWidget of class GnomeDbLoginDlg, performing any specialized
 * initialization needed for the widget to work correctly in this environment.
 * If a dialog box is used to initialize the widget, return NULL from this
 * function, and call data->callback with your new widget when it is done.
 */
static GtkWidget*
gb_gnome_db_logindlg_new (GbWidgetNewData *data)
{
  GtkWidget *widget, *login;
  GList *elem;

  login = gnome_db_login_new (NULL, NULL, NULL);

  widget = gnome_db_logindlg_new (GNOME_DB_LOGIN (login), _("Login"));

  /* Stop the normal OK operation since it can cause a crash. */
  elem = g_list_nth (GNOME_DIALOG (widget)->buttons, 0);
  gtk_signal_disconnect_by_data (GTK_OBJECT (elem->data), widget);

  /* We connect a close signal handler which always returns TRUE so that
     the built-in close functionality is skipped. */
  gtk_signal_connect (GTK_OBJECT (widget), "close",
		      GTK_SIGNAL_FUNC (gtk_true), NULL);

  /* Now we connect our normal delete_event handler. */
  gtk_signal_connect (GTK_OBJECT (widget), "delete_event",
		      GTK_SIGNAL_FUNC (editor_close_window), NULL);

  return widget;
}



/*
 * Creates the components needed to edit the extra properties of this widget.
 */
static void
gb_gnome_db_logindlg_create_properties (GtkWidget * widget, GbWidgetCreateArgData * data)
{
  gb_window_create_standard_properties (widget, data,
					Title, Type, Position, Modal,
					DefaultWidth, DefaultHeight,
					Shrink, Grow, AutoShrink,
					WMName, WMClass);
}



/*
 * Gets the properties of the widget. This is used for both displaying the
 * properties in the property editor, and also for saving the properties.
 */
static void
gb_gnome_db_logindlg_get_properties (GtkWidget *widget, GbWidgetGetArgData * data)
{
  gb_window_get_standard_properties (widget, data,
				     Title, Type, Position, Modal,
				     DefaultWidth, DefaultHeight,
				     Shrink, Grow, AutoShrink,
				     WMName, WMClass);
}



/*
 * Sets the properties of the widget. This is used for both applying the
 * properties changed in the property editor, and also for loading.
 */
static void
gb_gnome_db_logindlg_set_properties (GtkWidget * widget, GbWidgetSetArgData * data)
{
  gb_window_set_standard_properties (widget, data,
				     Title, Type, Position, Modal,
				     DefaultWidth, DefaultHeight,
				     Shrink, Grow, AutoShrink,
				     WMName, WMClass);
}



/*
 * Adds menu items to a context menu which is just about to appear!
 * Add commands to aid in editing a GnomeDbLoginDlg, with signals pointing to
 * other functions in this file.
 */
/*
static void
gb_gnome_db_logindlg_create_popup_menu (GtkWidget * widget, GbWidgetCreateMenuData * data)
{

}
*/



/*
 * Writes the source code needed to create this widget.
 * You have to output everything necessary to create the widget here, though
 * there are some convenience functions to help.
 */
static void
gb_gnome_db_logindlg_write_source (GtkWidget * widget, GbWidgetWriteSourceData * data)
{
  if (data->create_widget)
    {
      gchar *title;

      title = GTK_WINDOW (widget)->title;
      source_add (data, "  %s = gnome_db_logindlg_new (GNOME_DB_LOGIN (gnome_db_login_new (NULL, NULL, NULL)), %s);\n",
		  data->wname,
		  title ? source_make_string (title, data->use_gettext)
		        : "NULL");
    }

  gb_widget_write_standard_source (widget, data);

  /* The title is already set above, so we pass NULL to skip it. */
  gb_window_write_standard_source (widget, data,
				   NULL, Type, Position, Modal,
				   DefaultWidth, DefaultHeight,
				   Shrink, Grow, AutoShrink,
				   WMName, WMClass);
}



/*
 * Initializes the GbWidget structure.
 * I've placed this at the end of the file so we don't have to include
 * declarations of all the functions.
 */
GbWidget*
gb_gnome_db_logindlg_init ()
{
  /* Initialise the GTK type */
  gnome_db_logindlg_get_type();

  /* Initialize the GbWidget structure */
  gb_widget_init_struct(&gbwidget);

  /* Fill in the pixmap struct & tooltip */
  gbwidget.pixmap_struct = gnome_db_login_dlg_xpm;
  gbwidget.tooltip = _("Database login dialog");

  /* Fill in any functions that this GbWidget has */
  gbwidget.gb_widget_new		= gb_gnome_db_logindlg_new;
  gbwidget.gb_widget_create_properties	= gb_gnome_db_logindlg_create_properties;
  gbwidget.gb_widget_get_properties	= gb_gnome_db_logindlg_get_properties;
  gbwidget.gb_widget_set_properties	= gb_gnome_db_logindlg_set_properties;
  gbwidget.gb_widget_write_source	= gb_gnome_db_logindlg_write_source;
/*
  gbwidget.gb_widget_create_popup_menu	= gb_gnome_db_logindlg_create_popup_menu;
*/

  return &gbwidget;
}

