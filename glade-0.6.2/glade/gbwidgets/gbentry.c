
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
#include <gtk/gtkentry.h>
#include "../gb.h"

/* Include the 21x21 icon pixmap for this widget, to be used in the palette */
#include "../graphics/entry.xpm"

/*
 * This is the GbWidget struct for this widget (see ../gbwidget.h).
 * It is initialized in the init() function at the end of this file
 */
static GbWidget gbwidget;

static gchar *Editable = "Entry|GtkEditable::editable";
static gchar *Visible = "GtkEntry::text_visible";
static gchar *MaxLength = "GtkEntry::text_max_length";
static gchar *Text = "GtkEntry::text";


/******
 * NOTE: To use these functions you need to uncomment them AND add a pointer
 * to the function in the GbWidget struct at the end of this file.
 ******/

/*
 * Creates a new GtkWidget of class GtkEntry, performing any specialized
 * initialization needed for the widget to work correctly in this environment.
 * If a dialog box is used to initialize the widget, return NULL from this
 * function, and call data->callback with your new widget when it is done.
 * If the widget needs a special destroy handler, add a signal here.
 */
/*
   GtkWidget*
   gb_entry_new(GbWidgetNewData *data)
   {

   }
 */



/*
 * Creates the components needed to edit the extra properties of this widget.
 */
static void
gb_entry_create_properties (GtkWidget * widget, GbWidgetCreateArgData * data)
{
  property_add_bool (Editable, _("Editable:"), _("If the text can be edited"));
  property_add_bool (Visible, _("Text Visible:"),
		     _("If the text entered by the user will be shown. When turned off, the text typed in is displayed as asterix characters, which is useful for entering passwords"));
  property_add_int_range (MaxLength, _("Max Length:"),
			  _("The maximum length of the text"),
			  0, 10000, 1, 10, 1);
  property_add_string (Text, _("Text:"), _("The text to display"));
}



/*
 * Gets the properties of the widget. This is used for both displaying the
 * properties in the property editor, and also for saving the properties.
 */
static void
gb_entry_get_properties (GtkWidget * widget, GbWidgetGetArgData * data)
{
  gchar *entry_text = gtk_entry_get_text (GTK_ENTRY (widget));
  gb_widget_output_bool (data, Editable, GTK_EDITABLE (widget)->editable);
  gb_widget_output_bool (data, Visible, GTK_ENTRY (widget)->visible);
  gb_widget_output_int (data, MaxLength, GTK_ENTRY (widget)->text_max_length);
  gb_widget_output_translatable_string (data, Text, entry_text);
}



/*
 * Sets the properties of the widget. This is used for both applying the
 * properties changed in the property editor, and also for loading.
 */
static void
gb_entry_set_properties (GtkWidget * widget, GbWidgetSetArgData * data)
{
  gboolean editable, visible;
  gint max_length;
  gchar *text;

  editable = gb_widget_input_bool (data, Editable);
  if (data->apply)
    gtk_entry_set_editable (GTK_ENTRY (widget), editable);

  visible = gb_widget_input_bool (data, Visible);
  if (data->apply)
    gtk_entry_set_visibility (GTK_ENTRY (widget), visible);

  max_length = gb_widget_input_int (data, MaxLength);
  if (data->apply)
    gtk_entry_set_max_length (GTK_ENTRY (widget), max_length);

  text = gb_widget_input_string (data, Text);
  if (data->apply)
    gtk_entry_set_text (GTK_ENTRY (widget), text);
}



/*
 * Adds menu items to a context menu which is just about to appear!
 * Add commands to aid in editing a GtkEntry, with signals pointing to
 * other functions in this file.
 */
/*
   static void
   gb_entry_create_popup_menu(GtkWidget *widget, GbWidgetCreateMenuData *data)
   {

   }
 */



/*
 * Writes the source code needed to create this widget.
 * You have to output everything necessary to create the widget here, though
 * there are some convenience functions to help.
 */
static void
gb_entry_write_source (GtkWidget * widget, GbWidgetWriteSourceData * data)
{
  guint16 max_len = GTK_ENTRY (widget)->text_max_length;
  gchar *entry_text = gtk_entry_get_text (GTK_ENTRY (widget));

  if (data->create_widget)
    {
      if (max_len)
	{
	  source_add (data, "  %s = gtk_entry_new_with_max_length (%d);\n",
		      data->wname, max_len);
	}
      else
	{
	  source_add (data, "  %s = gtk_entry_new ();\n", data->wname);
	}
    }
  gb_widget_write_standard_source (widget, data);

  if (!GTK_EDITABLE (widget)->editable)
    {
      source_add (data, "  gtk_entry_set_editable (GTK_ENTRY (%s), FALSE);\n",
		  data->wname);
    }
  if (!GTK_ENTRY (widget)->visible)
    {
      source_add (data, "  gtk_entry_set_visibility (GTK_ENTRY (%s), FALSE);\n",
		  data->wname);
    }
  if (entry_text && strlen (entry_text) > 0)
    {
      source_add (data, "  gtk_entry_set_text (GTK_ENTRY (%s), %s);\n",
		  data->wname,
		  source_make_string (entry_text, data->use_gettext));
    }
}



/*
 * Initializes the GbWidget structure.
 * I've placed this at the end of the file so we don't have to include
 * declarations of all the functions.
 */
GbWidget *
gb_entry_init ()
{
  /* Initialise the GTK type */
  gtk_entry_get_type ();

  /* Initialize the GbWidget structure */
  gb_widget_init_struct (&gbwidget);

  /* Fill in the pixmap struct & tooltip */
  gbwidget.pixmap_struct = entry_xpm;
  gbwidget.tooltip = _("Text Entry");

  /* Fill in any functions that this GbWidget has */
  gbwidget.gb_widget_create_properties = gb_entry_create_properties;
  gbwidget.gb_widget_get_properties = gb_entry_get_properties;
  gbwidget.gb_widget_set_properties = gb_entry_set_properties;
/*
   gbwidget.gb_widget_new               = gb_entry_new;
   gbwidget.gb_widget_create_popup_menu = gb_entry_create_popup_menu;
 */
  gbwidget.gb_widget_write_source = gb_entry_write_source;

  return &gbwidget;
}
