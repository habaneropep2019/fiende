/*
 * GNOME menu editor revision 2
 * (C)1999
 *
 * Authors: John Ellis <johne@bellatlantic.net>
 *          Nat Friedman <nat@nat.org>
 *
 */

#include "gmenu.h"

static void menu_tree_delete_node_cb(GtkCTree *ctree, GtkCTreeNode *node, gpointer data);
static void menu_tree_delete_node(GtkCTree *ctree, GtkCTreeNode *node);
static void menu_tree_delete_path(GtkCTree *ctree, gchar *path);
static void menu_tree_delete_delete_cb(gint reply, gpointer data);

/*
 *-----------------------------------------------------------------------------
 * delete item functions
 *-----------------------------------------------------------------------------
 */

static void menu_tree_delete_node_cb(GtkCTree *ctree, GtkCTreeNode *node, gpointer data)
{
	Desktop_Data *dd;
	dd = gtk_ctree_node_get_row_data(ctree, node);

	if (isdir(dd->path))
		{
		gchar *dir_file;
		gchar *order_file;

		dir_file = g_concat_dir_and_file(dd->path, ".directory");
		if (g_file_exists(dir_file))
			{
			if (unlink(dir_file) < 0)
				g_warning("Failed to delete %s: %s\n", dir_file,
					g_strerror(errno));
			}
		g_free(dir_file);

		order_file = g_concat_dir_and_file(dd->path, ".order");
		if (g_file_exists(order_file))
			{
			if (unlink(order_file) < 0)
				g_warning("Failed to delete %s: %s\n", order_file,
					g_strerror(errno));
			}
		g_free(order_file);

		if (rmdir(dd->path) < 0)
			{
			g_warning("Could not remove the directory %s: %s\n", dd->path,
				g_strerror(errno));
			}
		}
	else
		{
		if (unlink(dd->path) < 0)
				g_warning("Could not remove the menu item %s: %s\n", dd->path,
					g_strerror(errno));
		}

	gtk_ctree_remove_node(ctree, node);
}

static void menu_tree_delete_node(GtkCTree *ctree, GtkCTreeNode *node)
{
	gtk_ctree_post_recursive(ctree, node,
			GTK_CTREE_FUNC(menu_tree_delete_node_cb), NULL);
}

static void menu_tree_delete_path(GtkCTree *ctree, gchar *path)
{
	GtkCTreeNode *node;
	GtkCTreeNode *sel;
	node = menu_tree_find_path(ctree, path);
	sel = menu_tree_get_selection(ctree);
	if (node == sel || gtk_ctree_is_ancestor(ctree, sel, node))
		{
		GtkCTreeNode *new;
		if (GTK_CTREE_ROW(node)->sibling)
			new = GTK_CTREE_ROW(node)->sibling;
		else
			new = GTK_CTREE_NODE_PREV(node);
		gtk_ctree_select(ctree, new);
		}

	menu_tree_delete_node(ctree, node);
}

static void menu_tree_delete_delete_cb(gint reply, gpointer data)
{
	gchar *path = data;
	if (reply == GNOME_YES)
		{
		menu_tree_delete_path(GTK_CTREE(tree), path);
		}
	g_free(path);
}

void menu_tree_delete_item(GtkWidget *ctree)
{
	GtkCTreeNode *node;
	Desktop_Data *dd;

	node = menu_tree_get_selection(GTK_CTREE(ctree));
	if (!node) return;

	dd = gtk_ctree_node_get_row_data(GTK_CTREE(ctree), node);
	if (!dd) return;

	if (strcmp(dd->path, user_apps_dir) == 0 ||
	    strcmp(dd->path, system_apps_dir) == 0 ||
	    strcmp(dd->path, system_applets_dir) == 0 ||
	    (system_apps_merge_dir && strcmp(dd->path, system_apps_merge_dir) == 0))
		{
		gnome_warning_dialog (_("You can not delete a top level submenu."));
		return;
		}

	if (!dd->editable)
		{
		gnome_warning_dialog("You can't delete that file.\nYou do not have the proper permissions.");
		return;
		}

	if (isfile(dd->path))
                {
                gnome_question_dialog (_("Delete this menu item?"),
                        (GnomeReplyCallback) menu_tree_delete_delete_cb,
			g_strdup(dd->path));
                return;
                }

	if (isdir(dd->path))
		{
		if (!GTK_CTREE_ROW(node)->children)
			{
			gnome_question_dialog (_("Delete empty submenu?"),
				(GnomeReplyCallback) menu_tree_delete_delete_cb,
				g_strdup(dd->path));
			}
		else
			{
			gnome_question_dialog(_("Are you sure you want to delete this submenu and all its contents?"),
				(GnomeReplyCallback) menu_tree_delete_delete_cb,
				g_strdup(dd->path));
			}
		return;
		}

	gnome_question_dialog (_("Delete this menu item?"),
		(GnomeReplyCallback) menu_tree_delete_delete_cb,
		g_strdup(dd->path));
}

