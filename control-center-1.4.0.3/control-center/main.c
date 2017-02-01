/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/* Copyright (C) 1998 Redhat Software Inc. 
 * Authors: Jonathan Blandford <jrb@redhat.com>
 */
#include <config.h>
#include <gnome.h>
#include <gdk/gdkkeysyms.h>
#include <libgnomeui/gnome-window-icon.h>
#include <stdio.h>
#include "callbacks.h"
#include "tree.h"
#include "corba-glue.h"
#include "capplet-manager.h"
#include "gtk-multiview.h"
#include "gdk_imlib.h"
#include "splash.h"
GtkWidget *main_window;
GtkWidget *exit_dialog;
GtkWidget *hpane;
GtkWidget *multiview;
GtkWidget *splash_screen;
GtkWidget *tree;
GtkWidget *help_menu_item;

GnomeAppBar *status_bar;
/* We create a random vbox to stuff the socket in when necessary */
GtkWidget *hack_widget;
gchar *init_cap = NULL;

/* Prototypes */

GtkWidget * create_exit_dialog (GList *apps);

static GnomeUIInfo mainMenu[] = {
        GNOMEUIINFO_MENU_EXIT_ITEM(exit_callback, NULL), 
        GNOMEUIINFO_END
};
static GnomeUIInfo helpMenu[] = {

        GNOMEUIINFO_ITEM_STOCK(N_("Help on control-center"), N_("Help with the GNOME control-center."),
                               help_callback, GNOME_STOCK_PIXMAP_HELP),
        GNOMEUIINFO_ITEM_STOCK("", N_("Help with the current configuration page."),
                               NULL, GNOME_STOCK_PIXMAP_HELP),
        GNOMEUIINFO_SEPARATOR,
        GNOMEUIINFO_ITEM_STOCK(N_("About"), N_("About the GNOME control-center."),
                               about_callback, GNOME_STOCK_MENU_ABOUT),
        
        GNOMEUIINFO_END
};
static GnomeUIInfo parentMenu[] = {
        GNOMEUIINFO_MENU_FILE_TREE(mainMenu),

        GNOMEUIINFO_MENU_HELP_TREE(helpMenu),

        {GNOME_APP_UI_ENDOFINFO}
};


