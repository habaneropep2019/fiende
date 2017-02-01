/*
 * GNOME menu editor revision 2
 * (C)1999
 *
 * Authors: John Ellis <johne@bellatlantic.net>
 *          Nat Friedman <nat@nat.org>
 *
 */

#include "gmenu.h"

static gint menu_tree_find_path_cb(gconstpointer a, gconstpointer b);

static void menu_tree_update_paths_cb(GtkCTree *ctree, GtkCTreeNode *node,  gpointer data);
static void menu_tree_sync_node_to_dentry(GtkWidget *ctree, GtkCTreeNode *node,
					  const GnomeDesktopEntry *dentry);
static void menu_tree_sync_node_to_dentry_and_path(GtkWidget *ctree, GtkCTreeNode *node,
					  const GnomeDesktopEntry *dentry, const gchar *path);

static void remove_node_cb(gpointer data);
static void menu_tree_add_folder(GtkWidget *ctree, GtkCTreeNode *parent);
static GtkCTreeNode *menu_tree_add_node_from_file(GtkWidget *ctree, GtkCTreeNode *parent,
			GtkCTreeNode *sibling, const gchar *file);

static void menu_tree_update_progressbar(GtkWidget *progressbar);
static void menu_tree_add_recurse_cb(GtkCTree *ctree, GtkCTreeNode *node, gpointer data);
static void ctree_node_count_cb(GtkCTree *ctree, GtkCTreeNode *node, gpointer data);
static gint ctree_node_count(GtkCTree *ctree);
static GtkWidget *new_top_pixmap_from_dentry_path(const gchar *path);

static void menu_tree_item_select_cb(GtkCTree *ctree, GtkCTreeNode *node, gint column, gpointer data);

/*
 *-----------------------------------------------------------------------------
 * Node search functions
 *-----------------------------------------------------------------------------
 */

static gint menu_tree_find_path_cb(gconstpointer a, gconstpointer b)
{
	if (!((Desktop_Data *)(a))->path) return 1;
	return strcmp(((Desktop_Data *)(a))->path, (gchar *)b);
}

GtkCTreeNode *menu_tree_find_path(GtkCTree *ctree, const gchar *path)
{
	return gtk_ctree_find_by_row_data_custom (GTK_CTREE(ctree), NULL, (gpointer)path, menu_tree_find_path_cb);
}

GtkCTreeNode *menu_tree_get_selection(GtkCTree *ctree)
{
	if (GTK_CLIST(ctree)->selection)
		{
		return ((GList *)(GTK_CLIST(ctree)->selection))->data;
		}
	return NULL;
}

gint menu_tree_node_is_editable(GtkWidget *ctree, GtkCTreeNode *node)
{
	Desktop_Data *dd;
	if (!node) return FALSE;
	dd = gtk_ctree_node_get_row_data(GTK_CTREE(ctree), node);
	return dd->editable;
}

/*
 *-----------------------------------------------------------------------------
 * Node update functions
 *-----------------------------------------------------------------------------
 */


static void menu_tree_update_paths_cb(GtkCTree *ctree, GtkCTreeNode *node,  gpointer data)
{
	GtkCTreeNode *parent;
	Desktop_Data *dd;
	Desktop_Data *dd_parent;
	gchar *new_path;

	parent = GTK_CTREE_ROW(node)->parent;
	dd = gtk_ctree_node_get_row_data(ctree, node);
	dd_parent = gtk_ctree_node_get_row_data(ctree, parent);

	new_path = g_strconcat(dd_parent->path, "/", dd->path + g_filename_index(dd->path), NULL);

	if (strcmp(edit_area_path(), dd->path) == 0)
		{
		edit_area_change_path(new_path);
		}

	g_free(dd->path);
	dd->path = new_path;
}

void menu_tree_update_paths(GtkWidget *ctree, GtkCTreeNode *node)
{
	gtk_ctree_pre_recursive(GTK_CTREE(ctree), node, menu_tree_update_paths_cb, NULL);
}

