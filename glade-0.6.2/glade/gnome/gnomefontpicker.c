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
#include "../graphics/gnome-fontpicker.xpm"

/*
 * This is the GbWidget struct for this widget (see ../gbwidget.h).
 * It is initialized in the init() function at the end of this file
 */
static GbWidget gbwidget;

/* Copied from gnome-font-picker.c */
#define DEF_PREVIEW_TEXT "AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz"

static gchar *Title = "GnomeFontPicker::title";
static gchar *PreviewText = "GnomeFontPicker::preview_text";
static gchar *Mode = "GnomeFontPicker::mode";
static gchar *ShowSize = "GnomeFontPicker::show_size";
static gchar *UseFont = "GnomeFontPicker::use_font";
static gchar *UseFontSize = "GnomeFontPicker::use_font_size";

static const gchar *GbModeChoices[] =
{
  "Pixmap",
  "Font Information",
  NULL
};
static const gint GbModeValues[] =
{
  GNOME_FONT_PICKER_MODE_PIXMAP,
  GNOME_FONT_PICKER_MODE_FONT_INFO
};
static const gchar *GbModeSymbols[] =
{
  "GNOME_FONT_PICKER_MODE_PIXMAP",
  "GNOME_FONT_PICKER_MODE_FONT_INFO"
};


static void gb_gnome_font_picker_set_property_states (GtkWidget *widget);

/******
 * NOTE: To use these functions you need to uncomment them AND add a pointer
 * to the function in the GbWidget struct at the end of this file.
 ******/

/*
 * Creates a new GtkWidget of class GnomeFontPicker, performing any specialized
 * initialization needed for the widget to work correctly in this environment.
 * If a dialog box is used to initialize the widget, return NULL from this
 * function, and call data->callback with your new widget when it is done.
 * If the widget needs a special destroy handler, add a signal here.
 */
static GtkWidget*
gb_gnome_font_picker_new (GbWidgetNewData *data)
{
  GtkWidget *new_widget;

  new_widget = gnome_font_picker_new ();

  /* FIXME: GnomeLibs 1.0.5 workaround - it doesn't set a reasonable default.*/
  GNOME_FONT_PICKER (new_widget)->use_font_in_label_size = 14;

  return new_widget;
}



/*
 * Creates the components needed to edit the extra properties of this widget.
 */
static void
gb_gnome_font_picker_create_properties (GtkWidget * widget, GbWidgetCreateArgData * data)
{
  property_add_string (Title, _("Title:"),
		       _("The title of the font selection dialog"));
  property_add_string (PreviewText, _("Preview Text:"),
		       _("The preview text to show in the font selection dialog"));
  property_add_choice (Mode, _("Mode:"),
		       _("What to display in the font picker button"),
		       GbModeChoices);
  property_add_bool (ShowSize, _("Show Size:"),
		     _("If the font size is shown as part of the font information"));
  property_add_bool (UseFont, _("Use Font:"),
		     _("If the selected font is used when displaying the font information"));
  property_add_int_range (UseFontSize, _("Use Size:"),
			  _("The size of the font to use in the font picker button"),
			  2, 1000, 1, 10, 1);
}



/*
 * Gets the properties of the widget. This is used for both displaying the
 * properties in the property editor, and also for saving the properties.
 */
static void
gb_gnome_font_picker_get_properties (GtkWidget *widget, GbWidgetGetArgData * data)
{
  gint i;

  gb_widget_output_translatable_string (data, Title,
					GNOME_FONT_PICKER (widget)->title);
  gb_widget_output_translatable_string (data, PreviewText,
					GNOME_FONT_PICKER (widget)->preview_text);

  for (i = 0; i < sizeof (GbModeValues) / sizeof (GbModeValues[0]); i++)
    {
      if (GbModeValues[i] == GNOME_FONT_PICKER (widget)->mode)
	gb_widget_output_choice (data, Mode, i, GbModeSymbols[i]);
    }

  gb_widget_output_bool (data, ShowSize,
			 GNOME_FONT_PICKER (widget)->show_size);
  gb_widget_output_bool (data, UseFont,
			 GNOME_FONT_PICKER (widget)->use_font_in_label);
  gb_widget_output_int (data, UseFontSize,
			GNOME_FONT_PICKER (widget)->use_font_in_label_size);

  if (data->action == GB_SHOWING)
    gb_gnome_font_picker_set_property_states (widget);
}


