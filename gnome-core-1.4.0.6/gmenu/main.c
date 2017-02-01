/*
 * GNOME menu editor revision 2
 * (C)2000
 *
 * Authors: John Ellis <johne@bellatlantic.net>
 *          Nat Friedman <nat@nat.org>
 *
 */

#include "gmenu.h"
#include <libgnomeui/gnome-window-icon.h>
/*
 *-----------------------------------------------------------------------------
 * global variables (public)
 *-----------------------------------------------------------------------------
 */

GtkWidget *app = NULL;
GtkWidget *tree = NULL;
GtkWidget *infolabel = NULL;
GtkWidget *infopixmap = NULL;

gchar *system_apps_dir = NULL;
gchar *system_apps_merge_dir = NULL;
gchar *system_applets_dir = NULL;
gchar *user_apps_dir = NULL;
gchar *system_pixmap_dir = NULL;
gchar *user_pixmap_dir = NULL;

static void new_submenu_pressed_cb(GtkWidget *widget, gpointer data);
static void new_item_pressed_cb(GtkWidget *widget, gpointer data);
static void delete_pressed_cb(GtkWidget *widget, gpointer data);
static void exit_pressed_cb(GtkWidget *widget, gpointer data);
static void move_up_cb(GtkWidget *widget, gpointer data);
static void move_down_cb(GtkWidget *widget, gpointer data);
static void sort_single_pressed_cb(GtkWidget *widget, gpointer data);
static void sort_recursive_pressed_cb(GtkWidget *widget, gpointer data);
static void about_pressed_cb(GtkWidget *widget, gpointer data);

static void about_close_cb(GtkWidget *widget, gpointer data);
static void about_dialog(void);

static gboolean delete_cb(GtkWidget *w, gpointer data);

/*
 *-----------------------------------------------------------------------------
 * Menu and Toolbar defines (private)
 *-----------------------------------------------------------------------------
 */

/* menu bar */
GnomeUIInfo file_menu[] = {
	GNOMEUIINFO_MENU_NEW_ITEM(N_("_New Submenu..."),
				  N_("Create a new submenu"),
				  new_submenu_pressed_cb, NULL),

	GNOMEUIINFO_MENU_NEW_ITEM(N_("New _Item..."),
				  N_("Create a new menu item"),
				  new_item_pressed_cb, NULL),
	
	{ GNOME_APP_UI_ITEM, N_("_Delete..."), NULL, delete_pressed_cb, NULL, NULL,
	  GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_CUT, 'D',
	  GDK_CONTROL_MASK, NULL },

	GNOMEUIINFO_SEPARATOR,

	GNOMEUIINFO_MENU_EXIT_ITEM(exit_pressed_cb, NULL),

	GNOMEUIINFO_END
};
GnomeUIInfo sort_menu[] = {
	{ GNOME_APP_UI_ITEM, N_("_Sort Submenu"), NULL,
	  sort_single_pressed_cb, NULL, NULL,
	  GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_SPELLCHECK,
	  'S', GDK_CONTROL_MASK, NULL },
	{ GNOME_APP_UI_ITEM, N_("Sort Submenu _Recursive"), NULL,
	  sort_recursive_pressed_cb, NULL, NULL,
	  GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_SPELLCHECK,
	  'R', GDK_CONTROL_MASK, NULL },
	{ GNOME_APP_UI_ENDOFINFO }
};
GnomeUIInfo help_menu[] = {
/*	{ GNOME_APP_UI_HELP, NULL, NULL, NULL, NULL, NULL,
	  GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL },*/
	  GNOMEUIINFO_MENU_ABOUT_ITEM(about_pressed_cb, NULL),
	  GNOMEUIINFO_END
};
GnomeUIInfo main_menu[] = {
	GNOMEUIINFO_MENU_FILE_TREE (file_menu),
	{ GNOME_APP_UI_SUBTREE, N_("_Sort"), NULL, sort_menu, NULL, NULL,
	  GNOME_APP_PIXMAP_NONE, NULL, 'S', GDK_MODIFIER_MASK, NULL },
	GNOMEUIINFO_MENU_HELP_TREE (help_menu),
	GNOMEUIINFO_END
};

