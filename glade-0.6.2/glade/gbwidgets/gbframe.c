
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

#include <gtk/gtkframe.h>
#include "../gb.h"

/* Include the 21x21 icon pixmap for this widget, to be used in the palette */
#include "../graphics/frame.xpm"

/*
 * This is the GbWidget struct for this widget (see ../gbwidget.h).
 * It is initialized in the init() function at the end of this file
 */
static GbWidget gbwidget;

static gchar *Label = "GtkFrame::label";
static gchar *LabelXAlign = "GtkFrame::label_xalign";
/* Frame's yalign is never used */
/*static gchar *LabelYAlign = "GtkFrame::label_yalign"; */
static gchar *Shadow = "GtkFrame::shadow_type";

static const gchar *GbShadowChoices[] =
{"None", "In", "Out",
 "Etched In", "Etched Out", NULL};
static const gint GbShadowValues[] =
{
  GTK_SHADOW_NONE,
  GTK_SHADOW_IN,
  GTK_SHADOW_OUT,
  GTK_SHADOW_ETCHED_IN,
  GTK_SHADOW_ETCHED_OUT
};
static const gchar *GbShadowSymbols[] =
{
  "GTK_SHADOW_NONE",
  "GTK_SHADOW_IN",
  "GTK_SHADOW_OUT",
  "GTK_SHADOW_ETCHED_IN",
  "GTK_SHADOW_ETCHED_OUT"
};

/******
 * NOTE: To use these functions you need to uncomment them AND add a pointer
 * to the function in the GbWidget struct at the end of this file.
 ******/

/*
 * Creates a new GtkWidget of class GtkFrame, performing any specialized
 * initialization needed for the widget to work correctly in this environment.
 * If a dialog box is used to initialize the widget, return NULL from this
 * function, and call data->callback with your new widget when it is done.
 * If the widget needs a special destroy handler, add a signal here.
 */
GtkWidget *
gb_frame_new (GbWidgetNewData * data)
{
  GtkWidget *new_widget = gtk_frame_new (NULL);
  if (data->action != GB_LOADING)
    gtk_container_add (GTK_CONTAINER (new_widget), editor_new_placeholder ());
  return new_widget;
}



/*
 * Creates the components needed to edit the extra properties of this widget.
 */
static void
gb_frame_create_properties (GtkWidget * widget, GbWidgetCreateArgData * data)
{
  property_add_string (Label, _("Label:"), _("The text to display"));
  property_add_float_range (LabelXAlign, _("Label X Align:"),
			    _("The horizontal alignment of the frame's label"),
			    0, 1, 0.01, 0.1, 0.01, 2);
  property_add_choice (Shadow, _("Shadow:"), _("The type of shadow of the frame"),
		       GbShadowChoices);
}



/*
 * Gets the properties of the widget. This is used for both displaying the
 * properties in the property editor, and also for saving the properties.
 */
static void
gb_frame_get_properties (GtkWidget * widget, GbWidgetGetArgData * data)
{
  gint i;

  gb_widget_output_translatable_string (data, Label,
					GTK_FRAME (widget)->label);
  gb_widget_output_float (data, LabelXAlign, GTK_FRAME (widget)->label_xalign);

  for (i = 0; i < sizeof (GbShadowValues) / sizeof (GbShadowValues[0]); i++)
    {
      if (GbShadowValues[i] == GTK_FRAME (widget)->shadow_type)
	gb_widget_output_choice (data, Shadow, i, GbShadowSymbols[i]);
    }
}



/*
 * Sets the properties of the widget. This is used for both applying the
 * properties changed in the property editor, and also for loading.
 */
static void
gb_frame_set_properties (GtkWidget * widget, GbWidgetSetArgData * data)
{
  gchar *label;
  gfloat label_xalign;
  gchar *shadow;
  gint i;

  label = gb_widget_input_string (data, Label);
  if (data->apply)
    gtk_frame_set_label (GTK_FRAME (widget), label);

  shadow = gb_widget_input_choice (data, Shadow);
  if (data->apply)
    {
      for (i = 0; i < sizeof (GbShadowValues) / sizeof (GbShadowValues[0]); i
	   ++)
	{
	  if (!strcmp (shadow, GbShadowChoices[i])
	      || !strcmp (shadow, GbShadowSymbols[i]))
	    {
	      gtk_frame_set_shadow_type (GTK_FRAME (widget), GbShadowValues[i]);
	      break;
	    }
	}
    }

  label_xalign = gb_widget_input_float (data, LabelXAlign);
  if (data->apply)
    gtk_frame_set_label_align (GTK_FRAME (widget), label_xalign, 0.5);
}



/*
 * Adds menu items to a context menu which is just about to appear!
 * Add commands to aid in editing a GtkFrame, with signals pointing to
 * other functions in this file.
 */
/*
   static void
   gb_frame_create_popup_menu(GtkWidget *widget, GbWidgetCreateMenuData *data)
   {

   }
 */



/*
 * Writes the source code needed to create this widget.
 * You have to output everything necessary to create the widget here, though
 * there are some convenience functions to help.
 */
static void
gb_frame_write_source (GtkWidget * widget, GbWidgetWriteSourceData * data)
{
  gfloat xalign;
  gint shadow = 0, i;
  gchar *label = GTK_FRAME (widget)->label;

  if (data->create_widget)
    {
      if (label)
	source_add (data, "  %s = gtk_frame_new (%s);\n", data->wname,
		    source_make_string (label, data->use_gettext));
      else
	source_add (data, "  %s = gtk_frame_new (NULL);\n", data->wname);
    }
  gb_widget_write_standard_source (widget, data);

  xalign = GTK_FRAME (widget)->label_xalign;
  if (xalign)
    {
      source_add (data,
		  "  gtk_frame_set_label_align (GTK_FRAME (%s), %g, 0.5);\n",
		  data->wname, xalign);
    }

  if (GTK_FRAME (widget)->shadow_type != GTK_SHADOW_ETCHED_IN)
    {
      for (i = 0; i < sizeof (GbShadowValues) / sizeof (GbShadowValues[0]); i
	   ++)
	if (GbShadowValues[i] == GTK_FRAME (widget)->shadow_type)
	  {
	    shadow = i;
	    break;
	  }
      source_add (data, "  gtk_frame_set_shadow_type (GTK_FRAME (%s), %s);\n",
		  data->wname, GbShadowSymbols[shadow]);
    }
}



/*
 * Initializes the GbWidget structure.
 * I've placed this at the end of the file so we don't have to include
 * declarations of all the functions.
 */
GbWidget *
gb_frame_init ()
{
  /* Initialise the GTK type */
  gtk_frame_get_type ();

  /* Initialize the GbWidget structure */
  gb_widget_init_struct (&gbwidget);

  /* Fill in the pixmap struct & tooltip */
  gbwidget.pixmap_struct = frame_xpm;
  gbwidget.tooltip = _("Frame");

  /* Fill in any functions that this GbWidget has */
  gbwidget.gb_widget_new = gb_frame_new;
  gbwidget.gb_widget_create_properties = gb_frame_create_properties;
  gbwidget.gb_widget_get_properties = gb_frame_get_properties;
  gbwidget.gb_widget_set_properties = gb_frame_set_properties;
/*
   gbwidget.gb_widget_create_popup_menu = gb_frame_create_popup_menu;
 */
  gbwidget.gb_widget_write_source = gb_frame_write_source;

  return &gbwidget;
}
