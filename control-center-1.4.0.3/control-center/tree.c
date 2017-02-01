/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

/* Copyright (C) 1998 Redhat Software Inc.
 * Authors: Jonathan Blandford <jrb@redhat.com>
 */
#include "tree.h"
#include "capplet-manager.h"
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <strings.h>
#include <sys/stat.h>
#include <unistd.h>

/*GdkPixmap *pixmap1;
GdkPixmap *pixmap2;
GdkPixmap *pixmap3;
GdkBitmap *mask1;
GdkBitmap *mask2;
GdkBitmap *mask3;*/

extern gchar *init_cap;
extern GnomeAppBar *status_bar;
static void selected_row_callback (GtkWidget *widget, GtkCTreeNode *node, gint column, gpointer unseen);
static void unselected_row_callback (GtkCList *clist, gint row, gint column, GdkEvent *event);

#define SMALL_ICON_SIZE 20
#if 0
gint button_press (GtkCTree *ctree, GdkEventButton *event, gpointer data)
{
        if (event->button == 1 && event->type == GDK_2BUTTON_PRESS) {
                /* g_print ("stoping the button press...\n");
                   gtk_signal_emit_stop_by_name (GTK_OBJECT (ctree), "button_press_event");*/
        }
        return TRUE;
}
#endif
static void
init_tree_handler (GtkWidget *ctree, GdkEvent *event, node_data *data)
{
        gtk_signal_disconnect_by_data (GTK_OBJECT (ctree), data);
        gnome_appbar_pop (GNOME_APPBAR(status_bar));
        if (data->gde->comment)
                gnome_appbar_push (GNOME_APPBAR (status_bar), (data->gde->comment));
        else
                gnome_appbar_push (GNOME_APPBAR (status_bar), (data->gde->name));
        launch_capplet (data, TRUE);
}
static gint
compare_last_dir (gchar *first, gchar *second)
{
        gboolean retval;
        gchar *temp1;
        gchar *temp2;

        temp1 = strdup (first);
        temp2 = strdup (second);
        (rindex(temp1, '/'))[0] = 0;
        (rindex(temp2, '/'))[0] = 0;

        retval = !strcmp (rindex (temp1, '/'), rindex (temp2, '/'));
        g_free (temp1);
        g_free (temp2);
        return retval;
}
static gboolean
compare_nodes (GnomeDesktopEntry *data1, GnomeDesktopEntry *data2)
{
        g_return_val_if_fail (data1, FALSE);
        g_return_val_if_fail (data2, FALSE);
        g_return_val_if_fail (data1->type, FALSE);
        g_return_val_if_fail (data2->type, FALSE);

        if (!strcmp (data1->type, "Directory") &&
            (!strcmp (data2->type, "Directory")))
                return compare_last_dir (data1->location,
                                         data2->location);
        else
                return (!strcmp (rindex (data1->location,'/'),
                                 rindex (data2->location,'/')));
}
/*
 * This function is used to generate a node starting at a directory.
 * It doesn't do all that complex error checking -- if something
 * happens, it just returns null and skips the directory.
 *
 * It will try to use node1's data over node2's if possible, and will
 * write into node1's field.  It should handle all memory, so there is
 * no need to free stuff from node2 after the merger.
 */
