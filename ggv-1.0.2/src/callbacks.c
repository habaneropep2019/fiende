/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

#include <config.h>
#include <gnome.h>
#include <gdk/gdkkeysyms.h>
#include <sys/stat.h>

#include "gtkscrollpane.h"
#include "gtkchecklist.h"
#include "gtkgs.h"
#include "ggvutils.h"
#include "prefs.h"
#include "ggvwindow.h"
#include "gsmessage.h"
#include "callbacks.h"

static void save_window_size(GtkWidget *window, gint *w, gint *h)
{
        gdk_window_get_geometry(window->window, NULL, NULL, w, h, NULL);
}

void recent_callback(GtkWidget *w, gpointer *data)
{
        char *error_msg;
        window_with_data *wwd = (window_with_data *) data;
        struct stat stat_rec;

        if (stat((char*)wwd->data, &stat_rec) == 0) {
                if (S_ISREG(stat_rec.st_mode)) {
                        load_gs(wwd->ggv, (char*)wwd->data);
                        return;
                }
                error_msg = g_strdup_printf(_("%s is not a regular file."), (char*)wwd->data);

        }
        else {
                error_msg = g_strdup_printf(_("%s does not exist."), (char*)wwd->data);
        }

        error_message(wwd->ggv,error_msg);
}


void unmark_all_pages_callback(GtkWidget *widget, gpointer data) 
{
        gint i, n;
        ggv_window *ggv = (ggv_window *) data;

        g_return_if_fail (ggv!=NULL);
        if((!GTK_GS(ggv->gs)->loaded) || (!GTK_GS(ggv->gs)->structured_doc))
                return;

        n = gtk_gs_document_numpages (GTK_GS (ggv->gs));
        gtk_clist_freeze(GTK_CLIST(ggv->pagelist));
        for (i = 0; i < n; i++)
                gtk_check_list_set_toggled (GTK_CHECK_LIST (ggv->pagelist), i, FALSE);
        gtk_clist_thaw(GTK_CLIST(ggv->pagelist));
}

void toggle_current_page_callback(GtkWidget *widget, gpointer data) 
{
        ggv_window *ggv = (ggv_window *) data;

        g_return_if_fail (ggv!=NULL);
        if((!GTK_GS(ggv->gs)->loaded) || (!GTK_GS(ggv->gs)->structured_doc))
                return;

        gtk_check_list_toggle_row (GTK_CHECK_LIST (ggv->pagelist),
                                   GTK_GS (ggv->gs)->current_page);
}

void toggle_even_pages_callback(GtkWidget *widget, gpointer data) 
{
        gint i, n;
        ggv_window *ggv = (ggv_window *) data;
        gboolean checked;

        g_return_if_fail (ggv!=NULL);
        if((!GTK_GS(ggv->gs)->loaded) || (!GTK_GS(ggv->gs)->structured_doc))
                return;

        n = gtk_gs_document_numpages (GTK_GS (ggv->gs));
        gtk_clist_freeze(GTK_CLIST(ggv->pagelist));
        for (i = 1; i < n; i += 2) {
                checked = gtk_check_list_get_toggled(GTK_CHECK_LIST (ggv->pagelist), i);
                gtk_check_list_set_toggled (GTK_CHECK_LIST (ggv->pagelist), i, !checked);
        }
        gtk_clist_thaw(GTK_CLIST(ggv->pagelist));
}

void toggle_odd_pages_callback(GtkWidget *widget, gpointer data) 
{
        gint i, n;
        ggv_window *ggv = (ggv_window *) data;
        gboolean checked;

        g_return_if_fail (ggv!=NULL);
        if((!GTK_GS(ggv->gs)->loaded) || (!GTK_GS(ggv->gs)->structured_doc))
                return;

        n = gtk_gs_document_numpages (GTK_GS (ggv->gs));
        gtk_clist_freeze(GTK_CLIST(ggv->pagelist));
        for (i = 0; i < n; i += 2) {
                checked = gtk_check_list_get_toggled(GTK_CHECK_LIST (ggv->pagelist), i);
                gtk_check_list_set_toggled (GTK_CHECK_LIST (ggv->pagelist), i, !checked);
        }
        gtk_clist_thaw(GTK_CLIST(ggv->pagelist));
}

void toggle_all_pages_callback(GtkWidget *widget, gpointer data) 
{
        gint i, n;
        ggv_window *ggv = (ggv_window *) data;
        gboolean checked;

        g_return_if_fail (ggv!=NULL);
        if((!GTK_GS(ggv->gs)->loaded) || (!GTK_GS(ggv->gs)->structured_doc))
                return;

        n = gtk_gs_document_numpages (GTK_GS (ggv->gs));
        gtk_clist_freeze(GTK_CLIST(ggv->pagelist));
        for (i = 0; i < n; i++) {
                checked = gtk_check_list_get_toggled(GTK_CHECK_LIST (ggv->pagelist), i);
                gtk_check_list_set_toggled (GTK_CHECK_LIST (ggv->pagelist), i, !checked);
        }
        gtk_clist_thaw(GTK_CLIST(ggv->pagelist));
}

static void about_destroy_callback(GtkObject *obj, GtkWidget **about)
{
	*about = NULL;
}

