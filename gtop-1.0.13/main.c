#include <config.h>

/*
 *   gtop - gnome system monitor
 *   Copyright (C) 1997,98 Radek Doulík
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "procview.h"
#include "memusage.h"
#include "properties.h"
#include "session.h"

#include <gtop-page.h>
#include <gtop-fsusage.h>

#include "tb_timer.xpm"
#include "tb_timer_stopped.xpm"

#include <gnome.h>
#include <libgnomeui/gnome-window-icon.h>

#include <glibtop/close.h>

#include <locale.h>
#include <assert.h>

extern GnomePropertyDescriptor GlobalProperty_Descriptor;

GnomeMDI *mdi;
gboolean gtop_is_running = TRUE;
GnomeClient *client;

static void about_cb (GtkWidget *widget, void *data);
static void start_stop_cb (GtkWidget *widget);

       void gtop_show_mtbar   (GtkWidget *, gpointer);
static void gtop_toggle_mtbar (GtkWidget *, gpointer);

static int
delete_event_handler (GtkWidget *widget)
{
	gnome_config_sync ();

        gtk_main_quit ();
	
        return TRUE;
}

static void
about_cb (GtkWidget *widget, void *data)
{
	GtkWidget *about;
	const gchar *authors[] = {
		"Radek Doulik (doulik@karlin.mff.cuni.cz)",
		"Martin Baulig (martin@home-of-linux.org)",
		NULL
	};
	
	about = gnome_about_new ( _("GTop"), VERSION,
				  /* copyright notice */
				  _("(C) 1998 the Free Software Foundation"),
				  authors,
				  /* another comments */
				  _("The GNOME System Monitor program"),
				  NULL);
	gtk_widget_show (about);
	
	return;
}

static void
start_stop_cb (GtkWidget *widget)
{
	GtkToolbarChild *child = g_list_nth
		(GTK_TOOLBAR (widget->parent)->children, 0)->data;

	gtop_is_running = !gtop_is_running;

	gnome_stock_set_icon (GNOME_STOCK (child->icon),
			      gtop_is_running ?
			      GNOME_STOCK_PIXMAP_TIMER :
			      GNOME_STOCK_PIXMAP_TIMER_STOP);
}

/* toolbar */

static GnomeUIInfo toolbar [] = {
	{ GNOME_APP_UI_ITEM, NULL, N_("Start / Stop sampling"),
	  start_stop_cb, NULL, NULL, GNOME_APP_PIXMAP_STOCK,
	  GNOME_STOCK_PIXMAP_TIMER, 0, 0, NULL },
	{ GNOME_APP_UI_SEPARATOR },
	{ GNOME_APP_UI_ITEM, NULL, N_("Set gtop properties"),
	  gtop_show_properties, NULL, NULL, GNOME_APP_PIXMAP_STOCK,
	  GNOME_STOCK_PIXMAP_PROPERTIES, 0, 0, NULL },
	{ GNOME_APP_UI_ENDOFINFO }
};

/* menu system */

GnomeUIInfo helpMenu [] = {
#if HAVE_LIBGTOP_DOCU
	GNOMEUIINFO_HELP ("libgtop"),
	GNOMEUIINFO_SEPARATOR,
#endif
	GNOMEUIINFO_HELP ("gtop"),
	GNOMEUIINFO_MENU_ABOUT_ITEM (about_cb, NULL),
	GNOMEUIINFO_END
};

GnomeUIInfo settingsMenu [] = {
	{ GNOME_APP_UI_TOGGLEITEM,
	  N_("Show _menubar"), N_("Show menubar"),
	  gtop_toggle_mtbar, (gpointer) FALSE, NULL,
	  GNOME_APP_PIXMAP_DATA, GNOME_APP_PIXMAP_NONE,
	  'm', GDK_CONTROL_MASK, NULL },
	{ GNOME_APP_UI_TOGGLEITEM,
	  N_("Show _toolbar"), N_("Show toolbar"),
	  gtop_toggle_mtbar, (gpointer) TRUE, NULL,
	  GNOME_APP_PIXMAP_DATA, GNOME_APP_PIXMAP_NONE,
	  't', GDK_CONTROL_MASK, NULL },
	GNOMEUIINFO_SEPARATOR,
	GNOMEUIINFO_MENU_PREFERENCES_ITEM (gtop_show_properties, NULL),
	GNOMEUIINFO_END
};
  
GnomeUIInfo emptyMenu[] = {
	GNOMEUIINFO_END
};

GnomeUIInfo mainMenu [] = {
	GNOMEUIINFO_MENU_FILE_TREE (fileMenu),
	GNOMEUIINFO_MENU_SETTINGS_TREE (settingsMenu),
	GNOMEUIINFO_MENU_WINDOWS_TREE (emptyMenu),
	GNOMEUIINFO_MENU_HELP_TREE (helpMenu),
	GNOMEUIINFO_END
};

#define ADD_PROPERTIES(x,y) gtop_property_object_list = g_list_append (gtop_property_object_list, gnome_property_object_new (& ## x ## Property_Descriptor, &gtop_properties. ## y ##))

