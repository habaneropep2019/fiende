
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

#include <string.h>

#ifdef USE_GNOME
#include <gnome.h>
#include "../glade_gnome.h"
#else
#include <gtk/gtkbutton.h>
#include <gtk/gtklabel.h>
#endif

#include "../gb.h"

/* Include the 21x21 icon pixmap for this widget, to be used in the palette */
#include "../graphics/button.xpm"

/*
 * GtkButton widget - unfortunately this has been 'overloaded' a bit to cope
 * with extra properties available in toolbar buttons and Gnome buttons.
 * Standard toolbar buttons can have an icon as well as a label.
 * Gnome buttons can be a stock button (which has a label and icon), or can
 * have a stock pixmap as well as a label if they are in a toolbar or in a
 * GnomeDialog/MessageBox.
 */


/*
 * This is the GbWidget struct for this widget (see ../gbwidget.h).
 * It is initialized in the init() function at the end of this file
 */
static GbWidget gbwidget;

static gchar *Label = "GtkButton::label";
/* This is only used for toolbar buttons. */
static gchar *Icon = "GtkButton::icon";

/* This is only used for normal/stock buttons, not toolbar buttons or Gnome
   dialog buttons. */
static gchar *Relief = "GtkButton::relief";

#ifdef USE_GNOME
/* This is used for Gnome Stock buttons. */
static gchar *StockButton = "GtkButton::stock_button";
static gchar *StockIcon = "GtkButton::stock_pixmap";
#endif

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


#ifdef USE_GNOME
static void gb_button_output_gnome_dialog_button_label (GtkWidget * widget,
							GbWidgetGetArgData * data,
							const gchar * Label,
							gboolean is_stock_button);
static void gb_button_input_gnome_dialog_button_label (GtkWidget * widget,
						       GbWidgetSetArgData * data,
						       const gchar * Label);
static gboolean gb_button_get_gnome_dialog_button_icon_and_label (GtkWidget *widget,
								  GtkWidget **icon_return,
								  GtkWidget **label_return);
static void gb_button_output_gnome_dialog_button_icon (GtkWidget * widget,
						       GbWidgetGetArgData * data,
						       const gchar * property_name,
						       gboolean is_stock_button);
static void gb_button_input_gnome_dialog_button_icon (GtkWidget * widget,
						      GbWidgetSetArgData * data,
						      const gchar * property_name);
static void gb_button_setup_normal_gnome_dialog_button (GtkWidget *widget);
static gboolean gb_button_set_stock_button (GtkWidget *widget,
					    GbWidgetSetArgData * data,
					    gchar *stock_button);
static void gb_button_set_stock_icon (GtkWidget *widget,
				      GbWidgetSetArgData * data,
				      gint stock_index);
static void gb_button_write_gnome_dialog_button_source (GtkWidget * widget,
							GbWidgetWriteSourceData * data);
#endif


/******
 * NOTE: To use these functions you need to uncomment them AND add a pointer
 * to the function in the GbWidget struct at the end of this file.
 ******/

/*
 * Creates a new GtkWidget of class GtkButton, performing any specialized
 * initialization needed for the widget to work correctly in this environment.
 * If a dialog box is used to initialize the widget, return NULL from this
 * function, and call data->callback with your new widget when it is done.
 * If the widget needs a special destroy handler, add a signal here.
 */
GtkWidget *
gb_button_new (GbWidgetNewData * data)
{
  GtkWidget *new_widget;

  /* SPECIAL CODE: to handle toolbar buttons. */
  MSG1 ("IN gb_button_new parent:%s",
	data->parent ? gtk_widget_get_name (data->parent) : "NULL");
  if (data->parent && GTK_IS_TOOLBAR (data->parent))
    {
      MSG ("   creating toolbar button...");
      new_widget = gb_toolbar_new_toolbar_button (data,
						  GTK_TOOLBAR_CHILD_BUTTON);
      if (new_widget)
	return new_widget;
    }

  if (data->action == GB_CREATING)
    new_widget = gtk_button_new_with_label (data->name);
  else
    {
      MSG ("   creating normal button with placeholder...");
      new_widget = gtk_button_new ();
      gtk_container_add (GTK_CONTAINER (new_widget), editor_new_placeholder());
    }
  return new_widget;
}


