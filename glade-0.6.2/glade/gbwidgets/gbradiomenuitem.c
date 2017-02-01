
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
#include <gtk/gtkradiomenuitem.h>
#include "../gb.h"
#include "../glade_gnome.h"

/* Include the 21x21 icon pixmap for this widget, to be used in the palette */
#include "../graphics/radiomenuitem.xpm"

/*
 * This is the GbWidget struct for this widget (see ../gbwidget.h).
 * It is initialized in the init() function at the end of this file
 */
static GbWidget gbwidget;

static gchar *Label = "RadioMenuItem|GtkItem::label";
static gchar *State = "RadioMenuItem|GtkCheckMenuItem::active";
static gchar *Toggle = "RadioMenuItem|GtkCheckMenuItem::always_show_toggle";
static gchar *Group = "GtkRadioMenuItem::group";

typedef struct _GbFindGroupData GbFindGroupData;
struct _GbFindGroupData
  {
    gchar *group_name;
    GSList *group;
  };

static void get_radio_menu_item_groups (GtkWidget * widget, GList ** groups);
static void find_parents_group (GtkWidget * widget, GSList ** group_list);
static void find_group (GtkWidget * widget, GbFindGroupData * find_group_data);
static void reset_group (GtkRadioMenuItem * radio_menu_item);
static void update_group (GSList * group);

/******
 * NOTE: To use these functions you need to uncomment them AND add a pointer
 * to the function in the GbWidget struct at the end of this file.
 ******/

/*
 * Creates a new GtkWidget of class GtkRadioMenuItem, performing any specialized
 * initialization needed for the widget to work correctly in this environment.
 * If a dialog box is used to initialize the widget, return NULL from this
 * function, and call data->callback with your new widget when it is done.
 * If the widget needs a special destroy handler, add a signal here.
 */
GtkWidget *
gb_radio_menu_item_new (GbWidgetNewData * data)
{
  GtkWidget *new_widget;
  GSList *group_list = NULL;

  if (data->parent)
    gtk_container_foreach (GTK_CONTAINER (data->parent),
			   (GtkCallback) find_parents_group, &group_list);

  if (data->action == GB_CREATING)
    new_widget = gtk_radio_menu_item_new_with_label (group_list, data->name);
  else
    new_widget = gtk_radio_menu_item_new (group_list);
  return new_widget;
}



/*
 * Creates the components needed to edit the extra properties of this widget.
 */
static void
gb_radio_menu_item_create_properties (GtkWidget * widget, GbWidgetCreateArgData
				      * data)
{
  property_add_text (Label, _("Label:"), _("The text to display"), 2);
  property_add_bool (State, _("Initially On:"),
		     _("If the radio menu item is initially on"));
  property_add_bool (Toggle, _("Indicator:"),
		     _("If the toggle indicator is always shown"));
  property_add_combo (Group, _("Group:"),
		      _("The radio menu item group (the default is all radio menu items with the same parent)"),
		      NULL);
}



/*
 * Gets the properties of the widget. This is used for both displaying the
 * properties in the property editor, and also for saving the properties.
 */
static void
gb_radio_menu_item_get_properties (GtkWidget * widget, GbWidgetGetArgData * data)
{
  gb_widget_output_child_label (widget, data, Label);

  gb_widget_output_bool (data, State, data->widget_data->flags & GLADE_ACTIVE);
  gb_widget_output_bool (data, Toggle, GTK_CHECK_MENU_ITEM (widget)->always_show_toggle);

  /* If we're showing we need to display the list of groups to choose from.
     We walk the tree of widgets in this component, and if a widget is
     a radio menu item, we see if it has a group and if it is already in the
     list and if not we add it. */
  if (data->action == GB_SHOWING)
    {
      GList *groups = NULL;
      get_radio_menu_item_groups (gtk_widget_get_toplevel (widget), &groups);
      property_set_combo_strings (Group, groups);
      g_list_free (groups);
    }

  gb_widget_output_combo (data, Group,
			  gtk_object_get_data (GTK_OBJECT (widget), Group));
}


static void
get_radio_menu_item_groups (GtkWidget * widget, GList ** groups)
{
  if (GTK_IS_RADIO_MENU_ITEM (widget))
    {
      gchar *group = gtk_object_get_data (GTK_OBJECT (widget), Group);
      gboolean found = FALSE;
      if (group)
	{
	  /* Check if group is already in list. */
	  GList *item = *groups;
	  while (item)
	    {
	      if (!strcmp ((gchar *) item->data, group))
		{
		  found = TRUE;
		  break;
		}
	      item = g_list_next (item);
	    }

	  if (!found)
	    *groups = g_list_insert_sorted (*groups, group, g_str_equal);
	}
    }

  if (GTK_IS_CONTAINER (widget))
    gtk_container_foreach (GTK_CONTAINER (widget),
			   (GtkCallback) get_radio_menu_item_groups, groups);
}



