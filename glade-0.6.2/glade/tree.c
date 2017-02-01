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

#include <gtk/gtkctree.h>
#include <gtk/gtkscrolledwindow.h>
#include <gtk/gtkwindow.h>

#include "gladeconfig.h"

#include "gbwidget.h"
#include "editor.h"
#include "tree.h"
#include "utils.h"

/* This key is used to store a pointer from each widget to its corresponding 
   node in the widget tree. */
#define GLADE_TREE_NODE_KEY "GLADE_TREE_NODE_KEY"


static GtkWidget *win_tree = NULL;
static GtkWidget *widget_tree = NULL;

/* private functions */

/* callback */
static void tree_on_select_node (GtkCTree     *tree,
				 GtkCTreeNode *node,
				 gint          column,
				 gpointer      dummy);
static void tree_on_unselect_node (GtkCTree     *tree,
				   GtkCTreeNode *node,
				   gint          column,
				   gpointer      dummy);
static gint tree_on_button_press (GtkCTree * ctree,
				  GdkEventButton * event,
				  gpointer widget);

/* utility */

static void tree_do_select (gpointer widget,
			    gpointer dummy);
static gboolean tree_node_is_selected (GtkCTree *ctree,
				       GtkCTreeNode *node);
static void delete_remove_child_info (GtkCTree * ctree,
				      GtkCTreeNode * node,
				      gpointer data);
static void tree_move_to_parent	(GtkWidget	*widget,
				 GtkWidget	*parent);

/* public functions */

/* This creates the widget tree window, with just the root tree. */
void
tree_init ()
{
  GtkWidget *scrolled_win;

  win_tree = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_uposition (win_tree, 510, 0);
  gtk_widget_set_name (win_tree, "GladeWidgetTree");
  gtk_signal_connect (GTK_OBJECT (win_tree), "delete_event",
		      GTK_SIGNAL_FUNC (tree_hide), NULL);
  gtk_window_set_title (GTK_WINDOW (win_tree), _ ("Widget Tree"));
  gtk_container_set_border_width (GTK_CONTAINER (win_tree), 0);
  gtk_window_set_wmclass (GTK_WINDOW (win_tree), "widget_tree", "Glade");

  widget_tree = gtk_ctree_new (1, 0);
  gtk_clist_set_row_height (GTK_CLIST (widget_tree), 21);
  gtk_clist_set_column_width (GTK_CLIST (widget_tree), 0, 200);
  gtk_clist_set_selection_mode (GTK_CLIST (widget_tree),
				GTK_SELECTION_EXTENDED);
  gtk_ctree_set_show_stub (GTK_CTREE (widget_tree), FALSE);
  gtk_ctree_set_indent (GTK_CTREE (widget_tree), 16);
  gtk_ctree_set_line_style (GTK_CTREE (widget_tree), GTK_CTREE_LINES_DOTTED);
  /* This may be useful for moving widgets around in future, but we don't
     support it now, so it will only confuse people. */
  /*gtk_clist_set_reorderable (GTK_CLIST (widget_tree), TRUE);*/

  scrolled_win = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (scrolled_win), widget_tree);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_win),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (win_tree), scrolled_win);
  gtk_widget_show (scrolled_win);

  gtk_signal_connect_after (GTK_OBJECT (widget_tree), "button_press_event",
			    GTK_SIGNAL_FUNC (tree_on_button_press), NULL);
  gtk_signal_connect_after (GTK_OBJECT (widget_tree), "tree_select_row",
			    GTK_SIGNAL_FUNC (tree_on_select_node), NULL);
  gtk_signal_connect_after (GTK_OBJECT (widget_tree), "tree_unselect_row",
			    GTK_SIGNAL_FUNC (tree_on_unselect_node), NULL);
  gtk_widget_set_usize (widget_tree, 250, 320);
  gtk_widget_show (widget_tree);
}

/* This shows the widget tree window. */
void
tree_show (GtkWidget * widget, gpointer data)
{
  if (!win_tree)
    tree_init ();
  gtk_widget_show (win_tree);
  gdk_window_show (GTK_WIDGET (win_tree)->window);
  gdk_window_raise (GTK_WIDGET (win_tree)->window);
  g_list_foreach (editor_get_selection (), tree_do_select, NULL);
}

/* This hides the widget tree window. */
gint
tree_hide (GtkWidget * widget, gpointer data)
{
  gtk_widget_hide (win_tree);
  return TRUE;
}


/* This adds a widget to the tree (if it is a widget that we are interested in,
 * i.e. a GbWidget). The widget must have already been added to the interface,
 * so we can determine its parent. It also recursively adds any children. */
