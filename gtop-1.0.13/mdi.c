#include <global.h>
#include <gtop-page.h>

#include <gtop-fsusage.h>
#include <gtop-memusage.h>
#include <gtop-procview.h>

#include <details.h>
#include <session.h>

#include <string.h>

static GnomeUIInfo addChildMenu [];
static GnomeUIInfo addMemChildMenu [];
static GnomeUIInfo addFsChildMenu [];
static GnomeUIInfo addProcChildMenu [];

static void	add_fs_child_cb		(GtkWidget *, gpointer);
static void	add_mem_child_cb	(GtkWidget *, gpointer);
static void	add_proc_child_cb	(GtkWidget *, gpointer);

static void	reap_viewless_childs	(void);

static void	add_view_cb		(void);
static void	add_toplevel_cb		(void);
static void	remove_view_cb		(void);

GnomeUIInfo fileMenu [] = {
	GNOMEUIINFO_MENU_NEW_ITEM
	(N_("New _View"), N_("Create new MDI view"), add_view_cb, NULL),
	GNOMEUIINFO_MENU_NEW_ITEM
	(N_("New _Window"), N_("Create new MDI toplevel view"),
	 add_toplevel_cb, NULL),
	GNOMEUIINFO_SEPARATOR,
	GNOMEUIINFO_MENU_NEW_SUBTREE (addChildMenu),
	GNOMEUIINFO_SEPARATOR,
	GNOMEUIINFO_MENU_CLOSE_ITEM (remove_view_cb, NULL),
	GNOMEUIINFO_MENU_EXIT_ITEM (gtop_quit, NULL),
	GNOMEUIINFO_END
};

static GnomeUIInfo addChildMenu [] = {
	GNOMEUIINFO_SUBTREE (N_("_Process View"), addProcChildMenu),
	GNOMEUIINFO_SUBTREE (N_("_Memory Usage"), addMemChildMenu),
	GNOMEUIINFO_SUBTREE (N_("_Filesystem Usage"), addFsChildMenu),
	GNOMEUIINFO_END
};

GnomeUIInfo addProcChildMenu [] = {
	GNOMEUIINFO_ITEM_DATA (N_("_All Processes"),
			       N_("Show all processes"),
			       add_proc_child_cb,
			       (gpointer) GTOP_PROCVIEW_ALL,
			       GNOME_APP_PIXMAP_NONE),
	GNOMEUIINFO_ITEM_DATA (N_("_User Processes"),
			       N_("Only show user processes"),
			       add_proc_child_cb,
			       (gpointer) GTOP_PROCVIEW_USER,
			       GNOME_APP_PIXMAP_NONE),
	GNOMEUIINFO_END
};

GnomeUIInfo addMemChildMenu [] = {
	GNOMEUIINFO_ITEM_DATA (N_("_Resident Sizes of Processes"),
			       N_("Show resident sizes of processes"),
			       add_mem_child_cb,
			       (gpointer) GTOP_MEMUSAGE_RESIDENT,
			       GNOME_APP_PIXMAP_NONE),
	GNOMEUIINFO_ITEM_DATA (N_("_Shared Sizes of Processes"),
			       N_("Show shared sizes of processes"),
			       add_mem_child_cb,
			       (gpointer) GTOP_MEMUSAGE_SHARED,
			       GNOME_APP_PIXMAP_NONE),
	GNOMEUIINFO_ITEM_DATA (N_("_Total Sizes of Processes"),
			       N_("Show total sizes of processes"),
			       add_mem_child_cb,
			       (gpointer) GTOP_MEMUSAGE_SIZE,
			       GNOME_APP_PIXMAP_NONE),
	GNOMEUIINFO_ITEM_DATA (N_("_Virtual Sizes of Processes"),
			       N_("Show virtual sizes of processes"),
			       add_mem_child_cb,
			       (gpointer) GTOP_MEMUSAGE_VIRTUAL,
			       GNOME_APP_PIXMAP_NONE),
	GNOMEUIINFO_ITEM_DATA (N_("S_wapped Sizes of Processes"),
			       N_("Show swapped sizes of processes"),
			       add_mem_child_cb,
			       (gpointer) GTOP_MEMUSAGE_SWAP,
			       GNOME_APP_PIXMAP_NONE),
	GNOMEUIINFO_END
};

