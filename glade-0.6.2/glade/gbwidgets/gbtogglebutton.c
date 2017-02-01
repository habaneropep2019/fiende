
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

#include <gtk/gtklabel.h>
#include <gtk/gtktogglebutton.h>
#include "../gb.h"

/* Include the 21x21 icon pixmap for this widget, to be used in the palette */
#include "../graphics/togglebutton.xpm"

/*
 * This is the GbWidget struct for this widget (see ../gbwidget.h).
 * It is initialized in the init() function at the end of this file
 */
static GbWidget gbwidget;

static gchar *Label = "ToggleButton|GtkButton::label";
/* This is only used for toolbar buttons. */
static gchar *Icon = "ToggleButton|GtkButton::icon";

/* This is only used for normal buttons, not toolbar buttons. */
static gchar *Relief = "ToggleButton|GtkButton::relief";

static gchar *State = "GtkToggleButton::active";


static const gchar *GbReliefChoices[] =
{
  "Normal",
  "Half",
  "None",
  NULL
};
static const gint GbReliefValues[] =
{
  GTK_RELIEF_NORMAL,
  GTK_RELIEF_HALF,
  GTK_RELIEF_NONE
};
static const gchar *GbReliefSymbols[] =
{
  "GTK_RELIEF_NORMAL",
  "GTK_RELIEF_HALF",
  "GTK_RELIEF_NONE"
};


/* draw_indicator is not really useful for toggle buttons. I think its only
   for radio/check buttons. */
/*static gchar *Indicator       = "GtkToggleButton::draw_indicator"; */


/******
 * NOTE: To use these functions you need to uncomment them AND add a pointer
 * to the function in the GbWidget struct at the end of this file.
 ******/

/*
 * Creates a new GtkWidget of class GtkToggleButton, performing any specialized
 * initialization needed for the widget to work correctly in this environment.
 * If a dialog box is used to initialize the widget, return NULL from this
 * function, and call data->callback with your new widget when it is done.
 * If the widget needs a special destroy handler, add a signal here.
 */
GtkWidget *
gb_toggle_button_new (GbWidgetNewData * data)
{
  GtkWidget *new_widget;

  /* SPECIAL CODE: to handle toolbar buttons. */
  if (data->parent && GTK_IS_TOOLBAR (data->parent))
    {
      new_widget = gb_toolbar_new_toolbar_button (data, GTK_TOOLBAR_CHILD_TOGGLEBUTTON);
      if (new_widget)
	return new_widget;
    }

  if (data->action == GB_CREATING)
    new_widget = gtk_toggle_button_new_with_label (data->name);
  else
    {
      new_widget = gtk_toggle_button_new ();
      gtk_container_add (GTK_CONTAINER (new_widget), editor_new_placeholder());
    }
  return new_widget;
}


/*
 * Creates the components needed to edit the extra properties of this widget.
 */
static void
gb_toggle_button_create_properties (GtkWidget * widget, GbWidgetCreateArgData
				    * data)
{
  property_add_text (Label, _("Label:"), _("The text to display"), 2);
  property_add_choice (Relief, _("Button Relief:"),
		       _("The relief style of the button"),
		       GbReliefChoices);
#ifdef USE_GNOME
  gb_button_create_child_icon_property (widget, data, Icon);
#else
  property_add_filename (Icon, _("Icon:"), _("The pixmap filename"));
#endif

  property_add_bool (State, _("Initially On:"),
		     _("If the toggle button is initially on"));
}



/*
 * Gets the properties of the widget. This is used for both displaying the
 * properties in the property editor, and also for saving the properties.
 */
static void
gb_toggle_button_get_properties (GtkWidget * widget, GbWidgetGetArgData * data)
{
  gboolean is_toolbar_button;
  gint i;

  is_toolbar_button = gb_toolbar_is_toolbar_button (widget);

  if (is_toolbar_button)
    {
      gb_toolbar_output_child_label (widget, data, Label);
      gb_toolbar_output_child_icon (widget, data, Icon);
    }
  else
    {
      gb_widget_output_child_label (widget, data, Label);
    }

  if (data->action == GB_SHOWING)
    {
      property_set_visible (Relief, !is_toolbar_button);
      property_set_visible (Icon, is_toolbar_button);
    }

  if (!is_toolbar_button)
    {
      for (i = 0; i < sizeof (GbReliefValues) / sizeof (GbReliefValues[0]); i++)
	{
	  if (GbReliefValues[i] == GTK_BUTTON (widget)->relief)
	    gb_widget_output_choice (data, Relief, i, GbReliefSymbols[i]);
	}
    }

  gb_widget_output_bool (data, State, data->widget_data->flags & GLADE_ACTIVE);
}



/*
 * Sets the properties of the widget. This is used for both applying the
 * properties changed in the property editor, and also for loading.
 */