GtkWidget *
create_exit_dialog (GList *apps)
{
        gchar *text[2];
        GtkWidget *hbox;
        GtkWidget *right_vbox;
        GtkWidget *retval;
        GtkWidget *label;
        GtkWidget *list;
        GtkWidget *pixmap = NULL;
        gint i = 0;
        char *s;

        /* we create the widgets */
        retval = gnome_dialog_new (_("Warning:"),_("Discard all changes"), GNOME_STOCK_BUTTON_CANCEL, NULL);
        gnome_dialog_set_default (GNOME_DIALOG (retval), 1);
        gnome_dialog_set_parent (GNOME_DIALOG (retval), GTK_WINDOW (main_window));

        /*...containers */
        hbox = gtk_hbox_new (FALSE, 5);
        right_vbox = gtk_vbox_new (FALSE, 5);

        /*...labels, etc */
        label = gtk_label_new (_("The following modules have had changes made, but not committed.  " \
                                 "If you would like to edit them, please double click on the appropriate entry."));
        gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
        gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);

        /* stolen straight from gnome-message-box (: */
        s = gnome_pixmap_file("gnome-warning.png");
        if (s)
                pixmap = gnome_pixmap_new_from_file(s);
        if ( (pixmap == NULL) ||
             (GNOME_PIXMAP(pixmap)->pixmap == NULL) ) {
                if (pixmap) gtk_widget_destroy(pixmap);
                s = gnome_pixmap_file("gnome-default.png");
                if (s)
                        pixmap = gnome_pixmap_new_from_file(s);
                else
                        pixmap = NULL;
        }

        /*...the list */
        list = gtk_clist_new (1);
        gtk_clist_set_selection_mode (GTK_CLIST (list), GTK_SELECTION_BROWSE);
        /*gtk_clist_set_policy (GTK_CLIST (list), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);*/
        text[1] = NULL;
        for (;apps ;apps = apps->next) {
                text[0] = ((node_data*)apps->data)->gde->name;
                gtk_clist_append (GTK_CLIST (list), text);
                gtk_clist_set_row_data (GTK_CLIST (list), i++, apps->data);
        }
        gtk_signal_connect (GTK_OBJECT (list),"select_row", GTK_SIGNAL_FUNC (exit_row_callback), NULL);
        gnome_dialog_button_connect (GNOME_DIALOG (retval), 0, exit_dialog_ok_callback, NULL);
        gnome_dialog_button_connect (GNOME_DIALOG (retval), 1, exit_dialog_cancel_callback, NULL);
        gtk_signal_connect (GTK_OBJECT (retval), "destroy", exit_dialog_close_callback, NULL);
        
        /* and put it all together */
        gtk_box_pack_start (GTK_BOX (right_vbox), label, FALSE, FALSE, 5);
        gtk_box_pack_start (GTK_BOX (right_vbox), list, FALSE, FALSE, 5);
        if (pixmap)
                gtk_box_pack_start (GTK_BOX(hbox), pixmap, FALSE, TRUE, 10);
        gtk_box_pack_start (GTK_BOX ( GNOME_DIALOG (retval)->vbox), hbox, FALSE, FALSE, 5);
        gtk_box_pack_start (GTK_BOX (hbox), right_vbox, FALSE, FALSE, 5);

        /* and shake it all about... */
        gtk_widget_show (label);
        gtk_widget_show (list);
        if (pixmap)
                gtk_widget_show (pixmap);
        gtk_widget_show (right_vbox);
        gtk_widget_show (hbox);
        
        return retval;
}

static void
gtk_widget_set_uposition_internal (GtkWidget *widget,
                                   gint	     x,
                                   gint	     y)
{
        /* woo hoo -- we love copying code. */
        /* but we need to set the uposition to -10, -10 */
  GtkWidgetAuxInfo *aux_info;
  guint aux_info_key_id = 0;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_WIDGET (widget));

  aux_info = g_malloc (sizeof (GtkWidgetAuxInfo));
  aux_info->x = -1;
  aux_info->y = -1;
  aux_info->width = 0;
  aux_info->height = 0;

  aux_info_key_id = g_quark_from_static_string ("gtk-aux-info");
  gtk_object_set_data_by_id (GTK_OBJECT (widget), aux_info_key_id, aux_info);
  
  aux_info->x = x;
  aux_info->y = y;
  
  if (GTK_WIDGET_REALIZED (widget) && GTK_IS_WINDOW (widget))
    {
      gdk_window_set_hints (widget->window, aux_info->x, aux_info->y, 0, 0, 0, 0, GDK_HINT_POS);
      gdk_window_move (widget->window, aux_info->x, aux_info->y);
    }
  
  if (GTK_WIDGET_VISIBLE (widget) && widget->parent)
    gtk_widget_size_allocate (widget, &widget->allocation);
}

#if 0
/*
 * This is a helper function used to find the "System Information"
 * capplet in the capplet tree.
 */
static gint
find_guname_helper (gconstpointer a, gconstpointer b)
{
        node_data *nd = (node_data *) a;

        if (nd == NULL || nd->gde == NULL || nd->gde->exec == NULL)
                return 1;

        if (strstr (nd->gde->exec[0], "guname") != NULL)
                return 0;

        return 1;
}
#endif

