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

#include <gnome.h>
#include "../gb.h"

/* Include the 21x21 icon pixmap for this widget, to be used in the palette */
#include "../graphics/gtk-dial.xpm"

/*
 * GtkDial - note that we only set the value & the upper bound. If the lower
 *	     bound is set to anything other than 0, the percentage displayed
 *	     is incorrect.
 */

/*
 * This is the GbWidget struct for this widget (see ../gbwidget.h).
 * It is initialized in the init() function at the end of this file
 */
static GbWidget gbwidget;

static gchar *UpdatePolicy = "GtkDial::update_policy";
static gchar *ViewOnly = "GtkDial::view_only";

static const gchar *Values[] =
{
  "GtkDial::value",
  "GtkDial::lower",
  "GtkDial::upper",
  "GtkDial::step",
  "GtkDial::page",
  "GtkDial::page_size",
};

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

static gboolean gb_dial_button_release (GtkWidget *widget,
					GdkEventButton *event,
					gpointer user_data);

/******
 * NOTE: To use these functions you need to uncomment them AND add a pointer
 * to the function in the GbWidget struct at the end of this file.
 ******/

/*
 * Creates a new GtkWidget of class GtkDial, performing any specialized
 * initialization needed for the widget to work correctly in this environment.
 * If a dialog box is used to initialize the widget, return NULL from this
 * function, and call data->callback with your new widget when it is done.
 * If the widget needs a special destroy handler, add a signal here.
 */
static GtkWidget*
gb_dial_new (GbWidgetNewData *data)
{
  GtkWidget *new_widget;

  new_widget = gtk_dial_new (GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 100,
								 0, 0, 0)));

  gtk_signal_connect_after (GTK_OBJECT (new_widget), "button_release_event",
			    GTK_SIGNAL_FUNC (gb_dial_button_release), NULL);

  return new_widget;
}


/* This is called when the mouse button is released, which may be after the
   dial has been dragged. We need to redraw it properly and show the new value.
*/
static gboolean
gb_dial_button_release (GtkWidget *widget, GdkEventButton *event,
			gpointer user_data)
{
  /* Redraw the widget, so that the selection is redrawn properly. */
  editor_refresh_widget (widget);

  /* If the widget's properties are being shown, update the value. */
  if (property_get_widget () == widget)
    {
      property_set_auto_apply (FALSE);
      property_set_float (Values[0], GTK_DIAL (widget)->adjustment->value);
      property_set_auto_apply (TRUE);
    }

  return FALSE;
}


/*
 * Creates the components needed to edit the extra properties of this widget.
 */
static void
gb_dial_create_properties (GtkWidget * widget, GbWidgetCreateArgData * data)
{
  property_add_bool (ViewOnly, _("View Only:"),
		     _("If the dial can not be changed by the user"));
  property_add_choice (UpdatePolicy, _("Policy:"),
		       _("The update policy of the dial"), GbPolicyChoices);
  property_add_adjustment (Values, GB_ADJUST_DEFAULT_LABELS);
  property_set_visible (Values[1], FALSE);
  property_set_visible (Values[3], FALSE);
  property_set_visible (Values[4], FALSE);
  property_set_visible (Values[5], FALSE);
}



/*
 * Gets the properties of the widget. This is used for both displaying the
 * properties in the property editor, and also for saving the properties.
 */
static void
gb_dial_get_properties (GtkWidget *widget, GbWidgetGetArgData * data)
{
  gint i;

  gb_widget_output_bool (data, ViewOnly, GTK_DIAL (widget)->view_only);

  for (i = 0; i < sizeof (GbPolicyValues) / sizeof (GbPolicyValues[0]); i++)
    {
      if (GbPolicyValues[i] == GTK_DIAL (widget)->policy)
	gb_widget_output_choice (data, UpdatePolicy, i, GbPolicySymbols[i]);
    }

  gb_widget_output_adjustment (data, Values, GTK_DIAL (widget)->adjustment);
}



