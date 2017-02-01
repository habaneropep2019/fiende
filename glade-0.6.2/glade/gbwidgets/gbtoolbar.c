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

#include <config.h>

#ifdef USE_GNOME
#include <gnome.h>
#include "../glade_gnome.h"
#else
#include <gtk/gtkbutton.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtkradiobutton.h>
#include <gtk/gtkspinbutton.h>
#include <gtk/gtktoolbar.h>
#include <gtk/gtkvbox.h>
#endif

#include "../gb.h"

/* Include the 21x21 icon pixmap for this widget, to be used in the palette */
#include "../graphics/toolbar.xpm"

/*
 * This is the GbWidget struct for this widget (see ../gbwidget.h).
 * It is initialized in the init() function at the end of this file
 */
static GbWidget gbwidget;

static const gchar *GbToolbarIconFilenameKey = "GbIconFilename";
/* Note that we store the icon index + 1 here, so that the default "NULL"
   value is not confused with an index of 0. */
static const gchar *GbToolbarStockIconKey = "GbStockIcon";


static const gchar *Size = "GtkToolbar::size";
static const gchar *Orientation = "GtkToolbar::orientation";
/* We can't use 'style' as that clashes with widget styles. */
static const gchar *Style = "GtkToolbar::type";
static const gchar *Space = "GtkToolbar::space_size";
static const gchar *SpaceStyle = "GtkToolbar::space_style";
static const gchar *Tooltips = "GtkToolbar::tooltips";
static const gchar *Relief = "GtkToolbar::relief";

/* For children of a toolbar */
static const gchar *GbNewToolbarGroup = "GtkToolbarChild::new_group";


static const gchar *GbOrientationChoices[] =
{"Horizontal", "Vertical", NULL};
static const gint GbOrientationValues[] =
{
  GTK_ORIENTATION_HORIZONTAL,
  GTK_ORIENTATION_VERTICAL
};
static const gchar *GbOrientationSymbols[] =
{
  "GTK_ORIENTATION_HORIZONTAL",
  "GTK_ORIENTATION_VERTICAL"
};

static const gchar *GbStyleChoices[] =
{"Icons", "Text", "Both", NULL};
static const gint GbStyleValues[] =
{
  GTK_TOOLBAR_ICONS,
  GTK_TOOLBAR_TEXT,
  GTK_TOOLBAR_BOTH
};
static const gchar *GbStyleSymbols[] =
{
  "GTK_TOOLBAR_ICONS",
  "GTK_TOOLBAR_TEXT",
  "GTK_TOOLBAR_BOTH"
};

static const gchar *GbSpaceStyleChoices[] =
{"Empty", "Line", NULL};
static const gint GbSpaceStyleValues[] =
{
  GTK_TOOLBAR_SPACE_EMPTY,
  GTK_TOOLBAR_SPACE_LINE
};
static const gchar *GbSpaceStyleSymbols[] =
{
  "GTK_TOOLBAR_SPACE_EMPTY",
  "GTK_TOOLBAR_SPACE_LINE"
};

static const gchar *GbReliefChoices[] =
{
  "Normal",
  "Half",
  "None",
  NULL
};
static const gint GbReliefValues[] =
{
  GTK_RELIEF_NORMAL,
  GTK_RELIEF_HALF,
  GTK_RELIEF_NONE
};
static const gchar *GbReliefSymbols[] =
{
  "GTK_RELIEF_NORMAL",
  "GTK_RELIEF_HALF",
  "GTK_RELIEF_NONE"
};

static void show_toolbar_dialog (GbWidgetNewData * data);
static void on_toolbar_dialog_ok (GtkWidget * widget,
				  GbWidgetNewData * data);
static void on_toolbar_dialog_destroy (GtkWidget * widget,
				       GbWidgetNewData * data);

static void update_toolbar_size (GtkWidget * widget, gint size);

static void gb_toolbar_insert_before (GtkWidget * menuitem,
				      GtkWidget * child);
static void gb_toolbar_insert_after (GtkWidget * menuitem,
				     GtkWidget * child);
static void gb_toolbar_insert (GtkWidget * child,
			       gint offset);

static gboolean gb_toolbar_get_toolbar_button_widgets (GtkWidget * widget,
						       GtkWidget ** icon_return,
						       GtkWidget ** label_return);
static void gb_toolbar_set_new_toolbar_group	(GtkWidget * toolbar,
						 GtkWidget * widget,
						 gboolean new_group);
#ifdef USE_GNOME
static void gb_toolbar_set_stock_child_icon (GtkWidget *widget,
					     GtkWidget *icon,
					     gint stock_index);
#endif
static void gb_toolbar_set_child_icon (GtkWidget *widget,
				       GbWidgetSetArgData * data,
				       GtkWidget *icon,
				       gchar *filename,
				       const gchar *property_name);
static void gb_toolbar_reset_child_icon (GtkWidget *widget,
					 GtkWidget *icon);
static GtkWidget* gb_toolbar_get_toolbar_radio_group_widget (GtkWidget *widget,
							     gboolean skip_widget,
							     gboolean find_default_group);
static void gb_toolbar_check_toolbar_radio_group (GtkWidget * widget);
static void gb_toolbar_convert_toolbar_button (GtkWidget * menuitem,
					       GtkWidget * widget);

/******
 * NOTE: To use these functions you need to uncomment them AND add a pointer
 * to the function in the GbWidget struct at the end of this file.
 ******/

/*
 * Creates a new GtkWidget of class GtkToolbar, performing any specialized
 * initialization needed for the widget to work correctly in this environment.
 * If a dialog box is used to initialize the widget, return NULL from this
 * function, and call data->callback with your new widget when it is done.
 * If the widget needs a special destroy handler, add a signal here.
 */
GtkWidget*
gb_toolbar_new(GbWidgetNewData *data)
{
  GtkWidget *new_widget;

  if (data->action == GB_LOADING)
    {
      new_widget = gtk_toolbar_new (GTK_ORIENTATION_HORIZONTAL,
				    GTK_TOOLBAR_BOTH);
      return new_widget;
    }
  else
    {
      show_toolbar_dialog (data);
      return NULL;
    }
}


void
gb_toolbar_add_child (GtkWidget *widget, GtkWidget *child, GbWidgetSetArgData *data)
{
  gchar *child_name;
  gboolean new_group;

  /* Tell the load functions to use the child properties array. */
  data->loading_type = GB_CHILD_PROPERTIES;
  new_group = load_bool (data, "new_group");
  data->loading_type = GB_STANDARD_PROPERTIES;

  if (new_group)
    gtk_toolbar_append_space (GTK_TOOLBAR (widget));

  /* See if this is a toolbar button. */
  /* FIXME: This should use a static string from somewhere. */
  child_name = load_get_value (data, "child_name");
  if (child_name && !strcmp (child_name, "Toolbar:button"))
    gb_toolbar_insert_toolbar_child (GTK_TOOLBAR (widget), child, -1);
  else
    gtk_toolbar_append_widget (GTK_TOOLBAR (widget), child, NULL, NULL);
}


