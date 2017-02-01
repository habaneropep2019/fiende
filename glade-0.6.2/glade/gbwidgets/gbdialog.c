
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

#include <string.h>
#include <gtk/gtkdialog.h>
#include "../gb.h"

/* Include the 21x21 icon pixmap for this widget, to be used in the palette */
#include "../graphics/dialog.xpm"

/*
 * This is the GbWidget struct for this widget (see ../gbwidget.h).
 * It is initialized in the init() function at the end of this file
 */
static GbWidget gbwidget;

/* These are the special children of the widget. */
static gchar *DialogVBox = "Dialog:vbox";
static gchar *DialogActionArea = "Dialog:action_area";

static gchar *Title = "Dialog|GtkWindow::title";
static gchar *Type = "Dialog|GtkWindow::type";
static gchar *Position = "Dialog|GtkWindow::position";
static gchar *Modal = "Dialog|GtkWindow::modal";
static gchar *DefaultWidth = "Dialog|GtkWindow::default_width";
static gchar *DefaultHeight = "Dialog|GtkWindow::default_height";
static gchar *Shrink = "Dialog|GtkWindow::allow_shrink";
static gchar *Grow = "Dialog|GtkWindow::allow_grow";
static gchar *AutoShrink = "Dialog|GtkWindow::auto_shrink";
static gchar *WMName = "Dialog|GtkWindow::wmclass_name";
static gchar *WMClass = "Dialog|GtkWindow::wmclass_class";


/******
 * NOTE: To use these functions you need to uncomment them AND add a pointer
 * to the function in the GbWidget struct at the end of this file.
 ******/

/*
 * Creates a new GtkWidget of class GtkDialog, performing any specialized
 * initialization needed for the widget to work correctly in this environment.
 * If a dialog box is used to initialize the widget, return NULL from this
 * function, and call data->callback with your new widget when it is done.
 * If the widget needs a special destroy handler, add a signal here.
 */
GtkWidget *
gb_dialog_new (GbWidgetNewData * data)
{
  GtkWidget *new_widget, *placeholder;

  new_widget = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (new_widget), data->name);
  gtk_window_set_policy (GTK_WINDOW (new_widget), TRUE, TRUE, FALSE);
  gtk_signal_connect (GTK_OBJECT (new_widget), "delete_event",
		      GTK_SIGNAL_FUNC (editor_close_window), NULL);

  /* We need to size the placeholders or the dialog is very small. */
  if (data->action != GB_LOADING)
    {
      placeholder = editor_new_placeholder ();
      gtk_widget_set_usize (placeholder, 300, 200);
      gtk_box_pack_start (GTK_BOX (GTK_DIALOG (new_widget)->vbox), placeholder,
			  TRUE, TRUE, 0);
      placeholder = editor_new_placeholder ();
      gtk_widget_set_usize (placeholder, 300, 40);
      gtk_box_pack_start (GTK_BOX (GTK_DIALOG (new_widget)->action_area),
			  placeholder, TRUE, TRUE, 0);
    }

  gb_widget_create_from (GTK_DIALOG (new_widget)->vbox,
			 data->action == GB_CREATING ? "dialog-vbox" : NULL);
  gb_widget_set_child_name (GTK_DIALOG (new_widget)->vbox, DialogVBox);

  gb_widget_create_from (GTK_DIALOG (new_widget)->action_area,
			 data->action == GB_CREATING ? "dialog-action_area"
						     : NULL);
  gb_widget_set_child_name (GTK_DIALOG (new_widget)->action_area,
			    DialogActionArea);

  return new_widget;
}



/*
 * Creates the components needed to edit the extra properties of this widget.
 */
static void
gb_dialog_create_properties (GtkWidget * widget, GbWidgetCreateArgData * data)
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
gb_dialog_get_properties (GtkWidget * widget, GbWidgetGetArgData * data)
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
gb_dialog_set_properties (GtkWidget * widget, GbWidgetSetArgData * data)
{
  gb_window_set_standard_properties (widget, data,
				     Title, Type, Position, Modal,
				     DefaultWidth, DefaultHeight,
				     Shrink, Grow, AutoShrink,
				     WMName, WMClass);
}



