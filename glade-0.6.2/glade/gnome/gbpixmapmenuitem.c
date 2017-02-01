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
#include "../glade_gnome.h"
#include "../glade_keys_dialog.h"

/* Include the 21x21 icon pixmap for this widget, to be used in the palette */
#include "../graphics/menuitem.xpm"

/*
 * This is the GbWidget struct for this widget (see ../gbwidget.h).
 * It is initialized in the init() function at the end of this file
 */
static GbWidget gbwidget;

static gchar *Label = "PixmapMenuItem|GtkItem::label";
static gchar *Justify = "PixmapMenuItem|GtkMenuItem::right_justify";


/******
 * NOTE: To use these functions you need to uncomment them AND add a pointer
 * to the function in the GbWidget struct at the end of this file.
 ******/

/*
 * Creates a new GtkWidget of class GtkPixmapMenuItem, performing any specialized
 * initialization needed for the widget to work correctly in this environment.
 * If a dialog box is used to initialize the widget, return NULL from this
 * function, and call data->callback with your new widget when it is done.
 * If the widget needs a special destroy handler, add a signal here.
 */
static GtkWidget*
gb_pixmap_menu_item_new (GbWidgetNewData *data)
{
  GtkWidget *new_widget;

  new_widget = gtk_pixmap_menu_item_new ();
  return new_widget;
}


void
gb_pixmap_menu_item_add_child (GtkWidget * widget, GtkWidget * child, GbWidgetSetArgData *data)
{
  if (GTK_IS_MENU (child))
    {
      MSG ("Trying to add a menu to a menu item");
      gtk_menu_item_set_submenu (GTK_MENU_ITEM (widget), child);
    }
}


/*
 * Creates the components needed to edit the extra properties of this widget.
 */
static void
gb_pixmap_menu_item_create_properties (GtkWidget * widget, GbWidgetCreateArgData * data)
{
  property_add_text (Label, _("Label:"), _("The text to display"), 2);
  property_add_bool (Justify, _("Right Justify:"),
		     _("If the menu item is right-justified"));
}



/*
 * Gets the properties of the widget. This is used for both displaying the
 * properties in the property editor, and also for saving the properties.
 */
static void
gb_pixmap_menu_item_get_properties (GtkWidget *widget, GbWidgetGetArgData * data)
{
  gboolean output_label = TRUE, output_rest = TRUE;
  gchar *icon_name;
  gint stock_index;

  /* If we are saving, we check for a stock Gnome menu item, and if so, we
     just save that. */
  if (data->action == GB_SAVING)
    {
      gint stock_item_index;

      stock_item_index = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (widget), GladeMenuItemStockIndexKey));
      /* The 'New' item is special. If it has a child menu, it must be a
	 GNOMEUIINFO_MENU_NEW_SUBTREE. If not, it is a
	 GNOMEUIINFO_MENU_NEW_ITEM, in which case the label is also output. */
      if (stock_item_index == GladeStockMenuItemNew)
	{
	  if (GTK_MENU_ITEM (widget)->submenu)
	    {
	      gb_widget_output_string (data, "stock_item",
				       "GNOMEUIINFO_MENU_NEW_SUBTREE");
	      output_label = FALSE;
	      output_rest = FALSE;
	    }
	  else
	    {
	      gb_widget_output_string (data, "stock_item",
				       "GNOMEUIINFO_MENU_NEW_ITEM");
	      output_rest = FALSE;
	    }
	}
      else if (stock_item_index != 0)
	{
	  gb_widget_output_string (data, "stock_item", GladeStockMenuItemSymbols[stock_item_index]);
	  output_label = FALSE;
	  output_rest = FALSE;
	}
    }

  if (output_label)
    gb_widget_output_child_label (widget, data, Label);

  if (output_rest)
    {
      gb_widget_output_bool (data, Justify,
			     GTK_MENU_ITEM (widget)->right_justify);

      /* We also output the stock icon if it has one. */
      if (data->action == GB_SAVING)
	{
	  icon_name = gtk_object_get_data (GTK_OBJECT (widget),
					   GladeMenuItemIconKey);
	  if (icon_name)
	    {
	      /* For Gnome we see if it is a stock icon, and if it is we output
		 the stock symbol. If not, we output it as an ordinary pixmap
		 filename. */
	      stock_index = glade_gnome_get_stock_menu_pixmap_index (icon_name);
	      if (stock_index != -1)
		{
		  gb_widget_output_string (data, "stock_icon",
					   GladeStockMenuPixmapSymbols[stock_index]);
		}
	      else
		{
		  save_pixmap_filename (data, "icon", icon_name);
		}
	    }
	}
    }
}



/*
 * Sets the properties of the widget. This is used for both applying the
 * properties changed in the property editor, and also for loading.
 */