static void menu_tree_sync_node_to_dentry(GtkWidget *ctree, GtkCTreeNode *node,
					  const GnomeDesktopEntry *dentry)
{
	Desktop_Data *dd;
	GdkPixmap *nopixmap;
	GdkBitmap *nomask;
	gchar *notext;
	guint8 spacing;
	gboolean is_leaf, expanded;

	if (!dentry) return;

	dd = gtk_ctree_node_get_row_data(GTK_CTREE(ctree), node);

	g_free(dd->name);
	dd->name = g_strdup(dentry->name);

	g_free(dd->comment);
	if (dentry->comment)
		dd->comment = g_strdup(dentry->comment);
	else
		dd->comment = g_strdup("");

	gtk_widget_destroy (dd->pixmap);
	dd->pixmap = NULL;
	if (dentry->icon)
		dd->pixmap = pixmap_load(dentry->icon);
	else
		dd->pixmap = pixmap_unknown();

	gtk_ctree_get_node_info(GTK_CTREE(tree), node, &notext, &spacing,
			&nopixmap, &nomask,
			&nopixmap, &nomask,
			&is_leaf, &expanded);

	gtk_ctree_set_node_info (GTK_CTREE(ctree), node, dd->name, spacing,
			GNOME_PIXMAP(dd->pixmap)->pixmap, GNOME_PIXMAP(dd->pixmap)->mask,
			GNOME_PIXMAP(dd->pixmap)->pixmap, GNOME_PIXMAP(dd->pixmap)->mask,
			is_leaf, expanded);

	if (strcmp(edit_area_path(), dd->path) == 0)
		{
		edit_area_set_to(dd);
		}

}

static void menu_tree_sync_node_to_dentry_and_path(GtkWidget *ctree, GtkCTreeNode *node,
					  const GnomeDesktopEntry *dentry, const gchar *path)
{
	Desktop_Data *dd;

	dd = gtk_ctree_node_get_row_data(GTK_CTREE(ctree), node);

	if (strcmp(edit_area_path(), dd->path) == 0)
		{
		edit_area_change_path(path);
		}

	g_free(dd->path);
	dd->path = g_strdup(path);
	
	menu_tree_sync_node_to_dentry(ctree, node, dentry);

	/* if a folder, resync all children */
	if (dd->isfolder)
		{
		menu_tree_update_paths(ctree, node);
		}

	save_order_of_dir(GTK_CTREE(ctree), node, FALSE);
}

void menu_tree_path_updated(GtkWidget *ctree, const gchar *old_path, const gchar *new_path, const GnomeDesktopEntry *dentry)
{
	GtkCTreeNode *node;

	node = menu_tree_find_path(GTK_CTREE(ctree), old_path);

	if (strcmp(old_path, new_path) == 0)
		menu_tree_sync_node_to_dentry(ctree, node, dentry);
	else
		menu_tree_sync_node_to_dentry_and_path(ctree, node, dentry, new_path);
}

/*
 *-----------------------------------------------------------------------------
 * Node insertion functions
 *-----------------------------------------------------------------------------
 */

static void remove_node_cb(gpointer data)
{
	desktop_data_free((Desktop_Data *)data);
}

GtkCTreeNode *menu_tree_insert_node(GtkWidget *ctree, GtkCTreeNode *parent,
			GtkCTreeNode *sibling, Desktop_Data *dd, gint expanded)
{
	GtkCTreeNode *node;
	gchar *buf[2];

	buf[0] = dd->name;
	buf[1] = NULL;

	if (dd->pixmap)
		{
		node = gtk_ctree_insert_node(GTK_CTREE(ctree), parent, sibling, buf, 5,
			GNOME_PIXMAP(dd->pixmap)->pixmap, GNOME_PIXMAP(dd->pixmap)->mask,
			GNOME_PIXMAP(dd->pixmap)->pixmap, GNOME_PIXMAP(dd->pixmap)->mask,
			!dd->isfolder, expanded);
		}
	else
		{
		node = gtk_ctree_insert_node(GTK_CTREE(ctree), parent, sibling, buf, 5,
			NULL, NULL,
			NULL, NULL,
			!dd->isfolder, expanded);
		}

	gtk_ctree_node_set_row_data_full (GTK_CTREE(ctree),
                                          node, dd, remove_node_cb);
	return node;
}

static GtkCTreeNode *menu_tree_add_node_from_file(GtkWidget *ctree, GtkCTreeNode *parent,
			GtkCTreeNode *sibling, const gchar *file)
{
	Desktop_Data *dd_parent;
	Desktop_Data *dd;
	gchar *path;

	dd_parent = gtk_ctree_node_get_row_data(GTK_CTREE(ctree), parent);
	path = g_concat_dir_and_file (dd_parent->path, file);

	dd = desktop_data_new_from_path(path);
	g_free(path);

	return menu_tree_insert_node(ctree, parent, sibling, dd, FALSE);
}

static gboolean 
is_file_ordered (const GList *orderlist, const char *file)
{
	const GList *li;

	for (li = orderlist; li != NULL; li = li->next) {
		char *ordered_file = li->data;
		
		if (strcmp (file, ordered_file) == 0)
			return TRUE;
	}

	return FALSE;
}

