
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

#include <gtk/gtkcolorsel.h>
#include "../gb.h"

/* Include the 21x21 icon pixmap for this widget, to be used in the palette */
#include "../graphics/colorseldialog.xpm"

/*
 * This is the GbWidget struct for this widget (see ../gbwidget.h).
 * It is initialized in the init() function at the end of this file
 */
static GbWidget gbwidget;

/* These are the special children of the widget. */
static gchar *ColorSelOKButton = "ColorSel:ok_button";
static gchar *ColorSelCancelButton = "ColorSel:cancel_button";
static gchar *ColorSelHelpButton = "ColorSel:help_button";

static gchar *Title = "ColorSelDialog|GtkWindow::title";
static gchar *Type = "ColorSelDialog|GtkWindow::type";
static gchar *Position = "ColorSelDialog|GtkWindow::position";
static gchar *Modal = "ColorSelDialog|GtkWindow::modal";
static gchar *DefaultWidth = "ColorSel|GtkWindow::default_width";
static gchar *DefaultHeight = "ColorSel|GtkWindow::default_height";
static gchar *Shrink = "ColorSelDialog|GtkWindow::allow_shrink";
static gchar *Grow = "ColorSelDialog|GtkWindow::allow_grow";
static gchar *AutoShrink = "ColorSelDialog|GtkWindow::auto_shrink";
static gchar *Policy = "ColorSelDialog|GtkColorSelection::policy";
static gchar *WMName = "ColorSelDialog|GtkWindow::wmclass_name";
static gchar *WMClass = "ColorSelDialog|GtkWindow::wmclass_class";


static const gchar *GbPolicyChoices[] =
{"Continuous", "Discontinuous", "Delayed",
 NULL};
static const gint GbPolicyValues[] =
{
  GTK_UPDATE_CONTINUOUS,
  GTK_UPDATE_DISCONTINUOUS,
  GTK_UPDATE_DELAYED
};
static const gchar *GbPolicySymbols[] =
{
  "GTK_UPDATE_CONTINUOUS",
  "GTK_UPDATE_DISCONTINUOUS",
  "GTK_UPDATE_DELAYED"
};


/******
 * NOTE: To use these functions you need to uncomment them AND add a pointer
 * to the function in the GbWidget struct at the end of this file.
 ******/

/*
 * Creates a new GtkWidget of class GtkColorSelectionDialog, performing any specialized
 * initialization needed for the widget to work correctly in this environment.
 * If a dialog box is used to initialize the widget, return NULL from this
 * function, and call data->callback with your new widget when it is done.
 * If the widget needs a special destroy handler, add a signal here.
 */
GtkWidget *
gb_color_selection_dialog_new (GbWidgetNewData * data)
{
  GtkWidget *new_widget = gtk_color_selection_dialog_new (_("Select Color"));

  GtkColorSelectionDialog *colorsel = GTK_COLOR_SELECTION_DIALOG (new_widget);

  gtk_signal_connect (GTK_OBJECT (new_widget), "delete_event",
		      GTK_SIGNAL_FUNC (editor_close_window), NULL);

  gb_widget_create_from (colorsel->ok_button,
			 data->action == GB_CREATING ? "ok_button" : NULL);
  gb_widget_set_child_name (colorsel->ok_button, ColorSelOKButton);

  gb_widget_create_from (colorsel->cancel_button,
			 data->action == GB_CREATING ? "cancel_button" : NULL);
  gb_widget_set_child_name (colorsel->cancel_button, ColorSelCancelButton);

  gb_widget_create_from (colorsel->help_button,
			 data->action == GB_CREATING ? "help_button" : NULL);
  gb_widget_set_child_name (colorsel->help_button, ColorSelHelpButton);

  return new_widget;
}



/*
 * Creates the components needed to edit the extra properties of this widget.
 */
