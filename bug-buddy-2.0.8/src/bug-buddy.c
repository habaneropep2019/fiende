/* Bug-buddy bug submitting program
 *
 * Copyright (C) 1999 - 2001 Jacob Berkman
 * Copyright 2000, 2001 Ximian, Inc.
 *
 * Author:  jacob berkman  <jacob@bug-buddy.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include "config.h"

#include <stdio.h>
#include <unistd.h>

#include <errno.h>

#include <gnome.h>
#include <libgnomeui/gnome-window-icon.h>
#include <glade/glade.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk-pixbuf/gnome-canvas-pixbuf.h>
#include <libgnomevfs/gnome-vfs.h>

#include <libxml/tree.h>
#include <libxml/parser.h>

#include <sys/types.h>
#include <signal.h>

#include "bug-buddy.h"
#include "util.h"
#include "distro.h"

#include "libglade-buddy.h"

#define d(x)

#define DRUID_PAGE_HEIGHT 440
#define DRUID_PAGE_WIDTH  600

const gchar *submit_type[] = {
	N_("Submit bug report"),
	N_("Only send report to yourself"),
	N_("Save report to file"),
	NULL
};

const gchar *crash_type[] = {
	N_("crashed application"),
	N_("core file"),
	N_("nothing"),
	NULL
};

const gchar *bug_type[] = {
	N_("Create a new bug report"),
	N_("Add more information to existing report"),
	NULL
};

PoptData popt_data;

DruidData druid_data;

static const struct poptOption options[] = {
	{ "name",        0, POPT_ARG_STRING, &popt_data.name,         0, N_("Name of contact"),                    N_("NAME") },
	{ "email",       0, POPT_ARG_STRING, &popt_data.email,        0, N_("Email address of contact"),           N_("EMAIL") },
	{ "package",     0, POPT_ARG_STRING, &popt_data.package,      0, N_("Package containing the program"),     N_("PACKAGE") },
	{ "package-ver", 0, POPT_ARG_STRING, &popt_data.package_ver,  0, N_("Version of the package"),             N_("VERSION") },
	{ "appname",     0, POPT_ARG_STRING, &popt_data.app_file,     0, N_("File name of crashed program"),       N_("FILE") },
	{ "pid",         0, POPT_ARG_STRING, &popt_data.pid,          0, N_("PID of crashed program"),             N_("PID") },
	{ "core",        0, POPT_ARG_STRING, &popt_data.core_file,    0, N_("Core file from program"),             N_("FILE") },
	{ "include",     0, POPT_ARG_STRING, &popt_data.include_file, 0, N_("Text file to include in the report"), N_("FILE") },
	{ NULL } 
};

static gboolean
update_crash_type (GtkWidget *w, gpointer data)
{
	CrashType new_type = GPOINTER_TO_INT (data);
	GtkWidget *table, *box, *scrolled;

	druid_data.crash_type = new_type;
	
	table = GET_WIDGET ("gdb-binary-table");
	box = GET_WIDGET ("gdb-core-box");
	scrolled = GET_WIDGET ("gdb-text-scrolled");

	switch (new_type) {
	case CRASH_DIALOG:
		gtk_widget_hide (box);
		gtk_widget_show (table);
		gtk_widget_set_sensitive (scrolled, TRUE);
		break;
	case CRASH_CORE:
		gtk_widget_hide (table);
		gtk_widget_show (box);
		gtk_widget_set_sensitive (scrolled, TRUE);
		break;
	case CRASH_NONE:
		gtk_widget_hide (table);
		gtk_widget_hide (box);
		gtk_widget_set_sensitive (scrolled, FALSE);
		break;
	}

	return FALSE;
}

static gboolean
update_submit_type (GtkWidget *w, gpointer data)
{
	SubmitType new_type = GPOINTER_TO_INT (data);
	GtkWidget *table, *box;
	druid_data.submit_type = new_type;

	table = GET_WIDGET ("email-to-table");
	box = GET_WIDGET ("email-file-gnome-entry");
	
	switch (new_type) {
	case SUBMIT_REPORT:
		gtk_widget_hide (box);
		gtk_widget_show (table);
		break;
	case SUBMIT_FILE:
		gtk_widget_hide (table);
		gtk_widget_show (box);
		break;
	case SUBMIT_TO_SELF:
		gtk_widget_hide (table);
		gtk_widget_hide (box);
		break;
	default:
		break;
	}

	gtk_widget_queue_resize (GET_WIDGET ("email-vbox"));

	return FALSE;
}

void
on_gdb_go_clicked (GtkWidget *w, gpointer data)
{
	druid_data.explicit_dirty = TRUE;
	start_gdb ();
}

void
on_gdb_stop_clicked (GtkWidget *button, gpointer data)
{
	GtkWidget *w;
	if (!druid_data.fd)
		return;

	w = gnome_question_dialog (_("gdb has not finished getting the "
				     "debugging information.\n"
				     "Kill the gdb process (the stack trace "
				     "will be incomplete)?"),
				   NULL, NULL);

	if (GNOME_YES == gnome_dialog_run_and_close (GNOME_DIALOG (w))) {
		if (druid_data.gdb_pid == 0) {
			d(g_warning (_("gdb has already exited")));
			return;
		}
		kill (druid_data.gdb_pid, SIGTERM);
		stop_gdb ();		
		kill (druid_data.app_pid, SIGCONT);
		druid_data.explicit_dirty = TRUE;
	}
}

void
on_product_list_select_row (GtkWidget *w, int row, int col, gpointer data)
{
	if (druid_data.state == STATE_PRODUCT)
		druid_set_sensitive (TRUE, TRUE, TRUE);
	druid_data.product = gtk_clist_get_row_data (GTK_CLIST (w), row);
	gtk_entry_set_text (GTK_ENTRY (GET_WIDGET ("email-to-entry")),
			    druid_data.product->bts->email);
}

void
on_product_list_unselect_row (GtkWidget *w, int row, int col, gpointer data)
{
	if (druid_data.state == STATE_PRODUCT)
		druid_set_sensitive (TRUE, FALSE, TRUE);
	druid_data.product = NULL;
}

void
on_component_list_select_row (GtkWidget *w, int row, int col, gpointer data)
{
	if (druid_data.state == STATE_COMPONENT)
		druid_set_sensitive (TRUE, TRUE, TRUE);
	druid_data.component = gtk_clist_get_row_data (GTK_CLIST (w), row);
}

void
on_component_list_unselect_row (GtkWidget *w, int row, int col, gpointer data)
{
	if (druid_data.state == STATE_COMPONENT)
		druid_set_sensitive (TRUE, FALSE, TRUE);
	druid_data.component = NULL;
}

static gboolean
on_timer (GtkProgress *progress)
{
	static int orient = 0;	
	gfloat val = gtk_progress_get_value (progress);
	if (val >= 100.0) {
		gtk_progress_bar_set_orientation (GTK_PROGRESS_BAR (progress),
						  (orient+1)%2);
		val = -5.0;
	}
	gtk_progress_set_value (progress, val+5.0);
	return TRUE;
}

void
on_version_list_select_row (GtkCList *list, gint row, gint col,
			    GdkEventButton *event, gpointer udata)
{	
	gchar *s;
	druid_data.selected_row = row;
	if (gtk_clist_get_text (list, row, 1, &s))
		gtk_entry_set_text (GTK_ENTRY (GET_WIDGET ("version-entry")), s);
	else
		gtk_editable_delete_text (
			GTK_EDITABLE (GET_WIDGET ("the-version-entry")), 0, -1);
			
	gtk_clist_get_text (list, row, 0, &s);
	gtk_label_set_text (GTK_LABEL (GET_WIDGET ("version-label")), _(s));
}

static void
add_to_clist (gpointer data, gpointer udata)
{
	char *row[3] = { NULL };
	Package *package = data;
	GtkWidget *clist = udata;

	row[0] = _(package->name);
	row[1] = package->version;

	gtk_clist_append (GTK_CLIST (clist), row);
}

void
stop_progress ()
{
	if (!druid_data.progress_timeout)
		return;

	gtk_timeout_remove (druid_data.progress_timeout);
	gtk_widget_hide (GET_WIDGET ("config_progress"));
}

void
append_packages ()
{
	GtkWidget *w;
	w = GET_WIDGET ("version-clist");

	gtk_clist_freeze (GTK_CLIST (w));
	g_slist_foreach (druid_data.packages, add_to_clist, w);
	gtk_clist_thaw (GTK_CLIST (w));

	stop_progress ();

	w = GET_WIDGET ("version-toggle");
	if (!gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (w)))
		return;

	on_druid_next_clicked (NULL, NULL);
}

void
update_selected_row (GtkWidget *w, gpointer data)
{
	gchar *s;
	gint row;
	if (druid_data.selected_row == -1)
		return;
	row = druid_data.selected_row;
	s = gtk_entry_get_text (GTK_ENTRY (GET_WIDGET ("the-version-entry")));
	gtk_clist_set_text (GTK_CLIST (GET_WIDGET ("version-clist")), row, 1, s);
}

void
on_file_radio_toggled (GtkWidget *radio, gpointer data)
{
	static GtkWidget *entry2 = NULL;
	int on = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (radio));
	if (!entry2)
		entry2 = glade_xml_get_widget (druid_data.xml,
					       "file_entry2");
	
	gtk_widget_set_sensitive (GTK_WIDGET (entry2), on);
}

GtkWidget *
stock_pixmap_buddy (gchar *w, char *n, char *a, int b, int c)
{
	return gnome_stock_pixmap_widget (NULL, n);
}

void
title_configure_size (GtkWidget *w, GtkAllocation *alloc, gpointer data)
{
	gnome_canvas_set_scroll_region (GNOME_CANVAS (w), 0.0, 0.0,
					alloc->width, alloc->height);
	gnome_canvas_item_set (druid_data.title_box,
			       "x1", 0.0,
			       "y1", 0.0,
			       "x2", (gfloat)alloc->width,
			       "y2", (gfloat)alloc->height,
			       "width_units", 1.0, NULL);

	gnome_canvas_item_set (druid_data.banner,
			       "x", 15.0,
			       "y", 24.0,
			       "anchor", GTK_ANCHOR_WEST, NULL);

	gnome_canvas_item_set (druid_data.logo,
			       "x", (gfloat)(alloc->width - 48),
			       "y", -2.0,
			       NULL);
}

void
side_configure_size (GtkWidget *w, GtkAllocation *alloc, gpointer data)
{
	gnome_canvas_set_scroll_region (GNOME_CANVAS (w), 0.0, 0.0,
					alloc->width, alloc->height);
	gnome_canvas_item_set (druid_data.side_box,
			       "x1", 0.0,
			       "y1", 0.0,
			       "x2", (gfloat)alloc->width,
			       "y2", (gfloat)alloc->height,
			       "width_units", 1.0, NULL);

	gnome_canvas_item_set (druid_data.side_image,
			       "x", 0.0,
			       "y", (gfloat)(alloc->height - 179),
			       "width", 68.0,
			       "height", 179.0, NULL);
}

static void
init_canvi (void)
{
	GdkPixbuf *pb;
	GnomeCanvasItem *root;
	GnomeCanvas *canvas;

	canvas = GNOME_CANVAS (GET_WIDGET ("title-canvas"));
	root = GNOME_CANVAS_ITEM (gnome_canvas_root (GNOME_CANVAS (canvas)));

	druid_data.title_box = 
		gnome_canvas_item_new (GNOME_CANVAS_GROUP (root),
				       gnome_canvas_rect_get_type (),
				       "fill_color", "black",
				       "outline_color", "black", NULL);

	druid_data.banner =
		gnome_canvas_item_new (GNOME_CANVAS_GROUP (root),
				       gnome_canvas_text_get_type (),
				       "fill_color", "white",
				       "font", "-adobe-helvetica-bold-r-normal-*-18-*-*-*-p-*-iso8859-1",
				       "fontset", "-adobe-helvetica-bold-r-normal-*-18-*-*-*-p-*-iso8859-1,*-r-*",
				       "anchor", GTK_ANCHOR_WEST,
				       NULL);	


	pb = gdk_pixbuf_new_from_file (BUDDY_DATADIR "/bug-core.png");
	druid_data.logo =
		gnome_canvas_item_new (GNOME_CANVAS_GROUP (root),
				       GNOME_TYPE_CANVAS_PIXBUF,
				       "pixbuf", pb, NULL);


	/*******************************************************/
	canvas = GNOME_CANVAS (GET_WIDGET ("side-canvas"));
	root = GNOME_CANVAS_ITEM (gnome_canvas_root (GNOME_CANVAS (canvas)));
	druid_data.side_box =
		gnome_canvas_item_new (GNOME_CANVAS_GROUP (root),
				       gnome_canvas_rect_get_type (),
				       "fill_color", "black",
				       "outline_color", "black", NULL);

	pb = gdk_pixbuf_new_from_file (BUDDY_DATADIR "/bug-flower.png");

	druid_data.side_image =
		gnome_canvas_item_new (GNOME_CANVAS_GROUP (root),
				       GNOME_TYPE_CANVAS_PIXBUF,
				       "pixbuf", pb, NULL);


	/*******************************************************/
	canvas = GNOME_CANVAS (GET_WIDGET ("gdb-canvas"));
	gnome_canvas_set_scroll_region (GNOME_CANVAS (canvas), -12.0, -12.0, 12.0, 12.0);
	root = GNOME_CANVAS_ITEM (gnome_canvas_root (GNOME_CANVAS (canvas)));
	
	pb = gdk_pixbuf_new_from_file (BUDDY_DATADIR "/bug-buddy.png");
	druid_data.throbber_pb = gdk_pixbuf_scale_simple (pb, 24, 24, GDK_INTERP_BILINEAR);
	gdk_pixbuf_unref (pb);

	druid_data.throbber =
		gnome_canvas_item_new (GNOME_CANVAS_GROUP (root),
				       GNOME_TYPE_CANVAS_PIXBUF,
				       "pixbuf", druid_data.throbber_pb,
				       "x", -6.0,
				       "y", -6.0,
				       "height", 24.0,
				       "width", 24.0,
				       NULL);
}