/*
 * Sets the properties of the widget. This is used for both applying the
 * properties changed in the property editor, and also for loading.
 */
static void
gb_radio_menu_item_set_properties (GtkWidget * widget, GbWidgetSetArgData * data)
{
  gboolean state, toggle;
  gchar *group_name;
  GSList *group = NULL, *current_group;
  GbFindGroupData find_group_data;

  gb_widget_input_child_label (widget, data, Label);

  state = gb_widget_input_bool (data, State);
  if (data->apply)
    {
      gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (widget), state);
      if (state)
	data->widget_data->flags |= GLADE_ACTIVE;
      else
	data->widget_data->flags &= ~GLADE_ACTIVE;
    }

  toggle = gb_widget_input_bool (data, Toggle);
  if (data->apply)
    gtk_check_menu_item_set_show_toggle (GTK_CHECK_MENU_ITEM (widget), toggle);

  /* Find any widgets in given group and set this widgets group.
     If group is NULL try to find radiomenuitems with same parent and use
     their group. If these don't succeed, set group to NULL. */
  group_name = gb_widget_input_combo (data, Group);
  if (data->apply)
    {
      if (group_name && group_name[0] == '\0')
	group_name = NULL;
      current_group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (widget));

      if (group_name == NULL)
	gtk_container_foreach (GTK_CONTAINER (widget->parent),
			       (GtkCallback) find_parents_group, &group);
      else
	{
	  find_group_data.group_name = group_name;
	  find_group_data.group = NULL;
	  find_group (gtk_widget_get_toplevel (widget), &find_group_data);
	  group = find_group_data.group;
	}

      g_free (gtk_object_get_data (GTK_OBJECT (widget), Group));
      gtk_object_set_data (GTK_OBJECT (widget), Group, g_strdup (group_name));

      /* This crashes if we set the group to NULL, so we have to reset the
         group ourself. We only set the group if it has changed. */
      if (group)
	{
	  if (group != current_group)
	    {
	      gtk_radio_menu_item_set_group (GTK_RADIO_MENU_ITEM (widget),
					     group);
	      update_group (current_group);
	      update_group (gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (widget)));
	    }
	}
      else
	{
	  if (current_group->next || current_group->data != widget)
	    {
	      reset_group (GTK_RADIO_MENU_ITEM (widget));
	      update_group (current_group);
	      update_group (gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (widget)));
	    }
	}
    }
}


/* This resets the radio menu item's group so it is in a group of its own.
   Unfortunately the radio menu item widget hasn't got a function to do this. */
static void
reset_group (GtkRadioMenuItem * radio_menu_item)
{
  GSList *slist;
  radio_menu_item->group = g_slist_remove (radio_menu_item->group,
					   radio_menu_item);
  /* Update the pointer to the group in all the other radio menu items. */
  for (slist = radio_menu_item->group; slist; slist = slist->next)
    GTK_RADIO_MENU_ITEM (slist->data)->group = radio_menu_item->group;
  radio_menu_item->group = g_slist_prepend (NULL, radio_menu_item);
}


static void
find_parents_group (GtkWidget * widget, GSList ** group)
{
  /* If a group has already been found, return. */
  if (*group)
    return;

  if (GTK_IS_RADIO_MENU_ITEM (widget))
    {
      gchar *group_name = gtk_object_get_data (GTK_OBJECT (widget), Group);
      /* Check if this radio menu item is using the default (parent's) group. */
      if (group_name == NULL)
	{
	  *group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (widget));
	}
    }
}


static void
find_group (GtkWidget * widget, GbFindGroupData * find_group_data)
{
  /* If group has been found just return. */
  if (find_group_data->group)
    return;

  if (GTK_IS_RADIO_MENU_ITEM (widget))
    {
      gchar *group_name = gtk_object_get_data (GTK_OBJECT (widget), Group);
      if (group_name && !strcmp (group_name, find_group_data->group_name))
	{
	  find_group_data->group
	    = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (widget));
	  return;
	}
    }

  if (GTK_IS_CONTAINER (widget))
    gtk_container_foreach (GTK_CONTAINER (widget),
			   (GtkCallback) find_group, find_group_data);
}


/* This tries to ensure that one and only one radio menu item in the given
   group is selected. It's used because GtkRadioMenuItem doesn't handle this
   when groups are changed. */