void
merge_nodes (GNode *node1, GNode *node2)
{
        GNode *child1, *child2;

        if ((node1 == NULL) || (node2 == NULL))
                return;

        /* first we merge data */
        if (node1->data == NULL) {
                node1->data = node2->data;
        } else if (node2->data != NULL) {
                GnomeDesktopEntry *entry1;
                GnomeDesktopEntry *entry2;

                entry1 = (GnomeDesktopEntry *) node1->data;
                entry2 = (GnomeDesktopEntry *) node2->data;

                if (entry1->name == NULL && entry2->name != NULL)
                        entry1->name = g_strdup (entry2->name);

                gnome_desktop_entry_free (node2->data);
                node2->data = NULL;
        }

        /* now we want to find subdirs to merge */
        /* it's not incredibly effecient, but it works... */
        for (child1 = node1->children; child1; child1 = child1->next) {
                for (child2 = node2->children; child2; child2 = child2->next) {
                        if (child1->data == NULL || child2->data == NULL)
                                continue;

                        if (compare_nodes (child1->data, child2->data)) {
                                if (child2->prev == NULL)
                                        child2->parent->children = child2->next;
                                else
                                        child2->prev->next = child2->next;
                                merge_nodes (child1, child2);
                        }
                }
        }

        if (node2->children) {
                for (child2 = node2->children; child2->next; child2 = child2->next)
                        child2->parent = node1;
                child2->next = node1->children;
                child2->next->prev = child2;
                node1->children = node2->children;
                node2->children = NULL;
        }
}

GNode *
read_directory (gchar *directory)
{
        DIR *parent_dir;
        struct dirent *child_dir;
        struct stat filedata;
        GNode *retval = g_node_new(NULL);

        parent_dir = opendir (directory);
        if (parent_dir == NULL)
                return NULL;

        while ((child_dir = readdir (parent_dir)) != NULL) {
                if (child_dir->d_name[0] != '.') {

                        /* we check to see if it is interesting. */
                        GString *name = g_string_new (directory);
                        g_string_append (name, "/");
                        g_string_append (name, child_dir->d_name);

                        if (stat (name->str, &filedata) != -1) {
                                gchar* test;
                                if (S_ISDIR (filedata.st_mode)) {
                                        /* it might be interesting... */
                                        GNode *next_dir = read_directory (name->str);
                                        if (next_dir)
                                                /* it is interesting!!! */
                                                g_node_prepend (retval, next_dir);
                                }
                                test = rindex(child_dir->d_name, '.');
                                if (test && !strcmp (".desktop", test)) {
                                        /* it's a .desktop file -- it's interesting for sure! */
                                        GNode *new_node = g_node_new (gnome_desktop_entry_load (name->str));
                                        g_node_prepend (retval, new_node);
                                }
                        }
                        g_string_free (name, TRUE);
                }
                else if (!strcmp (child_dir->d_name, ".directory")) {
                        GString *name = g_string_new (directory);
                        g_string_append (name, "/.directory");
                        retval->data = gnome_desktop_entry_load (name->str);
                        g_string_free (name, TRUE);
                }

        }
        
        closedir (parent_dir);

        if (retval->data == NULL) {
                GnomeDesktopEntry *entry;

                /* No `.directory' file.  Create it from the name of the directory.  */

                entry = g_new (GnomeDesktopEntry, 1);
                entry->name          = NULL;
                entry->comment       = NULL;
                entry->exec_length   = 0;
                entry->exec          = NULL;
                entry->tryexec       = NULL;
                entry->icon          = NULL;   
                entry->docpath       = NULL;
                entry->terminal      = FALSE;
                entry->type          = g_strdup ("Directory");   
                entry->location      = g_concat_dir_and_file (directory, ".directory");
                entry->geometry      = NULL;
                entry->multiple_args = FALSE;
                entry->is_kde        = FALSE;

                retval->data = entry;
        }

        if (retval->children == NULL) {
                if (retval->data) {
                        gnome_desktop_entry_free (retval->data);
                        retval->data = NULL;
                }
                return NULL;
        }

        return retval;
}