/*
 * Creates the components needed to edit the extra properties of this widget.
 */
static void
gb_button_create_properties (GtkWidget * widget, GbWidgetCreateArgData * data)
{
#ifdef USE_GNOME
  property_add_choice (StockButton, _("Stock Button:"),
		       _("The Gnome stock button to use"),
		       GladeStockButtonChoices);
  property_add_text (Label, _("Label:"), _("The text to display"), 2);
  property_add_choice (Relief, _("Button Relief:"),
		       _("The relief style of the button"),
		       GbReliefChoices);
  gb_button_create_child_icon_property (widget, data, Icon);
#else
  property_add_text (Label, _("Label:"), _("The text to display"), 2);
  property_add_choice (Relief, _("Button Relief:"),
		       _("The relief style of the button"),
		       GbReliefChoices);
  property_add_filename (Icon, _("Icon:"), _("The pixmap to display"));
#endif
}


#ifdef USE_GNOME
void
gb_button_create_child_icon_property (GtkWidget * widget,
				      GbWidgetCreateArgData * data,
				      gchar *property_name)
{
  GtkWidget *combo, *listitem, *hbox, *pixmap, *label;
  gchar *label_text;
  gint i;

  property_add_filename_with_combo (property_name, _("Icon:"),
				    _("The pixmap to display"), NULL);

  /* Now we add the stock pixmaps to the property combo. */
  combo = property_get_value_widget (property_name);
  g_return_if_fail (combo != NULL);

  for (i = 0; GladeStockPixmapChoices[i]; i++)
    {
      listitem = gtk_list_item_new ();
      gtk_widget_show (listitem);
      hbox = gtk_hbox_new (FALSE, 3);
      gtk_container_add (GTK_CONTAINER (listitem), hbox);
      gtk_widget_show (hbox);

      if (GladeStockPixmapValues[i])
	{
	  pixmap = gnome_stock_pixmap_widget (GTK_WIDGET (combo),
					      GladeStockPixmapValues[i]);
	  if (pixmap)
	    {
	      gtk_widget_show (pixmap);
	      gtk_box_pack_start (GTK_BOX (hbox), pixmap, FALSE, FALSE, 0);
	    }
	}

      label_text = _(GladeStockPixmapChoices[i]);
      label = gtk_label_new (label_text);
      gtk_widget_show (label);
      gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

      gtk_combo_set_item_string (GTK_COMBO (combo), GTK_ITEM (listitem),
				 label_text);
      gtk_container_add (GTK_CONTAINER (GTK_COMBO (combo)->list), listitem);
    }
}
#endif


/*
 * Gets the properties of the widget. This is used for both displaying the
 * properties in the property editor, and also for saving the properties.
 */
static void
gb_button_get_properties (GtkWidget * widget, GbWidgetGetArgData * data)
{
  gboolean relief_visible = FALSE;
  gint i;

#ifdef USE_GNOME
  gint stock_button_index;
  gboolean stock_item_visible;

  if (gb_toolbar_is_toolbar_button (widget))
    {
      gb_toolbar_output_child_label (widget, data, Label);
      gb_toolbar_output_child_icon (widget, data, Icon);
      stock_item_visible = FALSE;
    }
  else
    {
      stock_button_index = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (widget), StockButton));
      gb_widget_output_choice (data, StockButton, stock_button_index,
			       GladeStockButtonSymbols[stock_button_index]);
      stock_item_visible = TRUE;

      if (glade_gnome_is_gnome_dialog_button (widget))
	{
	  gb_button_output_gnome_dialog_button_label (widget, data, Label,
						      stock_button_index > 0);
	  gb_button_output_gnome_dialog_button_icon (widget, data, Icon,
						     stock_button_index > 0);
	}
      else
	{
	  relief_visible = TRUE;
	  gb_widget_output_child_label (widget, data, Label);

	  if (data->action == GB_SHOWING)
	    property_set_visible (Icon, FALSE);
	}
    }

  if (data->action == GB_SHOWING)
    {
      property_set_visible (StockButton, stock_item_visible);
    }

