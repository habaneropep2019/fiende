
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

#include <gtk/gtkhbox.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtknotebook.h>
#include <gtk/gtkspinbutton.h>
#include "../gb.h"
#include "../tree.h"

/* Include the 21x21 icon pixmap for this widget, to be used in the palette */
#include "../graphics/notebook.xpm"

/*
 * This is the GbWidget struct for this widget (see ../gbwidget.h).
 * It is initialized in the init() function at the end of this file
 */
static GbWidget gbwidget;

/* These are the special children of the widget. */
static gchar *NotebookTab = "Notebook:tab";

static gchar *ShowTabs = "GtkNotebook::show_tabs";
static gchar *ShowBorder = "GtkNotebook::show_border";
static gchar *TabPos = "GtkNotebook::tab_pos";
static gchar *Scrollable = "GtkNotebook::scrollable";
static gchar *TabHBorder = "GtkNotebook::tab_hborder";
static gchar *TabVBorder = "GtkNotebook::tab_vborder";
static gchar *Popups = "GtkNotebook::popup_enable";
static gchar *NumPages = "GtkNotebook::num_pages";

static const gchar *GbTabPosChoices[] =
{"Left", "Right", "Top", "Bottom", NULL};
static const gint GbTabPosValues[] =
{
  GTK_POS_LEFT,
  GTK_POS_RIGHT,
  GTK_POS_TOP,
  GTK_POS_BOTTOM
};
static const gchar *GbTabPosSymbols[] =
{
  "GTK_POS_LEFT",
  "GTK_POS_RIGHT",
  "GTK_POS_TOP",
  "GTK_POS_BOTTOM"
};


static void show_notebook_dialog (GbWidgetNewData * data);
static void on_notebook_dialog_ok (GtkWidget * widget,
				   GbWidgetNewData * data);
static void on_notebook_dialog_destroy (GtkWidget * widget,
					GbWidgetNewData * data);
static GtkWidget *gb_notebook_new_tab_label ();
static void gb_notebook_next_page (GtkWidget * menuitem, GtkNotebook * notebook);
static void gb_notebook_prev_page (GtkWidget * menuitem, GtkNotebook * notebook);
static void gb_notebook_update_num_pages (GtkNotebook *notebook);

/******
 * NOTE: To use these functions you need to uncomment them AND add a pointer
 * to the function in the GbWidget struct at the end of this file.
 ******/

/*
 * Creates a new GtkWidget of class GtkNotebook, performing any specialized
 * initialization needed for the widget to work correctly in this environment.
 * If a dialog box is used to initialize the widget, return NULL from this
 * function, and call data->callback with your new widget when it is done.
 * If the widget needs a special destroy handler, add a signal here.
 */
GtkWidget *
gb_notebook_new (GbWidgetNewData * data)
{
  GtkWidget *new_widget;

  if (data->action == GB_LOADING)
    {
      new_widget = gtk_notebook_new ();
      return new_widget;
    }
  else
    {
      show_notebook_dialog (data);
      return NULL;
    }
}


void
gb_notebook_add_child (GtkWidget *widget, GtkWidget *child,
		       GbWidgetSetArgData *data)
{
  /* See if this is a tab widget. */
  gchar *child_name = load_get_value (data, "child_name");

  if (child_name && !strcmp (child_name, NotebookTab))
    {
      /* We store the last tab read in 'last_child' */
      GList *elem;
      GtkWidget *notebook_page;
      gint pos = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (widget),
                                                       "last_child"));

      MSG1 ("Adding notebook tab: %i", pos);
      /* SPECIAL CODE to replace the notebooks default tab label with the
         loaded widget. We remove the page and add it with the new tab,
         just like in gb_widget_replace_child(). */

      elem = g_list_nth (GTK_NOTEBOOK (widget)->children, pos);
      if (elem)
	{
	  notebook_page = ((GtkNotebookPage*) elem->data)->child;
#if 1
	  gtk_notebook_set_tab_label (GTK_NOTEBOOK (widget), notebook_page,
				      child);
#else
	  gtk_widget_ref (notebook_page);
	  gtk_notebook_remove_page (GTK_NOTEBOOK (widget), pos);
	  gtk_notebook_insert_page (GTK_NOTEBOOK (widget), notebook_page,
				    child, pos);
	  gtk_widget_unref (notebook_page);
#endif
	}
      else
	{
	  g_warning ("Notebook tab found for non-existent page");
	  gtk_notebook_append_page (GTK_NOTEBOOK (widget),
				    editor_new_placeholder (), child);
	}
      gtk_object_set_data (GTK_OBJECT (widget), "last_child",
			   GINT_TO_POINTER (pos + 1));
    }
  else
    {
      /* We create a label, in case it does not appear in the XML file,
         or we are pasting into the notebook. */
      GtkWidget *label = gb_widget_new_full ("GtkLabel", FALSE, widget,
                                             NULL, 0, 0, NULL, GB_CREATING,
                                             NULL);
      g_return_if_fail (label != NULL);
      gb_widget_set_child_name (label, NotebookTab);

      gtk_notebook_append_page (GTK_NOTEBOOK (widget), child, label);
    }
}


