/*
 * GNOME menu editor revision 2
 * (C)1999
 *
 * Authors: John Ellis <johne@bellatlantic.net>
 *          Nat Friedman <nat@nat.org>
 *
 */

#include "gmenu.h"

/*
 *-----------------------------------------------------------------------------
 * .order file read and write functions
 *-----------------------------------------------------------------------------
 */

GList *get_order_of_dir(const gchar *dir)
{
	gchar buf[PATH_MAX+1];
	GList *list = NULL;
	gchar *order_file;
	FILE *f;

	g_return_val_if_fail (dir != NULL, NULL);

	order_file = g_concat_dir_and_file (dir, ".order");

	f = fopen (order_file, "r");
	g_free (order_file);

	if (f == NULL) {
		return NULL;
	}

	while (fgets(buf, sizeof(buf), f) != NULL) {
		char *buf_ptr;
		buf_ptr = strchr (buf,'\n');
		if (buf_ptr)
			*buf_ptr = '\0';
		if (buf[0] == '\0' > 0)
			list = g_list_prepend (list, g_strdup (buf));
	}

	fclose(f);

	return g_list_reverse (list);
}

void save_order_of_dir(GtkCTree *ctree, GtkCTreeNode *node, gboolean is_parent)
{
	GtkCTreeNode *parent;
	GtkCTreeNode *work;
	gchar *order_file;
	Desktop_Data *dd;
	FILE *f;

	if (node == NULL)
		return;

	if (is_parent) {
		dd = gtk_ctree_node_get_row_data(ctree, node);
		if (dd->isfolder)
			parent = node;
		else
			parent = GTK_CTREE_ROW(node)->parent;
	} else {
		parent = GTK_CTREE_ROW(node)->parent;
	}

	if (parent == NULL)
		return;

	dd = gtk_ctree_node_get_row_data(ctree, parent);
        order_file = g_concat_dir_and_file(dd->path, ".order");

	work = GTK_CTREE_ROW(parent)->children;
	if (work) {
		f = fopen(order_file, "w");
		if (f == NULL) {
			g_print(_("Unable to create file: %s\n"), order_file);
                        g_free(order_file);
                        return;
		}
		while (work) {
			dd = gtk_ctree_node_get_row_data(ctree, work);
			fprintf(f, "%s\n", g_basename (dd->path));
			work = GTK_CTREE_ROW (work)->sibling;
		}
		fclose(f);
	} else {
		/* the folder is empty, so delete the .order file */
		if (g_file_exists(order_file)) {
			if (unlink (order_file) < 0)
				g_print(_("unable to remove .order file: %s\n"), order_file);
		}
	}

	g_free(order_file);
}