/*
 * Sets the properties of the widget. This is used for both applying the
 * properties changed in the property editor, and also for loading.
 */
static void
gb_dial_set_properties (GtkWidget * widget, GbWidgetSetArgData * data)
{
  gboolean view_only;
  gchar *policy;
  gint i;

  view_only = gb_widget_input_bool (data, ViewOnly);
  if (data->apply)
    {
      gtk_dial_set_view_only (GTK_DIAL (widget), view_only);
    }

  policy = gb_widget_input_choice (data, UpdatePolicy);
  if (data->apply)
    {
      for (i = 0; i < sizeof (GbPolicyValues) / sizeof (GbPolicyValues[0]);
	   i++)
	{
	  if (!strcmp (policy, GbPolicyChoices[i])
	      || !strcmp (policy, GbPolicySymbols[i]))
	    {
	      gtk_dial_set_update_policy (GTK_DIAL (widget),
					  GbPolicyValues[i]);
	      break;
	    }
	}
    }

  if (gb_widget_input_adjustment (data, Values, GTK_DIAL (widget)->adjustment))
    gtk_signal_emit_by_name (GTK_OBJECT (GTK_DIAL (widget)->adjustment),
			     "changed");
}



/*
 * Adds menu items to a context menu which is just about to appear!
 * Add commands to aid in editing a GtkDial, with signals pointing to
 * other functions in this file.
 */
/*
static void
gb_dial_create_popup_menu (GtkWidget * widget, GbWidgetCreateMenuData * data)
{

}
*/



/*
 * Writes the source code needed to create this widget.
 * You have to output everything necessary to create the widget here, though
 * there are some convenience functions to help.
 */
static void
gb_dial_write_source (GtkWidget * widget, GbWidgetWriteSourceData * data)
{
  GtkAdjustment *adj = GTK_DIAL (widget)->adjustment;
  gint i;

  if (data->create_widget)
    {
      source_add (data,
		  "  %s = gtk_dial_new (GTK_ADJUSTMENT (gtk_adjustment_new (%g, 0.0, %g, 0.0, 0.0, 0.0)));\n",
		  data->wname, adj->value, adj->upper);
    }

  gb_widget_write_standard_source (widget, data);

  if (GTK_DIAL (widget)->view_only)
    {
      source_add (data,
		  "  gtk_dial_set_view_only (GTK_DIAL (%s), TRUE);\n",
		  data->wname);
    }

  if (GTK_DIAL (widget)->policy != GTK_UPDATE_CONTINUOUS)
    {
      for (i = 0; i < sizeof (GbPolicyValues) / sizeof (GbPolicyValues[0]);
	   i++)
	{
	  if (GbPolicyValues[i] == GTK_DIAL (widget)->policy)
	    source_add (data,
			"  gtk_dial_set_update_policy (GTK_DIAL (%s), %s);\n",
			data->wname, GbPolicySymbols[i]);
	}
    }
}



/*
 * Initializes the GbWidget structure.
 * I've placed this at the end of the file so we don't have to include
 * declarations of all the functions.
 */
GbWidget*
gb_dial_init ()
{
  /* Initialise the GTK type */
  gtk_dial_get_type();

  /* Initialize the GbWidget structure */
  gb_widget_init_struct(&gbwidget);

  /* Fill in the pixmap struct & tooltip */
  gbwidget.pixmap_struct = gtk_dial_xpm;
  gbwidget.tooltip = _("Dial");

  /* Fill in any functions that this GbWidget has */
  gbwidget.gb_widget_new		= gb_dial_new;
  gbwidget.gb_widget_create_properties	= gb_dial_create_properties;
  gbwidget.gb_widget_get_properties	= gb_dial_get_properties;
  gbwidget.gb_widget_set_properties	= gb_dial_set_properties;
  gbwidget.gb_widget_write_source	= gb_dial_write_source;
/*
  gbwidget.gb_widget_create_popup_menu	= gb_dial_create_popup_menu;
*/

  return &gbwidget;
}