static void
show_toolbar_dialog (GbWidgetNewData * data)
{
  GtkWidget *dialog, *vbox, *hbox, *label, *spinbutton;
  GtkObject *adjustment;

  dialog = glade_util_create_dialog (_("New toolbar"), data->parent,
				     on_toolbar_dialog_ok, data, &vbox);
  gtk_signal_connect (GTK_OBJECT (dialog), "destroy",
		      GTK_SIGNAL_FUNC (on_toolbar_dialog_destroy), data);

  hbox = gtk_hbox_new (FALSE, 5);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 5);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 10);
  gtk_widget_show (hbox);

  label = gtk_label_new (_("Number of items:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 5);
  gtk_widget_show (label);

  adjustment = gtk_adjustment_new (3, 1, 100, 1, 10, 10);
  spinbutton = glade_util_spin_button_new (GTK_OBJECT (dialog), "items",
					   GTK_ADJUSTMENT (adjustment), 1, 0);
  gtk_box_pack_start (GTK_BOX (hbox), spinbutton, TRUE, TRUE, 5);
  gtk_widget_set_usize (spinbutton, 50, -1);
  gtk_widget_grab_focus (spinbutton);
  gtk_widget_show (spinbutton);

  gtk_widget_show (dialog);
  gtk_grab_add (dialog);
}


static void
on_toolbar_dialog_ok (GtkWidget * widget, GbWidgetNewData * data)
{
  GtkWidget *new_widget, *spinbutton, *window, *placeholder;
  gint items, i;

  window = gtk_widget_get_toplevel (widget);

  /* Only call callback if placeholder/fixed widget is still there */
  if (gb_widget_can_finish_new (data))
    {
      spinbutton = gtk_object_get_data (GTK_OBJECT (window), "items");
      g_return_if_fail (spinbutton != NULL);
      items = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (spinbutton));

      new_widget = gtk_toolbar_new (GTK_ORIENTATION_HORIZONTAL,
				    GTK_TOOLBAR_BOTH);
      for (i = 0; i < items; i++)
	{
	  placeholder = editor_new_placeholder ();
	  gtk_toolbar_append_widget (GTK_TOOLBAR (new_widget), placeholder,
				     NULL, NULL);
	}
      gb_widget_initialize (new_widget, data);
      (*data->callback) (new_widget, data);
    }
  gtk_widget_destroy (window);
}


static void
on_toolbar_dialog_destroy (GtkWidget * widget,
			   GbWidgetNewData * data)
{
  gb_widget_free_new_data (data);
  gtk_grab_remove (widget);
}


/*
 * Creates the components needed to edit the extra properties of this widget.
 */
static void
gb_toolbar_create_properties (GtkWidget * widget, GbWidgetCreateArgData * data)
{
  property_add_int_range (Size, _("Size:"),
			  _("The number of items in the toolbar"),
			  0, 1000, 1, 10, 1);
  property_add_choice (Orientation, _("Orientation:"),
		       _("The toolbar orientation"),
		       GbOrientationChoices);
  property_add_choice (Style, _("Style:"),
		       _("The toolbar style"),
		       GbStyleChoices);
  property_add_int_range (Space, _("Space Size:"),
			  _("The size of spaces in the toolbar"),
			  0, 1000, 1, 10, 1);
  property_add_choice (SpaceStyle, _("Space Style:"),
		       _("The way spaces in the toolbar are displayed"),
		       GbSpaceStyleChoices);
  property_add_choice (Relief, _("Button Relief:"),
		       _("The relief style of the toolbar buttons"),
		       GbReliefChoices);
  property_add_bool (Tooltips, _("Tooltips:"), _("If tooltips are enabled"));
}



/*
 * Gets the properties of the widget. This is used for both displaying the
 * properties in the property editor, and also for saving the properties.
 */
static void
gb_toolbar_get_properties (GtkWidget * widget, GbWidgetGetArgData * data)
{
  gint i;

  if (data->action != GB_SAVING)
    gb_widget_output_int (data, Size, GTK_TOOLBAR (widget)->num_children);

  for (i = 0; i < sizeof (GbOrientationValues)
	 / sizeof (GbOrientationValues[0]); i++)
    {
      if (GbOrientationValues[i] == GTK_TOOLBAR (widget)->orientation)
	gb_widget_output_choice (data, Orientation, i, GbOrientationSymbols[i]);
    }

  for (i = 0; i < sizeof (GbStyleValues) / sizeof (GbStyleValues[0]); i++)
    {
      if (GbStyleValues[i] == GTK_TOOLBAR (widget)->style)
	gb_widget_output_choice (data, Style, i, GbStyleSymbols[i]);
    }

  gb_widget_output_int (data, Space, GTK_TOOLBAR (widget)->space_size);

  for (i = 0; i < sizeof (GbSpaceStyleValues) / sizeof (GbSpaceStyleValues[0]);
       i++)
    {
      if (GbSpaceStyleValues[i] == GTK_TOOLBAR (widget)->space_style)
	gb_widget_output_choice (data, SpaceStyle, i, GbSpaceStyleSymbols[i]);
    }

  for (i = 0; i < sizeof (GbReliefValues) / sizeof (GbReliefValues[0]); i++)
    {
      if (GbReliefValues[i] == GTK_TOOLBAR (widget)->relief)
	gb_widget_output_choice (data, Relief, i, GbReliefSymbols[i]);
    }

  gb_widget_output_bool (data, Tooltips,
		    GTK_TOOLTIPS (GTK_TOOLBAR (widget)->tooltips)->enabled);
}



/*
 * Sets the properties of the widget. This is used for both applying the
 * properties changed in the property editor, and also for loading.
 */
static void
gb_toolbar_set_properties (GtkWidget * widget, GbWidgetSetArgData * data)
{
  gchar *orientation, *style, *space_style, *relief;
  gint i, space, size;
  gboolean tooltips;

  size = gb_widget_input_int (data, Size);
  if (data->apply)
    update_toolbar_size (widget, size);

  orientation = gb_widget_input_choice (data, Orientation);
  if (data->apply)
    {
      for (i = 0; i < sizeof (GbOrientationValues)
	   / sizeof (GbOrientationValues[0]); i++)
	{
	  if (!strcmp (orientation, GbOrientationChoices[i])
	      || !strcmp (orientation, GbOrientationSymbols[i]))
	    {
	      gtk_toolbar_set_orientation (GTK_TOOLBAR (widget),
					   GbOrientationValues[i]);
	      break;
	    }
	}
    }

  style = gb_widget_input_choice (data, Style);
  if (data->apply)
    {
      for (i = 0; i < sizeof (GbStyleValues) / sizeof (GbStyleValues[0]); i++
	)
	{
	  if (!strcmp (style, GbStyleChoices[i])
	      || !strcmp (style, GbStyleSymbols[i]))
	    {
	      /* This avoids any problems with redrawing the selection. */
	      if (data->action == GB_APPLYING)
		editor_clear_selection (NULL);
	      gtk_toolbar_set_style (GTK_TOOLBAR (widget), GbStyleValues[i]);
	      /*editor_refresh_widget (widget);*/
	      break;
	    }
	}
    }

  space = gb_widget_input_int (data, Space);
  if (data->apply)
    gtk_toolbar_set_space_size (GTK_TOOLBAR (widget), space);

  space_style = gb_widget_input_choice (data, SpaceStyle);
  if (data->apply)
    {
      for (i = 0;
	   i < sizeof (GbSpaceStyleValues) / sizeof (GbSpaceStyleValues[0]);
	   i++)
	{
	  if (!strcmp (space_style, GbSpaceStyleChoices[i])
	      || !strcmp (space_style, GbSpaceStyleSymbols[i]))
	    {
	      gtk_toolbar_set_space_style (GTK_TOOLBAR (widget),
					   GbSpaceStyleValues[i]);
	      break;
	    }
	}
    }

  relief = gb_widget_input_choice (data, Relief);
  if (data->apply)
    {
      for (i = 0; i < sizeof (GbReliefValues) / sizeof (GbReliefValues[0]);
	   i++)
	{
	  if (!strcmp (relief, GbReliefChoices[i])
	      || !strcmp (relief, GbReliefSymbols[i]))
	    {
	      gtk_toolbar_set_button_relief (GTK_TOOLBAR (widget),
					     GbReliefValues[i]);
	      break;
	    }
	}
    }

  tooltips = gb_widget_input_bool (data, Tooltips);
  if (data->apply)
    gtk_toolbar_set_tooltips (GTK_TOOLBAR (widget), tooltips);
}