static void
gb_pixmap_menu_item_set_properties (GtkWidget * widget, GbWidgetSetArgData * data)
{
  gboolean input_label = TRUE, input_rest = TRUE;
  gboolean justify;
  GList *tmp_list;
  GtkAccelGroup *accel_group;
  guint key;

  /* If we are loading, check for a stock menu item. */
  if (data->action == GB_LOADING)
    {
      GnomeUIInfo *uiinfo;
      gchar *stock_item;
      GtkWidget *label, *pixmap = NULL;
      gint stock_item_index;

      stock_item = gb_widget_input_string (data, "stock_item");
      if (stock_item && stock_item[0])
	{
	  /* Special case for the NEW_SUBTREE. */
	  if (!strcmp (stock_item, "GNOMEUIINFO_MENU_NEW_SUBTREE"))
	    {
	      stock_item_index = GladeStockMenuItemNew;
	    }
	  else
	    {
	      stock_item_index = glade_util_string_array_index (GladeStockMenuItemSymbols, GladeStockMenuItemSize, stock_item);
	    }

	  if (stock_item_index != -1)
	    {
	      uiinfo = &GladeStockMenuItemValues[stock_item_index];
	      if (uiinfo->type == GNOME_APP_UI_ITEM_CONFIGURABLE)
		gnome_app_ui_configure_configurable (uiinfo);

	      if (uiinfo->pixmap_type == GNOME_APP_PIXMAP_STOCK)
		pixmap = gnome_stock_pixmap_widget (widget,
						    uiinfo->pixmap_info);

	      if (pixmap)
		{
		  gtk_widget_show (pixmap);
		  gtk_pixmap_menu_item_set_pixmap (GTK_PIXMAP_MENU_ITEM (widget), pixmap);
		}

	      label = gtk_accel_label_new ("");
	      gtk_label_parse_uline (GTK_LABEL (label), uiinfo->label);
	      gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	      gtk_widget_show (label);
	      gtk_accel_label_set_accel_widget (GTK_ACCEL_LABEL (label),
						widget);
	      gtk_container_add (GTK_CONTAINER (widget), label);

	      /* Add the configured accelerator key. */
	      if (uiinfo->accelerator_key != 0 && widget->parent
		  && GTK_IS_MENU (widget->parent))
		{
		  accel_group = GTK_MENU (widget->parent)->accel_group;
		  gtk_widget_add_accelerator (widget, "activate", accel_group,
					      uiinfo->accelerator_key,
					      uiinfo->ac_mods,
					      GTK_ACCEL_VISIBLE);
		}

	      /* Remember the index of the stock item. */
	      gtk_object_set_data (GTK_OBJECT (widget),
				   GladeMenuItemStockIndexKey,
				   GINT_TO_POINTER (stock_item_index));

	      /* The 'New' item can have a label. The rest can't. */
	      if (stock_item_index != GladeStockMenuItemNew)
		input_label = FALSE;
	      input_rest = FALSE;
	    }
	  else
	    {
	      load_add_error_message_with_tag (data,
					       GLADE_LINE_PROPERTY,
					       _("Invalid stock menu item"),
					       "stock_item", stock_item);
	    }
	}
    }

  if (input_label)
    gb_widget_input_child_label (widget, data, Label);

  if (input_rest)
    {
      justify = gb_widget_input_bool (data, Justify);
      if (data->apply)
	{
	  /* GtkMenuItem doesn't have a function for setting/clearing
	     right-justify */
	  GTK_MENU_ITEM (widget)->right_justify = justify;
	  gtk_widget_queue_resize (widget);
	}

      /* Load the stock or ordinary icon. */
      if (data->action == GB_LOADING)
	{
	  gchar *stock_icon = gb_widget_input_string (data, "stock_icon");
	  if (stock_icon && stock_icon[0])
	    {
	      gint stock_index;

	      stock_index = glade_util_string_array_index (GladeStockMenuPixmapSymbols, GladeStockMenuPixmapSize, stock_icon);

	      if (stock_index != -1)
		{
		  GtkWidget *pixmap = gnome_stock_pixmap_widget (widget, GladeStockMenuPixmapValues[stock_index]);
		  if (pixmap)
		    {
		      gtk_widget_show (pixmap);
		      gtk_pixmap_menu_item_set_pixmap (GTK_PIXMAP_MENU_ITEM (widget), pixmap);
		    }
		  /* FIXME: Note that we store the translated icon name.
		     Maybe we should store the index somewhere else instead. */
		  gtk_object_set_data_full (GTK_OBJECT (widget),
					    GladeMenuItemIconKey,
					    g_strdup (_(GladeStockMenuPixmapChoices[stock_index])),
					    g_free);
		}
	      else
		{
		  load_add_error_message_with_tag (data,
						   GLADE_LINE_PROPERTY,
						   _("Invalid stock menu pixmap"),
						   "stock_icon", stock_icon);
		}
	    }
	  else
	    {
	      GtkWidget *pixmap;
	      GdkColormap *colormap;
	      GdkPixmap *gdkpixmap;
	      GdkBitmap *mask;
	      gchar *icon_filename;

	      /* Now see if there is a normal icon pixmap. */
	      icon_filename = gb_widget_input_pixmap_filename (data, "icon");
	      if (icon_filename && icon_filename[0])
		{
		  glade_project_add_pixmap (data->project, icon_filename);
		  gtk_object_set_data_full (GTK_OBJECT (widget),
					    GladeMenuItemIconKey,
					    g_strdup (icon_filename), g_free);

		  colormap = gtk_widget_get_colormap (widget);
		  gdkpixmap = gdk_pixmap_colormap_create_from_xpm (NULL,
								   colormap,
								   &mask, NULL,
								   icon_filename);
		  if (gdkpixmap)
		    {
		      pixmap = gtk_pixmap_new (gdkpixmap, mask);
		      gdk_pixmap_unref (gdkpixmap);
		      gdk_bitmap_unref (mask);
		      gtk_widget_show (pixmap);
		      gtk_pixmap_menu_item_set_pixmap (GTK_PIXMAP_MENU_ITEM (widget), pixmap);
		    }
		  else
		    {
		      load_add_error_message_with_tag (data,
						       GLADE_LINE_PROPERTY,
						       _("Couldn't load pixmap"),
						       "icon", icon_filename);
		    }

		}
	    }
	}
      

      /* FIXME: should this be somewhere else? */
      /* If we are loading, install the 'activate' accelerator, if it has one,
	 so that is is visible. */
      if (data->action == GB_LOADING)
	{
	  tmp_list = data->accelerators;
	  while (tmp_list)
	    {
	      GladeAccelerator *accel = (GladeAccelerator *) tmp_list->data;
	      if (!strcmp (accel->signal, "activate")
		  && widget->parent && GTK_IS_MENU (widget->parent))
		{
		  key = glade_keys_dialog_find_key (accel->key);
		  accel_group = GTK_MENU (widget->parent)->accel_group;
		  gtk_widget_add_accelerator (widget, "activate", accel_group,
					      key, accel->modifiers,
					      GTK_ACCEL_VISIBLE);
		  break;
		}
	      tmp_list = tmp_list->next;
	    }
	}
    }
}



