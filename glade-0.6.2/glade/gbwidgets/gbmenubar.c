
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

#include <gtk/gtkbutton.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkmenubar.h>
#include <gtk/gtkmenuitem.h>
#include "../gb.h"
#include "../glade_gnome.h"
#include "../glade_menu_editor.h"

/* Include the 21x21 icon pixmap for this widget, to be used in the palette */
#include "../graphics/menubar.xpm"

/*
 * This is the GbWidget struct for this widget (see ../gbwidget.h).
 * It is initialized in the init() function at the end of this file
 */
static GbWidget gbwidget;

static gchar *Shadow = "GtkMenuBar::shadow_type";


static void on_menu_bar_size_request (GtkWidget * widget,
				      GtkRequisition *requisition,
				      gpointer data);
static void gb_menu_bar_on_edit_menu (GtkWidget *button,
				      gpointer data);
static void gb_menu_bar_on_edit_menu_activate (GtkWidget *menuitem,
					       GtkWidget *menubar);


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
 * Creates a new GtkWidget of class GtkMenuBar, performing any specialized
 * initialization needed for the widget to work correctly in this environment.
 * If a dialog box is used to initialize the widget, return NULL from this
 * function, and call data->callback with your new widget when it is done.
 * If the widget needs a special destroy handler, add a signal here.
 */
GtkWidget*
gb_menu_bar_new(GbWidgetNewData *data)
{
  GtkWidget *new_widget;

  new_widget = gtk_menu_bar_new ();

  gtk_signal_connect_after (GTK_OBJECT (new_widget), "size_request",
			    GTK_SIGNAL_FUNC (on_menu_bar_size_request),
			    NULL);

  return new_widget;
}


static void
on_menu_bar_size_request (GtkWidget * widget,
			  GtkRequisition *requisition,
			  gpointer data)
{
  /* Make sure we request a decent size. If we don't do this, when a menubar
     is created it appears about 3 pixels high which is not very good. */
  requisition->width = MAX (requisition->width, 32);
  requisition->height = MAX (requisition->height, 24);
}


/*
 * Creates the components needed to edit the extra properties of this widget.
 */
static void
gb_menu_bar_create_properties(GtkWidget *widget, GbWidgetCreateArgData *data)
{
  GtkWidget *property_table, *button;
  gint property_table_row;

  property_add_choice (Shadow, _("Shadow:"),
		       _("The type of shadow around the menubar"),
		       GbShadowChoices);

  /* Add a button for editing the menubar. */
  property_table = property_get_table_position (&property_table_row);
  button = gtk_button_new_with_label (_("Edit Menus..."));
  gtk_widget_show (button);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (gb_menu_bar_on_edit_menu), NULL);
  gtk_table_attach (GTK_TABLE (property_table), button, 0, 3,
		    property_table_row, property_table_row + 1,
		    GTK_FILL, GTK_FILL, 10, 10);
}


/* Make window behave like a dialog */
static void
dialogize (GtkWidget *menued, GtkWidget *parent_widget)
{
  GtkWidget *transient_parent;

  gtk_signal_connect (GTK_OBJECT (menued), "key_press_event",
		      GTK_SIGNAL_FUNC (glade_util_check_key_is_esc),
		      GINT_TO_POINTER (GladeEscDestroys));
  transient_parent = glade_util_get_toplevel (parent_widget);
  if (GTK_IS_WINDOW (transient_parent))
    {
      gtk_window_set_transient_for (GTK_WINDOW (menued),
				    GTK_WINDOW (transient_parent));
    }
}

static void
gb_menu_bar_on_edit_menu (GtkWidget *button,
			  gpointer data)
{
  GtkWidget *menubar, *menued;

  menubar = property_get_widget ();
  g_return_if_fail (GTK_IS_MENU_BAR (menubar));

  menued = glade_menu_editor_new (current_project, GTK_MENU_SHELL (menubar));
  dialogize (menued, button);
  gtk_widget_show (GTK_WIDGET (menued));
}


/*
 * Gets the properties of the widget. This is used for both displaying the
 * properties in the property editor, and also for saving the properties.
 */
static void
gb_menu_bar_get_properties(GtkWidget *widget, GbWidgetGetArgData *data)
{
  gint i;

  for (i = 0; i < sizeof (GbShadowValues) / sizeof (GbShadowValues[0]); i++)
    {
      if (GbShadowValues[i] == GTK_MENU_BAR (widget)->shadow_type)
	gb_widget_output_choice (data, Shadow, i, GbShadowSymbols[i]);
    }
}



/*
 * Sets the properties of the widget. This is used for both applying the
 * properties changed in the property editor, and also for loading.
 */