/* This updates the toolbar size to the given value, adding buttons or
   deleting items as necessary. */
static void
update_toolbar_size (GtkWidget * widget, gint size)
{
  GtkWidget *placeholder;
  gint current_size = GTK_TOOLBAR (widget)->num_children;

  if (current_size < size)
    {
      /* FIXME: This avoids any problems with redrawing the selection. */
      editor_clear_selection (NULL);

      while (current_size < size)
	{
	  placeholder = editor_new_placeholder ();
	  gtk_toolbar_append_widget (GTK_TOOLBAR (widget), placeholder,
				     NULL, NULL);
	  current_size++;
	}
    }
  else if (current_size > size)
    {
      GList *item = g_list_nth (GTK_TOOLBAR (widget)->children, size);
      while (item)
	{
	  GtkToolbarChild *child = (GtkToolbarChild*) item->data;
	  item = item->next;
	  if (child->type == GTK_TOOLBAR_CHILD_SPACE)
	    {
	      /* FIXME: This is naughty, but GtkToolbar has no way for removing
		 spaces. */
	      GTK_TOOLBAR (widget)->children = g_list_remove (GTK_TOOLBAR (widget)->children, child);
	      GTK_TOOLBAR (widget)->num_children--;
	      gtk_widget_queue_resize (widget);
	    }
	  else
	    {
	      gtk_container_remove (GTK_CONTAINER (widget), child->widget);
	    }
	}
    }
}


/*
 * Creates the child packing properties for children of this widget.
 */
static void
gb_toolbar_create_child_properties (GtkWidget * widget,
				    GbWidgetCreateChildArgData * data)
{
  property_add_bool (GbNewToolbarGroup, _("New Group:"),
		     _("Set True to start a new group of toolbar items"));
}


static void
gb_toolbar_get_child_properties (GtkWidget *widget, GtkWidget *child,
				 GbWidgetGetArgData *data)
{
  GtkToolbarChild *tchild;
  gint pos = 0;

  tchild = glade_util_find_toolbar_child (widget, child, &pos, NULL);
  if (pos == 0)
    {
      if (data->action == GB_SHOWING)
	{
	  gb_widget_output_bool (data, GbNewToolbarGroup, FALSE);
	  property_set_sensitive (GbNewToolbarGroup, FALSE);
	}
    }
  else
    {
      gboolean new_group = gb_toolbar_get_new_toolbar_group (widget, child);
      if (data->action == GB_SHOWING)
	{
	  property_set_sensitive (GbNewToolbarGroup, TRUE);
	  gb_widget_output_bool (data, GbNewToolbarGroup, new_group);
	}
      else if (new_group && data->action == GB_SAVING)
	{
	  save_start_tag (data, "child");
	  gb_widget_output_bool (data, GbNewToolbarGroup, new_group);
	  save_end_tag (data, "child");
	}
    }
}


static void
gb_toolbar_set_child_properties (GtkWidget *widget, GtkWidget *child,
				 GbWidgetSetArgData *data)
{
  gboolean new_group = gb_widget_input_bool (data, GbNewToolbarGroup);
  if (data->apply)
    gb_toolbar_set_new_toolbar_group (widget, child, new_group);
}

/*
 * Adds menu items to a context menu which is just about to appear!
 * Add commands to aid in editing a GtkToolbar, with signals pointing to
 * other functions in this file.
 */
static void
gb_toolbar_create_popup_menu(GtkWidget *widget, GbWidgetCreateMenuData *data)
{
  GtkWidget *menuitem;
  GList *child_element;
  GtkToolbarChild *child;

  if (data->child == NULL)
    return;

  /* Find the toolbar child corresponding to the button the mouse is over. */
  child = glade_util_find_toolbar_child (widget, data->child, NULL, &child_element);

  /* Commands for inserting new items. */
  if (child)
    {
      menuitem = gtk_menu_item_new_with_label (_("Insert Item Before"));
      gtk_widget_show (menuitem);
      gtk_menu_append (GTK_MENU (data->menu), menuitem);
      gtk_signal_connect (GTK_OBJECT (menuitem), "activate",
			  GTK_SIGNAL_FUNC (gb_toolbar_insert_before),
			  data->child);

      menuitem = gtk_menu_item_new_with_label (_("Insert Item After"));
      gtk_widget_show (menuitem);
      gtk_menu_append (GTK_MENU (data->menu), menuitem);
      gtk_signal_connect (GTK_OBJECT (menuitem), "activate",
			  GTK_SIGNAL_FUNC (gb_toolbar_insert_after),
			  data->child);
    }
}


static void
gb_toolbar_insert_before (GtkWidget * menuitem, GtkWidget * child)
{
  gb_toolbar_insert (child, -1);
}


static void
gb_toolbar_insert_after (GtkWidget * menuitem, GtkWidget * child)
{
  gb_toolbar_insert (child, 1);
}


static void
gb_toolbar_insert (GtkWidget * child, gint offset)
{
  GtkWidget *toolbar = child->parent, *placeholder;
  GtkToolbarChild *tbchild;
  gint pos;

  tbchild = glade_util_find_toolbar_child (toolbar, child, &pos, NULL);
  g_return_if_fail (tbchild != NULL);

  if (offset > 0)
    pos++;

  placeholder = editor_new_placeholder ();
  gtk_toolbar_insert_widget (GTK_TOOLBAR (toolbar), placeholder,
			     NULL, NULL, pos);
}





/*************************************************************************
 * Toolbar helper functions.
 *************************************************************************/

/* This returns TRUE if the widget is a special toolbar button, i.e. it
   has an icon and label. */