static void menu_tree_add_folder(GtkWidget *ctree, GtkCTreeNode *parent)
{
	DIR *dp; 
	struct dirent *dir;

	GtkCTreeNode *node = NULL;
	GList *orderlist = NULL;
	Desktop_Data *parent_data;

	parent_data = gtk_ctree_node_get_row_data(GTK_CTREE(ctree), parent);
	parent_data->expanded = TRUE;

	orderlist = get_order_of_dir(parent_data->path);
	if (orderlist != NULL) {
		GList *work = orderlist;
		while(work) {
			gchar *path;
			path = g_concat_dir_and_file(parent_data->path, work->data);
			if (g_file_exists(path)) {
				node = menu_tree_add_node_from_file(ctree, parent, NULL, work->data);
			}
			g_free(path);
			work = work->next;
		}
	}

	dp = opendir (parent_data->path);

	if(dp == NULL) {
		/* dir not found */ 
		goto cleanup;
	}

	while ((dir = readdir(dp)) != NULL) { 
		if (dir->d_ino > 0 && /* skips removed files */
		    strncmp (dir->d_name, ".", 1) != 0 && /* skip hidden files */
		    ! is_file_ordered (orderlist, dir->d_name) /* file not in ordered */) {
			node = menu_tree_add_node_from_file(ctree, parent, NULL, dir->d_name);
		}
	} 

	closedir(dp);

cleanup:
	if (orderlist != NULL) {
		g_list_foreach(orderlist,(GFunc)g_free,NULL);
		g_list_free(orderlist);
	}
}

/*
 *-----------------------------------------------------------------------------
 * Initial menu structure init functions
 *-----------------------------------------------------------------------------
 */

static void menu_tree_update_progressbar(GtkWidget *progressbar)
{
	gfloat val;
	if (!progressbar) return;
	val = gtk_progress_get_value(GTK_PROGRESS(progressbar));
	val += 1;
	if (val > 100) val = 0;
	gtk_progress_set_value(GTK_PROGRESS(progressbar), val);
	gtk_widget_draw(progressbar, NULL);
}

static void menu_tree_add_recurse_cb(GtkCTree *ctree, GtkCTreeNode *node, gpointer data)
{
	GtkWidget *progressbar = data;
	Desktop_Data *dd = gtk_ctree_node_get_row_data(GTK_CTREE(ctree), node);

	if (dd->isfolder)
		{
		if (!dd->expanded) menu_tree_add_folder(GTK_WIDGET(ctree), node);
		menu_tree_update_progressbar(progressbar);
		}
}

static void ctree_node_count_cb(GtkCTree *ctree, GtkCTreeNode *node, gpointer data)
{
	gint *p = data;
	*p = *p + 1;
}

static gint ctree_node_count(GtkCTree *ctree)
{
	gint count = 0;
	gtk_ctree_post_recursive (ctree, NULL, ctree_node_count_cb, &count);
	return count;
}

static GtkWidget *new_top_pixmap_from_dentry_path(const gchar *path)
{
	GtkWidget *pixmap = NULL;
	GnomeDesktopEntry *dentry = NULL;

	dentry = gnome_desktop_entry_load_unconditional (path);
	if (dentry != NULL) {
		if (dentry->icon != NULL)
			pixmap = gnome_stock_pixmap_widget_at_size (NULL, dentry->icon, 20, 20);
		gnome_desktop_entry_free(dentry);
	}

	if (pixmap == NULL)
		pixmap = pixmap_top();

	return pixmap;
}

