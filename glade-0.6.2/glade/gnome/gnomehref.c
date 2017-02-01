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
#include "../graphics/gnome-href.xpm"

/*
 * This is the GbWidget struct for this widget (see ../gbwidget.h).
 * It is initialized in the init() function at the end of this file
 */
static GbWidget gbwidget;

static gchar *HRefURL = "GnomeHRef::url";
static gchar *HRefLabel = "GnomeHRef::label";


/******
 * NOTE: To use these functions you need to uncomment them AND add a pointer
 * to the function in the GbWidget struct at the end of this file.
 ******/

/*
 * Creates a new GtkWidget of class GnomeHRef, performing any specialized
 * initialization needed for the widget to work correctly in this environment.
 * If a dialog box is used to initialize the widget, return NULL from this
 * function, and call data->callback with your new widget when it is done.
 * If the widget needs a special destroy handler, add a signal here.
 */
/*
static GtkWidget*
gb_gnome_href_new (GbWidgetNewData *data)
{

}
*/



/*
 * Creates the components needed to edit the extra properties of this widget.
 */
static void
gb_gnome_href_create_properties (GtkWidget * widget, GbWidgetCreateArgData * data)
{
  property_add_string (HRefURL, _("URL:"),
		       _("The URL to display when the button is clicked"));
  property_add_text (HRefLabel, _("Label:"),
		     _("The text to display in the button"), 2);
}



/*
 * Gets the properties of the widget. This is used for both displaying the
 * properties in the property editor, and also for saving the properties.
 */
static void
gb_gnome_href_get_properties (GtkWidget *widget, GbWidgetGetArgData * data)
{
  gchar *label_text;

  gb_widget_output_string (data, HRefURL, GNOME_HREF (widget)->url);

  gtk_label_get (GTK_LABEL (GNOME_HREF (widget)->label), &label_text);
  gb_widget_output_translatable_text (data, HRefLabel, label_text);
}



/*
 * Sets the properties of the widget. This is used for both applying the
 * properties changed in the property editor, and also for loading.
 */
static void
gb_gnome_href_set_properties (GtkWidget * widget, GbWidgetSetArgData * data)
{
  gchar *url, *label_text;

  url = gb_widget_input_string (data, HRefURL);
  if (data->apply)
    gnome_href_set_url (GNOME_HREF (widget), url && url[0] ? url : NULL);

  label_text = gb_widget_input_text (data, HRefLabel);
  if (data->apply)
    gnome_href_set_label (GNOME_HREF (widget), label_text);
  if (data->action == GB_APPLYING)
    g_free (label_text);
}



/*
 * Adds menu items to a context menu which is just about to appear!
 * Add commands to aid in editing a GnomeHRef, with signals pointing to
 * other functions in this file.
 */
/*
static void
gb_gnome_href_create_popup_menu (GtkWidget * widget, GbWidgetCreateMenuData * data)
{

}
*/



/*
 * Writes the source code needed to create this widget.
 * You have to output everything necessary to create the widget here, though
 * there are some convenience functions to help.
 */
static void
gb_gnome_href_write_source (GtkWidget * widget, GbWidgetWriteSourceData * data)
{
  if (data->create_widget)
    {
      gchar *url, *label_text;
 
      gtk_label_get (GTK_LABEL (GNOME_HREF (widget)->label), &label_text);
      url = GNOME_HREF (widget)->url;
      if (url)
	url = g_strdup (source_make_string (url, FALSE));

      source_add (data, "  %s = gnome_href_new (%s, %s);\n", data->wname,
		  url ? url : "NULL",
		  source_make_string (label_text, data->use_gettext));
      g_free (url);
    }

  gb_widget_write_standard_source (widget, data);
}



/*
 * Initializes the GbWidget structure.
 * I've placed this at the end of the file so we don't have to include
 * declarations of all the functions.
 */
GbWidget*
gb_gnome_href_init ()
{
  /* Initialise the GTK type */
  gnome_href_get_type();

  /* Initialize the GbWidget structure */
  gb_widget_init_struct(&gbwidget);

  /* Fill in the pixmap struct & tooltip */
  gbwidget.pixmap_struct = gnome_href_xpm;
  gbwidget.tooltip = _("Gnome HRef Link Button");

  /* Fill in any functions that this GbWidget has */
  gbwidget.gb_widget_create_properties	= gb_gnome_href_create_properties;
  gbwidget.gb_widget_get_properties	= gb_gnome_href_get_properties;
  gbwidget.gb_widget_set_properties	= gb_gnome_href_set_properties;
  gbwidget.gb_widget_write_source	= gb_gnome_href_write_source;
/*
  gbwidget.gb_widget_new		= gb_gnome_href_new;
  gbwidget.gb_widget_create_popup_menu	= gb_gnome_href_create_popup_menu;
*/

  return &gbwidget;
}