void about_callback(GtkWidget *widget, gpointer data) 
{
	static GtkWidget *about = NULL;

	gchar *authors[] = {
#if 0
                "GGV was originally based on gv 3.5.8,",
                "written by Johannes Plass <plass@thep.physik.uni-mainz.de>,",
                "which in turn was based on GhostView 1.5,",
                "written by Tim Theisen <tim@cs.wisc.edu>.",
		"Szekeres Istvan <szekeres@cyberspace.mht.bme.hu> and",
                "Jonathan Blandford <jrb@redhat.com>",
                "started the GNOME port with further contributions by",
                "(in alphabetical order)",
                "Daniel M. German <dmg@csg.uwaterloo.ca> (current maintainer),",
                "Werner Koerner <werner.koerner@zae.uni-wuerzburg.de>",
                "Tuomas J. Lukka <lukka@iki.fi>",
                "Jaka Mocnik <jaka@gnu.org>",
                "and the whole GNOME developers team.",
                "",
                "GGV would be a carcass without Ghostscript, written by Peter Deutsch.",
#else
                "Jonathan Blandford <jrb@redhat.com>",
                "Daniel M. German <dmg@csg.uwaterloo.ca>,",
                "Dan E. Kelley <dan.kelley@dal.ca>",
                "Werner Koerner <werner.koerner@zae.uni-wuerzburg.de>",
                "Tuomas J. Lukka <lukka@iki.fi>",
                "Jaka Mocnik <jaka@gnu.org>  (current maintainer)",
                "Johannes Plass <plass@thep.physik.uni-mainz.de>",
		"Istvan Szekeres <szekeres@cyberspace.mht.bme.hu>",
                "Tim Theisen <tim@cs.wisc.edu>",
#endif
		NULL
	};

        if(about)
                return;

        gtk_widget_push_visual (gdk_imlib_get_visual ());
        gtk_widget_push_colormap (gdk_imlib_get_colormap ());
        
	about = gnome_about_new(_("Gnome Ghostview"), VERSION,
				"Copyright (C) 1998-2001 the Free Software Foundation",
				(const gchar **) authors,
				_("PostScript(TM) document viewer.\n"
                                  "Based on Tim Theisen's excellent Ghostview application."),
				"ggv-splash.png");
        gtk_signal_connect(GTK_OBJECT(about), "destroy",
                           GTK_SIGNAL_FUNC(about_destroy_callback), &about);

        gtk_widget_show(about);
        gtk_widget_pop_colormap ();
        gtk_widget_pop_visual ();
}

void new_callback(GtkWidget *widget, gpointer data)
{
        open_window(NULL, -1, -1, -1, -1);
}

void button_press_callback(GtkWidget *widget, GdkEventButton *event,
                           gpointer data)
{
        ggv_window *ggv = (ggv_window *)data;
        gint button = event->button;

        switch(button) {
        case 1: {
                if(!ggv->pan) {
                        gint wx = 0, wy = 0;

                        gdk_window_get_pointer(widget->window, &wx, &wy, NULL);

                        ggv->pan = TRUE;
                        if(pan_cursor == NULL)
                                pan_cursor = gdk_cursor_new(GDK_FLEUR);

                        gtk_grab_add(widget);
                        gdk_pointer_grab(widget->window, FALSE,
                                         GDK_POINTER_MOTION_MASK |
                                         GDK_BUTTON_RELEASE_MASK, NULL,
                                         pan_cursor, GDK_CURRENT_TIME);
                        ggv->prev_x = wx;
                        ggv->prev_y = wy;
                }
                break;
        }
        case 2: {
                gfloat cx, cy;
                gint x = 0, y = 0;
                
                if (event->window != GTK_GS (ggv->gs)->pstarget)
                        break;

                x += event->x;
                y += event->y;

                cx = (gfloat)x/(gfloat)GTK_GS(ggv->gs)->width;
                cy = (gfloat)y/(gfloat)GTK_GS(ggv->gs)->height;
                zoom_to(ggv, ggv->zoom_magstep + 1);
                gtk_gs_set_center(GTK_GS(ggv->gs), cx, cy);
                break;
        }
        case 3: {
                gnome_popup_menu_do_popup(ggv->popup_menu, NULL, NULL, event, ggv);
                if(ggv->menus_vis_pu)
                        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(ggv->menus_vis_pu),
                                                       ggv->show_menus);
                if(ggv->toolbar_vis_pu)
                        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(ggv->toolbar_vis_pu),
                                                       ggv->show_toolbar);
                if(ggv->panel_vis_pu)
                        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(ggv->panel_vis_pu),
                                                       ggv->show_panel);
                break;
        }
        /* Support for wheel mice */
	case 4: {
		if(event->state & GDK_CONTROL_MASK)
			gtk_gs_scroll(GTK_GS(ggv->gs), -64, 0);
		else if(event->state & GDK_SHIFT_MASK)
			zoom_to(ggv, ggv->zoom_magstep+1);
		else
			gtk_gs_scroll(GTK_GS(ggv->gs), 0, -64);
		break;
        }
	case 5: {
		if(event->state & GDK_CONTROL_MASK)
			gtk_gs_scroll(GTK_GS(ggv->gs), 64, 0);
		else if(event->state & GDK_SHIFT_MASK)
			zoom_to(ggv, ggv->zoom_magstep-1);
		else
			gtk_gs_scroll(GTK_GS(ggv->gs), 0, 64);
		break;
        }
        default:
                break;
        }
        gtk_clist_select_row (GTK_CLIST (ggv->pagelist),  GTK_GS(ggv->gs)->current_page,1);

}

void button_release_callback(GtkWidget *widget, GdkEventButton *event,
				    gpointer data)
{
        ggv_window *ggv = (ggv_window *)data;

        switch(event->button) {
        case 1:
                if(ggv->pan) {
                        ggv->pan = FALSE;
                        gdk_pointer_ungrab(GDK_CURRENT_TIME);
                        gtk_grab_remove(widget);
                }
                break;
        case 2:
                ggv->cd->incrop = 0;
                break;
        default:
                break;
        }
}

void close_callback (GtkWidget *widget, ggv_window *ggv)
{
        close_window(ggv);
}

gint exit_callback(GtkWidget *widget, gpointer data) 
{

        ggv_window *ggv;
        GList *node;

        /* do we want to save anything before we exit??? */        
        while (window_list) {
                node = window_list;
                window_list = window_list->next;
                if(window_list)
                        window_list->prev = NULL;
                node->next = NULL;
                ggv = (ggv_window *)node->data;

                /* save size of the current window */
                gdk_window_get_size (GTK_WIDGET(ggv->main_window)->window,
                                     &ggv_default_width,
                                     &ggv_default_height);

                g_list_free(node);
                gtk_widget_destroy (GTK_WIDGET (ggv->main_window));
                g_free(ggv);
        }

	gtk_main_quit ();
	return 0;
}