#else
  if (gb_toolbar_is_toolbar_button (widget))
    {
      gb_toolbar_output_child_label (widget, data, Label);
      gb_toolbar_output_child_icon (widget, data, Icon);
    }
  else
    {
      relief_visible = TRUE;
      gb_widget_output_child_label (widget, data, Label);

      /* The Icon property is not applicable to normal buttons in GTK+. */
      if (data->action == GB_SHOWING)
	property_set_visible (Icon, FALSE);
    }
#endif

  if (data->action == GB_SHOWING)
    property_set_visible (Relief, relief_visible);

  if (relief_visible)
    {
      for (i = 0; i < sizeof (GbReliefValues) / sizeof (GbReliefValues[0]); i++)
	{
	  if (GbReliefValues[i] == GTK_BUTTON (widget)->relief)
	    gb_widget_output_choice (data, Relief, i, GbReliefSymbols[i]);
	}
    }
}


/*
 * Sets the properties of the widget. This is used for both applying the
 * properties changed in the property editor, and also for loading.
 */
static void
gb_button_set_properties (GtkWidget * widget, GbWidgetSetArgData * data)
{
  gboolean set_relief = FALSE;
  gchar *relief;
  gint i;

#ifdef USE_GNOME
  gchar *stock_button;
  gboolean status;

  if (gb_toolbar_is_toolbar_button (widget))
    {
      gb_toolbar_input_child_label (widget, data, Label);
      gb_toolbar_input_child_icon (widget, data, Icon);
    }
  else
    {
      stock_button = gb_widget_input_choice (data, StockButton);
      if (data->apply)
	{
	  status = gb_button_set_stock_button (widget, data, stock_button);
	  if (!status && data->action == GB_LOADING)
	    {
	      load_add_error_message_with_tag (data,
					       GLADE_LINE_PROPERTY,
					       _("Invalid stock button"),
					       StockButton, stock_button);
	    }
	}
      else if (glade_gnome_is_gnome_dialog_button (widget))
	{
	  gb_button_input_gnome_dialog_button_label (widget, data, Label);
	  gb_button_input_gnome_dialog_button_icon (widget, data, Icon);
	}
      else
	{
	  set_relief = TRUE;
	  gb_widget_input_child_label (widget, data, Label);
	}
    }

#else
  if (gb_toolbar_is_toolbar_button (widget))
    {
      gb_toolbar_input_child_label (widget, data, Label);
      gb_toolbar_input_child_icon (widget, data, Icon);
    }
  else
    {
      set_relief = TRUE;
      gb_widget_input_child_label (widget, data, Label);
    }
#endif

  if (set_relief)
    {
      relief = gb_widget_input_choice (data, Relief);
      if (data->apply)
	{
	  for (i = 0; i < sizeof (GbReliefValues) / sizeof (GbReliefValues[0]);
	       i++)
	    {
	      if (!strcmp (relief, GbReliefChoices[i])
		  || !strcmp (relief, GbReliefSymbols[i]))
		{
		  gtk_button_set_relief (GTK_BUTTON (widget),
					 GbReliefValues[i]);
		  break;
		}
	    }
	}
    }
}


/*
 * Adds menu items to a context menu which is just about to appear!
 * Add commands to aid in editing a GtkButton, with signals pointing to
 * other functions in this file.
 */