static void
show_notebook_dialog (GbWidgetNewData * data)
{
  GtkWidget *dialog, *vbox, *hbox, *label, *spinbutton;
  GtkObject *adjustment;

  dialog = glade_util_create_dialog (_("New notebook"), data->parent,
				     on_notebook_dialog_ok, data, &vbox);
  gtk_signal_connect (GTK_OBJECT (dialog), "destroy",
		      GTK_SIGNAL_FUNC (on_notebook_dialog_destroy), data);

  hbox = gtk_hbox_new (FALSE, 5);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 5);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 10);
  gtk_widget_show (hbox);

  label = gtk_label_new (_("Number of pages:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 5);
  gtk_widget_show (label);

  adjustment = gtk_adjustment_new (3, 1, 100, 1, 10, 10);
  spinbutton = glade_util_spin_button_new (GTK_OBJECT (dialog), "pages",
					   GTK_ADJUSTMENT (adjustment), 1, 0);
  gtk_box_pack_start (GTK_BOX (hbox), spinbutton, TRUE, TRUE, 5);
  gtk_widget_set_usize (spinbutton, 50, -1);
  gtk_widget_grab_focus (spinbutton);
  gtk_widget_show (spinbutton);

  gtk_widget_show (dialog);
  gtk_grab_add (dialog);
}


static void
on_notebook_dialog_ok (GtkWidget * widget, GbWidgetNewData * data)
{
  GtkWidget *new_widget, *spinbutton, *window;
  gint pages, i;

  window = gtk_widget_get_toplevel (widget);

  /* Only call callback if placeholder/fixed widget is still there */
  if (gb_widget_can_finish_new (data))
    {
      spinbutton = gtk_object_get_data (GTK_OBJECT (window), "pages");
      g_return_if_fail (spinbutton != NULL);
      pages = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (spinbutton));

      new_widget = gtk_notebook_new ();
      for (i = 0; i < pages; i++)
	{
	  gtk_notebook_append_page (GTK_NOTEBOOK (new_widget),
				    editor_new_placeholder (),
				    gb_notebook_new_tab_label ());
	}
      gb_widget_initialize (new_widget, data);
      (*data->callback) (new_widget, data);
    }
  gtk_widget_destroy (window);
}


static void
on_notebook_dialog_destroy (GtkWidget * widget,
			    GbWidgetNewData * data)
{
  gb_widget_free_new_data (data);
  gtk_grab_remove (widget);
}


GtkWidget *
gb_notebook_new_tab_label ()
{
  GtkWidget *label;

  label = gb_widget_new ("GtkLabel", NULL);
  g_return_val_if_fail (label != NULL, NULL);
  gb_widget_set_child_name (label, NotebookTab);
  return label;
}


/*
 * Creates the components needed to edit the extra properties of this widget.
 */
static void
gb_notebook_create_properties (GtkWidget * widget, GbWidgetCreateArgData * data)
{
  property_add_bool (ShowTabs, _("Show Tabs:"), _("If the notebook tabs are shown"));
  property_add_bool (ShowBorder, _("Show Border:"),
		     _("If the notebook border is shown, when the tabs are not shown"));
  property_add_choice (TabPos, _("Tab Pos:"),
		       _("The position of the notebook tabs"),
		       GbTabPosChoices);
  property_add_bool (Scrollable, _("Scrollable:"),
		     _("If the notebook tabs are scrollable"));
  property_add_int_range (TabHBorder, _("Tab Horz. Border:"),
			  _("The size of the notebook tabs' horizontal border"),
			  0, 1000, 1, 10, 1);
  property_add_int_range (TabVBorder, _("Tab Vert. Border:"),
			  _("The size of the notebook tabs' vertical border"),
			  0, 1000, 1, 10, 1);
  property_add_bool (Popups, _("Show Popup:"), _("If the popup menu is enabled"));
  property_add_int_range (NumPages, _("Number of Pages:"),
		          _("The number of notebook pages"),
		          1, 100, 1, 10, 1);
}



