
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

#include <math.h>

#include <gtk/gtkpixmap.h>
#include "../gb.h"

/* Include the 21x21 icon pixmap for this widget, to be used in the palette */
#include "../graphics/pixmap.xpm"

/*
 * This is the GbWidget struct for this widget (see ../gbwidget.h).
 * It is initialized in the init() function at the end of this file
 */
static GbWidget gbwidget;

static gchar *Filename = "GtkPixmap::filename";
static gchar *XAlign = "Pixmap|GtkMisc::xalign";
static gchar *YAlign = "Pixmap|GtkMisc::yalign";
static gchar *XPad = "Pixmap|GtkMisc::xpad";
static gchar *YPad = "Pixmap|GtkMisc::ypad";
static gchar *BuildInsensitive = "GtkPixmap::build_insensitive";

static gchar *GbFilenameKey = "GbFilename";

/******
 * NOTE: To use these functions you need to uncomment them AND add a pointer
 * to the function in the GbWidget struct at the end of this file.
 ******/

/*
 * Creates a new GtkWidget of class GtkPixmap, performing any specialized
 * initialization needed for the widget to work correctly in this environment.
 * If a dialog box is used to initialize the widget, return NULL from this
 * function, and call data->callback with your new widget when it is done.
 * If the widget needs a special destroy handler, add a signal here.
 */
GtkWidget *
gb_pixmap_new (GbWidgetNewData * data)
{
  GtkWidget *new_widget;
  new_widget = gtk_pixmap_new (gbwidget.gdkpixmap, gbwidget.mask);
  return new_widget;
}



/*
 * Creates the components needed to edit the extra properties of this widget.
 */
static void
gb_pixmap_create_properties (GtkWidget * widget, GbWidgetCreateArgData * data)
{
  property_add_filename (Filename, _("File:"), _("The pixmap filename"));
  property_add_float_range (XAlign, _("X Align:"),
			    _("The horizontal alignment"),
			    0, 1, 0.01, 0.1, 0.01, 2);
  property_add_float_range (YAlign, _("Y Align:"),
			    _("The vertical alignment"),
			    0, 1, 0.01, 0.1, 0.01, 2);
  property_add_int_range (XPad, _("X Pad:"), _("The horizontal padding"),
			  0, 1000, 1, 10, 1);
  property_add_int_range (YPad, _("Y Pad:"), _("The vertical padding"),
			  0, 1000, 1, 10, 1);
  property_add_bool (BuildInsensitive, _("Build Insensitive:"),
		     _("If an insensitive version of the pixmap is built automatically"));
}



/*
 * Gets the properties of the widget. This is used for both displaying the
 * properties in the property editor, and also for saving the properties.
 */
static void
gb_pixmap_get_properties (GtkWidget * widget, GbWidgetGetArgData * data)
{
  gchar *filename = gtk_object_get_data (GTK_OBJECT (widget), GbFilenameKey);
  gb_widget_output_pixmap_filename (data, Filename, filename);
  gb_widget_output_float (data, XAlign, GTK_MISC (widget)->xalign);
  gb_widget_output_float (data, YAlign, GTK_MISC (widget)->yalign);
  gb_widget_output_int (data, XPad, GTK_MISC (widget)->xpad);
  gb_widget_output_int (data, YPad, GTK_MISC (widget)->ypad);
  gb_widget_output_bool (data, BuildInsensitive,
			 GTK_PIXMAP (widget)->build_insensitive);
}



/*
 * Sets the properties of the widget. This is used for both applying the
 * properties changed in the property editor, and also for loading.
 */