static void
gb_gnome_font_picker_set_property_states (GtkWidget *widget)
{
  gboolean use_font_sens = FALSE, use_font_size_sens = FALSE;

  if (GNOME_FONT_PICKER (widget)->mode == GNOME_FONT_PICKER_MODE_FONT_INFO)
    {
      use_font_sens = TRUE;
      if (GNOME_FONT_PICKER (widget)->use_font_in_label)
	use_font_size_sens = TRUE;
    }
  property_set_sensitive (ShowSize, use_font_sens);
  property_set_sensitive (UseFont, use_font_sens);
  property_set_sensitive (UseFontSize, use_font_size_sens);
}



/*
 * Sets the properties of the widget. This is used for both applying the
 * properties changed in the property editor, and also for loading.
 */
static void
gb_gnome_font_picker_set_properties (GtkWidget * widget, GbWidgetSetArgData * data)
{
  gchar *title, *preview_text, *mode;
  gint i, use_font_size, new_use_font_size;
  gboolean show_size, use_font;
  gboolean new_use_font, set_use_font = FALSE;
  gboolean set_property_states = FALSE;

  title = gb_widget_input_string (data, Title);
  if (data->apply)
    gnome_font_picker_set_title (GNOME_FONT_PICKER (widget),
				 title && title[0]
				 ? title : dgettext ("gnome-libs",
						     "Pick a Font"));

  preview_text = gb_widget_input_string (data, PreviewText);
  if (data->apply)
    gnome_font_picker_set_preview_text (GNOME_FONT_PICKER (widget),
					preview_text && preview_text[0]
					? preview_text : DEF_PREVIEW_TEXT);

  mode = gb_widget_input_choice (data, Mode);
  if (data->apply)
    {
      for (i = 0; i < sizeof (GbModeValues) / sizeof (GbModeValues[0]); i++)
	{
	  if (!strcmp (mode, GbModeChoices[i])
	      || !strcmp (mode, GbModeSymbols[i]))
	    {
	      gnome_font_picker_set_mode (GNOME_FONT_PICKER (widget),
					  GbModeValues[i]);
	      set_property_states = TRUE;
	      break;
	    }
	}
    }

  show_size = gb_widget_input_bool (data, ShowSize);
  if (data->apply)
    gnome_font_picker_fi_set_show_size (GNOME_FONT_PICKER (widget),
					show_size);

  /* Remember the old values, so we only change what we need to. */
  new_use_font = GNOME_FONT_PICKER (widget)->use_font_in_label;
  new_use_font_size = GNOME_FONT_PICKER (widget)->use_font_in_label_size;

  use_font = gb_widget_input_bool (data, UseFont);
  if (data->apply)
    {
      new_use_font = use_font;
      set_use_font = TRUE;
    }

  use_font_size = gb_widget_input_int (data, UseFontSize);
  if (data->apply)
    {
      new_use_font_size = use_font_size;
      set_use_font = TRUE;
    }

  if (set_use_font)
    {
      gnome_font_picker_fi_set_use_font_in_label (GNOME_FONT_PICKER (widget),
						  new_use_font,
						  new_use_font_size);
      set_property_states = TRUE;
    }

  if ((data->action == GB_APPLYING) && set_property_states)
    gb_gnome_font_picker_set_property_states (widget);
}



/*
 * Adds menu items to a context menu which is just about to appear!
 * Add commands to aid in editing a GnomeFontPicker, with signals pointing to
 * other functions in this file.
 */