/*
 * Gets the properties of the widget. This is used for both displaying the
 * properties in the property editor, and also for saving the properties.
 */
static void
gb_notebook_get_properties (GtkWidget * widget, GbWidgetGetArgData * data)
{
  gint tab_pos, i;

  gb_widget_output_bool (data, ShowTabs, GTK_NOTEBOOK (widget)->show_tabs);
  gb_widget_output_bool (data, ShowBorder, GTK_NOTEBOOK (widget)->show_border);

  tab_pos = GTK_NOTEBOOK (widget)->tab_pos;
  for (i = 0; i < sizeof (GbTabPosValues) / sizeof (GbTabPosValues[0]); i++)
    {
      if (GbTabPosValues[i] == tab_pos)
	gb_widget_output_choice (data, TabPos, i, GbTabPosSymbols[i]);
    }

  gb_widget_output_bool (data, Scrollable, GTK_NOTEBOOK (widget)->scrollable);
  gb_widget_output_int (data, TabHBorder, GTK_NOTEBOOK (widget)->tab_hborder);
  gb_widget_output_int (data, TabVBorder, GTK_NOTEBOOK (widget)->tab_vborder);
  gb_widget_output_bool (data, Popups,
			 (GTK_NOTEBOOK (widget)->menu) ? TRUE : FALSE);

  /* Don't save the number of pages. */
  if (data->action != GB_SAVING)
    gb_widget_output_int (data, NumPages,
			  g_list_length (GTK_NOTEBOOK (widget)->children));
}



/*
 * Sets the properties of the widget. This is used for both applying the
 * properties changed in the property editor, and also for loading.
 */
static void
gb_notebook_set_properties (GtkWidget * widget, GbWidgetSetArgData * data)
{
  gboolean show_tabs, show_border, scrollable, popups;
  gchar *tab_pos;
  gint tab_border, i, num_pages;
  GtkWidget *new_label;

  show_tabs = gb_widget_input_bool (data, ShowTabs);
  if (data->apply)
    gtk_notebook_set_show_tabs (GTK_NOTEBOOK (widget), show_tabs);

  show_border = gb_widget_input_bool (data, ShowBorder);
  if (data->apply)
    gtk_notebook_set_show_border (GTK_NOTEBOOK (widget), show_border);

  tab_pos = gb_widget_input_choice (data, TabPos);
  if (data->apply)
    {
      for (i = 0; i < sizeof (GbTabPosValues) / sizeof (GbTabPosValues[0]); i
	   ++)
	{
	  if (!strcmp (tab_pos, GbTabPosChoices[i])
	      || !strcmp (tab_pos, GbTabPosSymbols[i]))
	    {
	      gtk_notebook_set_tab_pos (GTK_NOTEBOOK (widget), GbTabPosValues
					[i]);
	      break;
	    }
	}
    }

  scrollable = gb_widget_input_bool (data, Scrollable);
  if (data->apply)
    gtk_notebook_set_scrollable (GTK_NOTEBOOK (widget), scrollable);

  tab_border = gb_widget_input_int (data, TabHBorder);
  if (data->apply)
    gtk_notebook_set_tab_hborder (GTK_NOTEBOOK (widget), tab_border);

  tab_border = gb_widget_input_int (data, TabVBorder);
  if (data->apply)
    gtk_notebook_set_tab_vborder (GTK_NOTEBOOK (widget), tab_border);

  popups = gb_widget_input_bool (data, Popups);
  if (data->apply)
    {
      if (popups)
	gtk_notebook_popup_enable (GTK_NOTEBOOK (widget));
      else
	gtk_notebook_popup_disable (GTK_NOTEBOOK (widget));
    }
  
  /* Don't adjust the size of a notebook if loading a project 
   * as it is handled by other routines. */
  if (data->action != GB_LOADING)
    {
      num_pages = gb_widget_input_int (data, NumPages);
      if (data->apply)
	{
	  if (num_pages != g_list_length (GTK_NOTEBOOK (widget)->children))
	    {
	      if (num_pages > g_list_length (GTK_NOTEBOOK (widget)->children))
		{
		  while (num_pages > g_list_length (GTK_NOTEBOOK (widget)->children))
		    {
		      new_label = gb_notebook_new_tab_label ();
		      gtk_notebook_append_page (GTK_NOTEBOOK (widget),
						editor_new_placeholder (),
						new_label);
		      tree_add_widget (new_label);
		    }
		}
	      else
		{
		  while (num_pages < g_list_length (GTK_NOTEBOOK (widget)->children))
		    {
		      gtk_notebook_remove_page (GTK_NOTEBOOK (widget),
						num_pages);
		    }
		}
	    }
	}
    }
}