void
tree_add_widget (GtkWidget * widget)
{
  GbWidget *gbwidget;
  GtkWidget *parent;
  gchar *text[1];
  GtkCTreeNode *node, *parent_node;

  /* We only add it to the tree if it is a GbWidget and it hasn't already been
     added. */
  if (GB_IS_GB_WIDGET (widget))
    {
      if (!gtk_object_get_data (GTK_OBJECT (widget), GLADE_TREE_NODE_KEY))
	{
	  gbwidget = gb_widget_lookup (widget);
	  g_return_if_fail (gbwidget != NULL);

	  parent = glade_util_get_parent (widget);
	  if (parent == NULL)
	    parent_node = NULL;
	  else
	    parent_node = (GtkCTreeNode*) gtk_object_get_data (GTK_OBJECT (parent),
							       GLADE_TREE_NODE_KEY);

	  text[0] = gtk_widget_get_name (widget);
	  node = gtk_ctree_insert_node (GTK_CTREE (widget_tree),
					parent_node, NULL, text, 5,
					gbwidget->gdkpixmap, gbwidget->mask,
					gbwidget->gdkpixmap, gbwidget->mask,
					FALSE, FALSE);
	  gtk_object_set_data (GTK_OBJECT (widget), GLADE_TREE_NODE_KEY, node);
	  gtk_ctree_node_set_row_data (GTK_CTREE (widget_tree), node, widget);
	}
      else
	{
	  /* Update the name if necessary. This is needed because we may have
	     added this widget before (if it was created by its parent
	     automatically), but we have only just loaded its name. */
	  tree_rename_widget (widget, gtk_widget_get_name (widget));
	}
    }

  /* Now add any children. */
  gb_widget_children_foreach (widget, (GtkCallback) tree_add_widget, NULL);
}


void
tree_remove_widget (GtkWidget * widget)
{
  GtkCTreeNode *node;

  node = gtk_object_get_data (GTK_OBJECT (widget), GLADE_TREE_NODE_KEY);
  if (node == NULL)
    return;

  gtk_ctree_pre_recursive (GTK_CTREE (widget_tree), GTK_CTREE_NODE (node),
			   GTK_CTREE_FUNC (delete_remove_child_info), NULL);
  gtk_ctree_remove_node (GTK_CTREE (widget_tree), GTK_CTREE_NODE (node));
}


void
tree_clear (void)
{
  gtk_clist_clear (GTK_CLIST (widget_tree));
}


void
tree_freeze (void)
{
  gtk_clist_freeze (GTK_CLIST (widget_tree));
}


void
tree_thaw (void)
{
  gtk_clist_thaw (GTK_CLIST (widget_tree));
}


void
tree_rename_widget (GtkWidget * widget, const gchar * name)
{
  GtkCTreeNode *node;
  gchar *old_name;
  guint8 spacing;
  GdkPixmap *pixmap_closed, *pixmap_open;
  GdkBitmap *mask_closed, *mask_open;
  gboolean is_leaf, expanded;

  node = (GtkCTreeNode *) gtk_object_get_data (GTK_OBJECT (widget),
					       GLADE_TREE_NODE_KEY);
  if (node == NULL)
    return;

  gtk_ctree_get_node_info (GTK_CTREE (widget_tree), node, &old_name, &spacing,
			   &pixmap_closed, &mask_closed, &pixmap_open,
			   &mask_open, &is_leaf, &expanded);
  /* Only update it if the name has really changed. */
  if (strcmp (old_name, name))
    {
      gtk_ctree_set_node_info (GTK_CTREE (widget_tree), node, name, spacing,
			       pixmap_closed, mask_closed, pixmap_open,
			       mask_open, is_leaf, expanded);
    }
}


/* This is used when inserting an alignment or event box above a widget. */
void
tree_insert_widget_parent (GtkWidget * parent, GtkWidget * widget)
{
  tree_add_widget (parent);
  tree_move_to_parent (widget, parent);
}


/* This is used when removing an alignment or event box from above a widget.
   We move the widget beneath the grandparent. */
void
tree_remove_widget_parent (GtkWidget * grandparent,
			   GtkWidget * widget)
{
  GtkCTreeNode *parent_node, *node;

  parent_node = (GtkCTreeNode *) gtk_object_get_data (GTK_OBJECT (grandparent),
						      GLADE_TREE_NODE_KEY);
  node = (GtkCTreeNode *) gtk_object_get_data (GTK_OBJECT (widget),
					       GLADE_TREE_NODE_KEY);
  gtk_ctree_move (GTK_CTREE (widget_tree), node, parent_node, NULL);
}


/* Selects a node and makes sure it is visible on the tree */
static void
select_node (GtkCTree *ctree, GtkCTreeNode *node)
{
  GtkCTreeNode *n;

  gtk_ctree_select (GTK_CTREE (widget_tree), GTK_CTREE_NODE (node));

  for (n = GTK_CTREE_ROW (node)->parent; n; n = GTK_CTREE_ROW (n)->parent)
    if (!GTK_CTREE_ROW (n)->expanded)
      gtk_ctree_expand (ctree, n);

  if (gtk_ctree_node_is_visible (ctree, node) != GTK_VISIBILITY_FULL)
    gtk_ctree_node_moveto (ctree, node, 0, 0.5, 0.0);
}


