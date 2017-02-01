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
#include "../graphics/gnome-pixmapentry.xpm"

/*
 * This is the GbWidget struct for this widget (see ../gbwidget.h).
 * It is initialized in the init() function at the end of this file
 */
static GbWidget gbwidget;

/* These are the special children of the widget. */
static gchar *ChildGnomeFileEntry = "GnomePixmapEntry:file-entry";
static gchar *ComboEntry = "GnomeEntry:entry";

static gchar *Preview = "GnomePixmapEntry::preview";


/******
 * NOTE: To use these functions you need to uncomment them AND add a pointer
 * to the function in the GbWidget struct at the end of this file.
 ******/

/*
 * Creates a new GtkWidget of class GnomePixmapEntry, performing any specialized
 * initialization needed for the widget to work correctly in this environment.
 * If a dialog box is used to initialize the widget, return NULL from this
 * function, and call data->callback with your new widget when it is done.
 */
static GtkWidget*
gb_gnome_pixmap_entry_new (GbWidgetNewData *data)
{
  GtkWidget *new_widget, *fentry, *entry;

  new_widget = gnome_pixmap_entry_new (NULL, NULL, TRUE);

  fentry = gnome_pixmap_entry_gnome_file_entry (GNOME_PIXMAP_ENTRY (new_widget));
  gb_widget_create_from (fentry,
			 data->action == GB_CREATING ? "entry" : NULL);
  gb_widget_set_child_name (fentry, ChildGnomeFileEntry);

  /* I'm not sure this should be here. I think we need to sort out the
     gb_widget_create_from () related stuff. */
  entry = gnome_pixmap_entry_gtk_entry (GNOME_PIXMAP_ENTRY (new_widget));
  gb_widget_create_from (entry,
			 data->action == GB_CREATING ? "combo-entry" : NULL);
  gb_widget_set_child_name (entry, ComboEntry);

  return new_widget;
}



/*
 * Creates the components needed to edit the extra properties of this widget.
 */
static void
gb_gnome_pixmap_entry_create_properties (GtkWidget * widget, GbWidgetCreateArgData * data)
{
  property_add_bool (Preview, _("Preview:"),
		     _("If a small preview of the pixmap is displayed"));
}



/*
 * Gets the properties of the widget. This is used for both displaying the
 * properties in the property editor, and also for saving the properties.
 */
static void
gb_gnome_pixmap_entry_get_properties (GtkWidget *widget, GbWidgetGetArgData * data)
{
  gb_widget_output_bool (data, Preview,
			 GNOME_PIXMAP_ENTRY (widget)->do_preview);
}



/*
 * Sets the properties of the widget. This is used for both applying the
 * properties changed in the property editor, and also for loading.
 */
static void
gb_gnome_pixmap_entry_set_properties (GtkWidget * widget, GbWidgetSetArgData * data)
{
  gboolean preview;

  preview = gb_widget_input_bool (data, Preview);
  if (data->apply)
    {
      gnome_pixmap_entry_set_preview (GNOME_PIXMAP_ENTRY (widget), preview);
      /* FIXME: In GnomeLibs 1.0.1 it doesn't resize properly without this. */
      gtk_widget_queue_resize (widget);
    }
}


/*
 * Adds menu items to a context menu which is just about to appear!
 * Add commands to aid in editing a GnomePixmapEntry, with signals pointing to
 * other functions in this file.
 */
/*
static void
gb_gnome_pixmap_entry_create_popup_menu (GtkWidget * widget, GbWidgetCreateMenuData * data)
{

}
*/



/*
 * Writes the source code needed to create this widget.
 * You have to output everything necessary to create the widget here, though
 * there are some convenience functions to help.
 */
static void
gb_gnome_pixmap_entry_write_source (GtkWidget * widget,
				    GbWidgetWriteSourceData * data)
{
  GtkWidget *fentry, *gentry;
  gchar *title, *history_id;
  gchar *wname, *child_name;

  fentry = gnome_pixmap_entry_gnome_file_entry (GNOME_PIXMAP_ENTRY (widget));
  gentry = gnome_pixmap_entry_gnome_entry (GNOME_PIXMAP_ENTRY (widget));

  title = GNOME_FILE_ENTRY (fentry)->browse_dialog_title;
  if (title && title[0] == '\0')
    title = NULL;

  history_id = GNOME_ENTRY (gentry)->history_id;
  if (history_id && history_id[0] == '\0')
    history_id = NULL;

  if (data->create_widget)
    {
      if (title)
	title = g_strdup (source_make_string (title, data->use_gettext));

      source_add (data, "  %s = gnome_pixmap_entry_new (%s, %s, %s);\n",
		  data->wname,
		  history_id ? source_make_string (history_id, FALSE) : "NULL",
		  title ? title : "NULL",
		  GNOME_PIXMAP_ENTRY (widget)->do_preview ? "TRUE" : "FALSE");
      g_free (title);
    }
  gb_widget_write_standard_source (widget, data);


  /* We output the source code for the children here, since the code should
     not include calls to create the widgets. We need to specify that the
     names used are like: "GTK_COMBO (<combo-name>)->entry".
     We need to remember the dialog's name since data->wname
     will be overwritten. */
  wname = g_strdup (data->wname);

  source_add (data, "\n");
  child_name = gtk_widget_get_name (fentry);
  child_name = source_create_valid_identifier (child_name);
  source_add (data,
	      "  %s = gnome_pixmap_entry_gnome_file_entry (GNOME_PIXMAP_ENTRY (%s));\n",
	      child_name, wname);
  g_free (child_name);
  data->create_widget = FALSE;
  gb_widget_write_source (fentry, data);

  g_free (wname);
  data->write_children = FALSE;
}


static GtkWidget *
gb_gnome_pixmap_entry_get_child (GtkWidget * widget,
				 const gchar * child_name)
{
  if (!strcmp (child_name, ChildGnomeFileEntry))
    return gnome_pixmap_entry_gnome_file_entry (GNOME_PIXMAP_ENTRY (widget));
  else
    return NULL;
}



/*
 * Initializes the GbWidget structure.
 * I've placed this at the end of the file so we don't have to include
 * declarations of all the functions.
 */
GbWidget*
gb_gnome_pixmap_entry_init ()
{
  /* Initialise the GTK type */
  gnome_pixmap_entry_get_type();

  /* Initialize the GbWidget structure */
  gb_widget_init_struct(&gbwidget);

  /* Fill in the pixmap struct & tooltip */
  gbwidget.pixmap_struct = gnome_pixmapentry_xpm;
  gbwidget.tooltip = _("GnomePixmapEntry");

  /* Fill in any functions that this GbWidget has */
  gbwidget.gb_widget_new		= gb_gnome_pixmap_entry_new;
  gbwidget.gb_widget_create_properties	= gb_gnome_pixmap_entry_create_properties;
  gbwidget.gb_widget_get_properties	= gb_gnome_pixmap_entry_get_properties;
  gbwidget.gb_widget_set_properties	= gb_gnome_pixmap_entry_set_properties;
  gbwidget.gb_widget_write_source	= gb_gnome_pixmap_entry_write_source;
  gbwidget.gb_widget_get_child		= gb_gnome_pixmap_entry_get_child;
/*
  gbwidget.gb_widget_set_child_props	= gb_gnome_pixmap_entry_set_child_props;
  gbwidget.gb_widget_create_popup_menu	= gb_gnome_pixmap_entry_create_popup_menu;
*/

  return &gbwidget;
}

