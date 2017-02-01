
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

#include <gtk/gtktree.h>
#include "../gb.h"

/* Include the 21x21 icon pixmap for this widget, to be used in the palette */
#include "../graphics/tree.xpm"

/*
 * This is the GbWidget struct for this widget (see ../gbwidget.h).
 * It is initialized in the init() function at the end of this file
 */
static GbWidget gbwidget;

static gchar *Mode = "GtkTree::selection_mode";
static gchar *ViewMode = "GtkTree::view_mode";
static gchar *ViewLines = "GtkTree::view_line";

static const gchar *GbModeChoices[] =
{"Single", "Browse", "Multiple",
 "Extended", NULL};
static const gint GbModeValues[] =
{
  GTK_SELECTION_SINGLE,
  GTK_SELECTION_BROWSE,
  GTK_SELECTION_MULTIPLE,
  GTK_SELECTION_EXTENDED
};
static const gchar *GbModeSymbols[] =
{
  "GTK_SELECTION_SINGLE",
  "GTK_SELECTION_BROWSE",
  "GTK_SELECTION_MULTIPLE",
  "GTK_SELECTION_EXTENDED"
};

static const gchar *GbViewModeChoices[] =
{"Line", "Item", NULL};
static const gint GbViewModeValues[] =
{
  GTK_TREE_VIEW_LINE,
  GTK_TREE_VIEW_ITEM
};
static const gchar *GbViewModeSymbols[] =
{
  "GTK_TREE_VIEW_LINE",
  "GTK_TREE_VIEW_ITEM"
};

/******
 * NOTE: To use these functions you need to uncomment them AND add a pointer
 * to the function in the GbWidget struct at the end of this file.
 ******/

/*
 * Creates a new GtkWidget of class GtkTree, performing any specialized
 * initialization needed for the widget to work correctly in this environment.
 * If a dialog box is used to initialize the widget, return NULL from this
 * function, and call data->callback with your new widget when it is done.
 * If the widget needs a special destroy handler, add a signal here.
 */
/*
   GtkWidget*
   gb_tree_new(GbWidgetNewData *data)
   {

   }
 */



/*
 * Creates the components needed to edit the extra properties of this widget.
 */
static void
gb_tree_create_properties (GtkWidget * widget, GbWidgetCreateArgData * data)
{
  property_add_choice (Mode, _("Select Mode:"),
		       _("The selection mode of the tree"),
		       GbModeChoices);
  property_add_choice (ViewMode, _("View Mode:"),
		       _("The view mode of the tree"),
		       GbViewModeChoices);
  property_add_bool (ViewLines, _("View Lines:"),
		     _("If lines are displayed between items"));
}



/*
 * Gets the properties of the widget. This is used for both displaying the
 * properties in the property editor, and also for saving the properties.
 */
static void
gb_tree_get_properties (GtkWidget * widget, GbWidgetGetArgData * data)
{
  gint i;

  for (i = 0; i < sizeof (GbModeValues) / sizeof (GbModeValues[0]); i++)
    {
      if (GbModeValues[i] == GTK_TREE (widget)->selection_mode)
	gb_widget_output_choice (data, Mode, i, GbModeSymbols[i]);
    }

  for (i = 0; i < sizeof (GbViewModeValues) / sizeof (GbViewModeValues[0]);
       i++)
    {
      if (GbViewModeValues[i] == GTK_TREE (widget)->view_mode)
	gb_widget_output_choice (data, ViewMode, i, GbViewModeSymbols[i]);
    }

  gb_widget_output_bool (data, ViewLines, GTK_TREE (widget)->view_line);
}



/*
 * Sets the properties of the widget. This is used for both applying the
 * properties changed in the property editor, and also for loading.
 */
