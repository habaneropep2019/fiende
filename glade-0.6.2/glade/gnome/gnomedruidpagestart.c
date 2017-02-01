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
#include "../graphics/gnome-druid-page-start.xpm"

/*
 * This is the GbWidget struct for this widget (see ../gbwidget.h).
 * It is initialized in the init() function at the end of this file
 */
static GbWidget gbwidget;

static gchar *BackgroundColor = "GnomeDruidPageStart::background_color";
static gchar *LogoBackgroundColor = "GnomeDruidPageStart::logo_background_color";
static gchar *TextboxColor = "GnomeDruidPageStart::textbox_color";
static gchar *TextColor = "GnomeDruidPageStart::text_color";
static gchar *TitleColor = "GnomeDruidPageStart::title_color";
static gchar *Text = "GnomeDruidPageStart::text";
static gchar *Title = "GnomeDruidPageStart::title";
static gchar *LogoImage = "GnomeDruidPageStart::logo_image";
static gchar *WatermarkImage = "GnomeDruidPageStart::watermark_image";


/******
 * NOTE: To use these functions you need to uncomment them AND add a pointer
 * to the function in the GbWidget struct at the end of this file.
 ******/

/*
 * Creates a new GtkWidget of class GnomeDruidPageStart, performing any specialized
 * initialization needed for the widget to work correctly in this environment.
 * If a dialog box is used to initialize the widget, return NULL from this
 * function, and call data->callback with your new widget when it is done.
 */
static GtkWidget*
gb_gnome_druid_page_start_new (GbWidgetNewData *data)
{
  GtkWidget *new_widget;

  new_widget = gnome_druid_page_start_new_with_vals ("", "", NULL, NULL);
  return new_widget;
}

/*
 * Creates the components needed to edit the extra properties of this widget.
 */
static void
gb_gnome_druid_page_start_create_properties (GtkWidget * widget,
					     GbWidgetCreateArgData * data)
{
  property_add_string (Title, _("Title:"),
		       _("The title of the page"));
  property_add_text (Text, _("Text:"),
		     _("The main text of the page, "
		       "introducing people to the druid."), 4);
  property_add_color (TitleColor, _("Title Color:"),
		      _("The color of the title text"));
  property_add_color (TextColor, _("Text Color:"),
		      _("The color of the main text"));
  property_add_color (BackgroundColor, _("Back. Color:"),
		      _("The background color of the page"));
  property_add_color (LogoBackgroundColor, _("Logo Back. Color:"),
		      _("The background color around the logo"));
  property_add_color (TextboxColor, _("Text Box Color:"),
		      _("The background color of the main text area"));
  property_add_filename (LogoImage, ("Logo Image:"),
			 _("The logo to display in the top-right of the page"));
  property_add_filename (WatermarkImage, ("Watermark Image:"),
			 _("The main image to display on the left of the page."));
}



/*
 * Gets the properties of the widget. This is used for both displaying the
 * properties in the property editor, and also for saving the properties.
 */
static void
gb_gnome_druid_page_start_get_properties (GtkWidget *widget,
					  GbWidgetGetArgData * data)
{
  GnomeDruidPageStart *page;
	
  page = GNOME_DRUID_PAGE_START (widget);

  gb_widget_output_translatable_string (data, Title, page->title);
  gb_widget_output_translatable_text (data, Text, page->text);
  gb_widget_output_color (data, TitleColor, &page->title_color);
  gb_widget_output_color (data, TextColor, &page->text_color);
  gb_widget_output_color (data, BackgroundColor, &page->background_color);
  gb_widget_output_color (data, LogoBackgroundColor,
			  &page->logo_background_color);
  gb_widget_output_color (data, TextboxColor, &page->textbox_color);

  gb_widget_output_pixmap_filename (data, LogoImage,
				    gtk_object_get_data (GTK_OBJECT (widget),
							 LogoImage));
  gb_widget_output_pixmap_filename (data, WatermarkImage,
				    gtk_object_get_data (GTK_OBJECT (widget),
							 WatermarkImage));
}


