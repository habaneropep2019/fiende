
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

#include <gtk/gtklabel.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtkoptionmenu.h>
#include "../gb.h"

/* Include the 21x21 icon pixmap for this widget, to be used in the palette */
#include "../graphics/optionmenu.xpm"

/*
 * This is the GbWidget struct for this widget (see ../gbwidget.h).
 * It is initialized in the init() function at the end of this file
 */
static GbWidget gbwidget;

static gchar *Items = "GtkOptionMenu::items";
static gchar *Choice = "GtkOptionMenu::initial_choice";

#define	ITEMS_BUFFER_SIZE	8192

typedef struct _GbAddLabelData GbAddLabelData;
struct _GbAddLabelData
  {
    gchar *items;
    GtkWidget *option_menu;
    gboolean buffer_overflow;
    gint index;
    gint selected_index;
  };

typedef struct _GbWriteMenuItemSourceData GbWriteMenuItemSourceData;
struct _GbWriteMenuItemSourceData
  {
    GtkWidget *option_menu;
    GbWidgetWriteSourceData *data;
    gint index;
    gint selected_index;
  };

static void add_label (GtkWidget * widget, GbAddLabelData * add_label_data);
static void add_menuitem_to_source (GtkWidget * widget,
				    GbWriteMenuItemSourceData * write_data);
static void gb_option_menu_on_option_selected (GtkWidget *menu,
					       GtkWidget *option_menu);

/******
 * NOTE: To use these functions you need to uncomment them AND add a pointer
 * to the function in the GbWidget struct at the end of this file.
 ******/

/*
 * Creates a new GtkWidget of class GtkOptionMenu, performing any specialized
 * initialization needed for the widget to work correctly in this environment.
 * If a dialog box is used to initialize the widget, return NULL from this
 * function, and call data->callback with your new widget when it is done.
 * If the widget needs a special destroy handler, add a signal here.
 */
GtkWidget *
gb_option_menu_new (GbWidgetNewData * data)
{
  GtkWidget *new_widget;
  new_widget = gtk_option_menu_new ();
  gtk_option_menu_set_menu (GTK_OPTION_MENU (new_widget), gtk_menu_new ());
  gtk_signal_connect (GTK_OBJECT (GTK_OPTION_MENU (new_widget)->menu),
                      "deactivate",
		      GTK_SIGNAL_FUNC (gb_option_menu_on_option_selected),
                      new_widget);
  return new_widget;
}


static void
gb_option_menu_on_option_selected (GtkWidget *menu,
				   GtkWidget *option_menu)
{
  GtkWidget *active_item;
  gint item_index;
  
  if (property_get_widget () != option_menu)
    return;

  active_item = gtk_menu_get_active (GTK_MENU (menu));
  item_index = g_list_index (GTK_MENU_SHELL (menu)->children, active_item);

  property_set_auto_apply (FALSE);
  property_set_int (Choice, item_index);
  property_set_auto_apply (TRUE);
}



/*
 * Creates the components needed to edit the extra properties of this widget.
 */
static void
gb_option_menu_create_properties (GtkWidget * widget, GbWidgetCreateArgData *
				  data)
{
  property_add_text (Items, _("Items:"),
		     _("The items in the option menu, one per line"), 5);
  property_add_int_range (Choice, _("Initial Choice:"),
			  _("The index of the initially selected item"),
			  0, 10000, 1, 10, 1);
}



/*
 * Gets the properties of the widget. This is used for both displaying the
 * properties in the property editor, and also for saving the properties.
 */
static void
gb_option_menu_get_properties (GtkWidget * widget, GbWidgetGetArgData * data)
{
  GbAddLabelData add_label_data;
  gchar items[ITEMS_BUFFER_SIZE];

  /* Clear the buffer and make sure it's seen as valid */
  items[0] = '\0';
  add_label_data.items = items;
  add_label_data.option_menu = widget;
  add_label_data.buffer_overflow = FALSE;
  add_label_data.index = 0;
  add_label_data.selected_index = 0;
  gtk_container_foreach (GTK_CONTAINER (GTK_OPTION_MENU (widget)->menu),
			 (GtkCallback) add_label, &add_label_data);
  if (add_label_data.buffer_overflow)
    {
      gb_widget_output_text (data, Items, "");
      gb_widget_output_int (data, Choice, 0);
      if (data->action == GB_SHOWING)
	{
	  property_set_sensitive (Items, FALSE);
	  property_set_sensitive (Choice, FALSE);
	}
    }
  else
    {
      gb_widget_output_translatable_text_in_lines (data, Items, items);
      gb_widget_output_int (data, Choice, add_label_data.selected_index);
      if (data->action == GB_SHOWING)
	{
	  property_set_sensitive (Items, TRUE);
	  property_set_sensitive (Choice, TRUE);
	}
    }
}


static void
add_label (GtkWidget * widget, GbAddLabelData * add_label_data)
{
  gchar *label_text;
  GtkWidget *menuitem, *child = GTK_BIN (widget)->child;

  /* Return if list has already been marked invalid (i.e. not all items are
     labels or the list is too big. */
  if (add_label_data->buffer_overflow)
    return;

  /* The currently selected menuitem's child is held in the option menu's
     child rather than the menuitem. */
  menuitem = GTK_OPTION_MENU (add_label_data->option_menu)->menu_item;
  if (child == NULL && menuitem == widget)
    {
      child = GTK_BUTTON (add_label_data->option_menu)->child;
      add_label_data->selected_index = add_label_data->index;
    }
  add_label_data->index++;

  if (child && GTK_IS_LABEL (child))
    {
      gtk_label_get (GTK_LABEL (child), &label_text);
      if (strlen (add_label_data->items) + strlen (label_text) + 2
	  < ITEMS_BUFFER_SIZE)
	{
	  strcat (add_label_data->items, label_text);
	  strcat (add_label_data->items, "\n");
	  return;
	}
    }

  /* If we reach here, the list item is not a label or the list is too big */
  add_label_data->buffer_overflow = TRUE;
}