gboolean
gb_toolbar_is_toolbar_button (GtkWidget *widget)
{
  gchar *child_name = gb_widget_get_child_name (widget);
  return (child_name && !strcmp (child_name, "Toolbar:button"));
}


/* This finds the icon and/or label widgets within a toolbar button.
   If icon_return and label_return are non-NULL the pointers are placed there.
   it returns TRUE if it finds them OK. */
static gboolean
gb_toolbar_get_toolbar_button_widgets (GtkWidget *widget,
				      GtkWidget **icon_return,
				      GtkWidget **label_return)
{
  GtkWidget *child, *label, *icon;
  GList *vbox_children;

  g_return_val_if_fail (GTK_IS_BUTTON (widget), FALSE);

  child = GTK_BIN (widget)->child;
  g_return_val_if_fail (GTK_IS_VBOX (child), FALSE);

  vbox_children = GTK_BOX (child)->children;
  g_return_val_if_fail (g_list_length (vbox_children) == 2, FALSE);

  /* Note that the label is added first in gtktoolbar.c which is why
     it is first in the list. */
  label = ((GtkBoxChild*)vbox_children->data)->widget;
  icon = ((GtkBoxChild*)vbox_children->next->data)->widget;

  g_return_val_if_fail (GTK_IS_LABEL (label), FALSE);
  g_return_val_if_fail (!GB_IS_GB_WIDGET (label), FALSE);
#ifdef USE_GNOME
  g_return_val_if_fail (GTK_IS_PIXMAP (icon) || GNOME_IS_PIXMAP (icon), FALSE);
#else
  g_return_val_if_fail (GTK_IS_PIXMAP (icon), FALSE);
#endif
  g_return_val_if_fail (!GB_IS_GB_WIDGET (icon), FALSE);

  if (label_return)
    *label_return = label;
  if (icon_return)
    *icon_return = icon;
  return TRUE;
}


gboolean
gb_toolbar_get_new_toolbar_group	(GtkWidget	       *toolbar,
				 GtkWidget	       *widget)
{
  GtkToolbarChild *tbchild, *prev_tbchild;
  GList *elem;

  tbchild = glade_util_find_toolbar_child (toolbar, widget, NULL, &elem);
  g_return_val_if_fail (tbchild != NULL, FALSE);

  if (elem->prev == NULL)
    return FALSE;

  prev_tbchild = (GtkToolbarChild*) elem->prev->data;
  if (prev_tbchild->type == GTK_TOOLBAR_CHILD_SPACE)
    return TRUE;

  return FALSE;
}


/* This sets whether or not a toolbar child starts a new group on the toolbar,
   i.e. there is a space before it. */
static void
gb_toolbar_set_new_toolbar_group (GtkWidget	       *toolbar,
				 GtkWidget	       *widget,
				 gboolean		new_group)
{
  GtkToolbarChild *tbchild, *prev_tbchild;
  gint pos;
  GList *elem;
  GSList *current_group;
  gboolean check_groups = FALSE;
  GtkWidget *first_radio_widget = NULL;

  tbchild = glade_util_find_toolbar_child (toolbar, widget, &pos, &elem);
  g_return_if_fail (tbchild != NULL);

  if (elem->prev == NULL)
    return;

  prev_tbchild = (GtkToolbarChild*) elem->prev->data;

  if (new_group)
    {
      if (prev_tbchild->type != GTK_TOOLBAR_CHILD_SPACE)
	{
	  gtk_toolbar_insert_space (GTK_TOOLBAR (toolbar), pos);
	  /* We try to find the first radio item in the toolbar group, which is
	     using the default (NULL) radio group. If found we reset its radio
	     group, and indicate that the rest of the toolbar radio items in
	     the group should be checked. */
	  first_radio_widget = gb_toolbar_get_toolbar_radio_group_widget (widget, FALSE, TRUE);
	  if (first_radio_widget)
	    {
	      current_group = gb_radio_button_reset_radio_group (first_radio_widget);
	      gb_radio_button_update_radio_group (current_group);
	      gb_radio_button_update_radio_group (gtk_radio_button_group (GTK_RADIO_BUTTON (first_radio_widget)));
	      check_groups = TRUE;
	    }
	}
    }
  else
    {
      if (prev_tbchild->type == GTK_TOOLBAR_CHILD_SPACE)
	{
	  /* FIXME: This is naughty, but GtkToolbar has no way for removing
	     spaces. */
	  GTK_TOOLBAR (toolbar)->children = g_list_remove (GTK_TOOLBAR (toolbar)->children, prev_tbchild);
	  GTK_TOOLBAR (toolbar)->num_children--;
	  gtk_widget_queue_resize (toolbar);
	  check_groups = TRUE;
	}
    }

  /* If a space has been added or removed, we need to check all the radio
     groups until the next space on the toolbar. We don't check the first
     radio widget since its group has just been reset. */
  if (check_groups)
    {
      while (elem)
	{
	  tbchild = (GtkToolbarChild*) elem->data;
	  if (tbchild->type == GTK_TOOLBAR_CHILD_SPACE)
	    break;
	  if (tbchild->widget != first_radio_widget)
	    gb_toolbar_check_toolbar_radio_group (tbchild->widget);
	  elem = elem->next;
	}
    }
}


void
gb_toolbar_create_toolbar_button_popup_menu (GtkWidget	           *widget,
					     GbWidgetCreateMenuData *data)
{
  GtkWidget *menuitem;

  if (gb_toolbar_is_toolbar_button (widget))
    {
      menuitem = gtk_menu_item_new_with_label (_("Convert to ordinary button"));
      gtk_widget_show (menuitem);
      gtk_menu_append (GTK_MENU (data->menu), menuitem);
      gtk_signal_connect (GTK_OBJECT (menuitem), "activate",
			  GTK_SIGNAL_FUNC (gb_toolbar_convert_toolbar_button),
			  widget);
    }
}


static void
gb_toolbar_convert_toolbar_button (GtkWidget * menuitem,
				   GtkWidget * widget)
{
  GtkWidget *button;
  GtkToolbarChild *child;

  g_return_if_fail (widget->parent != NULL);
  child = glade_util_find_toolbar_child (widget->parent, widget, NULL, NULL);
  g_return_if_fail (child != NULL);

  /* We set the parent to NULL when calling gb_widget_new, otherwise it
     creates a new toolbar button! */
  if (child->type == GTK_TOOLBAR_CHILD_BUTTON)
    button = gb_widget_new ("GtkButton", NULL);
  else if (child->type == GTK_TOOLBAR_CHILD_TOGGLEBUTTON)
    button = gb_widget_new ("GtkToggleButton", NULL);
  else if (child->type == GTK_TOOLBAR_CHILD_RADIOBUTTON)
    button = gb_widget_new ("GtkRadioButton", NULL);
  else
    {
      g_warning ("Invalid toolbar button");
      return;
    }

  /* Set properties widget to NULL, in case the widget or parent is replaced */
  property_set_widget (NULL);

  gb_widget_replace_child (widget->parent, widget, button);
}



/* This writes the source code for creating toolbar buttons.
   Note that these are added to the toolbar here, so the standard code for
   adding them to the parent has to be skipped. */