void
tree_select_widget (GtkWidget * widget, gboolean select)
{
  GtkCTreeNode *node;
  gboolean selected;

  if (!GB_IS_GB_WIDGET (widget))
    return;

  node = gtk_object_get_data (GTK_OBJECT (widget), GLADE_TREE_NODE_KEY);
  if (node == NULL)
    return;

  selected = tree_node_is_selected (GTK_CTREE (widget_tree),
				    GTK_CTREE_NODE (node));

  if (select && !selected)
    select_node (GTK_CTREE (widget_tree), GTK_CTREE_NODE (node));
  else if (!select && selected)
    gtk_ctree_unselect (GTK_CTREE (widget_tree), GTK_CTREE_NODE (node));
}


/* private functions */
/* callback */

/* This is called when an item is selected. We also select the widget in the
   interface, but only if it is not already selected. We have to be careful
   to avoid an infinite loop. */
static void
tree_on_select_node (GtkCTree     *tree,
		     GtkCTreeNode *node,
		     gint          column,
		     gpointer      dummy)
{
  GtkWidget *widget;

  widget = (GtkWidget *) gtk_ctree_node_get_row_data (tree, node);
  g_return_if_fail (GTK_IS_WIDGET (widget));

  /* Since placeholders don't appear in the tree, we make sure that whenever
     a widget is selected, we deselect all placeholders. If we don't do this
     it is a bit confusing. */
  editor_deselect_all_placeholders ();

  if (!editor_is_selected (widget))
    editor_select_widget_control (GTK_WIDGET (widget));
}


/* This is called when an item is unselected. We also deselect the widget in
   the interface, but only if it is currently selected. We have to be careful
   to avoid an infinite loop. */
static void
tree_on_unselect_node (GtkCTree     *tree,
		       GtkCTreeNode *node,
		       gint          column,
		       gpointer      dummy)
{
  GtkWidget *widget;

  widget = (GtkWidget *) gtk_ctree_node_get_row_data (tree, node);
  g_return_if_fail (GTK_IS_WIDGET (widget));

  if (editor_is_selected (widget))
    editor_select_widget_control (GTK_WIDGET (widget));
}


/* This is called when a button is pressed in the CTree. If it was the
   right mouse button we show the popup menu for the widget. */
static gint
tree_on_button_press (GtkCTree       *ctree,
		      GdkEventButton *event,
		      gpointer        data)
{
  GtkCTreeNode *node;
  GtkWidget *widget;
  gint row, col;

  /* TODO: change to own menu */
  if (event->button == 3)
    {
      gtk_clist_get_selection_info (GTK_CLIST (ctree), event->x, event->y,
				    &row, &col);
      node = GTK_CTREE_NODE (g_list_nth (GTK_CLIST (ctree)->row_list, row));
      if (node == NULL)
	return FALSE;

      widget = GTK_WIDGET (gtk_ctree_node_get_row_data (ctree, node));
      gb_widget_show_popup_menu (GTK_WIDGET (widget), event);
      return TRUE;
    }
  return FALSE;
}


/* utility */

/* This is used as a callback to simply select the widget in the CTree. */
static void
tree_do_select (gpointer widget,
		gpointer dummy)
{
  tree_select_widget (GTK_WIDGET (widget), TRUE);
}


/* This returns TRUE if the given node is currently selected in the CTree. */
static gboolean
tree_node_is_selected (GtkCTree *ctree,
		       GtkCTreeNode *node)
{
  GList *elem = GTK_CLIST (ctree)->selection;

  while (elem)
    {
      if (GTK_CTREE_NODE (elem->data) == node)
	return TRUE;
      elem = elem->next;
    }
  return FALSE;
}


/* This moves the node corresponding to the given widget beneath the node
   corresponding to the given parent. If either widget is NULL, or they don't
   have a corresponding CTree node, it simply returns. */
static void
tree_move_to_parent (GtkWidget * widget, GtkWidget * parent)
{
  GtkCTreeNode *node, *parent_node;

  if (widget == NULL || parent == NULL)
    return;

  node = (GtkCTreeNode *) gtk_object_get_data (GTK_OBJECT (widget),
					       GLADE_TREE_NODE_KEY);
  parent_node = (GtkCTreeNode *) gtk_object_get_data (GTK_OBJECT (parent),
						      GLADE_TREE_NODE_KEY);
  if (parent_node == NULL || node == NULL)
    return;

  gtk_ctree_move (GTK_CTREE (widget_tree), node, parent_node, NULL);
}


/* This is called recursively to remove the pointers to CTree nodes in child
   widgets when a widget is destroyed. It is needed because the widgets get
   destroyed from top to bottom. */
static void
delete_remove_child_info (GtkCTree     *ctree,
			  GtkCTreeNode *node,
			  gpointer      data)
{
  GtkWidget *widget;

  widget = gtk_ctree_node_get_row_data (ctree, node);
  gtk_object_remove_data (GTK_OBJECT (widget), GLADE_TREE_NODE_KEY);
}