void menu_tree_populate(GtkWidget *ctree)
{
	Desktop_Data *dd;
	GtkWidget *pixmap;
	GtkCTreeNode *node;
	gchar *buf;
	gint c;

	GtkWidget *dialog;
	GtkWidget *label;
	GtkWidget *progressbar;

	/* loading dialog */
	dialog = gnome_dialog_new(_("GNOME menu editor"), NULL);

	label = gtk_label_new(_("One moment, reading menus..."));
	gtk_box_pack_start(GTK_BOX(GNOME_DIALOG(dialog)->vbox), label, FALSE, FALSE, 5);
	gtk_widget_show(label);

	progressbar = gtk_progress_bar_new();
	gtk_progress_set_activity_mode(GTK_PROGRESS(progressbar), TRUE);
	gtk_box_pack_start(GTK_BOX(GNOME_DIALOG(dialog)->vbox), progressbar, FALSE, FALSE, 5);
	gtk_widget_show(progressbar);

        gtk_widget_show(dialog);
	while(gtk_events_pending()) gtk_main_iteration();
	/* end of loading dialog init */

	gtk_clist_freeze(GTK_CLIST(ctree));

	/* user menu branch */
	buf = g_concat_dir_and_file(user_apps_dir, ".directory");
	pixmap = new_top_pixmap_from_dentry_path(buf);
	g_free(buf);
	dd = desktop_data_new(user_apps_dir, _("Favorites (user menus)"),
			      _("Top of user menus"), pixmap);
	node = menu_tree_insert_node(ctree, NULL, NULL, dd, TRUE);

	/* system menu branch */
	buf = g_concat_dir_and_file(system_apps_dir, ".directory");
	pixmap = new_top_pixmap_from_dentry_path(buf);
	g_free(buf);
	dd = desktop_data_new(system_apps_dir, _("Programs (system menus)"),
			      _("Top of system menus"), pixmap);
	node = menu_tree_insert_node(ctree, NULL, NULL, dd, TRUE);

	/* Programs to merge in menu branch */
	if (system_apps_merge_dir != NULL) {
		buf = g_concat_dir_and_file(system_apps_merge_dir, ".directory");
		pixmap = new_top_pixmap_from_dentry_path(buf);
		g_free(buf);
		dd = desktop_data_new(system_apps_merge_dir, _("Programs to be merged in (system menus)"),
				      _("Top of system merge menus"), pixmap);
		node = menu_tree_insert_node(ctree, NULL, NULL, dd, TRUE);
	}

	/* applets menu branch */
	buf = g_concat_dir_and_file(system_applets_dir, ".directory");
	pixmap = new_top_pixmap_from_dentry_path(buf);
	g_free(buf);
	dd = desktop_data_new(system_applets_dir, _("Applets (system menus)"),
			      _("Top of applet menus"), pixmap);
	node = menu_tree_insert_node(ctree, NULL, NULL, dd, TRUE);


	/* now load the entire menu tree */
	c = 0;
	while (ctree_node_count(GTK_CTREE(ctree)) > c)
		{
		c = ctree_node_count(GTK_CTREE(ctree));

		gtk_ctree_post_recursive(GTK_CTREE(ctree), NULL,
			menu_tree_add_recurse_cb, progressbar);
		}

	gtk_clist_thaw(GTK_CLIST(ctree));

	gnome_dialog_close(GNOME_DIALOG(dialog));
}

/*
 *-----------------------------------------------------------------------------
 * Signal init and callback functions
 *-----------------------------------------------------------------------------
 */

static void menu_tree_button_press_cb(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	GnomeUIInfo *uiinfo =  data;

	if (event->button == 2)
		{
		/* select row for dnd to work */
		GtkCTreeNode *node = NULL;
		gint row = -1;
		gint col;

		gtk_clist_get_selection_info (GTK_CLIST(widget), event->x, event->y, &row, &col);
		if (row != -1) node = gtk_ctree_node_nth(GTK_CTREE(widget), (guint)row);

		if (node)
			gtk_object_set_data(GTK_OBJECT(widget), "gmenu_drag_start", GINT_TO_POINTER(row));
		else
			gtk_object_set_data(GTK_OBJECT(widget), "gmenu_drag_start", NULL);
		}
	if (event->button == 3)
		{
		GtkWidget *menu;
		GtkCTreeNode *node;
		gint row = -1;
		gint col;

		gtk_clist_get_selection_info (GTK_CLIST(widget), event->x, event->y, &row, &col);
		node = gtk_ctree_node_nth(GTK_CTREE(widget), (guint)row);
		if (row == -1 || !node) return;

		gtk_ctree_select(GTK_CTREE(widget), node);

		menu = gnome_popup_menu_new (uiinfo);
		gnome_popup_menu_do_popup_modal (menu, NULL, NULL, event, NULL);
		gtk_widget_destroy (menu);
		}
}

static void menu_tree_item_select_cb(GtkCTree *ctree, GtkCTreeNode *node, gint column, gpointer data)
{
	Desktop_Data *dd;
	dd = gtk_ctree_node_get_row_data(GTK_CTREE(ctree), node);
	edit_area_set_to(dd);
	gtk_label_set(GTK_LABEL(infolabel), dd->comment);
	if (dd->editable)
		gnome_stock_set_icon(GNOME_STOCK(infopixmap), GNOME_STOCK_MENU_BOOK_OPEN);
	else
		gnome_stock_set_icon(GNOME_STOCK(infopixmap), GNOME_STOCK_MENU_BOOK_RED);
}

void menu_tree_init_signals(GtkWidget *ctree, GnomeUIInfo *tree_popup_uiinfo)
{
	GtkCTreeNode *node;

	gtk_signal_connect(GTK_OBJECT(ctree), "tree_select_row",
			GTK_SIGNAL_FUNC(menu_tree_item_select_cb), NULL);
	gtk_signal_connect(GTK_OBJECT(ctree), "button_press_event",
			GTK_SIGNAL_FUNC(menu_tree_button_press_cb), tree_popup_uiinfo);

	/* force selection of first item in list, so that edit area is synced */
	node = GTK_CTREE_NODE(GTK_CLIST(ctree)->row_list);
	gtk_ctree_select(GTK_CTREE(ctree), node);

	menu_tree_init_dnd(ctree);
}