gint delete_callback(GtkWidget *widget, GdkEventAny *event,
			    gpointer data)
{
        close_window((ggv_window *)data);

        return 0;
}

void file_open_cancel_callback(GtkWidget *widget, gpointer data)
{
        ggv_window *ggv = (ggv_window *)data;

        save_window_size(ggv->file_sel, &file_sel_width, &file_sel_height);
        gtk_widget_destroy(ggv->file_sel);
        ggv->file_sel = NULL;
}

void file_open_delete_callback(GtkWidget *widget, GdkEventAny *e,
				      gpointer data)
{
        file_open_cancel_callback(widget, data);
}

void file_open_ok_callback(GtkWidget *widget, gpointer data)
{
        ggv_window *ggv = (ggv_window *)data;
        gchar *file;
        gchar *base_filename;
        char *error_msg;
        struct stat stat_rec;

        save_window_size(ggv->file_sel, &file_sel_width, &file_sel_height);
	file = g_strdup(gtk_file_selection_get_filename(GTK_FILE_SELECTION(ggv->file_sel)));

        /* We have to verify that this file exists */
        if (stat(file,&stat_rec) == 0) {
                if (S_ISDIR(stat_rec.st_mode)) {
                        /* This is a directory, reset window and
                           try again */
                        if (strlen(file) > 0 && file[strlen(file)-1] != '/')
                                strcat(file,"/");
                        gtk_file_selection_set_filename(GTK_FILE_SELECTION(ggv->file_sel),
                                                        file);
                        return;
                }
        }
        else {
                /* We have to check if there is a wildcard in the directory name
                 */
                base_filename = g_basename(file);
                if (strchr(base_filename, '?' ) != NULL ||
                    strchr(base_filename, '*' ) != NULL) {
                        gtk_file_selection_complete(GTK_FILE_SELECTION(ggv->file_sel),
                                                    file);
                }
                           
                /* FIXME:  currently , if the file does not exist,
                   the window does not close, but it does not send a message.
                   If we send a message, the focus does not return to the
                   File window after the user clicks on it

                  error_msg = g_strdup_printf(_("File does not exist %s."), file);
                  error_message(ggv,error_msg);
                */
                return;
        }
        gtk_widget_destroy(ggv->file_sel);
        ggv->file_sel = NULL;

        if ((GTK_GS(ggv->gs)->loaded = load_gs(ggv, file)))
                recent_update();

        g_free(file);
}

void
watch_file_callback(GtkWidget *widget, gpointer data)
{
        ggv_window *ggv = (ggv_window *) data;
        GtkGS *gs;
        gs = GTK_GS(ggv->gs);
        
        gs->watch_doc = GTK_CHECK_MENU_ITEM(widget)->active;

	if(!gs->watch_doc) {
		if(gs->timer_tag)
                        gtk_timeout_remove(gs->timer_tag);
	}
        else {
		gs->timer_tag = gtk_timeout_add(GGV_WATCH_INTERVAL,
						 timer_callback, ggv);
        }
}

void
antialiasing_callback(GtkWidget *widget, gpointer data)
{
        ggv_window *ggv = (ggv_window *) data;
        
        GtkGS *gs;
        gs = GTK_GS(ggv->gs);
        
        gs->antialiased = GTK_CHECK_MENU_ITEM(widget)->active;

        /* We assume that if it gets here is because it was toggled,
           so no need to check what is the previous state of gs->antialiased */

        reload_callback(NULL, ggv);

}

void
override_paper_callback(GtkWidget *widget, gpointer data)
{
        ggv_window *ggv = (ggv_window *) data;
        
        GtkGS *gs;
        gs = GTK_GS(ggv->gs);
        
        gs->override_media = GTK_CHECK_MENU_ITEM(widget)->active;

        gtk_gs_set_pagemedia(gs, -1, gs->current_page);

        if (GTK_GS(ggv->gs)->loaded)
                gtk_gs_goto_page (GTK_GS(ggv->gs), GTK_GS(ggv->gs)->current_page);
}

void
override_orientation_callback(GtkWidget *widget, gpointer data)
{
        ggv_window *ggv = (ggv_window *) data;
        
        GtkGS *gs;
        gs = GTK_GS(ggv->gs);
        
        g_print("Overriding orientation GS [%d]\n", GTK_CHECK_MENU_ITEM(widget)->active);

        gtk_gs_set_override_orientation (gs, GTK_CHECK_MENU_ITEM(widget)->active);

        /* We assume that if it gets here is because it was toggled,
           so no need to check what is the previous state of gs->antialiased */

        if (GTK_GS(ggv->gs)->loaded)
                gtk_gs_goto_page (GTK_GS(ggv->gs), GTK_GS(ggv->gs)->current_page);
}

void orientation_callback(GtkWidget *widget, gpointer data)
{
        window_with_data *wwd = (window_with_data *) data;
        ggv_window *ggv = wwd->ggv;

        if (!GTK_CHECK_MENU_ITEM(widget)->active){
                /* This is a deactivate signal Do nothing*/
                return;
        }

        /* Set the new orientation */
        g_print("Setting fallback orientation GS [%d]\n",GPOINTER_TO_INT(wwd->data));
      
        gtk_gs_set_orientation (GTK_GS(ggv->gs), GPOINTER_TO_INT(wwd->data));

        if (GTK_GS(ggv->gs)->loaded)
                gtk_gs_goto_page (GTK_GS(ggv->gs), GTK_GS(ggv->gs)->current_page);
}


void show_menubar_callback(GtkWidget *widget, gpointer data)
{
        ggv_window *ggv = (ggv_window *) data;

        ggv->show_menus = GTK_CHECK_MENU_ITEM(widget)->active;

        if (ggv->show_menus) {
                if((GNOME_APP(ggv->main_window))->menubar == NULL) {
                        create_menus(ggv, ggv->menudata);
                        gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(ggv->menus_vis),
                                                        ggv->show_menus);
                        gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM(ggv->panel_vis),
                                                        ggv->show_panel);
                }
                gtk_widget_show ((GNOME_APP(ggv->main_window))->menubar->parent);
        } else {
                gtk_widget_hide ((GNOME_APP(ggv->main_window))->menubar->parent);
		gtk_widget_queue_resize (ggv->main_window);
        }
        if(ggv->menus_vis &&
           GTK_CHECK_MENU_ITEM(ggv->menus_vis)->active != ggv->show_menus)
                gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(ggv->menus_vis),
                                               ggv->show_menus);
}