static void
gb_menu_bar_set_properties(GtkWidget *widget, GbWidgetSetArgData *data)
{
  gchar *shadow;
  gint i;

  shadow = gb_widget_input_choice (data, Shadow);
  if (data->apply)
    {
      for (i = 0; i < sizeof (GbShadowValues) / sizeof (GbShadowValues[0]);
	   i++)
	{
	  if (!strcmp (shadow, GbShadowChoices[i])
	      || !strcmp (shadow, GbShadowSymbols[i]))
	    {
	      gtk_menu_bar_set_shadow_type (GTK_MENU_BAR (widget),
					    GbShadowValues[i]);
	      break;
	    }
	}
    }
}



/*
 * Adds menu items to a context menu which is just about to appear!
 * Add commands to aid in editing a GtkMenuBar, with signals pointing to
 * other functions in this file.
 */
static void
gb_menu_bar_create_popup_menu(GtkWidget *widget, GbWidgetCreateMenuData *data)
{
  GtkWidget *menuitem;

  menuitem = gtk_menu_item_new_with_label (_("Edit Menu"));
  gtk_widget_show (menuitem);
  gtk_menu_append (GTK_MENU (data->menu), menuitem);
  gtk_signal_connect (GTK_OBJECT (menuitem), "activate",
		      GTK_SIGNAL_FUNC (gb_menu_bar_on_edit_menu_activate),
		      widget);
}


static void
gb_menu_bar_on_edit_menu_activate (GtkWidget *menuitem,
				   GtkWidget *menubar)
{
  GtkWidget *menued;

  menued = glade_menu_editor_new (current_project, GTK_MENU_SHELL (menubar));
  dialogize (menued, menubar);
  gtk_widget_show (GTK_WIDGET (menued));
}


/*
 * Writes the source code needed to create this widget.
 * You have to output everything necessary to create the widget here, though
 * there are some convenience functions to help.
 */
static void
gb_menu_bar_write_source (GtkWidget * widget, GbWidgetWriteSourceData * data)
{
  gboolean created_menu = FALSE;
  gint shadow = 0, i;

#ifdef USE_GNOME
  /* For Gnome projects the menus are created using GnomeUIInfo structs, so
     we just create the start of the struct here. In a GnomeApp dock item,
     the code to add the menu to the GnomeApp is in output in gnomedockitem.c.
     If the menubar is not in a GnomeApp, we have to output the code to create
     it here. */
  if (data->project->gnome_support)
    {
      glade_gnome_start_menu_source (GTK_MENU_SHELL (widget), data);

      if (widget->parent && glade_gnome_is_app_dock_item (widget->parent))
	{
	  /* FIXME: should we set some standard properties? */
	  gb_widget_write_add_child_source (widget, data);
	  return;
	}
      else
	{
	  if (data->create_widget)
	    {
	      source_add (data, "  %s = gtk_menu_bar_new ();\n", data->wname);
	    }

	  gb_widget_write_standard_source (widget, data);

	  source_add (data,
		      "  gnome_app_fill_menu (GTK_MENU_SHELL (%s), %s_uiinfo,\n"
		      "                       NULL, FALSE, 0);\n",
		      data->wname, data->real_wname);
	  created_menu = TRUE;
	}
    }
#endif

  if (!created_menu)
    {
      if (data->create_widget)
	{
	  source_add (data, "  %s = gtk_menu_bar_new ();\n", data->wname);
	}

      gb_widget_write_standard_source (widget, data);
    }

  if (GTK_MENU_BAR (widget)->shadow_type != GTK_SHADOW_OUT)
    {
      for (i = 0; i < sizeof (GbShadowValues) / sizeof (GbShadowValues[0]);
	   i++)
	if (GbShadowValues[i] == GTK_MENU_BAR (widget)->shadow_type)
	  {
	    shadow = i;
	    break;
	  }
      source_add (data,
		  "  gtk_menu_bar_set_shadow_type (GTK_MENU_BAR (%s), %s);\n",
		  data->wname, GbShadowSymbols[shadow]);
    }
}



/*
 * Initializes the GbWidget structure.
 * I've placed this at the end of the file so we don't have to include
 * declarations of all the functions.
 */
GbWidget *
gb_menu_bar_init ()
{
  /* Initialise the GTK type */
  gtk_menu_bar_get_type ();

  /* Initialize the GbWidget structure */
  gb_widget_init_struct (&gbwidget);

  /* Fill in the pixmap struct & tooltip */
  gbwidget.pixmap_struct = menubar_xpm;
  gbwidget.tooltip = _("Menu Bar");

  /* Fill in any functions that this GbWidget has */
   gbwidget.gb_widget_new               = gb_menu_bar_new;
   gbwidget.gb_widget_create_properties = gb_menu_bar_create_properties;
   gbwidget.gb_widget_create_popup_menu = gb_menu_bar_create_popup_menu;
   gbwidget.gb_widget_get_properties    = gb_menu_bar_get_properties;
   gbwidget.gb_widget_set_properties    = gb_menu_bar_set_properties;
   gbwidget.gb_widget_write_source	= gb_menu_bar_write_source;

  return &gbwidget;
}