static void
gb_button_create_popup_menu (GtkWidget * widget, GbWidgetCreateMenuData * data)
{
  /* Add command to remove child label. */
  gb_widget_create_child_label_popup_menu (widget, data);

  /* Add commands for toolbar buttons. */
  gb_toolbar_create_toolbar_button_popup_menu (widget, data);
}



/*
 * Writes the source code needed to create this widget.
 * You have to output everything necessary to create the widget here, though
 * there are some convenience functions to help.
 */
static void
gb_button_write_source (GtkWidget * widget, GbWidgetWriteSourceData * data)
{
  GtkWidget *child = GTK_BUTTON (widget)->child;
  gchar *label_text;
  gboolean set_relief = TRUE;
  gint i;

#ifdef USE_GNOME
  if (glade_gnome_is_gnome_dialog_button (widget))
    {
      gb_button_write_gnome_dialog_button_source (widget, data);
      return;
    }
#endif

  if (data->create_widget)
    {
#ifdef USE_GNOME
      gint stock_button_index = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (widget), StockButton));
#endif

      if (gb_toolbar_is_toolbar_button (widget))
	{
	  gb_toolbar_write_toolbar_button_source (widget, data);
	  set_relief = FALSE;
	}
#ifdef USE_GNOME
      else if (stock_button_index > 0)
	{
	  source_add (data, "  %s = gnome_stock_button (%s);\n", data->wname,
		      GladeStockButtonSymbols[stock_button_index]);
	}
#endif
      else if (child && GTK_IS_LABEL (child) && !GB_IS_GB_WIDGET (child))
	{
	  label_text = glade_util_get_label_text (GTK_BIN (widget)->child);
	  /* If there is an underlined accelerator, set up the accel signal. */
	  if (strchr (label_text, '_'))
	    {
	      source_add (data, "  %s = gtk_button_new_with_label (\"\");\n",
			  data->wname);
	      gb_button_write_uline_accel_source (widget, data, label_text);
	    }
	  else
	    {
	      source_add (data, "  %s = gtk_button_new_with_label (%s);\n",
			  data->wname,
			  source_make_string (label_text, data->use_gettext));
	    }
	  g_free (label_text);
	}
      else
	{
	  source_add (data, "  %s = gtk_button_new ();\n", data->wname);
	}
    }

  gb_widget_write_standard_source (widget, data);

  if (set_relief && GTK_BUTTON (widget)->relief != GTK_RELIEF_NORMAL)
    {
      for (i = 0; i < sizeof (GbReliefValues) / sizeof (GbReliefValues[0]);
	   i++)
	{
	  if (GbReliefValues[i] == GTK_BUTTON (widget)->relief)
	    source_add (data,
			"  gtk_button_set_relief (GTK_BUTTON (%s), %s);\n",
			data->wname, GbReliefSymbols[i]);
	}
    }
}


void
gb_button_write_uline_accel_source (GtkWidget * widget,
				    GbWidgetWriteSourceData * data,
				    gchar *label_text)
{
  source_add_decl (data, "  guint %s_key;\n", data->real_wname);
  source_add (data,
	      "  %s_key = gtk_label_parse_uline (GTK_LABEL (GTK_BIN (%s)->child),\n"
	      "                                   %s);\n",
	      data->real_wname, data->wname,
	      source_make_string (label_text, data->use_gettext));

  data->need_accel_group = TRUE;
  source_add (data,
	      "  gtk_widget_add_accelerator (%s, \"clicked\", accel_group,\n"
	      "                              %s_key, GDK_MOD1_MASK, (GtkAccelFlags) 0);\n",
	      data->wname, data->real_wname);
}


static void
gb_button_destroy (GtkWidget * widget, GbWidgetDestroyData * data)
{
  gb_toolbar_destroy_child_icon (widget, data);
}


/*************************************************************************
 * Functions for handling buttons in GnomeDialog/MessageBox.
 *************************************************************************/

#ifdef USE_GNOME