void
gb_toolbar_write_toolbar_button_source (GtkWidget * widget,
				       GbWidgetWriteSourceData * data)
{
  GladeWidgetData *wdata = data->widget_data;
  GtkWidget *icon, *label, *radio_widget;
  gchar *parent_name, *label_text, *type, *radio_widget_name = NULL;
  gboolean has_icon;
#ifdef USE_GNOME
  gint stock_icon_index;
#endif
  gchar *icon_filename = gtk_object_get_data (GTK_OBJECT (widget),
					      GbToolbarIconFilenameKey);

  if (!gb_toolbar_get_toolbar_button_widgets (widget, &icon, &label))
    return;

  g_return_if_fail (widget->parent != NULL);
  parent_name = gtk_widget_get_name (widget->parent);
  parent_name = source_create_valid_identifier (parent_name);

  gtk_label_get (GTK_LABEL (label), &label_text);

  if (gb_toolbar_get_new_toolbar_group (widget->parent, widget))
    source_add (data, "  gtk_toolbar_append_space (GTK_TOOLBAR (%s));\n\n",
		parent_name);

  if (GTK_IS_RADIO_BUTTON (widget))
    {
      type = "GTK_TOOLBAR_CHILD_RADIOBUTTON";
      radio_widget = gb_toolbar_get_toolbar_radio_group_widget (widget, FALSE,
							       FALSE);
      if (radio_widget && radio_widget != widget)
	radio_widget_name = source_create_valid_identifier (gtk_widget_get_name (radio_widget));
    }
  else if (GTK_IS_TOGGLE_BUTTON (widget))
    type = "GTK_TOOLBAR_CHILD_TOGGLEBUTTON";
  else
    type = "GTK_TOOLBAR_CHILD_BUTTON";

  has_icon = FALSE;

#ifdef USE_GNOME
  stock_icon_index = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (widget),
							   GbToolbarStockIconKey));
  if (stock_icon_index)
    {
      has_icon = TRUE;
      source_add (data,
		  "  tmp_toolbar_icon = gnome_stock_pixmap_widget (%s, %s);\n",
		  data->component_name,
		  GladeStockPixmapSymbols[stock_icon_index]);
    }
  else if (icon_filename && icon_filename[0])
    {
      has_icon = TRUE;
      source_create_pixmap (data, "tmp_toolbar_icon", icon_filename, TRUE);
    }
#else
  if (icon_filename && icon_filename[0])
    {
      has_icon = TRUE;
      source_create_pixmap (data, "tmp_toolbar_icon", icon_filename, FALSE);
    }
#endif

  if (has_icon)
    {
      /* Make sure the tmp_toolbar_icon widget is declared. */
      source_ensure_decl (data, "  GtkWidget *tmp_toolbar_icon;\n");
    }

  label_text = g_strdup (source_make_string (label_text, data->use_gettext));
  source_add (data,
	      "  %s = gtk_toolbar_append_element (GTK_TOOLBAR (%s),\n"
	      "                                %s,\n"
	      "                                %s,\n"
	      "                                %s,\n"
	      "                                %s, NULL,\n"
	      "                                %s, NULL, NULL);\n",
	      data->wname, parent_name, type,
	      radio_widget_name ? radio_widget_name : "NULL",
	      label_text,
	      wdata->tooltip ? source_make_string (wdata->tooltip, data->use_gettext) : "NULL",
	      has_icon ? "tmp_toolbar_icon" : "NULL");

  g_free (label_text);
  g_free (radio_widget_name);
  g_free (parent_name);
}


/* This returns the first radio button widget in a toolbar with the same
   group as the given widget, or NULL if there isn't any.
   If the given widget has a group name set then it steps through
   the entire toolbar to find the first radio button with that group.
   If the given widget has its group set to NULL (i.e. using the default group)
   then it steps through the toolbar only until it finds a toolbar space
   item. */
static GtkWidget*
gb_toolbar_get_toolbar_radio_group_widget (GtkWidget *widget,
					  gboolean skip_widget,
					  gboolean find_default_group)
{
  GtkToolbarChild *tbchild;
  GList *elem;
  GtkWidget *group_widget = NULL;
  gchar *group = NULL, *tmp_group;

  g_return_val_if_fail (widget->parent != NULL, NULL);
  g_return_val_if_fail (GTK_IS_TOOLBAR (widget->parent), NULL);

  if (!find_default_group)
    /* FIXME: should use Group from gbradiobutton.c */
    group = gtk_object_get_data (GTK_OBJECT (widget), "GtkRadioButton::group");

  tbchild = glade_util_find_toolbar_child (widget->parent, widget, NULL,
					   &elem);
  g_return_val_if_fail (tbchild != NULL, NULL);

  /* Find the start of the toolbar group, or the first widget on the toolbar
     if the radio button has a group set. */
  while (elem->prev)
    {
      tbchild = (GtkToolbarChild*) elem->prev->data;
      if (group == NULL && tbchild->type == GTK_TOOLBAR_CHILD_SPACE)
	break;
      elem = elem->prev;
    }

  while (elem)
    {
      tbchild = (GtkToolbarChild*) elem->data;

      /* Skip the widget we are adding, if requested. */
      if (!skip_widget || tbchild->widget != widget)
	{
	  if (tbchild->type == GTK_TOOLBAR_CHILD_RADIOBUTTON
	      || (tbchild->type == GTK_TOOLBAR_CHILD_WIDGET
		  && GTK_IS_RADIO_BUTTON (tbchild->widget)))
	    {
	      tmp_group = gtk_object_get_data (GTK_OBJECT (tbchild->widget),
					       "GtkRadioButton::group");
	      if ((group == NULL && tmp_group == NULL)
		  || (group && tmp_group && !strcmp (group, tmp_group)))
		{
		  group_widget = tbchild->widget;
		  break;
		}
	    }
	  else if (group == NULL && tbchild->type == GTK_TOOLBAR_CHILD_SPACE)
	    break;
	}
      elem = elem->next;
    }

  return group_widget;
}