/* toolbar */
GnomeUIInfo toolbar[] = {
	{ GNOME_APP_UI_ITEM, N_("New Submenu"), N_("Create a new submenu"),
	  new_submenu_pressed_cb, NULL, NULL,
	  GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_PIXMAP_NEW, 0, 0, NULL },

	{ GNOME_APP_UI_ITEM, N_("New Item"), N_("Create a new item"),
	  new_item_pressed_cb, NULL, NULL, GNOME_APP_PIXMAP_STOCK,
	  GNOME_STOCK_PIXMAP_NEW, 0, 0, NULL },

	{ GNOME_APP_UI_ITEM, N_("Delete"), N_("Delete selected menu item"),
	  delete_pressed_cb, NULL, NULL,
	  GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_PIXMAP_CUT, 0, 0, NULL },
	  GNOMEUIINFO_SEPARATOR,
	{ GNOME_APP_UI_ITEM, N_("Move up"), N_("Move selected menu up"),
	  move_up_cb, NULL, NULL,
	  GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_PIXMAP_UP, 0, 0, NULL },
	{ GNOME_APP_UI_ITEM, N_("Move down"), N_("Move selected menu down"),
	  move_down_cb, NULL, NULL,
	  GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_PIXMAP_DOWN, 0, 0, NULL },
	  GNOMEUIINFO_SEPARATOR,
	{ GNOME_APP_UI_ITEM, N_("Sort Submenu"), N_("Sort selected submenu"),
	  sort_single_pressed_cb, NULL, NULL,
	  GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_PIXMAP_SPELLCHECK, 0, 0, NULL },
	  GNOMEUIINFO_END
};

/* tree popup menu */
GnomeUIInfo tree_popup[] = {
	GNOMEUIINFO_MENU_NEW_ITEM(N_("New _Item..."),
				  N_("Create a new menu item"),
				  new_item_pressed_cb, NULL),
	GNOMEUIINFO_MENU_NEW_ITEM(N_("_New Submenu..."),
				  N_("Create a new submenu"),
				  new_submenu_pressed_cb, NULL),

	GNOMEUIINFO_SEPARATOR,

	{ GNOME_APP_UI_ITEM, N_("_Delete..."), NULL, delete_pressed_cb, NULL, NULL,
	  GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_CUT, 'D',
	  GDK_CONTROL_MASK, NULL },

	GNOMEUIINFO_SEPARATOR,

	{ GNOME_APP_UI_ITEM, N_("Move up"), N_("Move selected menu up"),
	  move_up_cb, NULL, NULL,
	  GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_PIXMAP_UP, 0, 0, NULL },
	{ GNOME_APP_UI_ITEM, N_("Move down"), N_("Move selected menu down"),
	  move_down_cb, NULL, NULL,
	  GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_PIXMAP_DOWN, 0, 0, NULL },

	GNOMEUIINFO_SEPARATOR,

	{ GNOME_APP_UI_ITEM, N_("_Sort"), NULL,
	  (gpointer)sort_single_pressed_cb, NULL, NULL,
	  GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_SPELLCHECK,
	  'S', GDK_CONTROL_MASK, NULL },
	{ GNOME_APP_UI_ITEM, N_("Sort _Recursive"), NULL,
	  (gpointer)sort_recursive_pressed_cb, NULL, NULL,
	  GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_SPELLCHECK,
	  'R', GDK_CONTROL_MASK, NULL },

	GNOMEUIINFO_END
};

/*
 *-----------------------------------------------------------------------------
 * Menu and toolbar callbacks (private)
 *-----------------------------------------------------------------------------
 */

static void new_submenu_pressed_cb(GtkWidget *widget, gpointer data)
{
	menu_tree_new_folder(tree);
}

static void new_item_pressed_cb(GtkWidget *widget, gpointer data)
{
	menu_tree_new_item(tree);
}

static void delete_pressed_cb(GtkWidget *widget, gpointer data)
{
	menu_tree_delete_item(tree);
}

static void exit_pressed_cb(GtkWidget *widget, gpointer data)
{
	gtk_main_quit();
}

static void move_up_cb(GtkWidget *widget, gpointer data)
{
	menu_tree_move_up(tree);
}

static void move_down_cb(GtkWidget *widget, gpointer data)
{
	menu_tree_move_down(tree);
}

static void sort_single_pressed_cb(GtkWidget *widget, gpointer data)
{
	menu_tree_sort_selected(tree);
}

static void sort_recursive_pressed_cb(GtkWidget *widget, gpointer data)
{
	menu_tree_sort_selected_recursive(tree);
}

static void about_pressed_cb(GtkWidget *widget, gpointer data)
{
	about_dialog();
}

/*
 *-----------------------------------------------------------------------------
 * about window functions (private)
 *-----------------------------------------------------------------------------
 */

static GtkWidget *about = NULL;

static void
about_close_cb(GtkWidget *widget, gpointer data)
{
	about = NULL;
}

static void about_dialog(void)
{
	const gchar *authors[3];

	if (about)
	{
		gdk_window_show(about->window);
		gdk_window_raise(about->window);
		return; /* avoid duplicates */
	}

	authors[0] = "John Ellis <johne@bellatlantic.net>";
	authors[1] = "Nat Friedman <nat@nat.org>";
	authors[2] = NULL;

	about = gnome_about_new ( _("GNOME menu editor"), VERSION,
			"(C) 2000",
			authors,
			_("Released under the terms of the GNU Public License.\n"
			"GNOME menu editor."),
			NULL);
	gtk_signal_connect(GTK_OBJECT(about), "destroy",
			   GTK_SIGNAL_FUNC(about_close_cb), NULL);
	gtk_widget_show (about);
}