/*
 * Sets the properties of the widget. This is used for both applying the
 * properties changed in the property editor, and also for loading.
 */

static void
gb_gnome_druid_page_start_set_properties (GtkWidget * widget,
					  GbWidgetSetArgData * data)
{
  GnomeDruidPageStart *page;
  gchar *string, *old_filename;
  GdkColor *color;
  GdkImlibImage *image;
	
  page = GNOME_DRUID_PAGE_START (widget);

  string = gb_widget_input_string (data, Title);
  if (data->apply)
    gnome_druid_page_start_set_title (page, string);

  string = gb_widget_input_text (data, Text);
  if (data->apply)
    gnome_druid_page_start_set_text (page, string);
  if (data->action == GB_APPLYING)
    g_free (string);

  color = gb_widget_input_color (data, BackgroundColor);
  if (data->apply)
    gnome_druid_page_start_set_bg_color (page, color);

  color = gb_widget_input_color (data, LogoBackgroundColor);
  if (data->apply)
    gnome_druid_page_start_set_logo_bg_color (page, color);

  color = gb_widget_input_color (data, TextboxColor);
  if (data->apply)
    gnome_druid_page_start_set_textbox_color (page, color);

  color = gb_widget_input_color (data, TextColor);
  if (data->apply)
    gnome_druid_page_start_set_text_color (page, color);

  color = gb_widget_input_color (data, TitleColor);
  if (data->apply)
    gnome_druid_page_start_set_title_color (page, color);

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

      gnome_druid_page_start_set_logo (page, image);
    }
  if (data->action == GB_LOADING)
    g_free (string);

  string = gb_widget_input_pixmap_filename (data, WatermarkImage);
  if (data->apply)
    {
      if (string && string[0] == '\0')
	string = NULL;

      old_filename = gtk_object_get_data (GTK_OBJECT (widget), WatermarkImage);
      if (old_filename)
	{
	  glade_project_remove_pixmap (data->project, old_filename);
	  g_free (old_filename);
	}

      gtk_object_set_data (GTK_OBJECT (widget), WatermarkImage,
			   g_strdup (string));

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
						   WatermarkImage, string);
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

      gnome_druid_page_start_set_watermark (page, image);
    }
  if (data->action == GB_LOADING)
    g_free (string);
}


/*
 * Adds menu items to a context menu which is just about to appear!
 * Add commands to aid in editing a GnomeDruidPageStart, with signals pointing to
 * other functions in this file.
 */
/*
static void
gb_gnome_druid_page_start_create_popup_menu (GtkWidget * widget, GbWidgetCreateMenuData * data)
{

}
*/



/*
 * Writes the source code needed to create this widget.
 * You have to output everything necessary to create the widget here, though
 * there are some convenience functions to help.
 */