void
gtop_quit (void)
{
	gnome_config_sync ();

	if (gtop_properties.global.save_session)
		gtk_signal_emit_by_name (GTK_OBJECT (client), "save_yourself");

        gtk_main_quit ();
}

void
gtop_show_mtbar (GtkWidget *w, gpointer gp)
{
	GnomeUIInfo *mb = gnome_mdi_get_menubar_info (gnome_mdi_get_active_window (mdi));
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM
					(((GnomeUIInfo *)mb [1].moreinfo) [(gp) ? 1 : 0].widget), 1);
}

static void
gtop_toggle_mtbar (GtkWidget *w, gpointer gp)
{
	GList     *c = mdi->windows;
	GnomeApp  *app;
	GtkWidget *bw;
	gint       which = (gint) gtk_object_get_data
                (GTK_OBJECT (w), GNOMEUIINFO_KEY_UIDATA);

	while (c) {
		app = (GnomeApp *) c->data;
		bw = GTK_WIDGET (gnome_app_get_dock_item_by_name
			(app, (which) ? GNOME_APP_TOOLBAR_NAME : GNOME_APP_MENUBAR_NAME));

		if (GTK_CHECK_MENU_ITEM (w)->active) {
			gtk_widget_show (bw);
		} else {
			gtk_widget_hide (bw);
			gtk_widget_queue_resize (GTK_WIDGET (app));
		}
		c = c->next;
	}

	if (which)
		gtop_properties.global.show_toolbar = GTK_CHECK_MENU_ITEM (w)->active;
	else
		gtop_properties.global.show_menubar = GTK_CHECK_MENU_ITEM (w)->active;
}

int
main (int argc, char *argv[])
{
	gboolean restart_ok = FALSE;
	poptContext ctx;
	const char **args;

	setlocale (LC_ALL, "");
	bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);

	gnome_init_with_popt_table ("gtop", VERSION, argc, argv,
				    gtop_options, 0, &ctx);
	gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gnome-gtop.png");
	args = poptGetArgs (ctx);

	poptFreeContext (ctx);

	/* Check whether we have an old config file. */

	gtop_check_old_config ();

	/* Initialize LibGTop. */

	gtop_init_libgtop ();
	
	/* session management init */
        client = gnome_master_client ();

        gtk_signal_connect (GTK_OBJECT (client), "save_yourself",
                            GTK_SIGNAL_FUNC (save_state), (gpointer) argv[0]);
        gtk_signal_connect (GTK_OBJECT (client), "die",
                            GTK_SIGNAL_FUNC (client_die), NULL);

	/* prepare MDI interface. */

	mdi = GNOME_MDI (gnome_mdi_new ("gtop", _("GNOME System Monitor")));
	mdi->tab_pos = GTK_POS_TOP;

	gtk_signal_connect (GTK_OBJECT (mdi), "destroy",
			    GTK_SIGNAL_FUNC (delete_event_handler), NULL);

	/* initialize MDI. */
	gtop_mdi_init ();

	/* set up MDI menus and toolbar */
	gnome_mdi_set_menubar_template (mdi, mainMenu);
	gnome_mdi_set_toolbar_template (mdi, toolbar);

	/* and document menu and document list paths */
	gnome_mdi_set_child_menu_path (mdi, GNOME_MENU_FILE_STRING);
	gnome_mdi_set_child_list_path (mdi, GNOME_MENU_WINDOWS_PATH);

	/* Add properties (notebook pages). */
	ADD_PROPERTIES (Global, global);
	ADD_PROPERTIES (ProcView, procview);
	ADD_PROPERTIES (ProcFields, procfields);
	ADD_PROPERTIES (Summary, summary);
	ADD_PROPERTIES (SummaryColors, summary_colors);
	ADD_PROPERTIES (MemUsage, memusage);
       	ADD_PROPERTIES (FsUsage, fsusage);
	ADD_PROPERTIES (Graph, graph);

	/* Read properties. */
	gtop_init_properties ();

	/* set MDI mode */
	gnome_mdi_set_mode (mdi, gtop_properties.global.mdi_mode);

	/* Restore MDI session. */

	if ((gnome_client_get_flags (client) & GNOME_CLIENT_RESTORED) ||
	    gtop_properties.global.save_session) {
		gnome_config_push_prefix
			(gnome_client_get_config_prefix (client));

		restart_ok = gnome_mdi_restore_state
			(mdi, "MDI Session", gtop_page_create_from_config);

		gnome_config_pop_prefix ();
	}

	/* Add MDI pages. */
	gtop_mdi_start (!restart_ok);

	/* enter gtk main */
        gtk_main ();

	glibtop_close ();

	if (gtop_properties.global.save_session) {
		gnome_config_set_int
			("gtop/global/show_menubar",
			 gtop_properties.global.show_menubar);

		gnome_config_set_int
			("gtop/global/show_toolbar",
			 gtop_properties.global.show_toolbar);
		gnome_config_sync ();
	}
        
        return 0;
}