/*
 * Adds menu items to a context menu which is just about to appear!
 * Add commands to aid in editing a GtkPixmapMenuItem, with signals pointing to
 * other functions in this file.
 */
static void
gb_pixmap_menu_item_create_popup_menu (GtkWidget * widget,
				       GbWidgetCreateMenuData * data)
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
gb_pixmap_menu_item_write_source (GtkWidget * widget,
				  GbWidgetWriteSourceData * data)
{
  glade_gnome_write_menu_item_source (GTK_MENU_ITEM (widget), data);
}


/* Outputs source to add a child menu to a menu item. */
static void
gb_pixmap_menu_item_write_add_child_source (GtkWidget * parent,
					    const gchar *parent_name,
					    GtkWidget *child,
					    GbWidgetWriteSourceData * data)
{
  if (GTK_IS_MENU (child))
    {
      source_add (data,
		  "  gtk_menu_item_set_submenu (GTK_MENU_ITEM (%s), %s);\n",
		  parent_name, data->wname);
    }
  else
    {
      source_add (data, "  gtk_container_add (GTK_CONTAINER (%s), %s);\n",
		  parent_name, data->wname);
    }
}


static void
gb_pixmap_menu_item_destroy (GtkWidget * widget, GbWidgetDestroyData * data)
{
  gchar *filename;

  filename = gtk_object_get_data (GTK_OBJECT (widget), GladeMenuItemIconKey);
  /* If it is a pixmap from a file, rather than a stock icon, remove it from
     the project. */
  if (filename && glade_gnome_get_stock_menu_pixmap_index (filename) == -1)
    {
      glade_project_remove_pixmap (data->project, filename);
    }
}


/*
 * Initializes the GbWidget structure.
 * I've placed this at the end of the file so we don't have to include
 * declarations of all the functions.
 */
GbWidget*
gb_pixmap_menu_item_init ()
{
  /* Initialise the GTK type */
  gtk_pixmap_menu_item_get_type();

  /* Initialize the GbWidget structure */
  gb_widget_init_struct(&gbwidget);

  /* Fill in the pixmap struct & tooltip */
  gbwidget.pixmap_struct = menuitem_xpm;
  gbwidget.tooltip = _("Menu item with a pixmap");

  /* Fill in any functions that this GbWidget has */
  gbwidget.gb_widget_new		= gb_pixmap_menu_item_new;
  gbwidget.gb_widget_add_child		= gb_pixmap_menu_item_add_child;
  gbwidget.gb_widget_create_properties	= gb_pixmap_menu_item_create_properties;
  gbwidget.gb_widget_get_properties	= gb_pixmap_menu_item_get_properties;
  gbwidget.gb_widget_set_properties	= gb_pixmap_menu_item_set_properties;
  gbwidget.gb_widget_create_popup_menu	= gb_pixmap_menu_item_create_popup_menu;
  gbwidget.gb_widget_write_source	= gb_pixmap_menu_item_write_source;
  gbwidget.gb_widget_write_add_child_source = gb_pixmap_menu_item_write_add_child_source;
  gbwidget.gb_widget_destroy		= gb_pixmap_menu_item_destroy;

  return &gbwidget;
}