GtkWidget *
gb_toolbar_new_toolbar_button (GbWidgetNewData *data,
			       GtkToolbarChildType type)
{
  GtkToolbar *toolbar = GTK_TOOLBAR (data->parent);
  GtkWidget *new_widget, *vbox, *icon, *label;
  GbWidget * pixmap_gbwidget;

  /* If we are loading, we check that a toolbar button is needed. */
  MSG ("In gb_widget_new_toolbar_button");
  if (data->action == GB_LOADING)
    {
      gchar *child_name = load_get_value (data->loading_data, "child_name");
      if (!child_name || strcmp (child_name, "Toolbar:button"))
	return NULL;
    }

  if (type == GTK_TOOLBAR_CHILD_BUTTON)
    {
      new_widget = gtk_button_new ();
    }
  else if (type == GTK_TOOLBAR_CHILD_TOGGLEBUTTON)
    {
      new_widget = gtk_toggle_button_new ();
      gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (new_widget), FALSE);
    }
  else
    {
      new_widget = gtk_radio_button_new (NULL);
      gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (new_widget), FALSE);
    }

  /* Remember it is a special widget. */
  gb_widget_set_child_name (new_widget, "Toolbar:button");

  gtk_button_set_relief (GTK_BUTTON (new_widget), toolbar->relief);

  GTK_WIDGET_UNSET_FLAGS (new_widget, GTK_CAN_FOCUS);
  vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (new_widget), vbox);
  gtk_widget_show (vbox);

  label = gtk_label_new (data->name);
  gtk_box_pack_end (GTK_BOX (vbox), label, FALSE, FALSE, 0);
  if (toolbar->style != GTK_TOOLBAR_ICONS)
    gtk_widget_show (label);

  pixmap_gbwidget = gb_widget_lookup_class ("GtkPixmap");
  if (pixmap_gbwidget)
    {
      icon = gtk_pixmap_new (pixmap_gbwidget->gdkpixmap,
			     pixmap_gbwidget->mask);
      gtk_box_pack_end (GTK_BOX (vbox), icon, FALSE, FALSE, 0);
      if (toolbar->style != GTK_TOOLBAR_TEXT)
	gtk_widget_show (icon);
    }
  else
    g_warning ("Couldn't find GtkPixmap data");

  return new_widget;
}


/* SPECIAL CODE: This inserts a proper toolbar button (with an icon and label)
   into a toolbar at the given position. We need it since GtkToolbar creates
   and inserts the widget all in one go, which isn't good enough for us,
   as we want to set the events mask to add signal handlers to the buttons.
   This also sets the radio button group, if it is a radio button. */
void
gb_toolbar_insert_toolbar_child (GtkToolbar *toolbar,
				 GtkWidget *new_child,
				 gint position)
{
  GtkToolbarChild *child;
  GtkWidget *icon, *label;
  GtkToolbarChildType type;

  /* Determine if the new child is a proper toolbar button, i.e. it has
     a vbox with a pixmap and a label in it. If it isn't we just add it as
     a standard widget. */
  if (!gb_toolbar_is_toolbar_button (new_child))
    {
      gtk_toolbar_insert_widget (toolbar, new_child, NULL, NULL, position);
      return;
    }

  if (!gb_toolbar_get_toolbar_button_widgets (new_child, &icon, &label))
    return;
  if (GTK_IS_RADIO_BUTTON (new_child))
    type = GTK_TOOLBAR_CHILD_RADIOBUTTON;
  else if (GTK_IS_TOGGLE_BUTTON (new_child))
    type = GTK_TOOLBAR_CHILD_TOGGLEBUTTON;
  else
    type = GTK_TOOLBAR_CHILD_BUTTON;

  child = g_new (GtkToolbarChild, 1);
  child->widget = new_child;
  child->icon = icon;
  child->label = label;
  child->type = type;

  /* This is mainly copied from gtktoolbar.c */
  toolbar->children = g_list_insert (toolbar->children, child, position);
  toolbar->num_children++;

  if (type != GTK_TOOLBAR_CHILD_SPACE)
    gtk_widget_set_parent (child->widget, GTK_WIDGET (toolbar));

  if ((type != GTK_TOOLBAR_CHILD_SPACE) && GTK_WIDGET_VISIBLE (toolbar))
    {
      if (GTK_WIDGET_REALIZED (toolbar)
	  && !GTK_WIDGET_REALIZED (child->widget))
	gtk_widget_realize (child->widget);
	
      if (GTK_WIDGET_MAPPED (toolbar)
	  && !GTK_WIDGET_MAPPED (child->widget))
	gtk_widget_map (child->widget);
    }

  if (GTK_WIDGET_VISIBLE (toolbar) &&
      ((type == GTK_TOOLBAR_CHILD_SPACE) ||
       GTK_WIDGET_VISIBLE (child->widget)))
    gtk_widget_queue_resize (GTK_WIDGET (toolbar));

  if (type == GTK_TOOLBAR_CHILD_RADIOBUTTON)
    gb_toolbar_check_toolbar_radio_group (new_child);
}


/* This makes sure that if a toolbar widget is a radio button it is in the
   correct group. */
static void
gb_toolbar_check_toolbar_radio_group (GtkWidget * widget)
{
  GtkWidget *group_widget;
  GSList *current_group, *new_group;

  if (!GTK_IS_RADIO_BUTTON (widget))
    return;

  MSG1 ("Checking radio group of %s", gtk_widget_get_name (widget));
  /* We don't want the current widget returned, since its group may not have
     been set yet, as it is being added to its parent now. */
  group_widget = gb_toolbar_get_toolbar_radio_group_widget (widget, TRUE,
							    FALSE);
  if (group_widget)
    {
      MSG1 ("  group widget: %s", gtk_widget_get_name (group_widget));
    }
  current_group = gtk_radio_button_group (GTK_RADIO_BUTTON (widget));

  if (group_widget)
    {
      new_group = gtk_radio_button_group (GTK_RADIO_BUTTON (group_widget));
      if (current_group != new_group)
	{
	  if (current_group->data == widget)
	    current_group = current_group->next;
	  gtk_radio_button_set_group (GTK_RADIO_BUTTON (widget), new_group);
	  gb_radio_button_update_radio_group (current_group);
	  gb_radio_button_update_radio_group (gtk_radio_button_group (GTK_RADIO_BUTTON (widget)));
	}
    }
  else if (g_slist_length (current_group) != 1)
    {
      current_group = gb_radio_button_reset_radio_group (widget);
      gb_radio_button_update_radio_group (current_group);
      gb_radio_button_update_radio_group (gtk_radio_button_group (GTK_RADIO_BUTTON (widget)));
    }
}


void
gb_toolbar_output_child_label (GtkWidget * widget, GbWidgetGetArgData * data,
			       const gchar * Label)
{
  GtkWidget *child;
  gchar *label_text;

  if (!gb_toolbar_get_toolbar_button_widgets (widget, NULL, &child))
    return;

  gtk_label_get (GTK_LABEL (child), &label_text);
  gb_widget_output_translatable_text (data, Label, label_text);

  if (data->action == GB_SHOWING)
    property_set_sensitive (Label, TRUE);
}


/* This gets the child icon filename for toolbar buttons.
   This is for showing or saving. If the widget is not actually a toolbar
   button and we are showing the properties, then we clear the property and
   make it insensitive. */
void
gb_toolbar_output_child_icon (GtkWidget * widget,
			      GbWidgetGetArgData * data,
			      const gchar * property_name)
{
#ifdef USE_GNOME
  GtkWidget *combo;
  gint stock_icon_index;
#endif
  gchar *filename;

  if (gb_toolbar_is_toolbar_button (widget))
    {
      if (data->action == GB_SHOWING)
	{
	  property_set_visible (property_name, TRUE);

	  /* Make sure the icon filename is editable. */
#ifdef USE_GNOME
	  combo = property_get_value_widget (property_name);
	  g_return_if_fail (combo != NULL);
	  gtk_entry_set_editable (GTK_ENTRY (GTK_COMBO (combo)->entry),
				  TRUE);
	  property_set_sensitive_full (property_name, TRUE, TRUE, TRUE);
#endif
	}	  

      /* Find out if the icon is a stock icon, and if it is output that. */
#ifdef USE_GNOME
      stock_icon_index = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (widget), GbToolbarStockIconKey));
      if (stock_icon_index)
	{
	  /* We save the stock icon with a different tag, to make it easier
	     for external programs which read the XML. */
	  if (data->action == GB_SAVING)
	    {
	      gb_widget_output_string (data, "stock_pixmap", _(GladeStockPixmapSymbols[stock_icon_index]));
	    }
	  else
	    {
	      gb_widget_output_pixmap_filename (data, property_name, _(GladeStockPixmapChoices[stock_icon_index]));
	    }
	  return;
	}