GnomeUIInfo addFsChildMenu [] = {
	GNOMEUIINFO_ITEM_DATA (N_("_Total Filesystem Sizes"),
			       N_("Show total filesystem sizes"),
			       add_fs_child_cb,
			       (gpointer) GTOP_FSUSAGE_TOTAL,
			       GNOME_APP_PIXMAP_NONE),
	GNOMEUIINFO_ITEM_DATA (N_("_Used Filesystem Sizes"),
			       N_("Show used filesystem sizes"),
			       add_fs_child_cb,
			       (gpointer) GTOP_FSUSAGE_USED,
			       GNOME_APP_PIXMAP_NONE),
	GNOMEUIINFO_ITEM_DATA (N_("_Free Filesystem Sizes"),
			       N_("Show free filesystem sizes"),
			       add_fs_child_cb,
			       (gpointer) GTOP_FSUSAGE_FREE,
			       GNOME_APP_PIXMAP_NONE),
	GNOMEUIINFO_END
};

static void
add_fs_child_cb (GtkWidget *obj, gpointer type)
{
	GTopPage *page;
	gpointer uidata;

	uidata = gtk_object_get_data
		(GTK_OBJECT (obj), GNOMEUIINFO_KEY_UIDATA);

	page = gtop_page_new (GTOP_PAGE_FSUSAGE, GPOINTER_TO_INT (uidata));

	gnome_mdi_add_child (mdi, GNOME_MDI_CHILD (page));
	gnome_mdi_add_view (mdi, GNOME_MDI_CHILD (page));
}

static void
add_mem_child_cb (GtkWidget *obj, gpointer type)
{
	GTopPage *page;
	gpointer uidata;

	uidata = gtk_object_get_data
		(GTK_OBJECT (obj), GNOMEUIINFO_KEY_UIDATA);

	page = gtop_page_new (GTOP_PAGE_MEMUSAGE, GPOINTER_TO_INT (uidata));

	gnome_mdi_add_child (mdi, GNOME_MDI_CHILD (page));
	gnome_mdi_add_view (mdi, GNOME_MDI_CHILD (page));
}

static void
add_proc_child_cb (GtkWidget *obj, gpointer type)
{
	GTopPage *page;
	gpointer uidata;

	uidata = gtk_object_get_data
		(GTK_OBJECT (obj), GNOMEUIINFO_KEY_UIDATA);

	page = gtop_page_new (GTOP_PAGE_PROCVIEW, GPOINTER_TO_INT (uidata));

	gnome_mdi_add_child (mdi, GNOME_MDI_CHILD (page));
	gnome_mdi_add_view (mdi, GNOME_MDI_CHILD (page));
}

static void
add_view_cb (void)
{
	GnomeMDIChild *child = gnome_mdi_get_active_child (mdi);
	if (child) gnome_mdi_add_view (mdi, child);
}

static void
add_toplevel_cb (void)
{
	GnomeMDIChild *child = gnome_mdi_get_active_child (mdi);
	if (child) gnome_mdi_add_toplevel_view (mdi, child);
}

static void
reap_viewless_childs (void)
{
	gint removed;

	do {
		GList *c;

		removed = 0;

		for (c = mdi->children; c; c = c->next) {
			GnomeMDIChild *child = c->data;
			
			if (child->views == NULL) {
				gnome_mdi_remove_child (mdi, child, FALSE);
				removed = TRUE;
				break;
			}
		}

	} while (removed);
}

static void
remove_view_cb (void)
{
	GtkWidget *active_view = gnome_mdi_get_active_view (mdi);

	if (active_view)
		gnome_mdi_remove_view (mdi, active_view, FALSE);
	
	reap_viewless_childs ();
}

static void
view_changed_cb (GnomeMDI *mdi, GtkWidget *old_view)
{
	gchar *path, *label;
	
	if (mdi->active_view == NULL)
		return;

	/* note that you can't use item numbers to determine menu number,
	   because the first item could be the tear off one */

	if (IS_GTOP_FSUSAGE (mdi->active_view))
		label = gtop_fsusage_radio_items
			[GTOP_FSUSAGE (mdi->active_view)->data.ftype].label;
	else if (IS_GTOP_MEMUSAGE (mdi->active_view))
		label = gtop_memusage_radio_items
			[GTOP_MEMUSAGE (mdi->active_view)->data.ftype].label;
	else if (IS_GTOP_PROCVIEW (mdi->active_view))
		label = gtop_procview_radio_items
			[GTOP_PROCVIEW (mdi->active_view)->data.ftype].label;
	else
		label = NULL;

	if(label) {
		GtkWidget *submenu, *item = NULL; 
		gint pos = 0;

		label = (label [0] == '\0' ? "" : _(label));

		path = g_strconcat(GNOME_MENU_VIEW_PATH, label, NULL);

		submenu = gnome_app_find_menu_pos
			(GNOME_APP (mdi->active_window)->menubar, path, &pos);

		if(submenu)
			item = g_list_nth_data
				(GTK_MENU_SHELL (submenu)->children, pos - 1);

		if(item)
			gtk_menu_shell_activate_item
				(GTK_MENU_SHELL (submenu), item, TRUE);
		
		g_free(path);
	}
}

