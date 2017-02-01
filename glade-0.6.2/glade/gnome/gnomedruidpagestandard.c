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
#include "../graphics/gnome-druid-page-standard.xpm"

/*
 * This is the GbWidget struct for this widget (see ../gbwidget.h).
 * It is initialized in the init() function at the end of this file
 */
static GbWidget gbwidget;

static gchar *BackgroundColor = "GnomeDruidPageStandard::background_color";
static gchar *LogoBackgroundColor = "GnomeDruidPageStandard::logo_background_color";
static gchar *TitleColor = "GnomeDruidPageStandard::title_color";
static gchar *Title = "GnomeDruidPageStandard::title";
static gchar *LogoImage = "GnomeDruidPageStandard::logo_image";


static gchar *DruidPageStandardVBox = "GnomeDruidPageStandard:vbox";


/******
 * NOTE: To use these functions you need to uncomment them AND add a pointer
 * to the function in the GbWidget struct at the end of this file.
 ******/

/*
 * Creates a new GtkWidget of class GnomeDruidPageStandard, performing any specialized
 * initialization needed for the widget to work correctly in this environment.
 * If a dialog box is used to initialize the widget, return NULL from this
 * function, and call data->callback with your new widget when it is done.
 */
static GtkWidget*
gb_gnome_druid_page_standard_new (GbWidgetNewData *data)
{
  GtkWidget *new_widget, *placeholder;

  if (data->action == GB_CREATING)
    {
      new_widget = gnome_druid_page_standard_new_with_vals ("", NULL);
      placeholder = editor_new_placeholder ();
      gtk_box_pack_start (GTK_BOX (GNOME_DRUID_PAGE_STANDARD (new_widget)->vbox),
			  placeholder, TRUE, TRUE, 0);
    }
  else
    {
      new_widget = gnome_druid_page_standard_new_with_vals ("", NULL);
    }
  gb_widget_create_from (GNOME_DRUID_PAGE_STANDARD (new_widget)->vbox,
			 "druid-vbox");
  gb_widget_set_child_name (GNOME_DRUID_PAGE_STANDARD (new_widget)->vbox,
			    DruidPageStandardVBox);
  gtk_widget_show_all (new_widget);
  return new_widget;
}

/*
 * Creates the components needed to edit the extra properties of this widget.
 */
static void
gb_gnome_druid_page_standard_create_properties (GtkWidget * widget,
						GbWidgetCreateArgData * data)
{
  property_add_string (Title, _("Title:"),
		       _("The title of the page"));
  property_add_color (TitleColor, _("Title Color:"),
		      _("The color of the title text"));
  property_add_color (BackgroundColor, _("Back. Color:"),
		      _("The background color of the page"));
  property_add_color (LogoBackgroundColor, _("Logo Back. Color:"),
		      _("The background color around the logo"));
  property_add_filename (LogoImage, ("Logo Image:"),
			 _("The logo to display in the top-right of the page"));
}



/*
 * Gets the properties of the widget. This is used for both displaying the
 * properties in the property editor, and also for saving the properties.
 */
static void
gb_gnome_druid_page_standard_get_properties (GtkWidget *widget,
					     GbWidgetGetArgData * data)
{
  GnomeDruidPageStandard *page;
	
  page = GNOME_DRUID_PAGE_STANDARD (widget);

  gb_widget_output_translatable_string (data, Title, page->title);
  gb_widget_output_color (data, TitleColor, &page->title_color);
  gb_widget_output_color (data, BackgroundColor, &page->background_color);
  gb_widget_output_color (data, LogoBackgroundColor,
			  &page->logo_background_color);
  gb_widget_output_pixmap_filename (data, LogoImage,
				    gtk_object_get_data (GTK_OBJECT (widget),
							 LogoImage));
}


/*
 * Sets the properties of the widget. This is used for both applying the
 * properties changed in the property editor, and also for loading.
 */