void show_toolbar_callback(GtkWidget *widget, gpointer data)
{
        ggv_window *ggv = (ggv_window *) data;
        
	GnomeDockItem* dock_item = gnome_app_get_dock_item_by_name(GNOME_APP(ggv->main_window), 
				GNOME_APP_TOOLBAR_NAME);

	ggv->show_toolbar = GTK_CHECK_MENU_ITEM(widget)->active;
	
	
	g_assert(dock_item != NULL);

	if (ggv->show_toolbar) {
                gtk_widget_show (GTK_WIDGET(dock_item));
        } else {
                gtk_widget_hide (GTK_WIDGET(dock_item));
		gtk_widget_queue_resize (ggv->main_window);
        }
        if(ggv->toolbar_vis &&
           GTK_CHECK_MENU_ITEM(ggv->toolbar_vis)->active != ggv->show_toolbar)
                gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(ggv->toolbar_vis),
                                               ggv->show_toolbar);
}


void hide_panel_callback(GtkWidget *widget, gpointer data)
{
        ggv_window *ggv = (ggv_window *) data;

        ggv->show_panel = GTK_CHECK_MENU_ITEM(widget)->active;

        if (ggv->show_panel) {
                if(ggv->scrollpane == NULL)
                        create_sidebar(ggv);
                gtk_widget_show (ggv->sidebar);
        } else {
                gtk_widget_hide (ggv->sidebar);
		gtk_widget_queue_resize (ggv->main_window);
        }
        if(ggv->panel_vis &&
           GTK_CHECK_MENU_ITEM(ggv->panel_vis)->active != ggv->show_panel)
                gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(ggv->panel_vis),
                                               ggv->show_panel);
}

void motion_callback(GtkWidget *widget, GdkEventMotion *event,
                     gpointer data)
{
        ggv_window *ggv = (ggv_window *) data;

        if(ggv->pan) {
                gtk_gs_scroll(GTK_GS(ggv->gs), -event->x + ggv->prev_x, -event->y + ggv->prev_y);;
                ggv->prev_x = event->x;
                ggv->prev_y = event->y;
        }
        else {
#if 0
                mouse_moved (ggv->cd, ggv->gs->window, 
                             event->x, event->y,
                             ggv->main_window->allocation.width,
                             ggv->main_window->allocation.height);
#endif
        }
}

void interpreter_message_callback(GtkGS *gs, gchar *msg, gpointer data)
{
        ggv_window *ggv = (ggv_window *)data;
        add_gs_status_text (ggv, msg, TRUE);
}