static void
gb_pixmap_set_properties (GtkWidget * widget, GbWidgetSetArgData * data)
{
  gchar *filename, *old_filename;
  GdkColormap *colormap;
  GdkPixmap *gdkpixmap;
  GdkBitmap *mask;
  gfloat xalign, yalign;
  gint xpad, ypad;
  gboolean set_alignment = FALSE, set_padding = FALSE, build_insensitive;

  filename = gb_widget_input_pixmap_filename (data, Filename);
  if (data->apply)
    {
      if (filename && filename[0] == '\0')
	filename = NULL;

      old_filename = gtk_object_get_data (GTK_OBJECT (widget), GbFilenameKey);
      if (old_filename)
	{
	  glade_project_remove_pixmap (data->project, old_filename);
	  g_free (old_filename);
	}

      gtk_object_set_data (GTK_OBJECT (widget), GbFilenameKey,
			   g_strdup (filename));
      if (filename)
	{
	  glade_project_add_pixmap (data->project, filename);
	  colormap = gtk_widget_get_colormap (widget);
	  gdkpixmap = gdk_pixmap_colormap_create_from_xpm (NULL, colormap,
							   &mask, NULL,
							   filename);
	  if (!gdkpixmap)
	    {
	      if (data->action == GB_LOADING)
		{
		  load_add_error_message_with_tag (data,
						   GLADE_LINE_PROPERTY,
						   _("Couldn't create pixmap from file"),
						   Filename, filename);
		}
	      else
		{
		  glade_util_show_message_box (_("Couldn't create pixmap from file\n"), widget);
		}
	    }
	  else
	    {
	      gtk_pixmap_set (GTK_PIXMAP (widget), gdkpixmap, mask);
	      gdk_pixmap_unref (gdkpixmap);
	      gdk_bitmap_unref (mask);
	      editor_refresh_widget (widget);
	    }
	}
      else
	{
	  gtk_pixmap_set (GTK_PIXMAP (widget), gbwidget.gdkpixmap, gbwidget.mask);
	  editor_refresh_widget (widget);
	}
    }
  if (data->action == GB_LOADING)
    g_free (filename);

  xalign = gb_widget_input_float (data, XAlign);
  if (data->apply)
    set_alignment = TRUE;
  else
    xalign = GTK_MISC (widget)->xalign;

  yalign = gb_widget_input_float (data, YAlign);
  if (data->apply)
    set_alignment = TRUE;
  else
    yalign = GTK_MISC (widget)->yalign;

  if (set_alignment)
    gtk_misc_set_alignment (GTK_MISC (widget), xalign, yalign);

  xpad = gb_widget_input_int (data, XPad);
  if (data->apply)
    set_padding = TRUE;
  else
    xpad = GTK_MISC (widget)->xpad;

  ypad = gb_widget_input_int (data, YPad);
  if (data->apply)
    set_padding = TRUE;
  else
    ypad = GTK_MISC (widget)->ypad;

  if (set_padding)
    gtk_misc_set_padding (GTK_MISC (widget), xpad, ypad);

  build_insensitive = gb_widget_input_bool (data, BuildInsensitive);
  if (data->apply)
    {
      gtk_pixmap_set_build_insensitive (GTK_PIXMAP (widget),
					build_insensitive);
    }
}



/*
 * Adds menu items to a context menu which is just about to appear!
 * Add commands to aid in editing a GtkPixmap, with signals pointing to
 * other functions in this file.
 */
/*
   static void
   gb_pixmap_create_popup_menu(GtkWidget *widget, GbWidgetCreateMenuData *data)
   {

   }
 */



/*
 * Writes the source code needed to create this widget.
 * You have to output everything necessary to create the widget here, though
 * there are some convenience functions to help.
 */
static void
gb_pixmap_write_source (GtkWidget * widget, GbWidgetWriteSourceData * data)
{
  if (data->create_widget)
    {
      gchar *filename = gtk_object_get_data (GTK_OBJECT (widget), GbFilenameKey);
      source_create_pixmap (data, data->wname, filename, FALSE);
    }

  gb_widget_write_standard_source (widget, data);

  if (fabs (GTK_MISC (widget)->xalign - 0.5) > 0.0001
      || fabs (GTK_MISC (widget)->yalign - 0.5) > 0.0001)
    source_add (data, "  gtk_misc_set_alignment (GTK_MISC (%s), %g, %g);\n",
	 data->wname, GTK_MISC (widget)->xalign, GTK_MISC (widget)->yalign);

  if (GTK_MISC (widget)->xpad != 0 || GTK_MISC (widget)->ypad != 0)
    source_add (data, "  gtk_misc_set_padding (GTK_MISC (%s), %i, %i);\n",
	     data->wname, GTK_MISC (widget)->xpad, GTK_MISC (widget)->ypad);

  if (!GTK_PIXMAP (widget)->build_insensitive)
    {
      source_add (data,
		  "  gtk_pixmap_set_build_insensitive (GTK_PIXMAP (%s), FALSE);\n",
		  data->wname);
    }
}


static void
gb_pixmap_destroy (GtkWidget * widget, GbWidgetDestroyData * data)
{
  gchar *filename = gtk_object_get_data (GTK_OBJECT (widget), GbFilenameKey);
  if (filename)
    {
      glade_project_remove_pixmap (data->project, filename);
      g_free (filename);
    }
}


/*
 * Initializes the GbWidget structure.
 * I've placed this at the end of the file so we don't have to include
 * declarations of all the functions.
 */
GbWidget *
gb_pixmap_init ()
{
  /* Initialise the GTK type */
  gtk_pixmap_get_type ();

  /* Initialize the GbWidget structure */
  gb_widget_init_struct (&gbwidget);

  /* Fill in the pixmap struct & tooltip */
  gbwidget.pixmap_struct = pixmap_xpm;
  gbwidget.tooltip = _("Pixmap");

  /* Fill in any functions that this GbWidget has */
  gbwidget.gb_widget_new = gb_pixmap_new;
  gbwidget.gb_widget_create_properties = gb_pixmap_create_properties;
  gbwidget.gb_widget_get_properties = gb_pixmap_get_properties;
  gbwidget.gb_widget_set_properties = gb_pixmap_set_properties;
  gbwidget.gb_widget_write_source = gb_pixmap_write_source;
  gbwidget.gb_widget_destroy = gb_pixmap_destroy;
/*
   gbwidget.gb_widget_create_popup_menu = gb_pixmap_create_popup_menu;
 */

  return &gbwidget;
}