static void
update_group (GSList * group)
{
  GtkRadioMenuItem *menuitem;
  GSList *slist;
  gboolean found_selected = FALSE;
  for (slist = group; slist; slist = slist->next)
    {
      menuitem = GTK_RADIO_MENU_ITEM (slist->data);
      if (GTK_CHECK_MENU_ITEM (menuitem)->active)
	{
	  if (found_selected)
	    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menuitem),
					    FALSE);
	  else
	    found_selected = TRUE;
	}
    }
  /* If none are currently selected, select the first. */
  if (!found_selected)
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (group->data), TRUE);
}



/*
 * Adds menu items to a context menu which is just about to appear!
 * Add commands to aid in editing a GtkRadioMenuItem, with signals pointing to
 * other functions in this file.
 */
static void
gb_radio_menu_item_create_popup_menu (GtkWidget * widget, GbWidgetCreateMenuData
				      * data)
{
  /* Add command to remove child label. */
  gb_widget_create_child_label_popup_menu (widget, data);
}



/*
 * Writes the source code needed to create this widget.
 * You have to output everything necessary to create the widget here, though
 * there are some convenience functions to help.
 */
static void
gb_radio_menu_item_write_source (GtkWidget * widget, GbWidgetWriteSourceData *
				 data)
{
  GtkWidget *child = GTK_BIN (widget)->child;
  gchar *label_text;
  gchar buffer[256];
  gchar *group_name;

#ifdef USE_GNOME
  if (data->project->gnome_support)
    {
      glade_gnome_write_menu_item_source (GTK_MENU_ITEM (widget), data);
      return;
    }
#endif

  group_name = gtk_object_get_data (GTK_OBJECT (widget), Group);
  if (!group_name)
    group_name = gtk_widget_get_name (widget->parent);

  if (data->create_widget)
    {
      /* Make sure the temporary group list variable is declared. */
      group_name = source_create_valid_identifier (group_name);
      sprintf (buffer, "  GSList *%s_group = NULL;\n", group_name);
      source_ensure_decl (data, buffer);

      if (child && GTK_IS_LABEL (child) && !GB_IS_GB_WIDGET (child))
	{
	  label_text = glade_util_get_label_text (child);
	  /* If there is an underlined accelerator, set up the accel signal. */
	  if (strchr (label_text, '_'))
	    {
	      source_ensure_decl (data, "  guint tmp_key;\n");
	      source_add (data,
			  "  %s = gtk_radio_menu_item_new_with_label (%s_group, \"\");\n",
			  data->wname, group_name);
	      gb_menu_item_write_accel_source (widget, data, label_text);
	    }
	  else
	    {
	      source_add (data,
			  "  %s = gtk_radio_menu_item_new_with_label (%s_group, %s);\n",
			  data->wname, group_name,
			  source_make_string (label_text, data->use_gettext));
	    }
	  g_free (label_text);
	}
      else
	{
	  source_add (data, "  %s = gtk_radio_menu_item_new (%s_group);\n",
		      data->wname, group_name);
	}
      source_add (data,
      "  %s_group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (%s));\n",
		  group_name, data->wname);
      g_free (group_name);
    }

  gb_widget_write_standard_source (widget, data);

  if (data->widget_data->flags & GLADE_ACTIVE)
    {
      source_add (data,
      "  gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (%s), TRUE);\n",
		  data->wname);
    }

  if (GTK_CHECK_MENU_ITEM (widget)->always_show_toggle)
    {
      source_add (data,
		  "  gtk_check_menu_item_set_show_toggle (GTK_CHECK_MENU_ITEM (%s), TRUE);\n",
		  data->wname);
    }
}



/*
 * Initializes the GbWidget structure.
 * I've placed this at the end of the file so we don't have to include
 * declarations of all the functions.
 */
GbWidget *
gb_radio_menu_item_init ()
{
  /* Initialise the GTK type */
  gtk_radio_menu_item_get_type ();

  /* Initialize the GbWidget structure */
  gb_widget_init_struct (&gbwidget);

  /* Fill in the pixmap struct & tooltip */
  gbwidget.pixmap_struct = radiomenuitem_xpm;
  gbwidget.tooltip = _("Radio Menu Item");

  /* Fill in any functions that this GbWidget has */
  gbwidget.gb_widget_new = gb_radio_menu_item_new;
  gbwidget.gb_widget_create_properties = gb_radio_menu_item_create_properties;
  gbwidget.gb_widget_get_properties = gb_radio_menu_item_get_properties;
  gbwidget.gb_widget_set_properties = gb_radio_menu_item_set_properties;
  gbwidget.gb_widget_create_popup_menu = gb_radio_menu_item_create_popup_menu;
  gbwidget.gb_widget_write_source = gb_radio_menu_item_write_source;

  return &gbwidget;
}