static gint
check_intro_skip (gpointer data)
{
	if (druid_data.already_run && 
	    gtk_toggle_button_get_active (
		    GTK_TOGGLE_BUTTON (GET_WIDGET ("intro-skip-toggle"))))
		on_druid_next_clicked (NULL, NULL);

	return FALSE;
}

/* there should be no setting of default values here, I think */
static void
init_ui (void)
{
	GtkWidget *w, *m;
	gchar *s;
	int i;

	glade_xml_signal_autoconnect (druid_data.xml);

	load_config ();

	init_canvi ();

	w = GET_WIDGET ("druid-notebook");
	gtk_notebook_set_show_border (GTK_NOTEBOOK (w), FALSE);
	gtk_notebook_set_show_tabs (GTK_NOTEBOOK (w), FALSE);

	/* dialog crash page */
	w = GET_WIDGET ("gdb-binary-entry");
	s = popt_data.app_file;
	if (!s) {
		s = getenv ("GNOME_CRASHED_APPNAME");
		if (s)
			g_warning (_("$GNOME_CRASHED_APPNAME is deprecated.\n"
				     "Please use the --appname command line"
				     "argument instead."));
	}

	if (s)
		gtk_entry_set_text (GTK_ENTRY (w), s);

	w = GET_WIDGET ("gdb-pid-entry");
	s = popt_data.pid;
	if (!s) {
		s = getenv ("GNOME_CRASHED_PID");
		if (s)
			g_warning (_("$GNOME_CRASHED_PID is deprecated.\n"
				     "Please use the --pid command line"
				     "argument instead."));
	}

	if (s) {
		gtk_entry_set_text (GTK_ENTRY (w), s);
		druid_data.crash_type = CRASH_DIALOG;
	}

	/* core crash page */
	w = GET_WIDGET ("gdb-core-entry");
	if (popt_data.core_file) {
		gtk_entry_set_text (GTK_ENTRY (w), popt_data.core_file);
		druid_data.crash_type = CRASH_CORE;
	}

	/* package version */
	if (popt_data.package_ver) {
		w = GET_WIDGET ("the-version-entry");
		gtk_entry_set_text (GTK_ENTRY (w), popt_data.package_ver);
	}

        /* init some ex-radio buttons */
	m = gtk_menu_new ();
	for (i = 0; crash_type[i]; i++) {
		w = gtk_menu_item_new_with_label (_(crash_type[i]));
		gtk_signal_connect (GTK_OBJECT (w), "activate",
				    GTK_SIGNAL_FUNC (update_crash_type),
				    GINT_TO_POINTER (i));
		gtk_widget_show (w);
		gtk_menu_append (GTK_MENU (m), w);
	}
	w = GET_WIDGET ("gdb-option");
	gtk_option_menu_set_menu (GTK_OPTION_MENU (w), m);
	gtk_option_menu_set_history (GTK_OPTION_MENU (w), druid_data.crash_type);
	update_crash_type (NULL, GINT_TO_POINTER (druid_data.crash_type));

	/* init more ex-radio buttons */
	m = gtk_menu_new ();
	for (i = 0; submit_type[i]; i++) {
		w = gtk_menu_item_new_with_label (_(submit_type[i]));
		gtk_signal_connect (GTK_OBJECT (w), "activate",
				    GTK_SIGNAL_FUNC (update_submit_type),
				    GINT_TO_POINTER (i));
		gtk_widget_show (w);
		gtk_menu_append (GTK_MENU (m), w);
	}
	w = GET_WIDGET ("email-option");
	gtk_option_menu_set_menu (GTK_OPTION_MENU (w), m);
	gtk_option_menu_set_history (GTK_OPTION_MENU (w), druid_data.submit_type);
	update_submit_type (NULL, GINT_TO_POINTER (druid_data.submit_type));

	gnome_window_icon_set_from_default (GTK_WINDOW (GET_WIDGET ("druid-window")));

	w = GET_WIDGET ("product_list");
	gtk_clist_set_row_height (GTK_CLIST (w), CLIST_HEIGHT);
}