static void
gb_notebook_next_page (GtkWidget * menuitem, GtkNotebook * notebook)
{
  gtk_notebook_next_page (notebook);
}

static void
gb_notebook_prev_page (GtkWidget * menuitem, GtkNotebook * notebook)
{
  gtk_notebook_prev_page (notebook);
}

static void
gb_notebook_switch_prev (GtkWidget *menuitem, GtkNotebook *notebook)
{
  GList           *listnode;
  GtkWidget       *child;
  GtkNotebookPage *notepage;
  gint             current_page;

  current_page = gtk_notebook_get_current_page (notebook);

  listnode = g_list_nth (notebook->children, current_page);

  notepage = (GtkNotebookPage *) listnode->data;

  child = (GtkWidget *) notepage->child;
  if (child)
    {
      gtk_notebook_reorder_child (notebook, child, current_page - 1);
    }
}

static void
gb_notebook_switch_next (GtkWidget *menuitem, GtkNotebook *notebook)
{
  GList           *listnode;
  GtkWidget       *child;
  GtkNotebookPage *notepage;
  gint             current_page;

  current_page = gtk_notebook_get_current_page (notebook);

  listnode = g_list_nth (notebook->children, current_page);

  notepage = (GtkNotebookPage *) listnode->data;

  child = (GtkWidget *) notepage->child;
  if (child)
    {
      gtk_notebook_reorder_child (notebook, child, current_page + 1);
    }
}

static void
gb_notebook_insert_next (GtkWidget *menuitem, GtkNotebook *notebook)
{
  gint current_page;
  GtkWidget *new_label;

  current_page = gtk_notebook_get_current_page (notebook);

  new_label = gb_notebook_new_tab_label ();
  gtk_notebook_insert_page (notebook, editor_new_placeholder (),
			    new_label, current_page + 1);
  tree_add_widget (new_label);
  gb_notebook_update_num_pages (notebook);
}

static void
gb_notebook_insert_prev (GtkWidget *menuitem, GtkNotebook *notebook)
{
  gint current_page;
  GtkWidget *new_label;
	
  current_page = gtk_notebook_get_current_page (notebook);

  new_label = gb_notebook_new_tab_label ();
  gtk_notebook_insert_page (notebook, editor_new_placeholder (),
			    new_label, current_page);
  tree_add_widget (new_label);
  gb_notebook_update_num_pages (notebook);
}

static void
gb_notebook_delete_page (GtkWidget *menuitem, GtkNotebook *notebook)
{
  gtk_notebook_remove_page (notebook,
			    gtk_notebook_get_current_page (notebook));
  gb_notebook_update_num_pages (notebook);
}

 
/* This updates the number of pages property, if the notebook's properties are
   currently shown. */
static void
gb_notebook_update_num_pages (GtkNotebook *notebook)
{
  if (property_get_widget () == GTK_WIDGET (notebook))
    {
      property_set_auto_apply (FALSE);
      property_set_int (NumPages, g_list_length (notebook->children));
      property_set_auto_apply (TRUE);
    }
}

/*
 * Adds menu items to a context menu which is just about to appear!
 * Add commands to aid in editing a GtkNotebook, with signals pointing to
 * other functions in this file.
 */
