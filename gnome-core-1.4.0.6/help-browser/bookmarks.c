#include <config.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <glib.h>
#include <libgnome/gnome-defs.h>
#include <libgnome/gnome-i18n.h>
#include <libgnomeui/gnome-window-icon.h>
#include <gtk/gtk.h>

#include "bookmarks.h"

struct _bookmarks_struct {
    GtkWidget *window;
    GtkWidget *clist;
    GHashTable *table;
    BookmarksCB callback;
    gpointer data;
    gchar *file;
};

struct _bookmarks_entry {
    /* Someday this will have more info :-) */
    gchar *ref;
    gchar *title;
};

static void createBookmarksWindow(Bookmarks b,
				GtkWidget **window, GtkWidget **clist);
static void mouseDoubleClick(GtkCList *clist, gint row, gint column,
			     gint button, Bookmarks b);
static void freeEntry(gchar *key, struct _bookmarks_entry *val, gpointer bar);
static int hideBookmarksInt(GtkWidget *window);
static void loadBookmarks(Bookmarks b);
static void appendEntry(Bookmarks b, gchar *ref, gchar *title);
static void removeBookmark(GtkWidget *w, Bookmarks b);

Bookmarks newBookmarks(BookmarksCB callback, gpointer data, gchar *file)
{
    Bookmarks res;

    res = g_new0(struct _bookmarks_struct, 1);
    res->table = g_hash_table_new(g_str_hash, g_str_equal);
    res->file = NULL;
    reconfigBookmarks(res, callback, data, file);

    createBookmarksWindow(res, &res->window, &res->clist);

    loadBookmarks(res);

    return res;
}

void reconfigBookmarks(Bookmarks b, BookmarksCB callback,
		       gpointer data, gchar *file)
{
    gchar filename[BUFSIZ];
    
    b->callback = callback;
    b->data = data;
    if (b->file) {
	g_free(b->file);
    }
    if (file) {
	if (*(file) != '/') {
	    g_snprintf(filename, sizeof(filename), "%s/%s",
		       getenv("HOME"), file);
	} else {
	    g_snprintf(filename, sizeof(filename), "%s", file);
	}
        b->file = g_strdup(filename);
    } else {
	b->file = NULL;
    }
}

static void loadBookmarks(Bookmarks b)
{
    gchar buf[BUFSIZ];
    gchar ref[BUFSIZ];
    gchar title[BUFSIZ];
    FILE *f;
    
    if (! b->file) {
	return;
    }

    if (!(f = fopen(b->file, "r"))) {
	return;
    }

    while (fgets(buf, sizeof(buf), f)) {
	sscanf(buf, "%s %s", ref, title);
	appendEntry(b, ref, title);
    }

    fclose(f);
}

void saveBookmarks(Bookmarks b)
{
    struct _bookmarks_entry *entry;
    gint x;
    FILE *f;
    
    if (! b->file) {
	return;
    }

    if (!(f = fopen(b->file, "w"))) {
	return;
    }

    x = 0;
    while (x < GTK_CLIST(b->clist)->rows) {
	entry = gtk_clist_get_row_data(GTK_CLIST(b->clist), x);
	fprintf(f, "%s %s\n", entry->ref, entry->title);
	x++;
    }

    fclose(f);
}

static void appendEntry(Bookmarks b, gchar *ref, gchar *title)
{
    struct _bookmarks_entry *entry;
    gchar  *buf[2];
    gint x;
    
    entry = g_new0(struct _bookmarks_entry, 1);
    entry->ref = g_strdup(ref);
    entry->title = g_strdup(title);
    g_hash_table_insert(b->table, entry->ref, entry);

    buf[0] = ref;
    buf[1] = title;
    x = gtk_clist_append(GTK_CLIST(b->clist), buf);
    gtk_clist_set_row_data(GTK_CLIST(b->clist), x, entry);
}

static void freeEntry(gchar *key, struct _bookmarks_entry *val, gpointer bar)
{
    g_free(key);
    g_free(val);
}

void destroyBookmarks(Bookmarks b)
{
    /* Destroy the window */
    g_hash_table_foreach(b->table, (GHFunc)freeEntry, NULL);
    g_hash_table_destroy(b->table);
    if (b->file) {
	g_free(b->file);
    }
    g_free(b);
}