/*
 *-----------------------------------------------------------------------------
 * setup and exit functions
 *-----------------------------------------------------------------------------
 */

static gboolean
delete_cb(GtkWidget *w, gpointer data)
{
	gtk_main_quit();
	return FALSE;
}

#define MERGE_PATH "/etc/X11/applnk"

int main (int argc, char *argv[])
{
	GtkWidget *mainbox;
	GtkWidget *hbox;
	GtkWidget *vbox;
	GtkWidget *frame;
	GtkWidget *scrolled;
	GtkTooltips *tooltips;

	bindtextdomain(PACKAGE, GNOMELOCALEDIR);
	textdomain(PACKAGE);

	gnome_init ("GNOME menu editor", VERSION, argc, argv);
	gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gnome-gmenu.png");

	system_apps_dir = gnome_datadir_file("gnome/apps");
	system_applets_dir = gnome_datadir_file("applets");
	system_pixmap_dir = gnome_datadir_file("pixmaps");
	if (!system_apps_dir || !system_pixmap_dir || !system_applets_dir)
		{
		gnome_error_dialog(_("Unable to retrieve GNOME installation directory\n"));
		gtk_main();
		return 1;
		}


	if (g_file_test (MERGE_PATH, G_FILE_TEST_ISDIR)) {
		system_apps_merge_dir =
			gnome_config_get_string("/panel/Merge/Directory="
						MERGE_PATH);
		if (system_apps_dir != NULL &&
		     ! g_file_test (system_apps_merge_dir, G_FILE_TEST_ISDIR)) {
			g_free (system_apps_merge_dir);
			system_apps_merge_dir = NULL;
		}
	}

	user_apps_dir = check_for_dir(gnome_util_home_file("apps"));
	user_pixmap_dir = check_for_dir(gnome_util_home_file("pixmaps"));

	app = gnome_app_new ("gmenu",_("GNOME menu editor"));
	gtk_widget_set_usize (app, 600, 460);
	gtk_signal_connect(GTK_OBJECT(app), "delete_event",
			   GTK_SIGNAL_FUNC (delete_cb), NULL);

	gnome_app_create_menus_with_data (GNOME_APP(app), main_menu, app);
	gnome_app_create_toolbar (GNOME_APP(app), toolbar);
	tooltips = gtk_tooltips_new();

	mainbox = gtk_hpaned_new();
        gnome_app_set_contents(GNOME_APP(app),mainbox);
        gtk_widget_show (mainbox);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);
	gtk_paned_add1(GTK_PANED (mainbox), vbox);
	gtk_widget_show(vbox);

	scrolled = gtk_scrolled_window_new(NULL, NULL);
	gtk_widget_set_usize(scrolled, 200, -1);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled),
				GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
	gtk_box_pack_start(GTK_BOX(vbox),scrolled,TRUE,TRUE,0);
	gtk_widget_show(scrolled);

	gtk_widget_push_visual (gdk_imlib_get_visual ());
	gtk_widget_push_colormap (gdk_imlib_get_colormap ());

	/*
	 * The GtkCTree...
	 */
	tree = gtk_ctree_new(1, 0);
	gtk_clist_set_column_auto_resize (GTK_CLIST (tree), 0, TRUE);

	gtk_widget_pop_visual ();
	gtk_widget_pop_colormap ();
	
	gtk_clist_set_row_height(GTK_CLIST(tree),22);
	gtk_ctree_set_indent (GTK_CTREE (tree), 10);

	gtk_clist_set_selection_mode(GTK_CLIST(tree),
				     GTK_SELECTION_BROWSE);
	gtk_container_add (GTK_CONTAINER (scrolled), tree);
	gtk_widget_show(tree);

	/* tree info area */
	hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,0);
        gtk_widget_show (hbox);

	frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(frame),GTK_SHADOW_IN);
	gtk_box_pack_start(GTK_BOX(hbox),frame,FALSE,FALSE,0);
	gtk_widget_show(frame);

	infopixmap = gnome_stock_pixmap_widget_new(app, GNOME_STOCK_MENU_BLANK);
	gtk_container_add(GTK_CONTAINER(frame), infopixmap);
	gtk_widget_show(infopixmap);

	frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(frame),GTK_SHADOW_IN);
	gtk_box_pack_end(GTK_BOX(hbox),frame,TRUE,TRUE,0);
	gtk_widget_show(frame);

	infolabel = gtk_label_new("");
	gtk_container_add(GTK_CONTAINER(frame), infolabel);
	gtk_widget_set_usize(infolabel, 1, -1);
	gtk_misc_set_alignment (GTK_MISC (infolabel), 0.0, 0.0);
	gtk_widget_show(infolabel);

	/* edit area */
	vbox = edit_area_create();
	gtk_paned_add2(GTK_PANED(mainbox), vbox);
	gtk_widget_show(vbox);

	menu_tree_populate(tree);
	menu_tree_init_signals(tree, tree_popup);

	gtk_widget_show(app);
	
	gtk_main();
	return 0;
}

