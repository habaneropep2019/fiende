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
 * new item functions
 *-----------------------------------------------------------------------------
 */

void menu_tree_new_folder(GtkWidget *ctree)
{
	GtkCTreeNode *node;
	Desktop_Data *dd;
	Desktop_Data *dd_new;
	GtkCTreeNode *parent, *sibling;
	GtkWidget *pixmap;
	gchar *path;
	
	node = menu_tree_get_selection(GTK_CTREE(ctree));
	if (!node) return;

	dd = gtk_ctree_node_get_row_data(GTK_CTREE(ctree), node);

	if ((dd->isfolder && !menu_tree_node_is_editable(ctree, node)) ||
	    (!dd->isfolder && !menu_tree_node_is_editable(ctree, GTK_CTREE_ROW(node)->parent)) )
		{
		gnome_warning_dialog(_("You can't add an entry to that submenu.\nYou do not have the proper permissions."));
		return;
		}

	if (dd->isfolder)
		{
		parent = node;
		sibling = NULL;
		path = g_concat_dir_and_file(dd->path, _("New Folder"));
		gtk_ctree_expand (GTK_CTREE(ctree), node);
		}
	else
		{
		Desktop_Data *dd_parent;
		parent = GTK_CTREE_ROW(node)->parent;
		sibling = node;
		dd_parent = gtk_ctree_node_get_row_data(GTK_CTREE(ctree), parent);
		path = g_concat_dir_and_file(dd_parent->path, _("New Folder"));
		}

	if (g_file_exists(path))
		{
		g_free(path);
		return;
		}

	if (mkdir(path, 0755) < 0)
		{
		gnome_warning_dialog(_("Failed to create directory"));
		}
	else
		{
		gchar *dir_file;
		GnomeDesktopEntry* dentry;

		dir_file = g_concat_dir_and_file(path,".directory");

		dentry = g_new0(GnomeDesktopEntry, 1);
		dentry->name = g_strdup(_("New Folder"));
		dentry->type = g_strdup("Directory");
		dentry->icon = gnome_unconditional_datadir_file("pixmaps/gnome-folder.png");
		pixmap = pixmap_load(dentry->icon);
		save_desktop_entry_file(dentry, dir_file, FALSE, FALSE, FALSE);
		gnome_desktop_entry_free(dentry);

		g_free(dir_file);

		dd_new = desktop_data_new(path, _("New Folder"), "", pixmap);
		node = menu_tree_insert_node(ctree, parent, sibling, dd_new, TRUE);
		gtk_ctree_select(GTK_CTREE(ctree), node);
		edit_area_grab_name();
		save_order_of_dir(GTK_CTREE(ctree), node, FALSE);
		}

	g_free(path);
}

void menu_tree_new_item(GtkWidget *ctree)
{
	GtkCTreeNode *node;
	Desktop_Data *dd;
	Desktop_Data *dd_new;
	GtkCTreeNode *parent, *sibling;
	GtkWidget *pixmap;
	gchar *path;
	GnomeDesktopEntry* dentry;
	
	node = menu_tree_get_selection(GTK_CTREE(ctree));
	if (!node) return;

	dd = gtk_ctree_node_get_row_data(GTK_CTREE(ctree), node);

	if ((dd->isfolder && !menu_tree_node_is_editable(ctree, node)) ||
	    (!dd->isfolder && !menu_tree_node_is_editable(ctree, GTK_CTREE_ROW(node)->parent)) )
		{
		gnome_warning_dialog(_("You can't add an entry to that submenu.\nYou do not have the proper permissions."));
		return;
		}

	if (dd->isfolder)
		{
		parent = node;
		sibling = NULL;
		path = g_concat_dir_and_file(dd->path, _("untitled.desktop"));
		gtk_ctree_expand (GTK_CTREE(ctree), node);
		}
	else
		{
		Desktop_Data *dd_parent;
		parent = GTK_CTREE_ROW(node)->parent;
		sibling = node;
		dd_parent = gtk_ctree_node_get_row_data(GTK_CTREE(ctree), parent);
		path = g_concat_dir_and_file(dd_parent->path, _("untitled.desktop"));
		}

	if (g_file_exists(path))
		{
		g_free(path);
		return;
		}

	dentry = g_new0(GnomeDesktopEntry, 1);
	dentry->name = g_strdup(_("untitled"));
	dentry->type = g_strdup("Application");
	dentry->icon = gnome_unconditional_datadir_file("pixmaps/gnome-default.png");
	pixmap = pixmap_load(dentry->icon);
	save_desktop_entry_file(dentry, path, FALSE, FALSE, FALSE);
	gnome_desktop_entry_free(dentry);

	dd_new = desktop_data_new(path, _("untitled"), "", pixmap);
	node = menu_tree_insert_node(ctree, parent, sibling, dd_new, TRUE);

	gtk_ctree_select(GTK_CTREE(ctree), node);
	edit_area_grab_name();
	save_order_of_dir(GTK_CTREE(ctree), node, FALSE);

	g_free(path);
}