void key_pressed_event_callback(GtkWidget *widget, GdkEventKey *event,
                                gpointer data)
{
        ggv_window *ggv = (ggv_window *) data;
        gint key = event->keyval;
        gint orientation = gtk_gs_get_orientation (GTK_GS (ggv->gs));

        if (!GTK_GS(ggv->gs)->loaded)
                return;
        /*ugh. the possibilities! */
        switch (key) {
        case GDK_space:
                switch (orientation) {
                case GTK_GS_ORIENTATION_PORTRAIT:
                        if (!gtk_scrollpane_step(GTK_SCROLLPANE (ggv->scrollpane), 
                                                 GTK_SCROLLPANE_SCROLL_DOWN, TRUE)) {
				goto_page( ggv, GTK_GS(ggv->gs)->current_page + 1);
                                if(ggv->pane_auto_jump)
                                        gtk_scrollpane_goto_edge(GTK_SCROLLPANE (ggv->scrollpane),
                                                                 GTK_SCROLLPANE_GOTOEDGE_LOWER, 
                                                                 GTK_SCROLLPANE_GOTOEDGE_LOWER);
                        }
                        break;
                case GTK_GS_ORIENTATION_LANDSCAPE:
                        if (!gtk_scrollpane_step (GTK_SCROLLPANE (ggv->scrollpane),
                                                  GTK_SCROLLPANE_SCROLL_LEFT,TRUE)) {
  				goto_page( ggv, GTK_GS(ggv->gs)->current_page + 1);
                                if(ggv->pane_auto_jump)
                                        gtk_scrollpane_goto_edge(GTK_SCROLLPANE (ggv->scrollpane),
                                                                 GTK_SCROLLPANE_GOTOEDGE_UPPER, 
                                                                 GTK_SCROLLPANE_GOTOEDGE_UPPER);
                         }
                        break;
                case GTK_GS_ORIENTATION_SEASCAPE:
                        if (!gtk_scrollpane_step (GTK_SCROLLPANE (ggv->scrollpane),
                                                  GTK_SCROLLPANE_SCROLL_RIGHT,TRUE)) {
				goto_page( ggv, GTK_GS(ggv->gs)->current_page + 1);
                                if(ggv->pane_auto_jump)
                                        gtk_scrollpane_goto_edge(GTK_SCROLLPANE (ggv->scrollpane),
                                                                 GTK_SCROLLPANE_GOTOEDGE_LOWER, 
                                                                 GTK_SCROLLPANE_GOTOEDGE_LOWER);
                         }
                        break;
                case GTK_GS_ORIENTATION_UPSIDEDOWN:
                        if (!gtk_scrollpane_step(GTK_SCROLLPANE (ggv->scrollpane),
                                                 GTK_SCROLLPANE_SCROLL_UP,TRUE)) {
				goto_page( ggv, GTK_GS(ggv->gs)->current_page + 1);
                                if(ggv->pane_auto_jump)
                                        gtk_scrollpane_goto_edge(GTK_SCROLLPANE (ggv->scrollpane),
                                                                 GTK_SCROLLPANE_GOTOEDGE_UPPER, 
                                                                 GTK_SCROLLPANE_GOTOEDGE_UPPER);
                         }
                        break;
                }
                break;
        case GDK_BackSpace:
        case GDK_Delete:
                switch (orientation) {
                case GTK_GS_ORIENTATION_PORTRAIT:
                        if (!gtk_scrollpane_step(GTK_SCROLLPANE (ggv->scrollpane),
                                                 GTK_SCROLLPANE_SCROLL_UP,TRUE)) {
				goto_page( ggv, GTK_GS(ggv->gs)->current_page - 1);
                                if(ggv->pane_auto_jump)
                                        gtk_scrollpane_goto_edge(GTK_SCROLLPANE (ggv->scrollpane),
                                                                 GTK_SCROLLPANE_GOTOEDGE_UPPER, 
                                                                 GTK_SCROLLPANE_GOTOEDGE_UPPER);
                         }
                        break;
                case GTK_GS_ORIENTATION_LANDSCAPE:
                        if (!gtk_scrollpane_step(GTK_SCROLLPANE (ggv->scrollpane),
                                                 GTK_SCROLLPANE_SCROLL_RIGHT,TRUE)) {
				goto_page( ggv, GTK_GS(ggv->gs)->current_page - 1);
                                if(ggv->pane_auto_jump)
                                        gtk_scrollpane_goto_edge(GTK_SCROLLPANE (ggv->scrollpane),
                                                                 GTK_SCROLLPANE_GOTOEDGE_LOWER, 
                                                                 GTK_SCROLLPANE_GOTOEDGE_LOWER);
                         }
                        break;
                case GTK_GS_ORIENTATION_SEASCAPE:
                        if (!gtk_scrollpane_step(GTK_SCROLLPANE (ggv->scrollpane),
                                                 GTK_SCROLLPANE_SCROLL_LEFT,TRUE)) {
				goto_page( ggv, GTK_GS(ggv->gs)->current_page - 1);
                                if(ggv->pane_auto_jump)
                                        gtk_scrollpane_goto_edge(GTK_SCROLLPANE (ggv->scrollpane),
                                                                 GTK_SCROLLPANE_GOTOEDGE_UPPER, 
                                                                 GTK_SCROLLPANE_GOTOEDGE_UPPER);
                         }
                        break;
                case GTK_GS_ORIENTATION_UPSIDEDOWN:
                        if (!gtk_scrollpane_step(GTK_SCROLLPANE (ggv->scrollpane),
                                                 GTK_SCROLLPANE_SCROLL_DOWN,TRUE)) {
				goto_page( ggv, GTK_GS(ggv->gs)->current_page - 1);
                                if(ggv->pane_auto_jump)
                                        gtk_scrollpane_goto_edge(GTK_SCROLLPANE (ggv->scrollpane),
                                                                 GTK_SCROLLPANE_GOTOEDGE_LOWER, 
                                                                 GTK_SCROLLPANE_GOTOEDGE_LOWER);
                         }
                        break;
                }
                break;
        case GDK_Left:
                gtk_scrollpane_step(GTK_SCROLLPANE (ggv->scrollpane),GTK_SCROLLPANE_SCROLL_LEFT,FALSE);
                break;
        case GDK_Right:
                gtk_scrollpane_step(GTK_SCROLLPANE (ggv->scrollpane),GTK_SCROLLPANE_SCROLL_RIGHT,FALSE);
                break;
        case GDK_Up:
                gtk_scrollpane_step(GTK_SCROLLPANE (ggv->scrollpane),GTK_SCROLLPANE_SCROLL_UP,FALSE);
                break;
        case GDK_Down:
                gtk_scrollpane_step(GTK_SCROLLPANE (ggv->scrollpane),GTK_SCROLLPANE_SCROLL_DOWN,FALSE);
                break;
        case GDK_Page_Up:
		goto_page( ggv, GTK_GS(ggv->gs)->current_page - 1);
                if (ggv->pane_auto_jump)
                        gtk_scrollpane_goto_edge(GTK_SCROLLPANE(ggv->scrollpane), GTK_SCROLLPANE_GOTOEDGE_LOWER, GTK_SCROLLPANE_GOTOEDGE_LOWER);
                break;
        case GDK_Page_Down:
		goto_page( ggv, GTK_GS(ggv->gs)->current_page + 1);
                if (ggv->pane_auto_jump)
                        gtk_scrollpane_goto_edge(GTK_SCROLLPANE(ggv->scrollpane), GTK_SCROLLPANE_GOTOEDGE_LOWER, GTK_SCROLLPANE_GOTOEDGE_LOWER);
                break;
	case GDK_plus:
		zoomin_callback(ggv->gs, (gpointer *)ggv);
		break;
	case GDK_minus:
		zoomout_callback(ggv->gs, (gpointer *)ggv);
		break;
	default:
		goto DONT_STOP_SIGNAL;
        }
	gtk_signal_emit_stop_by_name (GTK_OBJECT(ggv->main_window), "key_press_event");
 DONT_STOP_SIGNAL:
	;
}

void first_page_callback(GtkWidget *widget, gpointer data)
{
        ggv_window *ggv = (ggv_window *)data;
        if(GTK_GS(ggv->gs)->loaded) {
                if(GTK_GS(ggv->gs)->structured_doc)
                        goto_page(ggv, 0);
                else {
                        gtk_gs_disable_interpreter(GTK_GS(ggv->gs));
                        gtk_gs_enable_interpreter(GTK_GS(ggv->gs));
                }
        }
}

void last_page_callback(GtkWidget *widget, gpointer data)
{
        ggv_window *ggv = (ggv_window *) data;
        if (GTK_GS(ggv->gs)->loaded)
	        goto_page(ggv, GTK_GS(ggv->gs)->doc->numpages - 1);
}

void next_page_callback(GtkWidget *widget, gpointer data)
{
        ggv_window *ggv = (ggv_window *) data;
        if (GTK_GS(ggv->gs)->loaded)
	        goto_page(ggv, GTK_GS(ggv->gs)->current_page + 1);
}