/*
static void
gb_gnome_font_picker_create_popup_menu (GtkWidget * widget, GbWidgetCreateMenuData * data)
{

}
*/



/*
 * Writes the source code needed to create this widget.
 * You have to output everything necessary to create the widget here, though
 * there are some convenience functions to help.
 */
static void
gb_gnome_font_picker_write_source (GtkWidget * widget, GbWidgetWriteSourceData * data)
{
  gchar *title, *preview_text;
  gint i;

  if (data->create_widget)
    source_add (data, "  %s = gnome_font_picker_new ();\n", data->wname);

  gb_widget_write_standard_source (widget, data);

  title = GNOME_FONT_PICKER (widget)->title;
  if (title && title[0]
      && strcmp (title, dgettext ("gnome-libs", "Pick a Font")))
    {
      source_add (data,
		  "  gnome_font_picker_set_title (GNOME_FONT_PICKER (%s), %s);\n",
		  data->wname, source_make_string (title, data->use_gettext));
    }

  preview_text = GNOME_FONT_PICKER (widget)->preview_text;
  if (preview_text && preview_text[0]
      && strcmp (preview_text, DEF_PREVIEW_TEXT))
    {
      source_add (data,
		  "  gnome_font_picker_set_preview_text (GNOME_FONT_PICKER (%s), %s);\n",
		  data->wname, source_make_string (preview_text, FALSE));
    }

  if (GNOME_FONT_PICKER (widget)->mode != GNOME_FONT_PICKER_MODE_PIXMAP)
    {
      for (i = 0; i < sizeof (GbModeValues) / sizeof (GbModeValues[0]); i++)
	{
	  if (GbModeValues[i] == GNOME_FONT_PICKER (widget)->mode)
	    source_add (data,
			"  gnome_font_picker_set_mode (GNOME_FONT_PICKER (%s), %s);\n",
			data->wname, GbModeSymbols[i]);
	}
    }

  if (GNOME_FONT_PICKER (widget)->mode == GNOME_FONT_PICKER_MODE_FONT_INFO)
    {
      if (!GNOME_FONT_PICKER (widget)->show_size)
	{
	  source_add (data,
		      "  gnome_font_picker_fi_set_show_size (GNOME_FONT_PICKER (%s), FALSE);\n",
		      data->wname);
	}

      if (GNOME_FONT_PICKER (widget)->use_font_in_label)
	{
	  source_add (data,
		      "  gnome_font_picker_fi_set_use_font_in_label (GNOME_FONT_PICKER (%s),\n"
		      "                                              TRUE, %i);\n",
		      data->wname,
		      GNOME_FONT_PICKER (widget)->use_font_in_label_size);
	}
    }
}



/*
 * Initializes the GbWidget structure.
 * I've placed this at the end of the file so we don't have to include
 * declarations of all the functions.
 */
GbWidget*
gb_gnome_font_picker_init ()
{
  /* Initialise the GTK type */
  gnome_font_picker_get_type();

  /* Initialize the GbWidget structure */
  gb_widget_init_struct(&gbwidget);

  /* Fill in the pixmap struct & tooltip */
  gbwidget.pixmap_struct = gnome_fontpicker_xpm;
  gbwidget.tooltip = _("Gnome Font Picker");

  /* Fill in any functions that this GbWidget has */
  gbwidget.gb_widget_new		= gb_gnome_font_picker_new;
  gbwidget.gb_widget_create_properties	= gb_gnome_font_picker_create_properties;
  gbwidget.gb_widget_get_properties	= gb_gnome_font_picker_get_properties;
  gbwidget.gb_widget_set_properties	= gb_gnome_font_picker_set_properties;
  gbwidget.gb_widget_write_source	= gb_gnome_font_picker_write_source;
/*
  gbwidget.gb_widget_create_popup_menu	= gb_gnome_font_picker_create_popup_menu;
*/

  return &gbwidget;
}