static void
gb_gnome_druid_page_standard_set_properties (GtkWidget * widget,
					     GbWidgetSetArgData * data)
{
  GnomeDruidPageStandard *page;
  gchar *string, *old_filename;
  GdkColor *color;
  GdkImlibImage *image;
	
  page = GNOME_DRUID_PAGE_STANDARD (widget);

  string = gb_widget_input_string (data, Title);
  if (data->apply)
    gnome_druid_page_standard_set_title (page, string);

  color = gb_widget_input_color (data, BackgroundColor);
  if (data->apply)
    gnome_druid_page_standard_set_bg_color (page, color);

  color = gb_widget_input_color (data, LogoBackgroundColor);
  if (data->apply)
    gnome_druid_page_standard_set_logo_bg_color (page, color);

  color = gb_widget_input_color (data, TitleColor);
  if (data->apply)
    gnome_druid_page_standard_set_title_color (page, color);

  string = gb_widget_input_pixmap_filename (data, LogoImage);
  if (data->apply)
    {
      if (string && string[0] == '\0')
	string = NULL;

      old_filename = gtk_object_get_data (GTK_OBJECT (widget), LogoImage);
      if (old_filename)
	{
	  glade_project_remove_pixmap (data->project, old_filename);
	  g_free (old_filename);
	}

      gtk_object_set_data (GTK_OBJECT (widget), LogoImage, g_strdup (string));

      if (string)
	{
	  glade_project_add_pixmap (data->project, string);
	  image = gdk_imlib_load_image (string);
	  if (image == NULL)
	    {
	      if (data->action == GB_LOADING)
		{
		  load_add_error_message_with_tag (data,
						   GLADE_LINE_PROPERTY,
						   _("Couldn't load image from file"),
						   LogoImage, string);
		}
	      else
		{
		  glade_util_show_message_box (_("Couldn't load image from file\n"), widget);
		}
	    }
	}
      else
	{
	  image = NULL;
	}

      gnome_druid_page_standard_set_logo (page, image);
    }
  if (data->action == GB_LOADING)
    g_free (string);
}

static GtkWidget *
gb_gnome_druid_page_standard_get_child (GtkWidget * widget,
					const gchar * child_name)
{
  if (!strcmp (child_name, DruidPageStandardVBox))
    return GNOME_DRUID_PAGE_STANDARD (widget)->vbox;
  else
    return NULL;
}


/*
 * Adds menu items to a context menu which is just about to appear!
 * Add commands to aid in editing a GnomeDruidPageStandard, with signals pointing to
 * other functions in this file.
 */
/*
static void
gb_gnome_druid_page_standard_create_popup_menu (GtkWidget * widget, GbWidgetCreateMenuData * data)
{

}
*/



/*
 * Writes the source code needed to create this widget.
 * You have to output everything necessary to create the widget here, though
 * there are some convenience functions to help.
 */
