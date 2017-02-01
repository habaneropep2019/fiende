/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
#include <config.h>
#include "capplet-manager.h"
#include "callbacks.h"
#include "control-center.h"
#include <gnome.h>
#include <libgnomeui/gnome-window-icon.h>
#include <gdk/gdkx.h>
#include "gtk-multiview.h"

/* variables */
extern GtkWidget *multiview;
extern GtkWidget *splash_screen;
extern GtkWidget *hack_widget;
extern GtkWidget *help_menu_item;
extern GtkWidget *main_window;

extern gchar *ior;
static gint page_count = 0;
static gint id = 0;
gint destroying = FALSE;
GList *capplet_list = NULL;
extern CORBA_Environment ev;


/* typedefs... */
typedef struct _capplet_entry capplet_entry;
struct _capplet_entry
{
        GtkWidget *socket;
        CORBA_Object *cco;
        gboolean alive;
};
typedef struct _timer_entry timer_entry;
struct _timer_entry
{
        /* we don't use nd->socket; b/c it can change, and we want one per timer_entry */
        GtkWidget *socket;
        gint count;
        guint timer_id;
};
/* prototypes */
static void try_button_callback(GtkWidget *widget, gpointer data);
static void revert_button_callback(GtkWidget *widget, gpointer data);
static void ok_button_callback(GtkWidget *widget, gpointer data);
static void cancel_button_callback(GtkWidget *widget, gpointer data);
static void close_capplet (gboolean show_splash, gpointer data);
static void exec_capplet (node_data *data);
static void shutdown_capplet (node_data *nd);
static void capplet_close_callback (GtkWidget *widget, gpointer data);
static void capplet_close_callback2 (GtkWidget *widget, gpointer data);

static void
splash_size_allocate (GtkWidget *splash, GtkAllocation *alloc)
{
        static gint x = 0;
        static gint y = 0;

        x = MAX (x, GTK_WIDGET (splash)->requisition.width);
        y = MAX (y, GTK_WIDGET (splash)->requisition.height);
}


