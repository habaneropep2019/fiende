/*
 * GNOME menu editor revision 2
 * (C)1999
 *
 * Authors: John Ellis <johne@bellatlantic.net>
 *          Nat Friedman <nat@nat.org>
 *
 */

#include "gmenu.h"

static void menu_tree_drag_begin(GtkWidget *widget, GdkDragContext *context, gpointer data);
static void menu_tree_set_dnd_data_cb(GtkWidget *widget, GdkDragContext *context,
				      GtkSelectionData *selection_data, guint info,
				      guint time, gpointer data);

static void menu_tree_moved_cb(GtkCTree *ctree, GtkCTreeNode *node, GtkCTreeNode *new_parent,
			       GtkCTreeNode *new_sibling, gpointer data);
static gboolean menu_tree_move_test_cb(GtkCTree *ctree, GtkCTreeNode *source_node,
				       GtkCTreeNode *new_parent, GtkCTreeNode *new_sibling);



/*
 *-----------------------------------------------------------------------------
 * tree drag and drop outside app functions
 *-----------------------------------------------------------------------------
 */

enum {
        TARGET_URI_LIST,
        TARGET_TEXT_PLAIN
};

static GtkTargetEntry tree_drag_types[] = {
        { "text/uri-list", 0, TARGET_URI_LIST },
        { "text/plain", 0, TARGET_TEXT_PLAIN }
};
static gint n_tree_drag_types = 2;

static void menu_tree_drag_begin(GtkWidget *widget, GdkDragContext *context, gpointer data)
{
	GtkCTreeNode *node;
	gpointer key;

	key = gtk_object_get_data(GTK_OBJECT(widget), "gmenu_drag_start");
	if (!key) return;

	node = gtk_ctree_node_nth (GTK_CTREE(widget), GPOINTER_TO_INT(key));
	if (!node) return;

	gtk_ctree_select(GTK_CTREE(widget), node);
}

static void menu_tree_set_dnd_data_cb(GtkWidget *widget, GdkDragContext *context,
				      GtkSelectionData *selection_data, guint info,
				      guint time, gpointer data)
{
	GtkCTreeNode *node;
	Desktop_Data *dd;

	node = menu_tree_get_selection(GTK_CTREE(widget));
	dd = gtk_ctree_node_get_row_data(GTK_CTREE(widget), node);

	if (node && dd && dd->path)
		{
		gchar *text = NULL;
		switch (info)
			{
			case TARGET_URI_LIST:
				text = g_strconcat("file:", dd->path, "\r\n", NULL);
				break;
			case TARGET_TEXT_PLAIN:
				text = g_strdup(dd->path);
				break;
			}
		gtk_selection_data_set (selection_data, selection_data->target,
					8, (guchar *)text, strlen(text));
		g_free(text);
		}
	else
		{
		gtk_selection_data_set (selection_data, selection_data->target,
					8, NULL, 0);
		}
}

/*
 *-----------------------------------------------------------------------------
 * tree drag and drop move functions
 *-----------------------------------------------------------------------------
 */

static void menu_tree_moved_cb(GtkCTree *ctree, GtkCTreeNode *node, GtkCTreeNode *new_parent,
			       GtkCTreeNode *new_sibling, gpointer data)
{
	if (data)
		{
		/*
		 * this happens before the move, we need this to get the original parent,
		 * because we can't save or move anything until the node moves
		 */
		gtk_object_set_user_data(GTK_OBJECT(ctree), GTK_CTREE_ROW(node)->parent);
		}
	else
		{
		/*
		 * this happens after the move
		 */
		GtkCTreeNode *old_parent;
		Desktop_Data *dd_node, *dd_parent;
		gchar *new_path, *old_path;

		old_parent = gtk_object_get_user_data(GTK_OBJECT(ctree));

		if (old_parent == new_parent)
			{
			/* nothing to move, only reordered */
			save_order_of_dir(ctree, new_parent, TRUE);
			return;
			}

		dd_node = gtk_ctree_node_get_row_data(ctree, node);
		dd_parent = gtk_ctree_node_get_row_data(ctree, GTK_CTREE_ROW(node)->parent);

		old_path = dd_node->path;
		new_path = g_strconcat(dd_parent->path, "/", old_path + g_filename_index(old_path), NULL);

		if (rename(old_path, new_path) < 0)
			{
			/* uh oh! */
			g_print("Failed to move file: %s\n", old_path);
			return;
			}

		g_free(new_path);
		menu_tree_update_paths(GTK_WIDGET(ctree), node);

		save_order_of_dir(ctree, old_parent, TRUE);
		save_order_of_dir(ctree, node, FALSE);
		}
}

static gboolean menu_tree_move_test_cb(GtkCTree *ctree, GtkCTreeNode *source_node,
				       GtkCTreeNode *new_parent, GtkCTreeNode *new_sibling)
{
	Desktop_Data *dd;
	Desktop_Data *dd_parent;

	dd = gtk_ctree_node_get_row_data(ctree, source_node);
	dd_parent = gtk_ctree_node_get_row_data(ctree, new_parent);

	if (!dd || ! dd_parent) return FALSE;

	if (!dd->editable || !dd_parent->editable) return FALSE;

	if (strcmp(dd->path, user_apps_dir) == 0 ||
	    strcmp(dd->path, system_apps_dir) == 0 ||
	    (system_apps_merge_dir && strcmp(dd->path, system_apps_merge_dir) == 0) ||
	    strcmp(dd->path, system_applets_dir) == 0 ) return FALSE;

	if (new_parent != GTK_CTREE_ROW(source_node)->parent)
		{
		GtkCTreeNode *work;
		gint index;

		index = g_filename_index(dd->path);
		work = GTK_CTREE_ROW(new_parent)->children;
                while (work)
			{
			Desktop_Data *dd_work;
			dd_work = gtk_ctree_node_get_row_data(ctree, work);
			if (strcmp(dd->path + index, dd_work->path + g_filename_index(dd_work->path)) == 0)
				return FALSE;
			work = GTK_CTREE_ROW(work)->sibling;
			}
		}

	return TRUE;
}

void menu_tree_init_dnd(GtkWidget *ctree)
{
	gtk_clist_set_reorderable(GTK_CLIST(ctree), TRUE);
	gtk_ctree_set_drag_compare_func(GTK_CTREE(ctree), menu_tree_move_test_cb);

	gtk_signal_connect(GTK_OBJECT(ctree),
			"tree_move", GTK_SIGNAL_FUNC(menu_tree_moved_cb), "before");
	gtk_signal_connect_after(GTK_OBJECT(ctree),
			"tree_move", GTK_SIGNAL_FUNC(menu_tree_moved_cb), NULL);

	gtk_drag_source_set(ctree, GDK_BUTTON2_MASK,
			tree_drag_types, n_tree_drag_types, GDK_ACTION_COPY);
	gtk_signal_connect(GTK_OBJECT(ctree), "drag_data_get",
			menu_tree_set_dnd_data_cb, NULL);
	gtk_signal_connect(GTK_OBJECT(ctree), "drag_begin",
			menu_tree_drag_begin, NULL);
}