#endif

      filename = gtk_object_get_data (GTK_OBJECT (widget),
				      GbToolbarIconFilenameKey);
      gb_widget_output_pixmap_filename (data, property_name, filename);
    }
  else
    {
      if (data->action == GB_SHOWING)
	{
	  gb_widget_output_pixmap_filename (data, property_name, "");
	  property_set_visible (property_name, FALSE);
	}
    }
}


/* This sets the child label for buttons/items/menuitems or subclasses.
   This is for applying or loading. */
void
gb_toolbar_input_child_label (GtkWidget * widget, GbWidgetSetArgData * data,
			      const gchar * Label)
{
  GtkWidget *child;
  gchar *label_text;

  label_text = gb_widget_input_text (data, Label);
  if (data->apply)
    {
      /* Toolbar button labels aren't underlined. */
      if (gb_toolbar_get_toolbar_button_widgets (widget, NULL, &child))
	gtk_label_set_text (GTK_LABEL (child), label_text);
    }

  /* This isn't very nice. When a text property is got from the property
     editor (i.e. when action is GB_APPLYING) it needs to be freed after. */
  if (data->action == GB_APPLYING)
    g_free (label_text);
}


/* This sets the icon for toolbar buttons. This is for applying or loading.
   It can be a stock Gnome icon or an icon filename. */
void
gb_toolbar_input_child_icon (GtkWidget * widget,
			    GbWidgetSetArgData * data,
			    const gchar * property_name)
{
  gchar *filename;
  GtkWidget *icon;
  gboolean handled_icon = FALSE;
#ifdef USE_GNOME
  gint stock_icon_index = -1;
#endif

  if (!gb_toolbar_get_toolbar_button_widgets (widget, &icon, NULL))
    return;

  /* First see if it has a stock pixmap. Note that the stock pixmap is
     saved with a different tag than is used internally. */
#ifdef USE_GNOME
  if (data->action == GB_LOADING)
    {
      filename = gb_widget_input_string (data, "stock_pixmap");
      if (filename && filename[0])
	{
	  stock_icon_index = glade_util_string_array_index (GladeStockPixmapSymbols, GladeStockPixmapSize, filename);
	  if (stock_icon_index != -1)
	    {
	      gb_toolbar_set_stock_child_icon (widget, icon, stock_icon_index);
	    }
	  else
	    {
	      load_add_error_message_with_tag (data,
					       GLADE_LINE_PROPERTY,
					       _("Invalid stock pixmap"),
					       "stock_pixmap", filename);
	    }
	  handled_icon = TRUE;
	}
    }
  else
    {
      /* We have to get the string from the widget ourself, rather than
	 calling property_get_filename() since that gets the filename stored
	 in the object data. */
      GtkWidget *value = property_get_value_widget (property_name);
      filename = gtk_entry_get_text (GTK_ENTRY (GTK_COMBO (value)->entry));
      if (filename && filename[0])
	{
	  stock_icon_index = glade_gnome_get_stock_pixmap_index (filename);
	  if (stock_icon_index != -1)
	    {
	      gb_toolbar_set_stock_child_icon (widget, icon, stock_icon_index);
	      handled_icon = TRUE;
	    }
	}
    }
#endif

  /* The icon isn't a Gnome stock pixmap, so we see if it is a pixmap file. */
  if (!handled_icon)
    {
      filename = gb_widget_input_pixmap_filename (data, property_name);
      if (data->apply)
	{
	  gb_toolbar_set_child_icon (widget, data, icon, filename,
				     property_name);
	}
      if (data->action == GB_LOADING)
	g_free (filename);
    }
}


#ifdef USE_GNOME
static void
gb_toolbar_set_stock_child_icon (GtkWidget *widget, GtkWidget *icon,
				 gint stock_index)
{
  GtkWidget *new_icon;
  GtkToolbarChild *child;

  child = glade_util_find_toolbar_child (widget->parent, widget, NULL, NULL);

  /* Reset any icon filename. */
  g_free (gtk_object_get_data (GTK_OBJECT (widget), GbToolbarIconFilenameKey));
  gtk_object_set_data (GTK_OBJECT (widget), GbToolbarIconFilenameKey, NULL);

  /* Now set the stock index. */
  gtk_object_set_data (GTK_OBJECT (widget), GbToolbarStockIconKey,
		       GINT_TO_POINTER (stock_index));

  if (stock_index > 0)
    {
      new_icon = gnome_stock_pixmap_widget (widget, GladeStockPixmapValues[stock_index]);
      if (new_icon)
	{
	  if (icon)
	    {
	      gtk_container_remove (GTK_CONTAINER (GTK_BIN (widget)->child),
				    icon);
	    }
	  gtk_widget_show (new_icon);
	  gtk_box_pack_end (GTK_BOX (GTK_BIN (widget)->child),
			    new_icon, FALSE, FALSE, 0);
	  child->icon = new_icon;
	}
    }
  else
    {
      gb_toolbar_reset_child_icon (widget, icon);
    }
}
#endif


static void
gb_toolbar_set_child_icon (GtkWidget *widget, GbWidgetSetArgData * data,
			   GtkWidget *icon, gchar *filename,
			   const gchar *property_name)
{
  gchar *old_filename;
  GdkColormap *colormap;
  GdkPixmap *gdkpixmap;
  GdkBitmap *mask;
  GtkWidget *new_icon;
  GtkToolbarChild *child;

  child = glade_util_find_toolbar_child (widget->parent, widget, NULL, NULL);

  if (filename && filename[0] == '\0')
    filename = NULL;

  /* Reset the stock index. */
  gtk_object_set_data (GTK_OBJECT (widget), GbToolbarStockIconKey, NULL);

  old_filename = gtk_object_get_data (GTK_OBJECT (widget),
				      GbToolbarIconFilenameKey);
  if (old_filename)
    {
      glade_project_remove_pixmap (data->project, old_filename);
      g_free (old_filename);
    }

  gtk_object_set_data (GTK_OBJECT (widget), GbToolbarIconFilenameKey,
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
					       property_name, filename);
	    }
	  else
	    {
	      glade_util_show_message_box (_("Couldn't create pixmap from file\n"), widget);
	    }
	}
      else
	{
	  if (icon)
	    {
	      gtk_container_remove (GTK_CONTAINER (GTK_BIN (widget)->child),
				    icon);
	    }
	  new_icon = gtk_pixmap_new (gdkpixmap, mask);
	  gdk_pixmap_unref (gdkpixmap);
	  gdk_bitmap_unref (mask);
	  gtk_widget_show (new_icon);
	  gtk_box_pack_end (GTK_BOX (GTK_BIN (widget)->child),
			    new_icon, FALSE, FALSE, 0);
	  child->icon = new_icon;
	}
    }
  else
    {
      gb_toolbar_reset_child_icon (widget, icon);
    }
}