node_data *
find_node_by_id (gint id)
{
        GList *test;

        for (test = capplet_list; test; test = test->next)
                if (((node_data*)test->data)->id == id)
                        return (node_data*)test->data;
        return NULL;
                    
}
static void
exec_capplet (node_data *data)
{
        gchar *temp;
        gint i;
        gchar *argv[4];
        GList *list;
        /* is the silly thing a multi-capplet */
        for (i = 1;data->gde->exec[i];i++) { 
                if (strstr (data->gde->exec[i], "--cap-id=")) {
                        for (list = capplet_list; list; list = list->next) {
                                if (strcmp (((node_data *)list->data)->gde->exec[0], data->gde->exec[0]) == 0) {
                                /* do multi-capplet stuff... */
                                        data->capplet = CORBA_Object_duplicate (((node_data *)list->data)->capplet, &ev);
                                        capplet_list = g_list_prepend (capplet_list, data);
                                        GNOME_capplet_new_multi_capplet(data->capplet,
                                                                        ((node_data *)list->data)->id,
                                                                        data->id,
                                                                        GDK_WINDOW_XWINDOW (data->socket->window),
                                                                        atoi (data->gde->exec[i] + 9),
                                                                        &ev);
                                        data->state = CAPPLET_ACTIVE;
                                        return; 
                                }
                        }
                }
        }


        /* set up the arguments for the capplet */
        temp = g_malloc (sizeof (char[11]));
        sprintf (temp, "--id=");
        sprintf (temp + 5, "%d", data->id);
        argv[0] = temp;

        temp = g_malloc (sizeof (char[17]));
        sprintf (temp, "--xid=");
        sprintf (temp + 6, "%ld",  GDK_WINDOW_XWINDOW (data->socket->window));
        argv[1] = temp;

        argv[2] = NULL;
        /*argv[3] = "--gtk-module=gle";*/
        capplet_list = g_list_prepend (capplet_list, data);
/*        g_print ("\ngdb %s\nb main\nr %s %s\n", data->gde->exec[0], argv[0], argv[1]);*/
        
        gnome_desktop_entry_launch_with_args (data->gde, 2, argv);
        data->state = CAPPLET_UNREGISTERED;
        g_free (argv[0]);
        g_free (argv[1]);
}
void
launch_capplet (node_data *data, gboolean exec_new)
{
        GtkWidget *vbox;
        GtkWidget *separator;
        GtkWidget *bbox;
        GtkWidget *frame;
        GtkWidget *page;
        GList *temp;
        gboolean show_splash = TRUE;
        gchar *help_string;
        guint signal_id;
        
        /* If the page has not been changed, nuke it, or tell it to hide itself */
        if ((page_count > 0) && (data->gde->exec_length)) {
                page = gtk_multiview_get_current (GTK_MULTIVIEW (multiview));
                for (temp = capplet_list; temp; temp = temp->next)
                        if (temp->data && ((node_data*)temp->data)->child == page) {
                                if (((node_data*)temp->data)->id == data->id) {
                                        return;
                                } else if (((node_data*)temp->data)->modified == FALSE) {
                                        show_splash = FALSE;
                                        if (((node_data*)temp->data)->capplet) {
                                                GNOME_capplet_cancel (((node_data*)temp->data)->capplet,
                                                                      ((node_data*)temp->data)->id,
                                                                      &ev);
                                                ((node_data*)temp->data)->capplet = NULL;
                                        }
                                        close_capplet (show_splash, (node_data*)temp->data);
                                } else {
                                        GNOME_capplet_page_hidden (((node_data*)temp->data)->capplet,
                                                                   ((node_data*)temp->data)->id,
                                                                   &ev);
                                }
                                break;
                        }
        }
        /* set up the notebook if needed */
        /* This capplet has not been started yet.  We need to do that. */
        if ((data->id == -1) && (data->gde->exec_length)) {
                GtkWidget *pixmap;
                vbox = gtk_vbox_new(FALSE, 0);
                gtk_signal_connect_after (GTK_OBJECT (vbox), "size_allocate", splash_size_allocate, NULL);
                
                data->socket = gtk_socket_new ();
                gtk_signal_connect (GTK_OBJECT (data->socket), "destroy", capplet_close_callback, data);
                separator = gtk_hseparator_new ();
                bbox = gtk_hbutton_box_new ();
                gtk_button_box_set_layout (GTK_BUTTON_BOX (bbox), GTK_BUTTONBOX_SPREAD);
                gtk_button_box_set_spacing (GTK_BUTTON_BOX (bbox), 5);
                gtk_button_box_set_child_size (GTK_BUTTON_BOX (bbox), 5, -1);
                gtk_container_set_border_width (GTK_CONTAINER (bbox), 5);
                
                pixmap = gnome_stock_pixmap_widget (NULL, GNOME_STOCK_PIXMAP_JUMP_TO);
                data->try_button = gnome_pixmap_button (pixmap, _("Try"));
                gtk_widget_set_sensitive (data->try_button, FALSE);
                gtk_container_add (GTK_CONTAINER (bbox), data->try_button);
                gtk_signal_connect (GTK_OBJECT (data->try_button), "clicked", GTK_SIGNAL_FUNC (try_button_callback), data);

                pixmap = gnome_stock_pixmap_widget (NULL, GNOME_STOCK_PIXMAP_REVERT);
                data->revert_button = gnome_pixmap_button (pixmap, _("Revert"));
                gtk_widget_set_sensitive (data->revert_button, FALSE);
                gtk_container_add (GTK_CONTAINER (bbox), data->revert_button);
                gtk_signal_connect (GTK_OBJECT (data->revert_button), "clicked", GTK_SIGNAL_FUNC (revert_button_callback), data);

                data->ok_button = gnome_stock_button (GNOME_STOCK_BUTTON_OK);
                gtk_widget_set_sensitive (data->ok_button, FALSE);
                gtk_container_add (GTK_CONTAINER (bbox), data->ok_button);
                gtk_signal_connect (GTK_OBJECT (data->ok_button), "clicked", GTK_SIGNAL_FUNC (ok_button_callback), data);

                data->cancel_button = gnome_stock_button (GNOME_STOCK_BUTTON_CANCEL);
                gtk_container_add (GTK_CONTAINER (bbox), data->cancel_button);
                gtk_signal_connect (GTK_OBJECT (data->cancel_button), "clicked", GTK_SIGNAL_FUNC (cancel_button_callback), data);
                /* put it all together */
                frame = gtk_frame_new (NULL);
                gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);
                gtk_container_set_border_width (GTK_CONTAINER (frame), GNOME_PAD_SMALL);
                gtk_container_add (GTK_CONTAINER (frame), data->socket);
                gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);
                gtk_box_pack_end (GTK_BOX (vbox), bbox, FALSE, FALSE, 0);
                gtk_box_pack_end (GTK_BOX (vbox), separator, FALSE, FALSE, GNOME_PAD_SMALL);

                /* set the help item */
                help_string = g_strdup_printf (_("Help with '%s' settings"),
                                               data->gde->name);
                gtk_label_set_text (GTK_LABEL (GTK_BIN (help_menu_item)->child),
                                    help_string);
                signal_id = (guint) gtk_object_get_data (GTK_OBJECT (help_menu_item), "help_signal_id");
                if (signal_id != 0)
                        gtk_signal_disconnect (GTK_OBJECT (help_menu_item), signal_id);
                signal_id = gtk_signal_connect (GTK_OBJECT (help_menu_item), "activate",
                                                item_help_callback, data);
                gtk_object_set_data (GTK_OBJECT (help_menu_item), "help_signal_id", (gpointer) signal_id);
                g_free (help_string);
                gtk_widget_show (help_menu_item);
                
                /* this is in case we ever go back to tabs:
                 * We want to keep tabs on this so we can change the text. */
                data->label = gtk_label_new (data->gde->name);
                gtk_multiview_append_child (GTK_MULTIVIEW (multiview), vbox);
                gtk_multiview_set_current (GTK_MULTIVIEW (multiview), vbox);
                gtk_widget_show_all (vbox);
                page_count++;
                data->child = vbox;
                data->id = id++;

                if (exec_new) {
                        exec_capplet (data);
                } else {
                        capplet_list = g_list_prepend (capplet_list, data);
                        gtk_ctree_select (data->ctree, data->node);
                }
        }
        if (data->child != NULL) {
                gtk_multiview_set_current (GTK_MULTIVIEW (multiview), data->child);
                if (data->icon)
                        gnome_window_icon_set_from_file (GTK_WINDOW (main_window),
                                                         data->icon);
                if (data->capplet && exec_new) {
                        GNOME_capplet_page_shown (data->capplet,
                                                  data->id,
                                                  &ev);
                }
                gtk_ctree_expand_recursive (data->ctree, data->node);
                gtk_ctree_select (data->ctree, data->node);
        }
}
void
revert_all (void)
{
        GList *list;
        node_data *nd;

        for (list = capplet_list; list; list = list->next) {
                nd = (node_data *) list->data;
                if (nd && nd->capplet)
                        GNOME_capplet_cancel (nd->capplet,nd->id, &ev);
        }
}