static void
generate_tree_helper (GtkCTree *ctree, GtkCTreeNode *parent, GNode *node)
{
        GNode *i;
        gint j;
        GtkCTreeNode *child;
        char *text[2];
        node_data *data;
        gchar exec_collapsed[100];
        gchar *icon;
        GnomePixmap *icon_gpixmap;
        GdkPixmap *icon_pixmap, *icon_mask;


        text[1] = NULL;
        for (i = node;i;i = i->next) {
                if (!i->data)
                        continue;
                icon_pixmap=NULL;
                icon_mask=NULL;
                icon=NULL;
                icon_gpixmap = NULL;
                if (((GnomeDesktopEntry *)i->data)->name)
                        text[0] = ((GnomeDesktopEntry *)i->data)->name;
                else
                        text[0] = "*MISSINGNAME*";

                if (((GnomeDesktopEntry *)i->data)->icon)
                        icon = ((GnomeDesktopEntry *)i->data)->icon;
                if (icon && g_file_exists (icon))
                        icon_gpixmap = (GnomePixmap *)gnome_stock_pixmap_widget_at_size(NULL,
                                                                                        icon,
                                                                                        SMALL_ICON_SIZE,
                                                                                        SMALL_ICON_SIZE);
                if (icon_gpixmap) {
                        icon_pixmap = icon_gpixmap->pixmap;
                        icon_mask   = icon_gpixmap->mask;
                }

                if (!strcmp(((GnomeDesktopEntry *)i->data)->type,"Directory"))
                        child = gtk_ctree_insert_node (ctree,parent,NULL, text, 3,
                                                       icon_pixmap, icon_mask,
                                                       icon_pixmap, icon_mask, FALSE,FALSE);
                else
                        child = gtk_ctree_insert_node (ctree,parent,NULL, text, 3,
                                                       icon_pixmap, icon_mask,
                                                       icon_pixmap, icon_mask, TRUE,FALSE);
                data = g_malloc (sizeof (node_data));
                data->gde = (GnomeDesktopEntry *)i->data;
                data->socket = NULL;
                data->node = child;
                data->id = -1;
                data->capplet = NULL;
                data->ctree = ctree;
                data->child = NULL;
                data->modified = FALSE;
                data->state = CAPPLET_INACTIVE;
                data->icon = g_strdup (icon);
                gtk_ctree_node_set_row_data (ctree, child, data);
                exec_collapsed[0] = '\0';
                if (data->gde->exec && init_cap) {
                        for (j = 0; data->gde->exec[j]; j++)
                                strncat (exec_collapsed, data->gde->exec[j], 99);
                        if (!strcmp (exec_collapsed, init_cap)) {
/*
                                GtkCTreeNode *temp = child;

                                for (;temp;temp = GTK_CTREE_ROW (temp)->parent)
                                gtk_ctree_expand (ctree, temp);
*/
                                gtk_ctree_select (ctree, child);

                                gtk_signal_connect_after (GTK_OBJECT (ctree), "expose_event",
                                                          GTK_SIGNAL_FUNC (init_tree_handler),
                                                          data);
                        }
                }

                if (i->children)
                        generate_tree_helper (ctree, child, i->children);
                if (parent == NULL)
                        gtk_ctree_expand_recursive (ctree, child);
        }
}

static gboolean
add_directories_from_GNOME_PATH (GList **list)
{
        char *gnome_path;
        char **split_gnome_path;
        char **p;

        gnome_path = getenv ("GNOME_PATH");
        if (gnome_path == NULL)
                return FALSE;

        split_gnome_path = g_strsplit (gnome_path, ":", -1);

        for (p = split_gnome_path; *p != NULL; p++) {
                char *directory;

                directory = g_concat_dir_and_file (*p, "share/control-center");
                *list = g_list_prepend (*list, directory);
        }

        g_strfreev (split_gnome_path);

        return TRUE;
}

static GList *
get_directory_list (void)
{
        GList *list;

        list = NULL;

        list = g_list_prepend (list, gnome_unconditional_datadir_file ("control-center"));
        list = g_list_prepend (list, gnome_util_home_file ("control-center"));

        add_directories_from_GNOME_PATH (&list);

        return list;
}

static void
free_directory_list (GList *list)
{
        GList *p;

        for (p = list; p != NULL; p = p->next)
                g_free (p->data);

        g_list_free (list);
}

static GNode *
read_all_directories (GList *list)
{
        GNode *root_node;
        GList *p;

        root_node = NULL;

        for (p = list; p != NULL; p = p->next) {
                GNode *new_node;

                new_node = read_directory ((char *) p->data);

                if (new_node == NULL)
                        continue;

                if (root_node == NULL)
                        root_node = new_node;
                else
                        merge_nodes (root_node, new_node);
        }

        return root_node;
}

