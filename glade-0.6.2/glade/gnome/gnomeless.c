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
#include "../graphics/gnome-less.xpm"

/*
 * This is the GbWidget struct for this widget (see ../gbwidget.h).
 * It is initialized in the init() function at the end of this file
 */
static GbWidget gbwidget;

static gchar *GnomeLessFont = "GnomeLess::font";

void gb_gnome_less_show_example_string (GtkWidget *widget);

/******
 * NOTE: To use these functions you need to uncomment them AND add a pointer
 * to the function in the GbWidget struct at the end of this file.
 ******/

/*
 * Creates a new GtkWidget of class GnomeLess, performing any specialized
 * initialization needed for the widget to work correctly in this environment.
 * If a dialog box is used to initialize the widget, return NULL from this
 * function, and call data->callback with your new widget when it is done.
 * If the widget needs a special destroy handler, add a signal here.
 */
static GtkWidget*
gb_gnome_less_new (GbWidgetNewData *data)
{
  GtkWidget *new_widget;

  new_widget = gnome_less_new ();
  gb_gnome_less_show_example_string (new_widget);
  return new_widget;
}



/* This shows an example string in the GnomeLess widget, so you can see
   what the font looks like. */
void
gb_gnome_less_show_example_string (GtkWidget *widget)
{
  gnome_less_show_string (GNOME_LESS (widget),
			  _("Example output from GnomeLess widget.\n"));
}


/*
 * Creates the components needed to edit the extra properties of this widget.
 */
static void
gb_gnome_less_create_properties (GtkWidget * widget, GbWidgetCreateArgData * data)
{
  property_add_font (GnomeLessFont, _("Font:"),
		     _("The font to use for the text"));
}



/*
 * Gets the properties of the widget. This is used for both displaying the
 * properties in the property editor, and also for saving the properties.
 */
static void
gb_gnome_less_get_properties (GtkWidget *widget, GbWidgetGetArgData * data)
{
  gchar *fontname;

  fontname = gtk_object_get_data (GTK_OBJECT (widget), GnomeLessFont);
  gb_widget_output_font (data, GnomeLessFont, NULL, fontname);
}



/*
 * Sets the properties of the widget. This is used for both applying the
 * properties changed in the property editor, and also for loading.
 */
static void
gb_gnome_less_set_properties (GtkWidget * widget, GbWidgetSetArgData * data)
{
  GdkFont *font;
  gchar *fontname;

  font = gb_widget_input_font (data, GnomeLessFont, &fontname);
  if (data->apply)
    {
      g_free (gtk_object_get_data (GTK_OBJECT (widget), GnomeLessFont));
      gtk_object_set_data (GTK_OBJECT (widget), GnomeLessFont,
			   g_strdup (fontname));

      gnome_less_set_font (GNOME_LESS (widget), font);
      gb_gnome_less_show_example_string (widget);
    }
}


/*
 * Adds menu items to a context menu which is just about to appear!
 * Add commands to aid in editing a GnomeLess, with signals pointing to
 * other functions in this file.
 */
/*
static void
gb_gnome_less_create_popup_menu (GtkWidget * widget, GbWidgetCreateMenuData * data)
{

}
*/



/*
 * Writes the source code needed to create this widget.
 * You have to output everything necessary to create the widget here, though
 * there are some convenience functions to help.
 */
static void
gb_gnome_less_write_source (GtkWidget * widget, GbWidgetWriteSourceData * data)
{
  gchar *fontname;

  if (data->create_widget)
    {
      source_add (data, "  %s = gnome_less_new ();\n", data->wname);
    }

  gb_widget_write_standard_source (widget, data);

  fontname = gtk_object_get_data (GTK_OBJECT (widget), GnomeLessFont);
  if (fontname)
    {
      source_add (data,
		  "  gnome_less_set_font (GNOME_LESS (%s),\n"
		  "                       gdk_font_load (%s));\n",
		  data->wname, source_make_string (fontname, FALSE));
    }
}


static void
gb_gnome_less_destroy (GtkWidget * widget, GbWidgetDestroyData * data)
{
  g_free (gtk_object_get_data (GTK_OBJECT (widget), GnomeLessFont));
}


/*
 * Initializes the GbWidget structure.
 * I've placed this at the end of the file so we don't have to include
 * declarations of all the functions.
 */
GbWidget*
gb_gnome_less_init ()
{
  /* Initialise the GTK type */
  gnome_less_get_type();

  /* Initialize the GbWidget structure */
  gb_widget_init_struct(&gbwidget);

  /* Fill in the pixmap struct & tooltip */
  gbwidget.pixmap_struct = gnome_less_xpm;
  gbwidget.tooltip = _("Gnome Less");

  /* Fill in any functions that this GbWidget has */
  gbwidget.gb_widget_new		= gb_gnome_less_new;
  gbwidget.gb_widget_create_properties	= gb_gnome_less_create_properties;
  gbwidget.gb_widget_get_properties	= gb_gnome_less_get_properties;
  gbwidget.gb_widget_set_properties	= gb_gnome_less_set_properties;
  gbwidget.gb_widget_write_source	= gb_gnome_less_write_source;
  gbwidget.gb_widget_destroy		= gb_gnome_less_destroy;
/*
  gbwidget.gb_widget_create_popup_menu	= gb_gnome_less_create_popup_menu;
*/

  return &gbwidget;
}