static void
try_button_callback(GtkWidget *widget, gpointer data)
{
        node_data *nd = (node_data *) data;
        if  (!nd->capplet)
                return;
        gtk_widget_set_sensitive (nd->try_button, FALSE);
        GNOME_capplet_try (nd->capplet,nd->id, &ev);
}
static void
revert_button_callback(GtkWidget *widget, gpointer data)
{
        node_data *nd = (node_data *) data;
        if (!nd->capplet)
                return;
        GNOME_capplet_revert (nd->capplet,nd->id, &ev);
        gtk_widget_set_sensitive (nd->try_button, FALSE);
        gtk_widget_set_sensitive (nd->revert_button, FALSE);
}
static void
ok_button_callback(GtkWidget *widget, gpointer data)
{
        GtkStyle *style;
        node_data *nd = (node_data *) data;
        style = gtk_widget_get_style (GTK_WIDGET (nd->ctree));

        if (nd->capplet)
                GNOME_capplet_ok (nd->capplet,nd->id, &ev);
        close_capplet (TRUE, data);
        gtk_ctree_node_set_foreground (nd->ctree, nd->node, &style->fg[GTK_STATE_NORMAL]);
}
static void
cancel_button_callback(GtkWidget *widget, gpointer data)
{
        GtkStyle *style;
        node_data *nd = (node_data *) data;
        style = gtk_widget_get_style (GTK_WIDGET (nd->ctree));

        if (nd->capplet)
                GNOME_capplet_cancel (nd->capplet,nd->id, &ev);
        close_capplet (TRUE, data);
        gtk_ctree_node_set_foreground (nd->ctree, nd->node, &style->fg[GTK_STATE_NORMAL]);
}
static void
capplet_close_callback (GtkWidget *widget, gpointer data)
{
        /* This function tries to clean up correctly. */
        /* If a capplet dies, it will (try to) shut it down gracefully... */
        /* It assumes that the capplet was not reparented already */
        node_data *nd = (node_data *) data;
        GtkStyle *style;

        if (destroying)
                return;

        if (nd->socket != NULL) {
                nd->socket = NULL;
                close_capplet (TRUE, nd);
                nd->state = CAPPLET_INACTIVE;
                style = gtk_widget_get_style (GTK_WIDGET (nd->ctree));

                if (style) {
                        gtk_ctree_node_set_foreground (nd->ctree, nd->node, &style->fg[GTK_STATE_NORMAL]);
                }
        }
}
static void
capplet_close_callback2 (GtkWidget *widget, gpointer data)
{
        /* This function tries to clean up correctly as well. */
        /* it has less to clean up though. */
        /* it needs to stop the timer callback. */
        timer_entry *te = (timer_entry *) data;
        if (te->timer_id)
                gtk_timeout_remove (te->timer_id);
        g_free (te);
}
static gint
timer_callback (gpointer data)
{
        timer_entry *te = (timer_entry *) data;

        /* I don't know if this can happen, but just to be absolutely
         * safe, we test it. */
        if ((GTK_SOCKET (te->socket)->plug_window != NULL)
            || te->count == 0) {
                gtk_widget_destroy (te->socket);
                te->timer_id = 0;
                /* capplet_close_callback2 will handle freeing te */
                return 0;
        }
        te->count--;
        return 1;
}
static void
queue_socket_destruction (node_data *nd)
{
        timer_entry *te;

        te = g_new (timer_entry, 1);
        te->socket = nd->socket;
        te->count = 15;
        te->timer_id = gtk_timeout_add (2000, timer_callback, te);
        gtk_widget_reparent (nd->socket, hack_widget);
        gtk_signal_disconnect_by_data (GTK_OBJECT (nd->socket),
                                       nd);
        gtk_signal_connect (GTK_OBJECT (nd->socket), "destroy", capplet_close_callback2, te);
}
static void
shutdown_capplet (node_data *nd) {
       if (nd->state == CAPPLET_ACTIVE &&
           nd->socket && GTK_SOCKET (nd->socket)->plug_window == NULL) {
               /* Uh oh... We are shutting down a capplet that hasn't realized
                * yet.  We need to reparent the socket; hide it, and, when it's
                * realized, have it go away. */
               /* To do this, we reparent the socket and hide it.
                * When the plug dies, so goes the socket */
               queue_socket_destruction (nd);
       }
       nd->socket = NULL;
       gtk_container_remove (GTK_CONTAINER (multiview), nd->child);
       nd->id = -1;
       nd->modified = FALSE;
       nd->state = CAPPLET_INACTIVE;
       capplet_list = g_list_remove (capplet_list, nd);
       if (nd->capplet) {
               CORBA_Object_release (nd->capplet, &ev);
               nd->capplet = NULL;
       }
       nd->child = NULL;
}
/* If show_splash is true, then that means that we want to handle whatever is underneath
 * (ie. putting the splash_screen back, changing the selected node etc. )
 * Otherwise we ignore what's underneath (ie. we're about to launch another one.) */