GtkWidget *
generate_tree (void)
{
        GtkWidget *retval;
        GNode *root_node;
        GList *directory_list;

        gtk_widget_push_colormap (gdk_imlib_get_colormap ());
        gtk_widget_push_visual (gdk_imlib_get_visual ());
        retval = gtk_ctree_new (1, 0);
#if 0
        /* I don't remember what this was for, but away it goes... -jrb */
        gtk_signal_connect (GTK_OBJECT (retval), "button_press_event",
                            GTK_SIGNAL_FUNC (button_press), NULL);
#endif
        /* First thing we want to do is to check directories to create the menus */

        gtk_clist_set_row_height(GTK_CLIST (retval),20);
        gtk_ctree_set_line_style (GTK_CTREE (retval), GTK_CTREE_LINES_DOTTED);
        gtk_ctree_set_expander_style (GTK_CTREE (retval), GTK_CTREE_EXPANDER_SQUARE);
        gtk_clist_set_column_width(GTK_CLIST (retval), 0, 150);

        gtk_ctree_set_indent (GTK_CTREE (retval), 15);
        gtk_clist_set_column_auto_resize (GTK_CLIST (retval), 0, TRUE);
        gtk_clist_set_selection_mode(GTK_CLIST(retval), GTK_SELECTION_SINGLE );
        gtk_signal_connect( GTK_OBJECT (retval),"tree_select_row", GTK_SIGNAL_FUNC (selected_row_callback), NULL);
        gtk_signal_connect( GTK_OBJECT (retval),"unselect_row", GTK_SIGNAL_FUNC (unselected_row_callback), NULL);

        directory_list = get_directory_list ();
        root_node = read_all_directories (directory_list);
        free_directory_list (directory_list);

        /* now we actually set up the tree... */
        /* we prolly want to use the gtree_insert_node function to do this,
         * but as it was written after the code here...
         *
         * we do user_node->children to avoid the root menu.
         */
        if (root_node != NULL) {
                generate_tree_helper (GTK_CTREE (retval), NULL, root_node->children);
        } else {
                g_warning ("No configuration applets were found.  This probably means that either the control-center or GNOME is incorrectly installed.\n");
                exit (1);
        }

        gtk_ctree_sort_recursive (GTK_CTREE (retval), NULL);

        gtk_widget_pop_colormap ();
        gtk_widget_pop_visual ();
        
        return retval;
}

static void
unselected_row_callback (GtkCList       *clist,
                         gint            row,
                         gint            column,
                         GdkEvent       *event)
{

        if (event != NULL) {
                gtk_clist_select_row (clist, row, column);
        }
}

static void
selected_row_callback (GtkWidget *widget, GtkCTreeNode *node, gint column, gpointer userdata)
{
        node_data *data;
        GnomeDesktopEntry *gde;
        GdkEvent *event = gtk_get_current_event();

        data = (node_data *) gtk_ctree_node_get_row_data (GTK_CTREE (widget),node);
        if (data == NULL)
                return;

        gde = (GnomeDesktopEntry *)data->gde;
        gnome_appbar_pop (GNOME_APPBAR (status_bar));
        if (gde->comment)
                gnome_appbar_push (GNOME_APPBAR (status_bar), (gde->comment));
        else
                gnome_appbar_push (GNOME_APPBAR (status_bar), (gde->name));

        /* && event->type == GDK_2BUTTON_PRESS) */
        if (event){
                GtkWidget *grabbing_widget;

                while ((grabbing_widget = gtk_grab_get_current ()) != NULL)
                        gtk_grab_remove (grabbing_widget);
                while (gtk_events_pending ())
                        gtk_main_iteration ();
                if (gdk_pointer_is_grabbed ())
                        gdk_pointer_ungrab (GDK_CURRENT_TIME);
                gdk_flush ();

                launch_capplet (data, TRUE);
        }
}

