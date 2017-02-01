/*
 * GNOME menu editor revision 2
 * (C)1999
 *
 * Authors: John Ellis <johne@bellatlantic.net>
 *          Nat Friedman <nat@nat.org>
 *
 */

#include "gmenu.h"

static void menu_tree_sort_warning();
static void menu_tree_sort_node(GtkCTree *ctree, GtkCTreeNode *node);
static void menu_tree_sort_selected_recursive_cb(GtkCTree *ctree, GtkCTreeNode *node, gpointer data);

/*
 *-----------------------------------------------------------------------------
 * move item functions
 *-----------------------------------------------------------------------------
 */

void menu_tree_move_up(GtkWidget *ctree)
{
	GtkCTreeNode *node = menu_tree_get_selection(GTK_CTREE(ctree));
	GtkCTreeNode *parent;
	GtkCTreeNode *sibling;

	if (!node) return;
	if (!menu_tree_node_is_editable(ctree, node)) return;
	parent = GTK_CTREE_ROW(node)->parent;
	if (!parent || GTK_CTREE_ROW(parent)->children == node) return;

	sibling = GTK_CTREE_ROW(parent)->children;
	while (sibling && GTK_CTREE_ROW(sibling)->sibling != node)
		{
		sibling = GTK_CTREE_ROW(sibling)->sibling;
		}

	gtk_ctree_move(GTK_CTREE(ctree), node, parent, sibling);

	save_order_of_dir(GTK_CTREE(ctree), node, FALSE);
}

void menu_tree_move_down(GtkWidget *ctree)
{
	GtkCTreeNode *node = menu_tree_get_selection(GTK_CTREE(ctree));
	GtkCTreeNode *sibling;

	if (!node) return;
	if (!menu_tree_node_is_editable(ctree, node)) return;
	if (!GTK_CTREE_ROW(node)->parent || !GTK_CTREE_ROW(node)->sibling) return;

	sibling = GTK_CTREE_ROW(node)->sibling;
	gtk_ctree_move(GTK_CTREE(ctree), node,
		       GTK_CTREE_ROW(node)->parent, GTK_CTREE_ROW(sibling)->sibling);

	save_order_of_dir(GTK_CTREE(ctree), node, FALSE);
}

/*
 *-----------------------------------------------------------------------------
 * sort item functions
 *-----------------------------------------------------------------------------
 */

static void menu_tree_sort_warning(void)
{
	gnome_warning_dialog("You do not have the proper permissions to be able to modify this menu item.");
}

static void menu_tree_sort_node(GtkCTree *ctree, GtkCTreeNode *node)
{
	Desktop_Data *dd;

	if (!node) return;

	dd = gtk_ctree_node_get_row_data(ctree, node);
        if (!dd->isfolder) node = GTK_CTREE_ROW(node)->parent;

	gtk_ctree_sort_node(ctree, node);
	save_order_of_dir(ctree, node, TRUE);
}

void menu_tree_sort_selected(GtkWidget *ctree)
{
	GtkCTreeNode *node;

	node = menu_tree_get_selection(GTK_CTREE(ctree));
	if (!menu_tree_node_is_editable(ctree, node) ||
	    !menu_tree_node_is_editable(ctree, GTK_CTREE_ROW(node)->parent) )
		{
		menu_tree_sort_warning();
		return;
		}

	menu_tree_sort_node(GTK_CTREE(ctree), node);
}

static void menu_tree_sort_selected_recursive_cb(GtkCTree *ctree, GtkCTreeNode *node, gpointer data)
{
	Desktop_Data *dd;

	if (!node) return;

	dd = gtk_ctree_node_get_row_data(ctree, node);

        if (dd->isfolder) menu_tree_sort_node(ctree, node);
}

void menu_tree_sort_selected_recursive(GtkWidget *ctree)
{
	GtkCTreeNode *node;
	Desktop_Data *dd;

	node = menu_tree_get_selection(GTK_CTREE(ctree));
	if (!node) return;
	if (!menu_tree_node_is_editable(ctree, node) ||
	    !menu_tree_node_is_editable(ctree, GTK_CTREE_ROW(node)->parent) )
		{
		menu_tree_sort_warning();
		return;
		}

	dd = gtk_ctree_node_get_row_data(GTK_CTREE(ctree),node);
        if (!dd->isfolder) node = GTK_CTREE_ROW(node)->parent;

	gtk_ctree_post_recursive(GTK_CTREE(ctree), node,
				 menu_tree_sort_selected_recursive_cb, NULL);
}

