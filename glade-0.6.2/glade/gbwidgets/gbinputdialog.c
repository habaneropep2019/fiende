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

#include <gtk/gtkinputdialog.h>
#include "../gb.h"

/* Include the 21x21 icon pixmap for this widget, to be used in the palette */
#include "../graphics/inputdialog.xpm"

/*
 * This is the GbWidget struct for this widget (see ../gbwidget.h).
 * It is initialized in the init() function at the end of this file
 */
static GbWidget gbwidget;

/* These are the special children of the widget. */
static gchar *InputDialogSaveButton = "InputDialog:save_button";
static gchar *InputDialogCloseButton = "InputDialog:close_button";

static gchar *Title = "InputDialog|GtkWindow::title";
static gchar *Type = "InputDialog|GtkWindow::type";
static gchar *Position = "InputDialog|GtkWindow::position";
static gchar *Modal = "InputDialog|GtkWindow::modal";
static gchar *DefaultWidth = "Input|GtkWindow::default_width";
static gchar *DefaultHeight = "Input|GtkWindow::default_height";
static gchar *Shrink = "InputDialog|GtkWindow::allow_shrink";
static gchar *Grow = "InputDialog|GtkWindow::allow_grow";
static gchar *AutoShrink = "InputDialog|GtkWindow::auto_shrink";
static gchar *WMName = "InputDialog|GtkWindow::wmclass_name";
static gchar *WMClass = "InputDialog|GtkWindow::wmclass_class";


/******
 * NOTE: To use these functions you need to uncomment them AND add a pointer
 * to the funtion in the GbWidget struct at the end of this file.
 ******/

/*
 * Creates a new GtkWidget of class GtkInputDialog, performing any specialized
 * initialization needed for the widget to work correctly in this environment.
 * If a dialog box is used to initialize the widget, return NULL from this
 * function, and call data->callback with your new widget when it is done.
 * If the widget needs a special destroy handler, add a signal here.
 */
GtkWidget*
gb_input_dialog_new (GbWidgetNewData *data)
{
  GtkWidget *new_widget = gtk_input_dialog_new ();

  GtkInputDialog *inputdlg = GTK_INPUT_DIALOG (new_widget);

  gtk_signal_connect (GTK_OBJECT (new_widget), "delete_event",
		      GTK_SIGNAL_FUNC (editor_close_window), NULL);

  gb_widget_create_from (inputdlg->save_button,
			 data->action == GB_CREATING ? "save_button" : NULL);
  gb_widget_set_child_name (inputdlg->save_button, InputDialogSaveButton);
  /* We need to set it sensitive so the user can select it. */
  gtk_widget_set_sensitive (inputdlg->save_button, TRUE);

  gb_widget_create_from (inputdlg->close_button,
			 data->action == GB_CREATING ? "close_button" : NULL);
  gb_widget_set_child_name (inputdlg->close_button, InputDialogCloseButton);

  return new_widget;
}



/*
 * Creates the components needed to edit the extra properties of this widget.
 */
static void
gb_input_dialog_create_properties (GtkWidget * widget, GbWidgetCreateArgData * data)
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
gb_input_dialog_get_properties (GtkWidget *widget, GbWidgetGetArgData * data)
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
gb_input_dialog_set_properties (GtkWidget * widget, GbWidgetSetArgData * data)
{
  gb_window_set_standard_properties (widget, data,
				     Title, Type, Position, Modal,
				     DefaultWidth, DefaultHeight,
				     Shrink, Grow, AutoShrink,
				     WMName, WMClass);
}



/*
 * Adds menu items to a context menu which is just about to appear!
 * Add commands to aid in editing a GtkInputDialog, with signals pointing to
 * other functions in this file.
 */
/*
static void
gb_input_dialog_create_popup_menu (GtkWidget * widget, GbWidgetCreateMenuData * data)
{

}
*/



/*
 * Writes the source code needed to create this widget.
 * You have to output everything necessary to create the widget here, though
 * there are some convenience functions to help.
 */
static void
gb_input_dialog_write_source (GtkWidget * widget, GbWidgetWriteSourceData * data)
{
  if (data->create_widget)
    {
      source_add (data, "  %s = gtk_input_dialog_new ();\n", data->wname);
    }

  gb_widget_write_standard_source (widget, data);

  gb_window_write_standard_source (widget, data,
				   Title, Type, Position, Modal,
				   DefaultWidth, DefaultHeight,
				   Shrink, Grow, AutoShrink,
				   WMName, WMClass);
}


static GtkWidget *
gb_input_dialog_get_child (GtkWidget * widget,
			   const gchar * child_name)
{
  if (!strcmp (child_name, InputDialogSaveButton))
    return GTK_INPUT_DIALOG (widget)->save_button;
  else if (!strcmp (child_name, InputDialogCloseButton))
    return GTK_INPUT_DIALOG (widget)->close_button;
  else
    return NULL;
}


/*
 * Initializes the GbWidget structure.
 * I've placed this at the end of the file so we don't have to include
 * declarations of all the functions.
 */
GbWidget*
gb_input_dialog_init ()
{
  /* Initialise the GTK type */
  gtk_input_dialog_get_type();

  /* Initialize the GbWidget structure */
  gb_widget_init_struct(&gbwidget);

  /* Fill in the pixmap struct & tooltip */
  gbwidget.pixmap_struct = inputdialog_xpm;
  gbwidget.tooltip = _("Input Dialog");

  /* Fill in any functions that this GbWidget has */
  gbwidget.gb_widget_new = gb_input_dialog_new;
  gbwidget.gb_widget_get_child = gb_input_dialog_get_child;
  gbwidget.gb_widget_create_properties	= gb_input_dialog_create_properties;
  gbwidget.gb_widget_get_properties	= gb_input_dialog_get_properties;
  gbwidget.gb_widget_set_properties	= gb_input_dialog_set_properties;
  gbwidget.gb_widget_write_source = gb_input_dialog_write_source;
/*
  gbwidget.gb_widget_create_popup_menu	= gb_input_dialog_create_popup_menu;
*/

  return &gbwidget;
}