gint
delete_me (GtkWidget *w, GdkEventAny *evt, gpointer data)
{
	save_config ();
	gtk_main_quit ();
	return FALSE;
}

int
main (int argc, char *argv[])
{
	GtkWidget *w;
	char *s;

	memset (&druid_data, 0, sizeof (druid_data));
	memset (&popt_data,  0, sizeof (popt_data));

	druid_data.crash_type = CRASH_NONE;
	druid_data.state = -1;

	srand (time (NULL));

	bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);

	gnome_init_with_popt_table (PACKAGE, VERSION, argc, argv, 
				    options, 0, NULL);
	gnome_window_icon_set_default_from_file (BUDDY_ICONDIR"/bug-buddy.png");

	gnome_vfs_init ();
	glade_gnome_init ();

	s = "bug-buddy.glade";
	if (!g_file_exists (s))
		s = BUDDY_DATADIR "/bug-buddy.glade";

	druid_data.xml = glade_xml_new (s, NULL);

	if (!druid_data.xml) {
		char *s2 = g_strdup_printf (_("Could not load '%s'.\n"
					      "Please make sure Bug Buddy was "
					      "installed correctly."), s);
		w = gnome_error_dialog (s2);
		gnome_dialog_run_and_close (GNOME_DIALOG (w));
		return 0;
	}

	init_ui ();

	gtk_widget_show (GET_WIDGET ("druid-window"));

	gtk_idle_add (check_intro_skip, NULL);
	
	druid_set_state (STATE_INTRO);

	gtk_main ();

	return 0;
}