void
gb_button_output_gnome_dialog_button_label (GtkWidget * widget,
					    GbWidgetGetArgData * data,
					    const gchar * property_name,
					    gboolean is_stock_button)
{
  GtkWidget *label;
  gchar *label_text;

  /* We can only edit the label & icon when the stock button is set
     to "None" (its index is 0). */
  if (!is_stock_button)
    {
      gb_button_get_gnome_dialog_button_icon_and_label (widget, NULL, &label);
      g_return_if_fail (label != NULL);

      label_text = glade_util_get_label_text (label);
      gb_widget_output_translatable_text (data, property_name, label_text);
      g_free (label_text);
    }

  if (data->action == GB_SHOWING)
    {
      property_set_sensitive (property_name, !is_stock_button);

      if (is_stock_button)
	gb_widget_output_text (data, property_name, "");
    }
}


/* This sets the child label for buttons/items/menuitems or subclasses.
   This is for applying or loading. */
void
gb_button_input_gnome_dialog_button_label (GtkWidget * widget,
					   GbWidgetSetArgData * data,
					   const gchar * Label)
{
  GtkWidget *label;
  gchar *label_text;

  label_text = gb_widget_input_text (data, Label);
  if (data->apply)
    {
      gb_button_setup_normal_gnome_dialog_button (widget);
      gb_button_get_gnome_dialog_button_icon_and_label (widget, NULL, &label);

      /* We use parse_uline so letters can be underlined. */
      gtk_label_parse_uline (GTK_LABEL (label), label_text);
    }
  /* This isn't very nice. When a text property is got from the property
     editor (i.e. when action is GB_APPLYING) it needs to be freed after. */
  if (data->action == GB_APPLYING)
    g_free (label_text);
}


/* This gets the stock icon & label for GnomeDialog buttons.
   This is for showing or saving. */
static void
gb_button_output_gnome_dialog_button_icon (GtkWidget * widget,
					   GbWidgetGetArgData * data,
					   const gchar * property_name,
					   gboolean is_stock_button)
{
  GtkWidget *combo;
  gint stock_icon_index;

  if (data->action == GB_SHOWING)
    {
      property_set_visible (property_name, TRUE);

      /* For GnomeDialog buttons, the icon filename can't be edited. */
      combo = property_get_value_widget (property_name);
      g_return_if_fail (combo != NULL);
      gtk_entry_set_editable (GTK_ENTRY (GTK_COMBO (combo)->entry),
			      FALSE);
      property_set_sensitive_full (property_name, !is_stock_button,
				   !is_stock_button, FALSE);
    }	  

  /* Find out if the icon is a stock icon, and if it is output that. */
  stock_icon_index = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (widget),
							   StockIcon));
  if (stock_icon_index)
    {
      /* We save the stock icon with a different tag, to make it easier
	 for external programs which read the XML. */
      if (data->action == GB_SAVING)
	{
	  gb_widget_output_string (data, "stock_pixmap",
				   _(GladeStockPixmapSymbols[stock_icon_index]));
	}
      else
	{
	  gb_widget_output_pixmap_filename (data, property_name,
					    _(GladeStockPixmapChoices[stock_icon_index]));
	}
      return;
    }
  else if (data->action == GB_SHOWING)
    {
      gb_widget_output_pixmap_filename (data, property_name, "");
    }
}


/* This sets the icon for toolbar buttons. This is for applying or loading.
   It can be a stock Gnome icon or an icon filename. */