static void
gb_color_selection_dialog_create_properties (GtkWidget * widget,
					     GbWidgetCreateArgData * data)
{
  gb_window_create_standard_properties (widget, data,
					Title, Type, Position, Modal,
					DefaultWidth, DefaultHeight,
					Shrink, Grow, AutoShrink,
					WMName, WMClass);
  property_add_choice (Policy, _("Update Policy:"),
		       _("The update policy of the color selection scales"),
		       GbPolicyChoices);
}



/*
 * Gets the properties of the widget. This is used for both displaying the
 * properties in the property editor, and also for saving the properties.
 */
static void
gb_color_selection_dialog_get_properties (GtkWidget * widget,
					  GbWidgetGetArgData * data)
{
  GtkColorSelectionDialog *colorseldlg;
  GtkColorSelection *colorsel;
  gint i;

  colorseldlg = GTK_COLOR_SELECTION_DIALOG (widget);
  colorsel = GTK_COLOR_SELECTION (colorseldlg->colorsel);

  gb_window_get_standard_properties (widget, data,
				     Title, Type, Position, Modal,
				     DefaultWidth, DefaultHeight,
				     Shrink, Grow, AutoShrink,
				     WMName, WMClass);

  for (i = 0; i < sizeof (GbPolicyValues) / sizeof (GbPolicyValues[0]); i++)
    {
      if (GbPolicyValues[i] == colorsel->policy)
	gb_widget_output_choice (data, Policy, i, GbPolicySymbols[i]);
    }
}



/*
 * Sets the properties of the widget. This is used for both applying the
 * properties changed in the property editor, and also for loading.
 */
static void
gb_color_selection_dialog_set_properties (GtkWidget * widget,
					  GbWidgetSetArgData * data)
{
  GtkColorSelectionDialog *colorseldlg;
  GtkColorSelection *colorsel;
  gchar *policy;
  gint i;

  colorseldlg = GTK_COLOR_SELECTION_DIALOG (widget);
  colorsel = GTK_COLOR_SELECTION (colorseldlg->colorsel);

  gb_window_set_standard_properties (widget, data,
				     Title, Type, Position, Modal,
				     DefaultWidth, DefaultHeight,
				     Shrink, Grow, AutoShrink,
				     WMName, WMClass);

  policy = gb_widget_input_choice (data, Policy);
  if (data->apply)
    {
      for (i = 0; i < sizeof (GbPolicyValues) / sizeof (GbPolicyValues[0]);
	   i++)
	{
	  if (!strcmp (policy, GbPolicyChoices[i])
	      || !strcmp (policy, GbPolicySymbols[i]))
	    {
	      gtk_color_selection_set_update_policy (colorsel,
						     GbPolicyValues[i]);
	      break;
	    }
	}
    }
}



/*
 * Adds menu items to a context menu which is just about to appear!
 * Add commands to aid in editing a GtkColorSelectionDialog, with signals pointing to
 * other functions in this file.
 */
/*
   static void
   gb_color_selection_dialog_create_popup_menu(GtkWidget *widget, GbWidgetCreateMenuData *data)
   {

   }
 */



/*
 * Writes the source code needed to create this widget.
 * You have to output everything necessary to create the widget here, though
 * there are some convenience functions to help.
 */