static void
gb_notebook_create_popup_menu (GtkWidget * widget, GbWidgetCreateMenuData * data)
{
  GtkWidget *menuitem;
  gint current_page, num_pages;

  current_page = gtk_notebook_get_current_page (GTK_NOTEBOOK (widget));
  num_pages = g_list_length (GTK_NOTEBOOK (widget)->children);

  menuitem = gtk_menu_item_new_with_label (_("Previous Page"));
  gtk_widget_show (menuitem);
  if (current_page == 0)
    gtk_widget_set_sensitive (menuitem, FALSE);
  gtk_menu_append (GTK_MENU (data->menu), menuitem);
  gtk_signal_connect (GTK_OBJECT (menuitem), "activate",
	    GTK_SIGNAL_FUNC (gb_notebook_prev_page), GTK_NOTEBOOK (widget));

  menuitem = gtk_menu_item_new_with_label (_("Next Page"));
  gtk_widget_show (menuitem);
  if (current_page == num_pages - 1)
    gtk_widget_set_sensitive (menuitem, FALSE);
  gtk_menu_append (GTK_MENU (data->menu), menuitem);
  gtk_signal_connect (GTK_OBJECT (menuitem), "activate",
	    GTK_SIGNAL_FUNC (gb_notebook_next_page), GTK_NOTEBOOK (widget));
  
  menuitem = gtk_menu_item_new_with_label (_("Delete Page"));
  gtk_widget_show (menuitem);
  gtk_menu_append (GTK_MENU (data->menu), menuitem);
  gtk_signal_connect (GTK_OBJECT (menuitem), "activate",
	    GTK_SIGNAL_FUNC (gb_notebook_delete_page), GTK_NOTEBOOK (widget));
  
  menuitem = gtk_menu_item_new_with_label (_("Switch Next"));
  gtk_widget_show (menuitem);
  if (current_page == num_pages - 1)
    gtk_widget_set_sensitive (menuitem, FALSE);
  gtk_menu_append (GTK_MENU (data->menu), menuitem);
  gtk_signal_connect (GTK_OBJECT (menuitem), "activate",
	    GTK_SIGNAL_FUNC (gb_notebook_switch_next), GTK_NOTEBOOK (widget));
  
  menuitem = gtk_menu_item_new_with_label (_("Switch Previous"));
  gtk_widget_show (menuitem);
  if (current_page == 0)
    gtk_widget_set_sensitive (menuitem, FALSE);
  gtk_menu_append (GTK_MENU (data->menu), menuitem);
  gtk_signal_connect (GTK_OBJECT (menuitem), "activate",
	    GTK_SIGNAL_FUNC (gb_notebook_switch_prev), GTK_NOTEBOOK (widget));
  
  menuitem = gtk_menu_item_new_with_label (_("Insert Page After"));
  gtk_widget_show (menuitem);
  /*  if (current_page == num_pages - 1)
      gtk_widget_set_sensitive (menuitem, FALSE);*/
  gtk_menu_append (GTK_MENU (data->menu), menuitem);
  gtk_signal_connect (GTK_OBJECT (menuitem), "activate",
	    GTK_SIGNAL_FUNC (gb_notebook_insert_next), GTK_NOTEBOOK (widget));
  
  menuitem = gtk_menu_item_new_with_label (_("Insert Page Before"));
  gtk_widget_show (menuitem);
  /*if (current_page == 0)
    gtk_widget_set_sensitive (menuitem, FALSE);*/
  gtk_menu_append (GTK_MENU (data->menu), menuitem);
  gtk_signal_connect (GTK_OBJECT (menuitem), "activate",
	    GTK_SIGNAL_FUNC (gb_notebook_insert_prev), GTK_NOTEBOOK (widget));
}



/*
 * Writes the source code needed to create this widget.
 * You have to output everything necessary to create the widget here, though
 * there are some convenience functions to help.
 */