static void
gb_button_input_gnome_dialog_button_icon (GtkWidget * widget,
					  GbWidgetSetArgData * data,
					  const gchar * property_name)
{
  gint stock_icon_index;
  gchar *filename;

  /* GnomeDialog button icons can only be stock pixmaps.
     Note that it is saved with a different tag than is used internally. */
  if (data->action == GB_LOADING)
    {
      filename = gb_widget_input_string (data, "stock_pixmap");
      if (filename && filename[0])
	{
	  stock_icon_index = glade_util_string_array_index (GladeStockPixmapSymbols, GladeStockPixmapSize, filename);
	  if (stock_icon_index != -1)
	    {
	      gb_button_set_stock_icon (widget, data, stock_icon_index);
	    }
	  else
	    {
	      load_add_error_message_with_tag (data,
					       GLADE_LINE_PROPERTY,
					       _("Invalid stock pixmap"),
					       "stock_pixmap", filename);
	    }
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
	      gb_button_set_stock_icon (widget, data, stock_icon_index);
	    }
	}
    }
}


/* This tries to find the icon and label in a gnome dialog button.
   The button may have just an label child, or it may have a hbox with a
   label & an icon. Note that this won't work with stock buttons.
   It returns TRUE if the button is already set up as a GnomeDialog button. */
static gboolean
gb_button_get_gnome_dialog_button_icon_and_label (GtkWidget *widget,
						  GtkWidget **icon_return,
						  GtkWidget **label_return)
{
  GtkWidget *child, *hbox_child, *icon = NULL, *label = NULL;
  GList *children;

  child = GTK_BIN (widget)->child;

  if (child && GTK_IS_LABEL (child))
    {
      if (label_return)
	*label_return = child;
      return FALSE;
    }

  /* The button's child must be a hbox with one child. */
  if (child && GTK_IS_HBOX (child)
      && g_list_length (GTK_BOX (child)->children) == 1)
    {
      children = GTK_BOX (child)->children;
      hbox_child = ((GtkBoxChild*) children->data)->widget;

      /* The hbox's child must also be a hbox with < 2 children. */
      if (GTK_IS_HBOX (hbox_child)
	  && g_list_length (GTK_BOX (hbox_child)->children) <= 2)
	{
	  children = GTK_BOX (hbox_child)->children;

	  while (children)
	    {
	      hbox_child = ((GtkBoxChild*) children->data)->widget;

	      if (GNOME_IS_STOCK (hbox_child) || GNOME_IS_PIXMAP (hbox_child))
		{
		  icon = hbox_child;
		}
	      else if (GTK_IS_LABEL (hbox_child))
		{
		  label = hbox_child;
		}
	      children = children->next;
	    }
	}
    }

  if (icon_return)
    *icon_return = icon;
  if (label_return)
    *label_return = label;

  return (label != NULL);
}


/* This tries to set the button to the given stock button, and returns TRUE
   if it succeeds. */