static void
gb_tree_set_properties (GtkWidget * widget, GbWidgetSetArgData * data)
{
  gchar *mode, *view_mode;
  gint i;
  gboolean view_lines;

  mode = gb_widget_input_choice (data, Mode);
  if (data->apply)
    {
      for (i = 0; i < sizeof (GbModeValues) / sizeof (GbModeValues[0]); i++)
	{
	  if (!strcmp (mode, GbModeChoices[i])
	      || !strcmp (mode, GbModeSymbols[i]))
	    {
	      gtk_tree_set_selection_mode (GTK_TREE (widget), GbModeValues[i]);
	      break;
	    }
	}
    }

  view_mode = gb_widget_input_choice (data, ViewMode);
  if (data->apply)
    {
      for (i = 0; i < sizeof (GbViewModeValues) / sizeof (GbViewModeValues[0]);
	   i++)
	{
	  if (!strcmp (view_mode, GbViewModeChoices[i])
	      || !strcmp (view_mode, GbViewModeSymbols[i]))
	    {
	      gtk_tree_set_view_mode (GTK_TREE (widget), GbViewModeValues[i]);
	      break;
	    }
	}
    }

  view_lines = gb_widget_input_bool (data, ViewLines);
  if (data->apply)
    gtk_tree_set_view_lines (GTK_TREE (widget), view_lines);
}



/*
 * Adds menu items to a context menu which is just about to appear!
 * Add commands to aid in editing a GtkTree, with signals pointing to
 * other functions in this file.
 */
/*
   static void
   gb_tree_create_popup_menu(GtkWidget *widget, GbWidgetCreateMenuData *data)
   {

   }
 */



/*
 * Writes the source code needed to create this widget.
 * You have to output everything necessary to create the widget here, though
 * there are some convenience functions to help.
 */
static void
gb_tree_write_source (GtkWidget * widget, GbWidgetWriteSourceData * data)
{
  gint i;

  if (data->create_widget)
    {
      source_add (data, "  %s = gtk_tree_new ();\n", data->wname);
    }

  gb_widget_write_standard_source (widget, data);

  if (GTK_TREE (widget)->selection_mode != GTK_SELECTION_SINGLE)
    {
      for (i = 0; i < sizeof (GbModeValues) / sizeof (GbModeValues[0]); i++)
	{
	  if (GbModeValues[i] == GTK_TREE (widget)->selection_mode)
	    source_add (data, "  gtk_tree_set_selection_mode (GTK_TREE (%s), %s);\n",
			data->wname, GbModeSymbols[i]);
	}
    }

  if (GTK_TREE (widget)->view_mode != GTK_TREE_VIEW_LINE)
    {
      for (i = 0; i < sizeof (GbViewModeValues) / sizeof (GbViewModeValues[0]);
	   i++)
	{
	  if (GbViewModeValues[i] == GTK_TREE (widget)->view_mode)
	    source_add (data, "  gtk_tree_set_view_mode (GTK_TREE (%s), %s);\n",
			data->wname, GbViewModeSymbols[i]);
	}
    }

  if (!GTK_TREE (widget)->view_line)
    source_add (data, "  gtk_tree_set_view_lines (GTK_TREE (%s), FALSE);\n",
		data->wname);
}



/*
 * Initializes the GbWidget structure.
 * I've placed this at the end of the file so we don't have to include
 * declarations of all the functions.
 */
GbWidget *
gb_tree_init ()
{
  /* Initialise the GTK type */
  gtk_tree_get_type ();

  /* Initialize the GbWidget structure */
  gb_widget_init_struct (&gbwidget);

  /* Fill in the pixmap struct & tooltip */
  gbwidget.pixmap_struct = tree_xpm;
  gbwidget.tooltip = _("Tree");

  /* Fill in any functions that this GbWidget has */
  gbwidget.gb_widget_create_properties = gb_tree_create_properties;
  gbwidget.gb_widget_get_properties = gb_tree_get_properties;
  gbwidget.gb_widget_set_properties = gb_tree_set_properties;
  gbwidget.gb_widget_write_source = gb_tree_write_source;
/*
   gbwidget.gb_widget_new               = gb_tree_new;
   gbwidget.gb_widget_create_popup_menu = gb_tree_create_popup_menu;
 */

  return &gbwidget;
}