static void
gb_toolbar_reset_child_icon (GtkWidget *widget, GtkWidget *icon)
{
  GtkWidget *new_icon;
  GbWidget * pixmap_gbwidget;
  GtkToolbarChild *child;

  child = glade_util_find_toolbar_child (widget->parent, widget, NULL, NULL);

  pixmap_gbwidget = gb_widget_lookup_class ("GtkPixmap");
  g_return_if_fail (pixmap_gbwidget != NULL);
      
  if (icon)
    {
      gtk_container_remove (GTK_CONTAINER (GTK_BIN (widget)->child), icon);
    }

  new_icon = gtk_pixmap_new (pixmap_gbwidget->gdkpixmap,
			     pixmap_gbwidget->mask);
  gtk_widget_show (new_icon);
  gtk_box_pack_end (GTK_BOX (GTK_BIN (widget)->child),
		    new_icon, FALSE, FALSE, 0);
  child->icon = new_icon;
}


void
gb_toolbar_destroy_child_icon (GtkWidget * widget, GbWidgetDestroyData * data)
{
  gchar *filename;

  if (gb_toolbar_is_toolbar_button (widget))
    {
      filename = gtk_object_get_data (GTK_OBJECT (widget),
				      GbToolbarIconFilenameKey);
      if (filename)
	{
	  glade_project_remove_pixmap (data->project, filename);
	  g_free (filename);
	}
    }
}


/*************************************************************************
 * END Toolbar helper functions.
 *************************************************************************/


/*
 * Writes the source code needed to create this widget.
 * You have to output everything necessary to create the widget here, though
 * there are some convenience functions to help.
 */
static void
gb_toolbar_write_source (GtkWidget * widget, GbWidgetWriteSourceData * data)
{
  const gchar *orientation = GbOrientationSymbols[0];
  const gchar *style = GbStyleSymbols[0];
  gint i;

  for (i = 0;
       i < sizeof (GbOrientationValues) / sizeof (GbOrientationValues[0]); i++)
    {
      if (GbOrientationValues[i] == GTK_TOOLBAR (widget)->orientation)
	orientation = GbOrientationSymbols[i];
    }

  for (i = 0; i < sizeof (GbStyleValues) / sizeof (GbStyleValues[0]); i++)
    {
      if (GbStyleValues[i] == GTK_TOOLBAR (widget)->style)
	style = GbStyleSymbols[i];
    }

  if (data->create_widget)
    {
      source_add (data, "  %s = gtk_toolbar_new (%s, %s);\n", data->wname,
		  orientation, style);
    }

  gb_widget_write_standard_source (widget, data);

  if (GTK_TOOLBAR (widget)->space_size != 5)
    source_add (data, "  gtk_toolbar_set_space_size (GTK_TOOLBAR (%s), %i);\n",
		data->wname, GTK_TOOLBAR (widget)->space_size);

  if (GTK_TOOLBAR (widget)->space_style != GTK_TOOLBAR_SPACE_EMPTY)
    {
      for (i = 0;
	   i < sizeof (GbSpaceStyleValues) / sizeof (GbSpaceStyleValues[0]);
	   i++)
	{
	  if (GbSpaceStyleValues[i] == GTK_TOOLBAR (widget)->space_style)
	    source_add (data,
			"  gtk_toolbar_set_space_style (GTK_TOOLBAR (%s), %s);\n",
			data->wname, GbSpaceStyleSymbols[i]);
	}
    }

  if (GTK_TOOLBAR (widget)->relief != GTK_RELIEF_NORMAL)
    {
      for (i = 0; i < sizeof (GbReliefValues) / sizeof (GbReliefValues[0]);
	   i++)
	{
	  if (GbReliefValues[i] == GTK_TOOLBAR (widget)->relief)
	    source_add (data,
			"  gtk_toolbar_set_button_relief (GTK_TOOLBAR (%s), %s);\n",
			data->wname, GbReliefSymbols[i]);
	}
    }

  if (!GTK_TOOLTIPS (GTK_TOOLBAR (widget)->tooltips)->enabled)
    source_add (data,
		"  gtk_toolbar_set_tooltips (GTK_TOOLBAR (%s), FALSE);\n",
		data->wname);
}


/* Outputs source to add a child widget to a hbox/vbox. */
static void
gb_toolbar_write_add_child_source (GtkWidget * parent,
				   const gchar *parent_name,
				   GtkWidget *child,
				   GbWidgetWriteSourceData * data)
{
  /* For toolbar buttons, the widgets are added to the toolbar when
     created, so we skip this. For standard widgets, we add them here. */
  if (!gb_toolbar_is_toolbar_button (child))
    {
      if (gb_toolbar_get_new_toolbar_group (parent, child))
	source_add (data,
		    "  gtk_toolbar_append_space (GTK_TOOLBAR (%s));\n\n",
		    parent_name);

      source_add (data,
		  "  gtk_toolbar_append_widget (GTK_TOOLBAR (%s), %s, %s, NULL);\n",
		  parent_name, data->wname,
		  data->widget_data->tooltip ? source_make_string (data->widget_data->tooltip, data->use_gettext) : "NULL");
    }
}


/*
 * Initializes the GbWidget structure.
 * I've placed this at the end of the file so we don't have to include
 * declarations of all the functions.
 */
GbWidget *
gb_toolbar_init ()
{
  /* Initialise the GTK type */
  gtk_toolbar_get_type ();

  /* Initialize the GbWidget structure */
  gb_widget_init_struct (&gbwidget);

  /* Fill in the pixmap struct & tooltip */
  gbwidget.pixmap_struct = toolbar_xpm;
  gbwidget.tooltip = _("Toolbar");

  /* Fill in any functions that this GbWidget has */
   gbwidget.gb_widget_new               = gb_toolbar_new;
  gbwidget.gb_widget_add_child = gb_toolbar_add_child;
  gbwidget.gb_widget_create_properties = gb_toolbar_create_properties;
  gbwidget.gb_widget_get_properties = gb_toolbar_get_properties;
  gbwidget.gb_widget_set_properties = gb_toolbar_set_properties;
  gbwidget.gb_widget_create_child_properties = gb_toolbar_create_child_properties;
  gbwidget.gb_widget_get_child_properties = gb_toolbar_get_child_properties;
  gbwidget.gb_widget_set_child_properties = gb_toolbar_set_child_properties;
  gbwidget.gb_widget_create_popup_menu = gb_toolbar_create_popup_menu;
  gbwidget.gb_widget_write_source = gb_toolbar_write_source;
  gbwidget.gb_widget_write_add_child_source = gb_toolbar_write_add_child_source;

  return &gbwidget;
}