static void
close_capplet (gboolean show_splash, gpointer data)
{
        node_data *nd = (node_data *) data;
        GList *temp;

        shutdown_capplet (nd);
        if (--page_count == 0) {
                if (show_splash) {
                        gtk_multiview_set_current (GTK_MULTIVIEW (multiview), splash_screen);
                        gnome_window_icon_set_from_default (GTK_WINDOW (main_window));
                        gtk_widget_hide (help_menu_item);
                }
        } else if (show_splash) {
                /* we need to find the current page, and highlight it. */
                GtkWidget *page;

                page = gtk_multiview_get_current (GTK_MULTIVIEW (multiview));
                for (temp = capplet_list; temp; temp = temp->next)
                        if (((node_data *) temp->data)->child == page) {
                                GtkCTreeNode *ctnode;
                                gtk_ctree_select (((node_data *) temp->data)->ctree,
                                                  ((node_data *) temp->data)->node);
                                for (ctnode = ((node_data *) temp->data)->node;
                                     ctnode; ctnode = GTK_CTREE_ROW (ctnode)->parent) {
                                        gtk_ctree_expand (((node_data *) temp->data)->ctree,
                                                          ((node_data *) temp->data)->node);
                                }
                                GNOME_capplet_page_shown (((node_data*)temp->data)->capplet,
                                                          ((node_data*)temp->data)->id,
                                                          &ev);

                        }
        }
}