static void
gb_color_selection_dialog_write_source (GtkWidget * widget,
					GbWidgetWriteSourceData * data)
{
  GtkColorSelection *colorsel = GTK_COLOR_SELECTION (GTK_COLOR_SELECTION_DIALOG
						     (widget)->colorsel);
  gint i;
  gchar *wname, *child_name;

  if (data->create_widget)
    {
      source_add (data, "  %s = gtk_color_selection_dialog_new (%s);\n",
		  data->wname,
		  source_make_string (GTK_WINDOW (widget)->title,
				      data->use_gettext));
    }

  gb_widget_write_standard_source (widget, data);

  /* The title is already set above, so we pass NULL to skip it. */
  gb_window_write_standard_source (widget, data,
				   NULL, Type, Position, Modal,
				   DefaultWidth, DefaultHeight,
				   Shrink, Grow, AutoShrink,
				   WMName, WMClass);

  if (colorsel->policy != GTK_UPDATE_CONTINUOUS)
    {
      for (i = 0; i < sizeof (GbPolicyValues) / sizeof (GbPolicyValues[0]);
	   i++)
	{
	  if (GbPolicyValues[i] == colorsel->policy)
	    source_add (data,
			"  gtk_color_selection_set_update_policy (GTK_COLOR_SELECTION (GTK_COLOR_SELECTION_DIALOG (%s)->colorsel), %s);\n",
			data->wname, GbPolicySymbols[i]);
	}
    }


  /* We output the source code for the buttons here, but we don't want them
     to be created. We need to remember the dialog's name since data->wname
     will be overwritten. */
  wname = g_strdup (data->wname);

  source_add (data, "\n");
  child_name = gtk_widget_get_name (GTK_COLOR_SELECTION_DIALOG (widget)->ok_button);
  child_name = source_create_valid_identifier (child_name);
  source_add (data, "  %s = GTK_COLOR_SELECTION_DIALOG (%s)->ok_button;\n",
	      child_name, wname);
  g_free (child_name);
  data->create_widget = FALSE;
  gb_widget_write_source (GTK_COLOR_SELECTION_DIALOG (widget)->ok_button,
			  data);

  child_name = gtk_widget_get_name (GTK_COLOR_SELECTION_DIALOG (widget)->cancel_button);
  child_name = source_create_valid_identifier (child_name);
  source_add (data, "  %s = GTK_COLOR_SELECTION_DIALOG (%s)->cancel_button;\n",
	      child_name, wname);
  g_free (child_name);
  data->create_widget = FALSE;
  gb_widget_write_source (GTK_COLOR_SELECTION_DIALOG (widget)->cancel_button,
			  data);

  child_name = gtk_widget_get_name (GTK_COLOR_SELECTION_DIALOG (widget)->help_button);
  child_name = source_create_valid_identifier (child_name);
  source_add (data, "  %s = GTK_COLOR_SELECTION_DIALOG (%s)->help_button;\n",
	      child_name, wname);
  g_free (child_name);
  data->create_widget = FALSE;
  gb_widget_write_source (GTK_COLOR_SELECTION_DIALOG (widget)->help_button,
			  data);

  g_free (wname);

  data->write_children = FALSE;
}



static GtkWidget *
gb_color_selection_dialog_get_child (GtkWidget * widget,
				     const gchar * child_name)
{
  if (!strcmp (child_name, ColorSelOKButton))
    return GTK_COLOR_SELECTION_DIALOG (widget)->ok_button;
  else if (!strcmp (child_name, ColorSelCancelButton))
    return GTK_COLOR_SELECTION_DIALOG (widget)->cancel_button;
  else if (!strcmp (child_name, ColorSelHelpButton))
    return GTK_COLOR_SELECTION_DIALOG (widget)->help_button;
  else
    return NULL;
}


/*
 * Initializes the GbWidget structure.
 * I've placed this at the end of the file so we don't have to include
 * declarations of all the functions.
 */
GbWidget *
gb_color_selection_dialog_init ()
{
  /* Initialise the GTK type */
  gtk_color_selection_dialog_get_type ();

  /* Initialize the GbWidget structure */
  gb_widget_init_struct (&gbwidget);

  /* Fill in the pixmap struct & tooltip */
  gbwidget.pixmap_struct = colorseldialog_xpm;
  gbwidget.tooltip = _("Color Selection Dialog");

  /* Fill in any functions that this GbWidget has */
  gbwidget.gb_widget_new = gb_color_selection_dialog_new;
  gbwidget.gb_widget_create_properties = gb_color_selection_dialog_create_properties;
  gbwidget.gb_widget_get_properties = gb_color_selection_dialog_get_properties;
  gbwidget.gb_widget_set_properties = gb_color_selection_dialog_set_properties;
  gbwidget.gb_widget_get_child = gb_color_selection_dialog_get_child;
  gbwidget.gb_widget_write_source = gb_color_selection_dialog_write_source;
/*
   gbwidget.gb_widget_create_popup_menu = gb_color_selection_dialog_create_popup_menu;
 */

  return &gbwidget;
}