static void
gb_notebook_write_source (GtkWidget * widget, GbWidgetWriteSourceData * data)
{
  gint i;

  if (data->create_widget)
    {
      source_add (data, "  %s = gtk_notebook_new ();\n", data->wname);
    }

  /* We reset the last_child index, so as the tab widgets are written out
     they will start at page 0. */
  gtk_object_set_data (GTK_OBJECT (widget), "last_child",
		       GINT_TO_POINTER (-1));

  gb_widget_write_standard_source (widget, data);

  if (!GTK_NOTEBOOK (widget)->show_tabs)
    source_add (data,
		"  gtk_notebook_set_show_tabs (GTK_NOTEBOOK (%s), FALSE);\n",
		data->wname);
  if (!GTK_NOTEBOOK (widget)->show_border)
    source_add (data,
		"  gtk_notebook_set_show_border (GTK_NOTEBOOK (%s), FALSE);\n",
		data->wname);
  if (GTK_NOTEBOOK (widget)->tab_pos != GTK_POS_TOP)
    {
      for (i = 0; i < sizeof (GbTabPosValues) / sizeof (GbTabPosValues[0]); i
	   ++)
	{
	  if (GbTabPosValues[i] == GTK_NOTEBOOK (widget)->tab_pos)
	    source_add (data,
		    "  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (%s), %s);\n",
			data->wname, GbTabPosSymbols[i]);
	}
    }
  if (GTK_NOTEBOOK (widget)->scrollable)
    source_add (data,
		"  gtk_notebook_set_scrollable (GTK_NOTEBOOK (%s), TRUE);\n",
		data->wname);

  if (GTK_NOTEBOOK (widget)->tab_hborder != 2)
    source_add (data,
		"  gtk_notebook_set_tab_hborder (GTK_NOTEBOOK (%s), %i);\n",
		data->wname, GTK_NOTEBOOK (widget)->tab_hborder);
  if (GTK_NOTEBOOK (widget)->tab_vborder != 2)
    source_add (data,
		"  gtk_notebook_set_tab_vborder (GTK_NOTEBOOK (%s), %i);\n",
		data->wname, GTK_NOTEBOOK (widget)->tab_vborder);
  if (GTK_NOTEBOOK (widget)->menu)
    source_add (data,
		"  gtk_notebook_popup_enable (GTK_NOTEBOOK (%s));\n",
		data->wname);
}


/* Outputs source to add a child widget to a GtkNotebook. */
static void
gb_notebook_write_add_child_source (GtkWidget * parent,
				    const gchar *parent_name,
				    GtkWidget *child,
				    GbWidgetWriteSourceData * data)
{
  gchar *child_name;

  child_name = gb_widget_get_child_name (child);

  /* See if this is a notebook tab widget. */
  if (child_name && !strcmp (child_name, NotebookTab))
    {
      /* We store the last tab written in 'last_child' */
      gint col = GPOINTER_TO_INT (gtk_object_get_data (GTK_OBJECT (parent),
						       "last_child"));
      /* We use the special function to add the tab to the notebook. */
      source_add (data,
		  "  gtk_notebook_set_tab_label (GTK_NOTEBOOK (%s), gtk_notebook_get_nth_page (GTK_NOTEBOOK (%s), %i), %s);\n",
		  parent_name, parent_name, col + 1, data->wname);

      gtk_object_set_data (GTK_OBJECT (parent), "last_child",
			   GINT_TO_POINTER (col + 1));
    }
  else
    {
      source_add (data, "  gtk_container_add (GTK_CONTAINER (%s), %s);\n",
		  parent_name, data->wname);
    }
}


/*
 * Initializes the GbWidget structure.
 * I've placed this at the end of the file so we don't have to include
 * declarations of all the functions.
 */
GbWidget *
gb_notebook_init ()
{
  /* Initialise the GTK type */
  gtk_notebook_get_type ();

  /* Initialize the GbWidget structure */
  gb_widget_init_struct (&gbwidget);

  /* Fill in the pixmap struct & tooltip */
  gbwidget.pixmap_struct = notebook_xpm;
  gbwidget.tooltip = _("Notebook");

  /* Fill in any functions that this GbWidget has */
  gbwidget.gb_widget_new = gb_notebook_new;
  gbwidget.gb_widget_add_child = gb_notebook_add_child;

  gbwidget.gb_widget_create_properties = gb_notebook_create_properties;
  gbwidget.gb_widget_get_properties = gb_notebook_get_properties;
  gbwidget.gb_widget_set_properties = gb_notebook_set_properties;
  gbwidget.gb_widget_write_source = gb_notebook_write_source;
  gbwidget.gb_widget_write_add_child_source = gb_notebook_write_add_child_source;
  gbwidget.gb_widget_create_popup_menu = gb_notebook_create_popup_menu;

  return &gbwidget;
}