void addToBookmarks(Bookmarks b, gchar *ref, gchar *title)
{
    struct _bookmarks_entry *entry;
    
    entry = g_hash_table_lookup(b->table, ref);
    if (entry) {
	return;
    }

    if (!title)
	appendEntry(b, ref, "");
    else    
        appendEntry(b, ref, title);
}

static void mouseDoubleClick(GtkCList *clist, gint row, gint column,
			     gint button, Bookmarks b)
{
    struct _bookmarks_entry *entry;

    entry = gtk_clist_get_row_data(GTK_CLIST(clist), row);
    if (b->callback) {
	(b->callback)(entry->ref);
    }
}

static void removeBookmark(GtkWidget *w, Bookmarks b)
{
    gint row;
    GList *list;

    list = GTK_CLIST (b->clist)->selection;
    while (list)
      {
	row = GPOINTER_TO_INT (list->data);
	list = list->next;
	
	gtk_clist_remove (GTK_CLIST (b->clist), row);
      }
}

void showBookmarks(Bookmarks b)
{
    gtk_widget_show(GTK_WIDGET(b->window));
}

void hideBookmarks(Bookmarks b)
{
    gtk_widget_hide(GTK_WIDGET(b->window));
}

static int hideBookmarksInt(GtkWidget *window)
{
    gtk_widget_hide(GTK_WIDGET(window));

    return TRUE;
}

static void createBookmarksWindow(Bookmarks b, GtkWidget **window,
				  GtkWidget **clist)
{
    GtkWidget *box, *button, *sw;
    gchar *titles[2] = { N_("Bookmark"), N_("Page Title") };
    static int translated;

    if (!translated){
	    titles [0] = _(titles [0]);
	    titles [1] = _(titles [1]);
	    translated = 1;
    }
    /* Main Window */
    *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(*window), _("Gnome Help Bookmarks"));
    gnome_window_icon_set_from_default (GTK_WINDOW (*window));
    gtk_widget_set_usize (*window, 500, 200);

    /* Vbox */
    box = gtk_vbox_new(FALSE, 5);
    gtk_container_set_border_width (GTK_CONTAINER (box), 5);
    gtk_container_add(GTK_CONTAINER(*window), box);
    gtk_widget_show(box);

    /* Buttons */
    button = gtk_button_new_with_label(_("Remove"));
    gtk_box_pack_start(GTK_BOX(box), button, FALSE, FALSE, 0);
    gtk_widget_show(button);

    /* The clist */
    *clist = gtk_clist_new_with_titles(2, titles);
    gtk_clist_set_selection_mode(GTK_CLIST(*clist), GTK_SELECTION_SINGLE);
#ifndef GTK_HAVE_FEATURES_1_1_4
    gtk_clist_set_policy(GTK_CLIST(*clist),
			 GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
#endif
    gtk_clist_column_titles_show(GTK_CLIST(*clist));
    gtk_clist_column_titles_passive(GTK_CLIST(*clist));
    gtk_clist_set_column_justification(GTK_CLIST(*clist), 0,
				       GTK_JUSTIFY_LEFT);
    gtk_clist_set_column_width(GTK_CLIST(*clist), 0, 280);
    gtk_clist_set_column_justification(GTK_CLIST(*clist), 1,
				       GTK_JUSTIFY_LEFT);
    gtk_clist_set_column_width(GTK_CLIST(*clist), 1, 280);

#ifdef GTK_HAVE_FEATURES_1_1_4
    sw = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw),
				   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(sw), *clist);
    gtk_box_pack_start(GTK_BOX(box), sw, TRUE, TRUE, 0);
    gtk_widget_show(sw);
#else
    gtk_box_pack_start(GTK_BOX(box), *clist, TRUE, TRUE, 0);
#endif
    gtk_widget_show(*clist);

    /* Set callbacks */
    gtk_signal_connect(GTK_OBJECT (button), "clicked",
		       GTK_SIGNAL_FUNC(removeBookmark), b);
    gtk_signal_connect(GTK_OBJECT (*window), "destroy",
		       GTK_SIGNAL_FUNC(hideBookmarksInt), NULL);
    gtk_signal_connect(GTK_OBJECT (*window), "delete_event",
		       GTK_SIGNAL_FUNC(hideBookmarksInt), NULL);
    gtk_signal_connect_after(GTK_OBJECT(*clist), "select_row",
		       GTK_SIGNAL_FUNC(mouseDoubleClick), b);
}