void previous_page_callback(GtkWidget *widget, gpointer data) 
{
        ggv_window *ggv = (ggv_window *) data;

        if (GTK_GS(ggv->gs)->loaded)
		goto_page(ggv, GTK_GS(ggv->gs)->current_page - 1);
}

void open_callback(GtkWidget *widget, gpointer data)
{
        ggv_window *ggv = (ggv_window *)data;
        gchar *dirname = NULL;

        if(ggv->file_sel)
                return;

	ggv->file_sel = gtk_file_selection_new(_("GGv: Open File"));
        gtk_window_set_default_size(GTK_WINDOW(ggv->file_sel), file_sel_width,
                                               file_sel_height);
        gtk_window_set_position(GTK_WINDOW(ggv->file_sel), GTK_WIN_POS_MOUSE);
	gtk_signal_connect(GTK_OBJECT(ggv->file_sel), "delete_event",
			   GTK_SIGNAL_FUNC(file_open_delete_callback), data);
	gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(ggv->file_sel)->cancel_button),
			   "clicked",
			   GTK_SIGNAL_FUNC(file_open_cancel_callback), data);
	gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(ggv->file_sel)->ok_button),
			   "clicked",
			   GTK_SIGNAL_FUNC(file_open_ok_callback), data);

        gtk_file_selection_hide_fileop_buttons(GTK_FILE_SELECTION(ggv->file_sel));

        /* Switch to the directory where the current file came from */
        if ((GTK_GS (ggv->gs)->gs_filename != NULL) &&
	    ((dirname = g_dirname(GTK_GS (ggv->gs)->gs_filename)) != NULL) &&
            (strcmp(dirname, ".") != 0)) {
                gtk_file_selection_complete(GTK_FILE_SELECTION(ggv->file_sel),
                                                    dirname);
	}
        if(dirname)
		g_free (dirname);
        gtk_widget_show(ggv->file_sel);
}

void preferences_callback(GtkWidget *widget, gpointer data) {
        open_prefs_dialog(&prefs_dialog);
}

void print_callback(GtkWidget *widget, gpointer data)
{
        ggv_window *ggv = (ggv_window *) data;
        gchar *print_command;
        GtkGS *gs = GTK_GS(ggv->gs);

        if(GTK_GS(ggv->gs)->loaded) {
                /* make sure we print the _uncompressed_ file in case it
		 * the loaded one was compressed!
                 * Quote the filename to allow special characters.
		 */
 		gchar *filename;
                filename = ggv_quote_filename (GTK_GS_GET_PS_FILE(gs));
 		print_command = g_strdup_printf(gs_print_cmd, filename);
                g_free (filename);

                if(print_command) {
                        gnome_execute_shell(NULL, print_command);
                        g_free(print_command);
                }
        }
}

void print_marked_pages_callback(GtkWidget *widget, gpointer data)
{
        ggv_window *ggv = (ggv_window *)data;
        gchar *tmp_name, *print_command;

        if(GTK_GS(ggv->gs)->loaded) {
                if(gtk_gs_count_marked_pages(GTK_GS(ggv->gs)) == 0) {
			GtkWidget *dialog, *label;
         		int button;

        		dialog = gnome_message_box_new (
                 		_("There are no marked pages.\nDo you want to print the entire document?"),
				GNOME_MESSAGE_BOX_QUESTION,
                 		GNOME_STOCK_BUTTON_YES,
                 		GNOME_STOCK_BUTTON_NO,
                 		NULL);
			
		 	if (ggv)
                 		gnome_dialog_set_parent (GNOME_DIALOG (dialog), GTK_WINDOW(ggv->main_window));
		
			/*gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);*/

			gnome_dialog_set_default(GNOME_DIALOG(dialog), GNOME_NO);

         		button = gnome_dialog_run_and_close (GNOME_DIALOG (dialog));
			
			if(button == GNOME_YES)
				print_callback(widget, data);

	                return;
	        }

                tmp_name = g_malloc(256);
                tmpnam(tmp_name);

                save_marked_pages(ggv, tmp_name);

                print_command = g_strdup_printf(gs_print_cmd, tmp_name);
                if(print_command) {
                        gnome_execute_shell(NULL, print_command);
                        /* FIXME: file tmp_name should be unlinked after
                           printing... */
                        g_free(print_command);
                }

                g_free(tmp_name);
        }
}

void recenter_page_callback(GtkWidget *widget, gpointer data)
{
        ggv_window *ggv = (ggv_window *) data;

        if (GTK_GS(ggv->gs)->loaded)
                gtk_gs_center_page (GTK_GS (ggv->gs));
}

void reload_callback(GtkWidget *widget, gpointer data) 
{
        ggv_window *ggv = (ggv_window *) data;

        if (GTK_GS(ggv->gs)->loaded) {
                gchar *fname = g_strdup (GTK_GS (ggv->gs)->gs_filename);
                GTK_GS(ggv->gs)->loaded = load_gs (ggv, fname);
                g_free (fname);
        }
}

void file_save_cancel_callback(GtkWidget *widget, gpointer data)
{
        ggv_window *ggv = (ggv_window *)data;

        save_window_size(ggv->save_file_sel, &file_sel_width, &file_sel_height);
        gtk_widget_destroy(ggv->save_file_sel);
        ggv->save_file_sel = NULL;
}

void file_save_delete_callback(GtkWidget *widget, GdkEventAny *e,
                               gpointer data)
{
        file_save_cancel_callback(widget, data);
}