static void
app_destroy_cb (GnomeApp *app, GTopStatusBarData *d)
{
	/* empty */
}

static void
drag_data_received (GtkWidget *widget, GdkDragContext *context,
		    gint x, gint y, GtkSelectionData *data,
		    guint info, guint32 time)
{
	if (data->format == 1) {
		GnomeMDIChild *child = gtop_page_create_from_config (data->data);
		
		gnome_mdi_add_child (mdi, child);
		gnome_mdi_add_view (mdi, child);
	}

	gtk_drag_finish (context, FALSE, FALSE, time);
}

static gint
app_created_idle (GnomeApp *app)
{
	GnomeUIInfo *mb = gnome_mdi_get_menubar_info (app);

	while (gtk_events_pending ()) { gtk_main_iteration_do (FALSE); gdk_flush (); }

	/* set menu/toolbar visibility */
	if (!gtop_properties.global.show_menubar) {
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM
						(((GnomeUIInfo *)mb [1].moreinfo) [0].widget),
						1);
		gtop_properties.global.show_menubar = 0;
	}
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM
					(((GnomeUIInfo *)mb [1].moreinfo) [0].widget),
					gtop_properties.global.show_menubar);
	if (!gtop_properties.global.show_toolbar) {
		gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM
						(((GnomeUIInfo *)mb [1].moreinfo) [1].widget),
						1);
		gtop_properties.global.show_toolbar = 0;
	}
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM
					(((GnomeUIInfo *)mb [1].moreinfo) [1].widget),
					gtop_properties.global.show_toolbar);
	return FALSE;
}

static void
app_created_cb (GnomeMDI *mdi, GnomeApp *app)
{
	GTopStatusBarData *d;
	gint x, y, width, height;

	d = gtop_statusbar_new ();
	gtop_statusbar_update (d);

	gnome_app_set_statusbar_custom (app, d->container, d->status_bar);
        gnome_app_install_menu_hints (app, gnome_mdi_get_menubar_info (app));

	gtk_idle_add ((GtkFunction) app_created_idle, (gpointer) app);

	gtk_signal_connect (GTK_OBJECT (app), "drag_data_received",
			    GTK_SIGNAL_FUNC (drag_data_received), NULL);

	gtk_drag_dest_set (GTK_WIDGET (app),
			   GTK_DEST_DEFAULT_ALL,
			   gtop_target_table, 1,
			   GDK_ACTION_COPY | GDK_ACTION_MOVE);

	gtk_signal_connect (GTK_OBJECT (app), "destroy",
			    GTK_SIGNAL_FUNC (app_destroy_cb), d);

	if (gnome_parse_geometry (initial_geometry, &x, &y, &width, &height)) {
		if ((width != -1) && (height != -1))
			gtk_window_set_default_size
				(GTK_WINDOW (app), width, height);

		if ((x != -1) && (y != -1))
			gtk_widget_set_uposition (GTK_WIDGET (app), x, y);
	}
}

void
gtop_mdi_init (void)
{
	gtk_signal_connect(GTK_OBJECT (mdi), "app_created",
			   GTK_SIGNAL_FUNC (app_created_cb), NULL);
}

void
gtop_mdi_start (gboolean init)
{
	GTopPage *page;
	GtkWidget *view;

	gtk_signal_connect(GTK_OBJECT (mdi), "view_changed",
			   GTK_SIGNAL_FUNC (view_changed_cb), NULL);

	if (!init) return;

	page = gtop_page_new (GTOP_PAGE_PROCVIEW, GTOP_PROCVIEW_ALL);

	gnome_mdi_add_child (mdi, GNOME_MDI_CHILD (page));
	gnome_mdi_add_view (mdi, GNOME_MDI_CHILD (page));

	view = gnome_mdi_get_active_view (mdi);

	page = gtop_page_new (GTOP_PAGE_MEMUSAGE, GTOP_MEMUSAGE_RESIDENT);

	gnome_mdi_add_child (mdi, GNOME_MDI_CHILD (page));
	gnome_mdi_add_view (mdi, GNOME_MDI_CHILD (page));

	page = gtop_page_new (GTOP_PAGE_FSUSAGE, GTOP_FSUSAGE_FREE);

	gnome_mdi_add_child (mdi, GNOME_MDI_CHILD (page));
	gnome_mdi_add_view (mdi, GNOME_MDI_CHILD (page));

	gnome_mdi_set_active_view (mdi, view);
}