static void
gb_gnome_druid_page_start_write_source (GtkWidget * widget,
					GbWidgetWriteSourceData * data)
{
  GnomeDruidPageStart *page;
  GdkColor *color;
  gchar *filename;

  /* These are all from gnome-druid-page-start.c */
  const GdkColor default_background_color	= { 0,  6400,  6400, 28672 };
  const GdkColor default_textbox_color		= { 0, 65280, 65280, 65280 };
  const GdkColor default_logo_background_color	= { 0, 65280, 65280, 65280 };
  const GdkColor default_title_color		= { 0, 65280, 65280, 65280 };
  const GdkColor default_text_color		= { 0,     0,     0,     0 };

  page = GNOME_DRUID_PAGE_START (widget);

  if (data->create_widget)
    {
      source_add (data, "  %s = gnome_druid_page_start_new ();\n",
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
		  "  gnome_druid_page_start_set_bg_color (GNOME_DRUID_PAGE_START (%s), &%s_bg_color);\n",
		  data->wname, data->real_wname);
    }

  color = &page->textbox_color;
  if (!gdk_color_equal (color, &default_textbox_color))
    {
      source_add_decl (data,
		       "  GdkColor %s_textbox_color = { 0, %i, %i, %i };\n",
		       data->real_wname,
		       color->red, color->green, color->blue);
      source_add (data,
		  "  gnome_druid_page_start_set_textbox_color (GNOME_DRUID_PAGE_START (%s), &%s_textbox_color);\n",
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
		  "  gnome_druid_page_start_set_logo_bg_color (GNOME_DRUID_PAGE_START (%s), &%s_logo_bg_color);\n",
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
		  "  gnome_druid_page_start_set_title_color (GNOME_DRUID_PAGE_START (%s), &%s_title_color);\n",
		  data->wname, data->real_wname);
    }

  color = &page->text_color;
  if (!gdk_color_equal (color, &default_text_color))
    {
      source_add_decl (data,
		       "  GdkColor %s_text_color = { 0, %i, %i, %i };\n",
		       data->real_wname,
		       color->red, color->green, color->blue);
      source_add (data,
		  "  gnome_druid_page_start_set_text_color (GNOME_DRUID_PAGE_START (%s), &%s_text_color);\n",
		  data->wname, data->real_wname);
    }

  if (strcmp (page->title, ""))
    {
      source_add (data,
		  "  gnome_druid_page_start_set_title (GNOME_DRUID_PAGE_START (%s), %s);\n",
		  data->wname,
		  source_make_string (page->title, data->use_gettext));
    }

  if (strcmp (page->text, ""))
    {
      source_add (data,
		  "  gnome_druid_page_start_set_text (GNOME_DRUID_PAGE_START (%s), %s);\n",
		  data->wname,
		  source_make_string (page->text, data->use_gettext));
    }

  filename = gtk_object_get_data (GTK_OBJECT (widget), LogoImage);
  if (filename && filename[0])
    {
      source_add (data,
		  "  gnome_druid_page_start_set_logo (GNOME_DRUID_PAGE_START (%s),\n"
		  "                                   create_image (\"%s/%s\"));\n",
		  data->wname, data->program_name, g_basename (filename));
    }

  filename = gtk_object_get_data (GTK_OBJECT (widget), WatermarkImage);
  if (filename && filename[0])
    {
      source_add (data,
		  "  gnome_druid_page_start_set_watermark (GNOME_DRUID_PAGE_START (%s),\n"
		  "                                        create_image (\"%s/%s\"));\n",
		  data->wname, data->program_name, g_basename (filename));
    }
}



/*
 * Initializes the GbWidget structure.
 * I've placed this at the end of the file so we don't have to include
 * declarations of all the functions.
 */
GbWidget*
gb_gnome_druid_page_start_init ()
{
  /* Initialise the GTK type */
  gnome_druid_page_start_get_type();

  /* Initialize the GbWidget structure */
  gb_widget_init_struct(&gbwidget);

  /* Fill in the pixmap struct & tooltip */
  gbwidget.pixmap_struct = gnome_druid_page_start_xpm;
  gbwidget.tooltip = _("Druid Start Page");

  /* Fill in any functions that this GbWidget has */
  gbwidget.gb_widget_new		= gb_gnome_druid_page_start_new;
  gbwidget.gb_widget_create_properties	= gb_gnome_druid_page_start_create_properties;
  gbwidget.gb_widget_get_properties	= gb_gnome_druid_page_start_get_properties;
  gbwidget.gb_widget_set_properties	= gb_gnome_druid_page_start_set_properties;
  gbwidget.gb_widget_write_source	= gb_gnome_druid_page_start_write_source;
/*
  gbwidget.gb_widget_create_popup_menu	= gb_gnome_druid_page_start_create_popup_menu;
*/

  return &gbwidget;
}