/*
 * Adds menu items to a context menu which is just about to appear!
 * Add commands to aid in editing a GtkDialog, with signals pointing to
 * other functions in this file.
 */
/*
   static void
   gb_dialog_create_popup_menu(GtkWidget *widget, GbWidgetCreateMenuData *data)
   {

   }
 */



/*
 * Writes the source code needed to create this widget.
 * You have to output everything necessary to create the widget here, though
 * there are some convenience functions to help.
 */
static void
gb_dialog_write_source (GtkWidget * widget, GbWidgetWriteSourceData * data)
{
  gchar *wname, *child_name;

  if (data->create_widget)
    {
      source_add (data, "  %s = gtk_dialog_new ();\n", data->wname);
    }

  gb_widget_write_standard_source (widget, data);

  gb_window_write_standard_source (widget, data,
				   Title, Type, Position, Modal,
				   DefaultWidth, DefaultHeight,
				   Shrink, Grow, AutoShrink,
				   WMName, WMClass);

  /* We output the source code for the children here, since the code should
     not include calls to create the widgets. We need to specify that the
     names used are like: "GTK_DIALOG (<dialog-name>)->vbox".
     We need to remember the dialog's name since data->wname
     will be overwritten. */
  wname = g_strdup (data->wname);

  source_add (data, "\n");
  child_name = gtk_widget_get_name (GTK_DIALOG (widget)->vbox);
  child_name = source_create_valid_identifier (child_name);
  source_add (data, "  %s = GTK_DIALOG (%s)->vbox;\n",
	      child_name, wname);
  g_free (child_name);
  data->create_widget = FALSE;
  gb_widget_write_source (GTK_DIALOG (widget)->vbox, data);

  /* action_area is a child of vbox so I had to add a kludge to stop it
     being written as a normal child - we need to do it here so that we
     don't output code to create it. */
  child_name = gtk_widget_get_name (GTK_DIALOG (widget)->action_area);
  child_name = source_create_valid_identifier (child_name);
  source_add (data, "  %s = GTK_DIALOG (%s)->action_area;\n",
	      child_name, wname);
  g_free (child_name);
  data->create_widget = FALSE;
  gb_widget_write_source (GTK_DIALOG (widget)->action_area, data);

  g_free (wname);
  data->write_children = FALSE;
}



static GtkWidget *
gb_dialog_get_child (GtkWidget * widget,
		     const gchar * child_name)
{
  if (!strcmp (child_name, DialogVBox))
    return GTK_DIALOG (widget)->vbox;
  else if (!strcmp (child_name, DialogActionArea))
    return GTK_DIALOG (widget)->action_area;
  else
    return NULL;
}


/*
 * Initializes the GbWidget structure.
 * I've placed this at the end of the file so we don't have to include
 * declarations of all the functions.
 */
GbWidget *
gb_dialog_init ()
{
  /* Initialise the GTK type */
  gtk_dialog_get_type ();

  /* Initialize the GbWidget structure */
  gb_widget_init_struct (&gbwidget);

  /* Fill in the pixmap struct & tooltip */
  gbwidget.pixmap_struct = dialog_xpm;
  gbwidget.tooltip = _("Dialog");

  /* Fill in any functions that this GbWidget has */
  gbwidget.gb_widget_new = gb_dialog_new;
  gbwidget.gb_widget_create_properties = gb_dialog_create_properties;
  gbwidget.gb_widget_get_properties = gb_dialog_get_properties;
  gbwidget.gb_widget_set_properties = gb_dialog_set_properties;
  gbwidget.gb_widget_get_child = gb_dialog_get_child;
  gbwidget.gb_widget_write_source = gb_dialog_write_source;
/*
   gbwidget.gb_widget_create_popup_menu = gb_dialog_create_popup_menu;
 */

  return &gbwidget;
}
