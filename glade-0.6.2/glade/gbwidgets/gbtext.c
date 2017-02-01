
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
#include <gtk/gtktext.h>
#include "../gb.h"

/* Include the 21x21 icon pixmap for this widget, to be used in the palette */
#include "../graphics/text.xpm"

/*
 * This is the GbWidget struct for this widget (see ../gbwidget.h).
 * It is initialized in the init() function at the end of this file
 */
static GbWidget gbwidget;

static gchar *Editable = "Text|GtkEditable::editable";
static gchar *Text = "GtkText::text";


/******
 * NOTE: To use these functions you need to uncomment them AND add a pointer
 * to the function in the GbWidget struct at the end of this file.
 ******/

/*
 * Creates a new GtkWidget of class GtkText, performing any specialized
 * initialization needed for the widget to work correctly in this environment.
 * If a dialog box is used to initialize the widget, return NULL from this
 * function, and call data->callback with your new widget when it is done.
 * If the widget needs a special destroy handler, add a signal here.
 */
GtkWidget *
gb_text_new (GbWidgetNewData * data)
{
  return gtk_text_new (NULL, NULL);

}



/*
 * Creates the components needed to edit the extra properties of this widget.
 */
static void
gb_text_create_properties (GtkWidget * widget, GbWidgetCreateArgData * data)
{
  property_add_bool (Editable, _("Editable:"), _("If the text can be edited"));
  property_add_text (Text, _("Text:"), _("The text to display"), 5);
}



/*
 * Gets the properties of the widget. This is used for both displaying the
 * properties in the property editor, and also for saving the properties.
 */
static void
gb_text_get_properties (GtkWidget * widget, GbWidgetGetArgData * data)
{
  gchar *text;
  gb_widget_output_bool (data, Editable, GTK_EDITABLE (widget)->editable);
  text = gtk_editable_get_chars (GTK_EDITABLE (widget), 0, -1);
  gb_widget_output_translatable_text (data, Text, text);
  g_free (text);
}



/*
 * Sets the properties of the widget. This is used for both applying the
 * properties changed in the property editor, and also for loading.
 */
static void
gb_text_set_properties (GtkWidget * widget, GbWidgetSetArgData * data)
{
  gboolean editable;
  gchar *text;

  editable = gb_widget_input_bool (data, Editable);
  if (data->apply)
    gtk_text_set_editable (GTK_TEXT (widget), editable);

  text = gb_widget_input_text (data, Text);
  if (data->apply)
    {
      gtk_editable_delete_text (GTK_EDITABLE (widget), 0, -1);
      text = text ? text : "";
      gtk_text_insert (GTK_TEXT (widget), NULL, NULL, NULL, text, strlen (text));
    }
  if (data->action == GB_APPLYING)
    g_free (text);
}



/*
 * Adds menu items to a context menu which is just about to appear!
 * Add commands to aid in editing a GtkText, with signals pointing to
 * other functions in this file.
 */
/*
   static void
   gb_text_create_popup_menu(GtkWidget *widget, GbWidgetCreateMenuData *data)
   {

   }
 */



/*
 * Writes the source code needed to create this widget.
 * You have to output everything necessary to create the widget here, though
 * there are some convenience functions to help.
 */
static void
gb_text_write_source (GtkWidget * widget, GbWidgetWriteSourceData * data)
{
  gchar *text;

  if (data->create_widget)
    {
      source_add (data, "  %s = gtk_text_new (NULL, NULL);\n", data->wname);
    }
  gb_widget_write_standard_source (widget, data);

  if (GTK_EDITABLE (widget)->editable)
    {
      source_add (data, "  gtk_text_set_editable (GTK_TEXT (%s), TRUE);\n",
		  data->wname);
    }

  text = gtk_editable_get_chars (GTK_EDITABLE (widget), 0, -1);
  /* Note: there may be a limit to the size of the text we can include in
     source code, and it may be better to break it into lines. */
  if (text && strlen (text) > 0)
    {
      gchar *text_output = source_make_string (text, data->use_gettext);

      /* Note: we use the length of the text rather than the result of
         souce_make_string() since the escaped characters will be converted
         back to the original string when compiled. */
      source_add (data,
		  "  gtk_text_insert (GTK_TEXT (%s), NULL, NULL, NULL,\n"
		  "                   %s, %i);\n",
		  data->wname, text_output, strlen (text));
      g_free (text);
    }
}


/* This just redraws the entire widget when it is scrolled, to make sure that
   it isn't messed up due to our selection handles. */
static void
gb_text_adjustment_changed (GtkAdjustment *adjustment,
			    GtkWidget     *widget)
{
  /* We check that this is a widget in the interface being created rather
     than part of Glade's interface. */
  if (GB_IS_GB_WIDGET (widget))
    gtk_widget_queue_clear (widget);
}


static gboolean
gb_text_emission_hook (GtkObject *object,
		       guint signal_id,
		       guint n_params,
		       GtkArg *params,
		       gpointer data)
{
  GtkObject *hadjustment, *vadjustment;
  GtkObject *old_hadjustment, *old_vadjustment;

  g_return_val_if_fail (GTK_IS_TEXT (object), FALSE);

  hadjustment = GTK_VALUE_OBJECT (params[0]);
  vadjustment = GTK_VALUE_OBJECT (params[1]);

  old_hadjustment = gtk_object_get_data (object, "scrollhadjustment");
  if (hadjustment != old_hadjustment)
    {
      gtk_object_set_data (object, "scrollhadjustment", hadjustment);

      if (hadjustment)
	gtk_signal_connect (hadjustment, "value_changed",
			    (GtkSignalFunc) gb_text_adjustment_changed,
			    object);
    }

  old_vadjustment = gtk_object_get_data (object, "scrollvadjustment");
  if (vadjustment != old_vadjustment)
    {
      gtk_object_set_data (object, "scrollvadjustment", vadjustment);

      if (vadjustment)
	gtk_signal_connect (vadjustment, "value_changed",
			    (GtkSignalFunc) gb_text_adjustment_changed,
			    object);
    }

  return TRUE;
}



/*
 * Initializes the GbWidget structure.
 * I've placed this at the end of the file so we don't have to include
 * declarations of all the functions.
 */
GbWidget *
gb_text_init ()
{
  GtkWidgetClass *klass;

  /* Initialise the GTK type */
  gtk_text_get_type ();

  /* Add a signal emission hook so we can connect signal handlers to the
     scrollbar adjustments to redraw the GtkText when necessary. */
  klass = gtk_type_class (gtk_text_get_type ());
  gtk_signal_add_emission_hook (klass->set_scroll_adjustments_signal,
				gb_text_emission_hook, NULL);

  /* Initialize the GbWidget structure */
  gb_widget_init_struct (&gbwidget);

  /* Fill in the pixmap struct & tooltip */
  gbwidget.pixmap_struct = text_xpm;
  gbwidget.tooltip = _("Text Box");

  /* Fill in any functions that this GbWidget has */
  gbwidget.gb_widget_new = gb_text_new;
  gbwidget.gb_widget_create_properties = gb_text_create_properties;
  gbwidget.gb_widget_get_properties = gb_text_get_properties;
  gbwidget.gb_widget_set_properties = gb_text_set_properties;
/*
   gbwidget.gb_widget_create_popup_menu = gb_text_create_popup_menu;
 */
  gbwidget.gb_widget_write_source = gb_text_write_source;

  return &gbwidget;
}
