#include <config.h>
#include <gtk/gtk.h>
#include <glib.h>
#include <libgnomeui/gnome-window-icon.h>

#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

#include "toc.h"
#include "toc-man.h"
#include "toc-ghelp.h"
#include "toc-info.h"

static gint normalCursor(GtkWidget *widget);
static void newSelection(GtkWidget *tree);
static void initToc(GtkWidget *toc, GtkSignalFunc selectCallback);
static int hideTocInt(GtkWidget *window);

struct _toc_config toc_config[] = {
    { "/usr/man",              TOC_MAN_TYPE   },
    { "/usr/local/man",        TOC_MAN_TYPE   },
    { "/usr/X11R6/man",        TOC_MAN_TYPE   },
    { "/usr/info",             TOC_INFO_TYPE  },
    { "/opt/gnome/share/gnome/help",       TOC_GHELP_TYPE },
    { "/usr/local/share/gnome/help",       TOC_GHELP_TYPE },
    { "/usr/local/gnome/share/gnome/help", TOC_GHELP_TYPE },
    { "/usr/share/gnome/help",             TOC_GHELP_TYPE },
    { NULL, 0 }
};

static gint normalCursor(GtkWidget *widget)
{
    gdk_window_set_cursor(GTK_WIDGET(widget)->window, NULL);

    /* Only execute one */
    return FALSE;
}

void setWatch(GtkWidget *widget)
{
    static GdkCursor *watchCursor = NULL;

    if (! watchCursor) {
	watchCursor = gdk_cursor_new(GDK_WATCH);
    }

    gdk_window_set_cursor(GTK_WIDGET(widget)->window, watchCursor);
}

void unsetWatch(GtkWidget *widget)
{
    gtk_idle_add_priority(GTK_PRIORITY_LOW, (GtkFunction)normalCursor,
			  widget);
}

void collapseBranch(GtkTree *tree)
{
    GtkTreeItem *parentItem;
    GtkTreeItem *item;
    GList *selected;

    selected = GTK_TREE_SELECTION(tree);
    if (!selected) {
	return;
    }

    item = GTK_TREE_ITEM(selected->data);

    parentItem =
	GTK_TREE_ITEM(GTK_TREE(GTK_WIDGET(item)->parent)->tree_owner);

    gtk_tree_item_collapse(parentItem);
}

static void newSelection(GtkWidget *tree)
{
    GtkTreeItem *item;
    GList *selected;
    char *path;
    GtkFunction selectCallback;
    gchar buf[BUFSIZ];

    selected = GTK_TREE_SELECTION(tree);
    if (!selected) {
	return;
    }
    
    item = GTK_TREE_ITEM(selected->data);
    path = gtk_object_get_data(GTK_OBJECT(item), "URL");
    if (path) {
	selectCallback = gtk_object_get_data(GTK_OBJECT(tree),
					     "selectCallback");
	if (!strstr(path, ":"))
		g_snprintf(buf, sizeof(buf), "file:%s", path);
	else
		g_snprintf(buf, sizeof(buf), "%s", path);
	(selectCallback)(buf);
    }
}

void showToc(GtkWidget *window)
{
    gtk_widget_show(GTK_WIDGET(window));
}

void hideToc(GtkWidget *window)
{
    gtk_widget_hide(GTK_WIDGET(window));
}

static int hideTocInt(GtkWidget *window)
{
    gtk_widget_hide(GTK_WIDGET(window));

    return FALSE;
}