static gboolean
gb_button_set_stock_button (GtkWidget *widget,
			    GbWidgetSetArgData * data,
			    gchar *stock_button)
{
  GtkWidget *new_button, *child, *icon, *label;
  gint i, stock_index = -1;
  gboolean label_sensitive = FALSE;
  gchar *label_text = NULL;

  if (stock_button && stock_button[0] && strcmp (stock_button, "None"))
    {
      /* First we check it is a valid stock button name, and find the real
	 string identifier. We start at 1 to skip "None". */
      for (i = 1; i < GladeStockButtonSize; i++)
	{
	  if (!strcmp (stock_button, GladeStockButtonChoices[i])
	      || !strcmp (stock_button, GladeStockButtonSymbols[i]))
	    {
	      stock_index = i;
	      break;
	    }
	}

      /* If we didn't find it, return FALSE. */
      if (stock_index == -1)
	return FALSE;

      /* Reset any stock icon. */
      gtk_object_set_data (GTK_OBJECT (widget), StockIcon, NULL);

      /* We save the stock button index in the widget. */
      gtk_object_set_data (GTK_OBJECT (widget), StockButton,
			   GINT_TO_POINTER (stock_index));

      /* We create a stock button, move its contents into our own button,
	 then destroy it. */
      new_button = gnome_stock_button (GladeStockButtonValues[stock_index]);
      child = GTK_BIN (new_button)->child;
      gtk_widget_ref (child);
      gtk_container_remove (GTK_CONTAINER (new_button), child);
      if (GTK_BIN (widget)->child)
	gtk_container_remove (GTK_CONTAINER (widget), GTK_BIN (widget)->child);
      gtk_container_add (GTK_CONTAINER (widget), child);
      gtk_widget_unref (child);
      gtk_widget_unref (new_button);
    }
  else
    {
      gtk_object_set_data (GTK_OBJECT (widget), StockButton, NULL);

      /* Change the button back to a normal button with a simple label. */
      if (glade_gnome_is_gnome_dialog_button (widget))
	{
	  gb_button_setup_normal_gnome_dialog_button (widget);
	  gb_button_get_gnome_dialog_button_icon_and_label (widget, &icon,
							    &label);
	  if (icon)
	    gtk_container_remove (GTK_CONTAINER (icon->parent), icon);
	}
      else
	{
	  child = GTK_BIN (widget)->child;
	  if (child)
	    gtk_container_remove (GTK_CONTAINER (widget), child);

	  label = gtk_label_new ("");
	  gtk_widget_show (label);
	  gtk_container_add (GTK_CONTAINER (widget), label);
	}

      label_text = gtk_widget_get_name (widget);
      gtk_label_parse_uline (GTK_LABEL (label), label_text);
      label_sensitive = TRUE;
    }

  /* If the widget's properties are displayed, we update the sensitivity of
     the label, according to whether a stock item is selected. */
  if (data->action == GB_APPLYING
      && property_get_widget () == widget)
    {
      property_set_sensitive (Label, label_sensitive);
      property_set_sensitive_full (Icon, label_sensitive, label_sensitive,
				   !glade_gnome_is_gnome_dialog_button (widget));

      /* Turn off auto-apply, and set the label. */
      property_set_auto_apply (FALSE);
      property_set_text (Label, label_text);
      property_set_filename (Icon, "");
      property_set_auto_apply (TRUE);
    }

  return TRUE;
}


/* This tries to set the button's icon to the given stock pixmap. */
static void
gb_button_set_stock_icon (GtkWidget *widget,
			  GbWidgetSetArgData * data,
			  gint stock_index)
{
  GtkWidget *icon, *label, *child, *new_icon;

  gb_button_setup_normal_gnome_dialog_button (widget);
  gb_button_get_gnome_dialog_button_icon_and_label (widget, &icon, &label);
  child = GTK_BIN (widget)->child;

  if (stock_index > 0)
    {
      /* We save the stock button index in the widget. */
      gtk_object_set_data (GTK_OBJECT (widget), StockIcon,
			   GINT_TO_POINTER (stock_index));

      /* We create a stock icon and replace the existing icon. */
      new_icon = gnome_stock_pixmap_widget (widget, GladeStockPixmapValues[stock_index]);

      if (icon)
	{
	  gtk_container_remove (GTK_CONTAINER (icon->parent), icon);
	}
      if (new_icon)
	{
	  gtk_widget_show (new_icon);
	  gtk_box_pack_start (GTK_BOX (label->parent), new_icon,
			      FALSE, FALSE, 0);
	}
    }
  else
    {
      gtk_object_set_data (GTK_OBJECT (widget), StockIcon, NULL);

      if (icon)
	{
	  gtk_container_remove (GTK_CONTAINER (icon->parent), icon);
	}
    }
}


/* This will check if the button contains the widgets in a normal GnomeDialog
   button (a label and an optional stock icon), and if not it will remove the
   existing children and add the standard widgets and a label. */