static void
gb_toggle_button_set_properties (GtkWidget * widget, GbWidgetSetArgData * data)
{
  gboolean set_relief = FALSE;
  gchar *relief;
  gint i;
  gboolean state;

  if (gb_toolbar_is_toolbar_button (widget))
    {
      gb_toolbar_input_child_label (widget, data, Label);
      gb_toolbar_input_child_icon (widget, data, Icon);
    }
  else
    {
      gb_widget_input_child_label (widget, data, Label);
      set_relief = TRUE;
    }

  if (set_relief)
    {
      relief = gb_widget_input_choice (data, Relief);
      if (data->apply)
	{
	  for (i = 0; i < sizeof (GbReliefValues) / sizeof (GbReliefValues[0]);
	       i++)
	    {
	      if (!strcmp (relief, GbReliefChoices[i])
		  || !strcmp (relief, GbReliefSymbols[i]))
		{
		  gtk_button_set_relief (GTK_BUTTON (widget),
					 GbReliefValues[i]);
		  break;
		}
	    }
	}
    }

  state = gb_widget_input_bool (data, State);
  if (data->apply)
    {
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (widget), state);
      if (state)
	data->widget_data->flags |= GLADE_ACTIVE;
      else
	data->widget_data->flags &= ~GLADE_ACTIVE;
    }
}



/*
 * Adds menu items to a context menu which is just about to appear!
 * Add commands to aid in editing a GtkToggleButton, with signals pointing to
 * other functions in this file.
 */
static void
gb_toggle_button_create_popup_menu (GtkWidget * widget, GbWidgetCreateMenuData
				    * data)
{
  /* Add command to remove child label. */
  gb_widget_create_child_label_popup_menu (widget, data);

  /* Add commands for toolbar buttons. */
  gb_toolbar_create_toolbar_button_popup_menu (widget, data);
}



/*
 * Writes the source code needed to create this widget.
 * You have to output everything necessary to create the widget here, though
 * there are some convenience functions to help.
 */
static void
gb_toggle_button_write_source (GtkWidget * widget, GbWidgetWriteSourceData * data)
{
  GtkWidget *child = GTK_BUTTON (widget)->child;
  gchar *label_text;
  gboolean set_relief = TRUE;
  gint i;

  if (data->create_widget)
    {
      if (gb_toolbar_is_toolbar_button (widget))
	{
	  gb_toolbar_write_toolbar_button_source (widget, data);
	  set_relief = FALSE;
	}
      else if (child && GTK_IS_LABEL (child) && !GB_IS_GB_WIDGET (child))
	{
	  label_text = glade_util_get_label_text (GTK_BIN (widget)->child);
	  /* If there is an underlined accelerator, set up the accel signal. */
	  if (strchr (label_text, '_'))
	    {
	      source_add (data,
			  "  %s = gtk_toggle_button_new_with_label (\"\");\n",
			  data->wname);
	      gb_button_write_uline_accel_source (widget, data, label_text);
	    }
	  else
	    {
	      source_add (data,
			  "  %s = gtk_toggle_button_new_with_label (%s);\n",
			  data->wname,
			  source_make_string (label_text, data->use_gettext));
	    }
	  g_free (label_text);
	}
      else
	{
	  source_add (data, "  %s = gtk_toggle_button_new ();\n", data->wname);
	}
    }
  gb_widget_write_standard_source (widget, data);

  if (set_relief && GTK_BUTTON (widget)->relief != GTK_RELIEF_NORMAL)
    {
      for (i = 0; i < sizeof (GbReliefValues) / sizeof (GbReliefValues[0]);
	   i++)
	{
	  if (GbReliefValues[i] == GTK_BUTTON (widget)->relief)
	    source_add (data,
			"  gtk_button_set_relief (GTK_BUTTON (%s), %s);\n",
			data->wname, GbReliefSymbols[i]);
	}
    }

  if (data->widget_data->flags & GLADE_ACTIVE)
    {
      source_add (data,
	  "  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (%s), TRUE);\n",
		  data->wname);
    }
}


static void
gb_toggle_button_destroy (GtkWidget * widget, GbWidgetDestroyData * data)
{
  gb_toolbar_destroy_child_icon (widget, data);
}



/*
 * Initializes the GbWidget structure.
 * I've placed this at the end of the file so we don't have to include
 * declarations of all the functions.
 */
GbWidget *
gb_toggle_button_init ()
{
  /* Initialise the GTK type */
  gtk_toggle_button_get_type ();

  /* Initialize the GbWidget structure */
  gb_widget_init_struct (&gbwidget);

  /* Fill in the pixmap struct & tooltip */
  gbwidget.pixmap_struct = togglebutton_xpm;
  gbwidget.tooltip = _("Toggle Button");

  /* Fill in any functions that this GbWidget has */
  gbwidget.gb_widget_new = gb_toggle_button_new;
  gbwidget.gb_widget_create_properties = gb_toggle_button_create_properties;
  gbwidget.gb_widget_get_properties = gb_toggle_button_get_properties;
  gbwidget.gb_widget_set_properties = gb_toggle_button_set_properties;
  gbwidget.gb_widget_create_popup_menu = gb_toggle_button_create_popup_menu;
  gbwidget.gb_widget_write_source = gb_toggle_button_write_source;
  gbwidget.gb_widget_destroy = gb_toggle_button_destroy;

  return &gbwidget;
}