static void initToc(GtkWidget *toc, GtkSignalFunc selectCallback)
{
    GtkWidget *tree, *item, *subtree;

    tree = gtk_object_get_data(GTK_OBJECT(toc), "TheTree");

    gtk_signal_connect(GTK_OBJECT(tree), "selection_changed",
		       (GtkSignalFunc)newSelection, NULL);
    gtk_object_set_data(GTK_OBJECT(tree), "selectCallback", selectCallback);

    /* Add one new item for each type */
    /* This really should iterate through the toc_config list */

    /* MAN PAGES */
    
    item = gtk_tree_item_new_with_label(_("Man Pages"));
    gtk_tree_append(GTK_TREE(tree), item);
    gtk_signal_connect_object(GTK_OBJECT(item), "expand",
			      (GtkSignalFunc)expandManPagesRoot,
			      (gpointer)item);
    subtree = gtk_tree_new();
    gtk_tree_set_selection_mode(GTK_TREE(subtree), GTK_SELECTION_SINGLE);
    gtk_tree_item_set_subtree(GTK_TREE_ITEM(item), subtree);
    gtk_widget_show(item);

    /* INFO PAGES */
    
    item = gtk_tree_item_new_with_label(_("Info Pages"));
    gtk_tree_append(GTK_TREE(tree), item);
    gtk_signal_connect_object(GTK_OBJECT(item), "expand",
			      (GtkSignalFunc)expandInfoRoot,
			      (gpointer)item);
    subtree = gtk_tree_new();
    gtk_tree_set_selection_mode(GTK_TREE(subtree), GTK_SELECTION_SINGLE);
    gtk_tree_item_set_subtree(GTK_TREE_ITEM(item), subtree);
    gtk_widget_show(item);

    /* GNOME HELP */
    
    item = gtk_tree_item_new_with_label(_("GNOME Help"));
    gtk_tree_append(GTK_TREE(tree), item);
    gtk_signal_connect_object(GTK_OBJECT(item), "expand",
			      (GtkSignalFunc)expandGHelpRoot,
			      (gpointer)item);
    subtree = gtk_tree_new();
    gtk_tree_set_selection_mode(GTK_TREE(subtree), GTK_SELECTION_SINGLE);
    gtk_tree_item_set_subtree(GTK_TREE_ITEM(item), subtree);
    gtk_widget_show(item);
}

GtkWidget *createToc(GtkSignalFunc selectCallback)
{
    GtkWidget *window, *tree, *box, *scrolled_win;
    GtkWidget *button;

    /* Main Window */
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Gnome Help TOC");
    gnome_window_icon_set_from_default (GTK_WINDOW (window));
    gtk_widget_set_usize (window, 300, 200);
    /*gtk_widget_set_uposition (window, 20, 20);*/

    /* Vbox */
    box = gtk_vbox_new(FALSE, 5);
    gtk_container_set_border_width (GTK_CONTAINER (box), 5);
    gtk_container_add(GTK_CONTAINER(window), box);
    gtk_widget_show(box);

    /* Buttons */
    button = gtk_button_new_with_label("Collapse");
    gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 0);
    gtk_widget_show(button);
    
    /* Scrolled window */
    scrolled_win = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_win),
				    GTK_POLICY_AUTOMATIC,
				    GTK_POLICY_AUTOMATIC);
    gtk_box_pack_start(GTK_BOX(box), scrolled_win, TRUE, TRUE, 0);
    gtk_widget_show(scrolled_win);

    /* Tree */
    tree = gtk_tree_new();
    gtk_container_add(GTK_CONTAINER(scrolled_win), tree);
    gtk_tree_set_selection_mode(GTK_TREE(tree), GTK_SELECTION_SINGLE);
    gtk_widget_show(tree);

    gtk_object_set_data(GTK_OBJECT(window), "TheTree", tree);
    gtk_signal_connect_object(GTK_OBJECT(button), "clicked",
			      (GtkSignalFunc)collapseBranch, (gpointer)tree);

    gtk_signal_connect (GTK_OBJECT (window), "destroy",
			GTK_SIGNAL_FUNC(hideTocInt), NULL);
    gtk_signal_connect (GTK_OBJECT (window), "delete_event",
			GTK_SIGNAL_FUNC(hideTocInt), NULL);


    initToc(window, selectCallback);
    
    /* Show the whole thing */
    return window;
}