void file_save_ok_callback(GtkWidget *widget, gpointer data)
{
        gchar *base_filename, *flash;
        char *error_msg;
        struct stat stat_rec;
        ggv_window *ggv = (ggv_window *)data;
        gchar *file;

        save_window_size(ggv->save_file_sel, &file_sel_width, &file_sel_height);
	file = g_strdup(gtk_file_selection_get_filename(GTK_FILE_SELECTION(ggv->save_file_sel)));

        if(stat(file,&stat_rec) == 0) {
                if(S_ISDIR(stat_rec.st_mode)) {
                        /* This is a directory, reset window and
                           try again */
                        if (strlen(file) > 0 && file[strlen(file)-1] != '/')
                                strcat(file,"/");
                        gtk_file_selection_set_filename(GTK_FILE_SELECTION(ggv->save_file_sel),
                                                        file);
                        g_free(file);
                        return;
                }
                
        }
        else {
                /* We have to check if there is a wildcard in the directory name
                 */
                base_filename = g_basename(file);
                if (strchr(base_filename, '?' ) != NULL ||
                    strchr(base_filename, '*' ) != NULL) {
                        gtk_file_selection_complete(GTK_FILE_SELECTION(ggv->save_file_sel),
                                                    file);
                        g_free(file);
                        return;
                }
                /* At this point, we know the file does not exist */
        }

        gtk_widget_destroy(ggv->save_file_sel);
        ggv->save_file_sel = NULL;

        save_marked_pages(ggv, file);

        g_free(file);
}

void save_callback(GtkWidget *widget, gpointer data) {
        ggv_window *ggv = (ggv_window *) data;
        char *error_msg;
        gchar *dirname;

        if (GTK_GS(ggv->gs)->loaded) {
                if(ggv->save_file_sel)
                        return;

                if(gtk_gs_count_marked_pages(GTK_GS(ggv->gs)) == 0) {
                        error_msg = g_strdup(_("There are no marked pages."));
                        error_message(ggv, error_msg);
                        return;
                }

                ggv->save_file_sel = gtk_file_selection_new(_("GGv: Save marked pages"));
                gtk_window_set_default_size(GTK_WINDOW(ggv->save_file_sel), file_sel_width,
                                            file_sel_height);
                gtk_window_set_position(GTK_WINDOW(ggv->save_file_sel), GTK_WIN_POS_MOUSE);
                gtk_signal_connect(GTK_OBJECT(ggv->save_file_sel), "delete_event",
                                   GTK_SIGNAL_FUNC(file_save_delete_callback), data);
                gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(ggv->save_file_sel)->cancel_button),
                                   "clicked",
                                   GTK_SIGNAL_FUNC(file_save_cancel_callback), data);
                gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(ggv->save_file_sel)->ok_button),
                                   "clicked",
                                   GTK_SIGNAL_FUNC(file_save_ok_callback), data);
                
                gtk_file_selection_hide_fileop_buttons(GTK_FILE_SELECTION(ggv->save_file_sel));
                
                /* Switch to the directory where the current file came from */
                if ((GTK_GS (ggv->gs)->gs_filename != NULL) &&
		    ((dirname = g_dirname(GTK_GS (ggv->gs)->gs_filename)) != NULL)) {
                        gtk_file_selection_complete(GTK_FILE_SELECTION(ggv->save_file_sel),
                                                    dirname);
			g_free (dirname);
		}
                gtk_widget_show(ggv->save_file_sel);
        }
}

void save_as_callback(GtkWidget *widget, gpointer data) 
{
        ggv_window *ggv = (ggv_window *) data;
        if (GTK_GS(ggv->gs)->loaded) {
        }
}


void scrollpane_middle_click_callback(GtkWidget *widget, gpointer data)
{
        ggv_window *ggv = (ggv_window *) data;

        if (GTK_GS(ggv->gs)->loaded) {
		goto_page( ggv, GTK_GS(ggv->gs)->current_page - 1);
        }
}

void scrollpane_right_click_callback(GtkWidget *widget, gpointer data)
{
        ggv_window *ggv = (ggv_window *) data;

        if (GTK_GS(ggv->gs)->loaded) {
		goto_page( ggv, GTK_GS(ggv->gs)->current_page + 1);
        }
}

void
page_toggled_callback (GtkWidget *widget, gint row, gboolean toggled, gpointer data)
{
        ggv_window *ggv = (ggv_window *)data;

        GTK_GS(ggv->gs)->pages_marked[row] = toggled;
}

void select_page_callback(GtkWidget *widget, gint row, gint column,
				 GdkEventButton *event, gpointer data)
{
        ggv_window *ggv = (ggv_window *)data;

        g_return_if_fail(widget != NULL);


        /* If event is null then it is not from the user */
        if (event == NULL || event->button == 1) {
                if(GTK_GS(ggv->gs)->current_page != row) {
                        gtk_gs_goto_page(GTK_GS(ggv->gs), row);
                        row = GTK_GS(ggv->gs)->current_page; /* make sure */
                        set_page_sensitivities(ggv, row);
                }
        }
}

void drop_callback(GtkWidget *widget, GdkDragContext *context,
                   gint x, gint y, GtkSelectionData *selection_data,
                   guint info, guint time, gpointer data) {
	GList *names, *list;
        ggv_window *ggv = (ggv_window *)data;

	switch (info) {
	case TARGET_URI_LIST:
		list = names = gnome_uri_list_extract_filenames (selection_data->data);
		while (names) {
                        if(GTK_GS(ggv->gs)->loaded) {
                                open_window(names->data, -1, -1, -1, -1);
                        }
                        else if (load_gs (ggv, names->data))
                                GTK_GS(ggv->gs)->loaded = TRUE;

			names = names->next;
		}
		gnome_uri_list_free_strings (list);
		break;
	default:
                break;
	}
}

void paper_callback(GtkWidget *widget, gpointer *data)
{
        window_with_data *wwd = (window_with_data *) data;
        ggv_window *ggv = wwd->ggv;
        GtkGS *gs = GTK_GS(ggv->gs);

        if (!GTK_CHECK_MENU_ITEM(widget)->active){
                /* This is a deactivate signal */
                return;
        }

        gs->default_page_media = GPOINTER_TO_INT(wwd->data);
        gtk_gs_set_pagemedia(gs, -1, gs->current_page);
}

void zoom_callback(GtkWidget *widget, gpointer data)
{
        window_with_data *wwd = (window_with_data *) data;
        ggv_window *ggv = wwd->ggv;
        int new_mag = GPOINTER_TO_INT(wwd->data);


        if (!GTK_CHECK_MENU_ITEM(widget)->active){
                /* This is a deactivate signal */
                return;
        }

        if (ggv->zoom_magstep == new_mag) {
                return;
        }
        zoom_to(ggv, new_mag);
}