static GtkWidget *
create_window ()
{
/*        GtkCTreeNode *guname_node;*/
        GtkWidget *retval;
        GtkWidget *vbox;
        GtkWidget *sw;
        GtkWidget *window;
        GtkWidget *container;
        
        /* create the app */
        retval = gnome_app_new ("control-center", _("Control Center"));
	gtk_signal_connect (GTK_OBJECT (retval), "delete_event",
                            GTK_SIGNAL_FUNC (exit_callback), retval);
	gnome_app_create_menus (GNOME_APP (retval), parentMenu);
        help_menu_item = helpMenu[1].widget;
        exit_dialog = NULL;

        /* Create the status bar */
        status_bar = GNOME_APPBAR(gnome_appbar_new(FALSE, TRUE,
                                               GNOME_PREFERENCES_USER));
	gnome_app_set_statusbar(GNOME_APP(retval), GTK_WIDGET(status_bar));

	gnome_app_install_menu_hints(GNOME_APP (retval), parentMenu);

        /* create the components */
        vbox = gtk_vbox_new (FALSE,0);
        hpane = gtk_hpaned_new();
        gtk_paned_handle_size (GTK_PANED (hpane), 10);
        gtk_paned_gutter_size (GTK_PANED (hpane), 10);
        tree = generate_tree ();
        sw = gtk_scrolled_window_new (GTK_CLIST (tree)->hadjustment, GTK_CLIST (tree)->vadjustment);
        /* This is sorta evil */
        gtk_widget_set_usize (sw, 220, -1);
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
        container = gtk_frame_new(NULL);
        gtk_container_set_border_width (GTK_CONTAINER (container), 0);
        gtk_frame_set_shadow_type (GTK_FRAME (container), GTK_SHADOW_NONE);
        multiview = gtk_multiview_new ();
        gtk_container_add (GTK_CONTAINER (container), multiview);

        splash_screen = create_splash_screen ();
        /*gdk_imlib_load_file_to_pixmap ("splash.png", &temp_splash, NULL);*/

        /* we put it all together... */
        gtk_container_add (GTK_CONTAINER (sw), tree);
        gtk_paned_add1 (GTK_PANED (hpane), sw);
        gtk_paned_add2 (GTK_PANED (hpane), container); 
        gtk_container_add (GTK_CONTAINER (multiview), splash_screen);
        gtk_multiview_set_current (GTK_MULTIVIEW (multiview), splash_screen);
        gtk_box_pack_end (GTK_BOX (vbox), hpane, TRUE, TRUE, 0);
        gnome_app_set_contents(GNOME_APP(retval), vbox);
        
        /* and make everyting visible */
        gtk_widget_show_all (retval);
        gtk_widget_hide (help_menu_item);
        /*gdk_window_set_back_pixmap (splash_screen->window, temp_splash, FALSE); */

        /* THIS IS AN UGLY HACK */
        /* We create a window -- override redirect and off screen, to dump
         * our unused sockets in.  We do this b/c reparenting a hidden socket
         * causes trouble */
        window = gtk_window_new (GTK_WINDOW_POPUP);
        hack_widget = gtk_vbox_new (FALSE, 0);
        gtk_widget_set_usize (window, 100, 100);
        gtk_widget_set_uposition_internal (window, -300, -300);
        gtk_widget_show_all (window);

#if 0
        /*
         * If there is an entry for guname in the tree, launch it as
         * the default capplet.
         */
        /* I don't like this.  Plus, it has a race condition.  Commented out for now - jrb */
        guname_node = gtk_ctree_find_by_row_data_custom (GTK_CTREE (tree), NULL, NULL, find_guname_helper);
        if (guname_node != NULL) {
                node_data *guname_nd;

                guname_nd = gtk_ctree_node_get_row_data (GTK_CTREE (tree), guname_node);
                launch_capplet (guname_nd, TRUE);
        }
#endif

        return retval;
}



gint
main (int argc, char *argv[])
{
	bindtextdomain(PACKAGE, GNOMELOCALEDIR);
	textdomain(PACKAGE);

        control_center_corba_gtk_init(&argc,argv);
        gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/control-center.png");
        main_window = create_window ();
        control_center_corba_gtk_main (&argc, argv);
        return 0;
}