static void
gb_gnome_druid_page_standard_write_source (GtkWidget * widget,
					   GbWidgetWriteSourceData * data)
{
  GnomeDruidPageStandard *page;
  GdkColor *color;
  gchar *filename;
  gchar *wname, *child_name;

  /* These are all from gnome-druid-page-start.c */
  const GdkColor default_background_color	= { 0,  6400,  6400, 28672 };
  const GdkColor default_logo_background_color	= { 0, 65280, 65280, 65280 };
  const GdkColor default_title_color		= { 0, 65280, 65280, 65280 };

  page = GNOME_DRUID_PAGE_STANDARD (widget);

  if (data->create_widget)
    {
      source_add (data,
		  "  %s = gnome_druid_page_standard_new_with_vals (\"\", NULL);\n",
		  data->wname);
    }

  gb_widget_write_standard_source (widget, data);

  color = &page->background_color;
  if (!gdk_color_equal (color, &default_background_color))
    {
      source_add_decl (data,
		       "  GdkColor %s_bg_color = { 0, %i, %i, %i };\n",
		       data->real_wname,
		       color->red, color->green, color->blue);
      source_add (data,
		  "  gnome_druid_page_standard_set_bg_color (GNOME_DRUID_PAGE_STANDARD (%s), &%s_bg_color);\n",
		  data->wname, data->real_wname);
    }

  color = &page->logo_background_color;
  if (!gdk_color_equal (color, &default_logo_background_color))
    {
      source_add_decl (data,
		       "  GdkColor %s_logo_bg_color = { 0, %i, %i, %i };\n",
		       data->real_wname,
		       color->red, color->green, color->blue);
      source_add (data,
		  "  gnome_druid_page_standard_set_logo_bg_color (GNOME_DRUID_PAGE_STANDARD (%s), &%s_logo_bg_color);\n",
		  data->wname, data->real_wname);
    }

  color = &page->title_color;
  if (!gdk_color_equal (color, &default_title_color))
    {
      source_add_decl (data,
		       "  GdkColor %s_title_color = { 0, %i, %i, %i };\n",
		       data->real_wname,
		       color->red, color->green, color->blue);
      source_add (data,
		  "  gnome_druid_page_standard_set_title_color (GNOME_DRUID_PAGE_STANDARD (%s), &%s_title_color);\n",
		  data->wname, data->real_wname);
    }

  if (strcmp (page->title, ""))
    {
      source_add (data,
		  "  gnome_druid_page_standard_set_title (GNOME_DRUID_PAGE_STANDARD (%s), %s);\n",
		  data->wname,
		  source_make_string (page->title, data->use_gettext));
    }

  filename = gtk_object_get_data (GTK_OBJECT (widget), LogoImage);
  if (filename && filename[0])
    {
      source_add (data,
		  "  gnome_druid_page_standard_set_logo (GNOME_DRUID_PAGE_STANDARD (%s),\n"
		  "                                      create_image (\"%s/%s\"));\n",
		  data->wname, data->program_name, g_basename (filename));
    }

  /* We output the source code for the children here, since the code should
     not include calls to create the widgets. We need to specify that the
     names used are like: "GTK_COMBO (<combo-name>)->entry".
     We need to remember the dialog's name since data->wname
     will be overwritten. */
  wname = g_strdup (data->wname);

  source_add (data, "\n");
  child_name = gtk_widget_get_name (GNOME_DRUID_PAGE_STANDARD (widget)->vbox);
  child_name = source_create_valid_identifier (child_name);
  source_add (data, "  %s = GNOME_DRUID_PAGE_STANDARD (%s)->vbox;\n",
	      child_name, wname);
  g_free (child_name);
  data->create_widget = FALSE;
  gb_widget_write_source (GNOME_DRUID_PAGE_STANDARD (widget)->vbox, data);

  g_free (wname);
  data->write_children = FALSE;
}



/*
 * Initializes the GbWidget structure.
 * I've placed this at the end of the file so we don't have to include
 * declarations of all the functions.
 */
GbWidget*
gb_gnome_druid_page_standard_init ()
{
  /* Initialise the GTK type */
  gnome_druid_page_standard_get_type();

  /* Initialize the GbWidget structure */
  gb_widget_init_struct(&gbwidget);

  /* Fill in the pixmap struct & tooltip */
  gbwidget.pixmap_struct = gnome_druid_page_standard_xpm;
  gbwidget.tooltip = _("Druid Standard Page");

  /* Fill in any functions that this GbWidget has */
  gbwidget.gb_widget_new		= gb_gnome_druid_page_standard_new;
  gbwidget.gb_widget_create_properties	= gb_gnome_druid_page_standard_create_properties;
  gbwidget.gb_widget_get_properties	= gb_gnome_druid_page_standard_get_properties;
  gbwidget.gb_widget_set_properties	= gb_gnome_druid_page_standard_set_properties;
  gbwidget.gb_widget_get_child		= gb_gnome_druid_page_standard_get_child;
  gbwidget.gb_widget_write_source	= gb_gnome_druid_page_standard_write_source;
/*
  gbwidget.gb_widget_create_popup_menu	= gb_gnome_druid_page_standard_create_popup_menu;
*/

  return &gbwidget;
}