void zoomin_callback(GtkWidget *widget, gpointer *data)
{
        ggv_window *ggv = (ggv_window *)data;

        /* if there is an option in the zoom menu, we set it,
           that will zoom to the corresponding zoom mag. step
           as a side effect. */

        /* If it does not exist (the function returned FALSE)
           then do the call explicitly */
 
        if (!set_corresponding_zoom_menu_option(ggv, ggv->zoom_magstep+1))
                zoom_to(ggv, ggv->zoom_magstep+1);
}


void zoomout_callback(GtkWidget *widget, gpointer *data)
{
        ggv_window *ggv = (ggv_window *)data;

        /* if there is an option in the zoom menu, we set it,
           that will zoom to the corresponding zoom mag. step
           as a side effect. */

        /* If it does not exist (the function returned FALSE)
           then do the call explicitly */


        if (!set_corresponding_zoom_menu_option(ggv, ggv->zoom_magstep-1))
                zoom_to(ggv, ggv->zoom_magstep-1);
}

void prefs_destroy_callback(GtkWidget *w, ggv_prefs *p)
{
        p->pbox = NULL;
}

void prefs_apply_callback(GtkWidget *w, gint page, ggv_prefs *p)
{
        GtkWidget *active;
        gint i;

        if( page != -1 ) return;

        if(gs_cmd)
                g_free(gs_cmd);
        gs_cmd = g_strdup(gtk_entry_get_text(GTK_ENTRY(p->gs)));
        if(gs_scan_pdf_cmd)
                g_free(gs_scan_pdf_cmd);
        gs_scan_pdf_cmd = g_strdup(gtk_entry_get_text(GTK_ENTRY(p->scan_pdf)));
        if (gs_ungzip_cmd)
                g_free (gs_ungzip_cmd);
        gs_ungzip_cmd = g_strdup (gtk_entry_get_text (GTK_ENTRY (p->unzip)));
        if (gs_unbzip2_cmd)
		g_free (gs_unbzip2_cmd);
        gs_unbzip2_cmd = g_strdup (gtk_entry_get_text (GTK_ENTRY (p->unbzip2)));
        if(gs_print_cmd)
                g_free(gs_print_cmd);
        gs_print_cmd = g_strdup(gtk_entry_get_text(GTK_ENTRY(p->print)));

        active = gtk_menu_get_active(GTK_MENU(gtk_option_menu_get_menu(GTK_OPTION_MENU(p->media))));
        for(i = 0; i < PAPER_SIZE_COUNT; i++)
                if(active == p->media_choice[i]) {
                        gtk_gs_set_default_page_media(i);
                        break;
                }

        active = gtk_menu_get_active(GTK_MENU(gtk_option_menu_get_menu(GTK_OPTION_MENU(p->zoom))));
        for(i = 0; i < MENU_ZOOM_SIZE - 1; i++){
                if(active == p->zoom_choice[i]) {
                        gs_default_magnification = ZoomMenuMagnificationSteps[i];
                        gtk_gs_set_default_zoom_factor(ggv_compute_zoom(gs_default_magnification));
                        break;
                }
        }

        active = gtk_menu_get_active(GTK_MENU(gtk_option_menu_get_menu(GTK_OPTION_MENU(p->orientation))));
        for(i = 0; i < MENU_ORIENTATION_SIZE; i++){
                if(active == p->orientation_choice[i]) {
                        gtk_gs_set_default_orientation(i);
                        break;
                }
        }

        gtk_gs_set_default_respect_eof(GTK_TOGGLE_BUTTON(p->respect_eof)->active);
        gtk_gs_set_default_watch_doc(GTK_TOGGLE_BUTTON(p->watch)->active);
        gtk_gs_set_default_antialiased(GTK_TOGGLE_BUTTON(p->aa)->active);
        gtk_gs_set_default_override_media(GTK_TOGGLE_BUTTON(p->override_media)->active);
        gtk_gs_set_default_override_orientation(GTK_TOGGLE_BUTTON(p->override_orientation)->active);

        gs_panel = GTK_TOGGLE_BUTTON(p->tbar)->active;
        gs_save_geometry = GTK_TOGGLE_BUTTON(p->savegeo)->active;
	gs_toolbar = GTK_TOGGLE_BUTTON(p->toolbar)->active;
        gs_menubar = GTK_TOGGLE_BUTTON(p->mbar)->active;
        gs_auto_jump = GTK_TOGGLE_BUTTON(p->auto_jump)->active;

        apply_gs_prefs(window_list);
}

void prefs_help_callback(GtkWidget *w, gint page, ggv_prefs *p)
{
	gchar *helpfile;

	helpfile = gnome_help_file_find_file("ggv", "prefs.html");
	if (helpfile) {
                gchar *url;
		url = g_strconcat ("ghelp:", helpfile, NULL);
                gnome_help_goto (NULL, url);
		g_free (url);
		g_free(helpfile);
	} else {
		gnome_error_dialog (_("Couldn't find the GGv manual!"));
	}
}

void prefs_changed_callback(GtkWidget *w, ggv_prefs *p)
{
	gnome_property_box_changed(p->pbox);
}

/* Reload file if it was changed */
gint timer_callback (gpointer data)
{
	ggv_window *ggv;
        GtkGS *gs;
	struct stat sbuf;

        g_assert (data);

	ggv = (ggv_window *) data;
        gs = GTK_GS (ggv->gs);

        /* remove timer if no valid file is loaded */
	if (!GTK_GS(ggv->gs)->loaded || !ggv->gs || !gs->watch_doc || !gs->gs_psfile) {
                gs->timer_tag = 0;
		return FALSE;
	}

	if (!gs->busy && !stat(gs->gs_filename, &sbuf) &&
	    (sbuf.st_mtime != gs->mtime) &&
	    (time (NULL) - sbuf.st_mtime >= GGV_WATCH_TIMEOUT))
		reload_callback (NULL, ggv);

        return TRUE;
}