static void
gb_button_setup_normal_gnome_dialog_button (GtkWidget *widget)
{
  GtkWidget *child, *hbox, *hbox2, *label;
  gboolean existing_label = FALSE;

  /* If it is already a standard button, just return. */
  if (gb_button_get_gnome_dialog_button_icon_and_label (widget, NULL, NULL))
    return;

  child = GTK_BIN (widget)->child;
  if (child)
    {
      if (GTK_IS_LABEL (child))
	{
	  label = child;
	  gtk_widget_ref (label);
	  existing_label = TRUE;
	}
      gtk_container_remove (GTK_CONTAINER (widget), child);
    }

  /* Add the standard hbox and hbox child, just like in gnome-stock.c. */
  hbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox);
  gtk_container_add (GTK_CONTAINER (widget), hbox);

  hbox2 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox2);
  gtk_box_pack_start (GTK_BOX (hbox), hbox2, TRUE, FALSE, 7);

  if (!existing_label)
    {
      label = gtk_label_new ("");
      gtk_widget_show(label);
    }
  gtk_box_pack_end (GTK_BOX (hbox2), label, FALSE, FALSE, 7);
  if (existing_label)
    gtk_widget_unref (label);
}


/* We need to use gnome_dialog_append_button_with_pixmap() to add the
   button to the dialog. */
static void
gb_button_write_gnome_dialog_button_source (GtkWidget * widget,
					    GbWidgetWriteSourceData * data)
{
  gint stock_button_index, stock_icon_index;
  GtkWidget *label;
  gchar *label_text;

  stock_button_index = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (widget), StockButton));
  if (stock_button_index > 0)
    {
      source_add (data, "  gnome_dialog_append_button (GNOME_DIALOG (%s), %s);\n",
		  data->component_name,
		  GladeStockButtonSymbols[stock_button_index]);
    }
  else
    {
      gb_button_get_gnome_dialog_button_icon_and_label (widget, NULL, &label);
      g_return_if_fail (label != NULL);

      stock_icon_index = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (widget), StockIcon));
      gtk_label_get (GTK_LABEL (label), &label_text);

      if (stock_icon_index > 0)
	{
	  source_add (data,
		      "  gnome_dialog_append_button_with_pixmap (GNOME_DIALOG (%s),\n"
		      "                                          %s, %s);\n",
		      data->component_name,
		      source_make_string (label_text, data->use_gettext),
		      GladeStockPixmapSymbols[stock_icon_index]);
	}
      else
	{
	  source_add (data,
		      "  gnome_dialog_append_button (GNOME_DIALOG (%s), %s);\n",
		      data->component_name,
		      source_make_string (label_text, data->use_gettext));
	}
    }

  /* We want to get a pointer to the widget so we can output the standard
     source, in case handlers have been added to the buttons.
     We can just get the last widget in the GnomeDialog's list of buttons.
     Note that we have to make sure that the button isn't added to its
     parent widget again elsewhere. */
  source_add (data, "  %s = GTK_WIDGET (g_list_last (GNOME_DIALOG (%s)->buttons)->data);\n",
	      data->wname, data->component_name);

  /* We set this to FALSE, so the code to add it to its parent is skipped. */
  data->create_widget = FALSE;

  gb_widget_write_standard_source (widget, data);
}
#endif


/*
 * Initializes the GbWidget structure.
 * I've placed this at the end of the file so we don't have to include
 * declarations of all the functions.
 */
GbWidget *
gb_button_init ()
{
  /* Initialise the GTK type */
  gtk_button_get_type ();

  /* Initialize the GbWidget structure */
  gb_widget_init_struct (&gbwidget);

  /* Fill in the pixmap struct & tooltip */
  gbwidget.pixmap_struct = button_xpm;
  gbwidget.tooltip = _("Button");

  /* Fill in any functions that this GbWidget has */
  gbwidget.gb_widget_new = gb_button_new;
  gbwidget.gb_widget_create_properties = gb_button_create_properties;
  gbwidget.gb_widget_get_properties = gb_button_get_properties;
  gbwidget.gb_widget_set_properties = gb_button_set_properties;
  gbwidget.gb_widget_create_popup_menu = gb_button_create_popup_menu;
  gbwidget.gb_widget_write_source = gb_button_write_source;
  gbwidget.gb_widget_destroy = gb_button_destroy;

  return &gbwidget;
}