/*
 * Sets the properties of the widget. This is used for both applying the
 * properties changed in the property editor, and also for loading.
 */
static void
gb_option_menu_set_properties (GtkWidget * widget, GbWidgetSetArgData * data)
{
  gchar *items;
  GtkWidget *menu, *menuitem;
  gint choice;

  items = gb_widget_input_text (data, Items);
  if (data->apply)
    {
      gchar *pos = items;
      gchar *items_end = &items[strlen (items)];

      menu = gtk_menu_new ();
      gtk_signal_connect (GTK_OBJECT (menu), "deactivate",
			  GTK_SIGNAL_FUNC (gb_option_menu_on_option_selected),
			  widget);

      while (pos < items_end)
	{
	  gchar *item_end = strchr (pos, '\n');
	  if (item_end == NULL)
	    item_end = items_end;
	  *item_end = '\0';

	  menuitem = gtk_menu_item_new_with_label (pos);
	  gtk_widget_show (menuitem);
	  gtk_menu_append (GTK_MENU (menu), menuitem);

	  pos = item_end + 1;
	}
      gtk_option_menu_set_menu (GTK_OPTION_MENU (widget), menu);
    }
  if (data->action == GB_APPLYING)
    g_free (items);

  choice = gb_widget_input_int (data, Choice);
  if (data->apply)
    {
      gtk_option_menu_set_history (GTK_OPTION_MENU (widget), choice);
    }
}



/*
 * Adds menu items to a context menu which is just about to appear!
 * Add commands to aid in editing a GtkOptionMenu, with signals pointing to
 * other functions in this file.
 */
/*
   static void
   gb_option_menu_create_popup_menu(GtkWidget *widget, GbWidgetCreateMenuData *data)
   {

   }
 */



/*
 * Writes the source code needed to create this widget.
 * You have to output everything necessary to create the widget here, though
 * there are some convenience functions to help.
 */
static void
gb_option_menu_write_source (GtkWidget * widget, GbWidgetWriteSourceData * data)
{
  GbWriteMenuItemSourceData write_data;

  if (data->create_widget)
    {
      source_add (data, "  %s = gtk_option_menu_new ();\n", data->wname);
    }

  gb_widget_write_standard_source (widget, data);

  source_add_decl (data, "  GtkWidget *%s_menu;\n", data->real_wname);

  source_add (data, "  %s_menu = gtk_menu_new ();\n", data->real_wname);

  /* Make sure the temporary menuitem widget is declared, if we need it. */
  if (GTK_MENU_SHELL (GTK_OPTION_MENU (widget)->menu)->children)
    source_ensure_decl (data, "  GtkWidget *glade_menuitem;\n");

  write_data.option_menu = widget;
  write_data.data = data;
  write_data.index = 0;
  write_data.selected_index = 0;
  gtk_container_foreach (GTK_CONTAINER (GTK_OPTION_MENU (widget)->menu),
			 (GtkCallback) add_menuitem_to_source, &write_data);

  source_add (data,
	    "  gtk_option_menu_set_menu (GTK_OPTION_MENU (%s), %s_menu);\n",
	      data->wname, data->real_wname);

  if (write_data.selected_index != 0)
    source_add (data,
	      "  gtk_option_menu_set_history (GTK_OPTION_MENU (%s), %i);\n",
		data->wname, write_data.selected_index);
}


static void
add_menuitem_to_source (GtkWidget * widget,
			GbWriteMenuItemSourceData * write_data)
{
  gchar *label_text;
  GtkWidget *menuitem, *child = GTK_BIN (widget)->child;

  menuitem = GTK_OPTION_MENU (write_data->option_menu)->menu_item;
  if (child == NULL && menuitem == widget)
    {
      child = GTK_BUTTON (write_data->option_menu)->child;
      write_data->selected_index = write_data->index;
    }

  if (child && GTK_IS_LABEL (child))
    {
      gtk_label_get (GTK_LABEL (child), &label_text);

      source_add (write_data->data,
		  "  glade_menuitem = gtk_menu_item_new_with_label (%s);\n"
		  "  gtk_widget_show (glade_menuitem);\n"
		  "  gtk_menu_append (GTK_MENU (%s_menu), glade_menuitem);\n",
		  source_make_string (label_text,
				      write_data->data->use_gettext),
		  write_data->data->real_wname);
    }

  write_data->index++;
}


/*
 * Initializes the GbWidget structure.
 * I've placed this at the end of the file so we don't have to include
 * declarations of all the functions.
 */
GbWidget *
gb_option_menu_init ()
{
  /* Initialise the GTK type */
  gtk_option_menu_get_type ();

  /* Initialize the GbWidget structure */
  gb_widget_init_struct (&gbwidget);

  /* Fill in the pixmap struct & tooltip */
  gbwidget.pixmap_struct = optionmenu_xpm;
  gbwidget.tooltip = _("Option Menu");

  /* Fill in any functions that this GbWidget has */
  gbwidget.gb_widget_new = gb_option_menu_new;
  gbwidget.gb_widget_create_properties = gb_option_menu_create_properties;
  gbwidget.gb_widget_get_properties = gb_option_menu_get_properties;
  gbwidget.gb_widget_set_properties = gb_option_menu_set_properties;
  gbwidget.gb_widget_write_source = gb_option_menu_write_source;
/*
   gbwidget.gb_widget_create_popup_menu = gb_option_menu_create_popup_menu;
 */

  return &gbwidget;
}
