/* bug-buddy bug submitting program
 *
 * Copyright (C) Jacob Berkman
 *
 * Author:  Jacob Berkman  <jberkman@andrew.cmu.edu>
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

#include <config.h>
#include <gnome.h>

#include <stdio.h>

#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <math.h>

#include <libart_lgpl/libart.h>

#include "bug-buddy.h"
#include "util.h"

#define d(x)

static gint
animate (gpointer data)
{
	double affine[6];

	art_affine_rotate (affine, -36);
	gnome_canvas_item_affine_relative (druid_data.throbber, affine);
	return TRUE;
}

static void
start_animation (void)
{
	g_return_if_fail (druid_data.throbber_id == 0);

	druid_data.throbber_id = gtk_timeout_add (150, animate, NULL);
}

static void
stop_animation (void)
{
	g_return_if_fail (druid_data.throbber_id != 0);

	gtk_timeout_remove (druid_data.throbber_id);
	druid_data.throbber_id = 0;
}

void
start_gdb (void)
{
	static gchar *old_app = NULL;
	static gchar *old_extra = NULL;
	static CrashType old_type = -1;
	gchar *app = NULL, *extra = NULL;

	d(g_message (_("Obtaining stack trace... (%d)"), druid_data.crash_type));

	switch (druid_data.crash_type) {
	case CRASH_NONE:
		return;
	case CRASH_DIALOG:
		app = gtk_entry_get_text (GTK_ENTRY (GET_WIDGET ("gdb-binary-entry")));
		extra = gtk_entry_get_text (GTK_ENTRY (GET_WIDGET ("gdb-pid-entry")));
		druid_data.app_pid = atoi (extra);
		kill (druid_data.app_pid, SIGCONT);
		if (druid_data.explicit_dirty ||
		    (old_type != CRASH_DIALOG) ||
		    (!old_app || strcmp (app, old_app)) ||
		    (!old_extra || strcmp (extra, old_extra))) {
			get_trace_from_pair (app, extra);
		}
		break;
	case CRASH_CORE:
		extra = gtk_entry_get_text (GTK_ENTRY (GET_WIDGET ("gdb-core-entry")));
		if (druid_data.explicit_dirty ||
		    (old_type != CRASH_CORE) ||
		    (!old_extra || strcmp (extra, old_extra))) {
			get_trace_from_core (extra);
		}
		break;
	default:
		g_assert_not_reached ();
		break;
	}

	g_free (old_extra);
	old_extra = g_strdup (extra);

	g_free (old_app);
	old_app = g_strdup (app);

	old_type = druid_data.crash_type;
}

void
stop_gdb (void)
{
	if (!druid_data.ioc) {
		d(g_message (_("gdb has already exited")));
		return;
	}
	
	g_io_channel_close (druid_data.ioc);
	waitpid (druid_data.gdb_pid, NULL, 0);
	
	druid_data.gdb_pid = 0;

	druid_set_sensitive (TRUE, TRUE, TRUE);
	stop_animation ();

	druid_data.fd = 0;
	druid_data.ioc = NULL;
	gtk_widget_set_sensitive (GTK_WIDGET (GET_WIDGET ("gdb-stop")), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET (GET_WIDGET ("gdb-go")), TRUE);

	if (GTK_TOGGLE_BUTTON (GET_WIDGET ("gdb-continue-toggle"))->active)
		druid_set_state (STATE_DESC);
	return;
}

void 
get_trace_from_core (const gchar *core_file)
{
	gchar *gdb_cmd;
	gchar buf[1024];
	gchar *binary = NULL;
	int status;
	FILE *f;

	gdb_cmd = g_strdup_printf ("gdb --batch --core=%s", core_file);

	f = popen (gdb_cmd, "r");
	g_free (gdb_cmd);

	if (!f) {
		gchar *s = g_strdup_printf (_("Unable to process core file with gdb:\n"
					      "'%s'"), core_file);
		GtkWidget *d = gnome_error_dialog (s);
		g_free (s);
		gnome_dialog_run_and_close (GNOME_DIALOG (d));
		return;
	}

	while (fgets(buf, 1024, f) != NULL) {
		if (!binary && !strncmp(buf, "Core was generated", 16)) {
			gchar *s;
			gchar *ptr = buf;
			while (*ptr != '`' && *ptr !='\0') ptr++;
			if (*ptr == '`') {
				ptr++;
				s = ptr;
				while (*ptr != '\'' && *ptr !=' ' && *ptr !='\0') ptr++;
				*ptr = '\0';
				binary = g_strdup(s);
			}
		}
	}

	status = pclose(f);

	if (!binary) {
		gchar *s = g_strdup_printf (_("Unable to determine which binary created\n"
					      "'%s'"), core_file);
		GtkWidget *d = gnome_error_dialog (s);
		g_free (s);
		gnome_dialog_run_and_close (GNOME_DIALOG (d));
		return;
	}	

	if (!popt_data.app_file) {
		d(g_message ("Setting binary: %s", binary));
		popt_data.app_file = g_strdup (binary);
	}

	get_trace_from_pair (binary, core_file);
	g_free (binary);
}

static gboolean
handle_gdb_input (GIOChannel *ioc, GIOCondition condition, gpointer data)
{	
	GtkWidget *w = NULL;
	gchar buf[1024];
	guint len;
	
	if (condition == G_IO_HUP) {
		stop_gdb ();
		return FALSE;
	}

 gdb_try_read:
	switch (g_io_channel_read (ioc, buf, 1024, &len)) {
	case G_IO_ERROR_NONE:
		break;
	case G_IO_ERROR_AGAIN:
		goto gdb_try_read;
	default:
		d(g_warning (_("Error on read... aborting")));
		stop_gdb ();
		return FALSE;
	}

	w = GET_WIDGET ("gdb-text");
	gtk_text_set_point (GTK_TEXT (w),
			    gtk_text_get_length (GTK_TEXT (w)));
	gtk_text_insert (GTK_TEXT (w), 
			 NULL, NULL, NULL, buf, len);

	return TRUE;
}

void
get_trace_from_pair (const gchar *app, const gchar *extra)
{
	GtkWidget *d;
	int fd;
	char *app2;
	char *args[] = { "gdb",
			 "--batch", 
			 "--quiet",
			 "--command=" BUDDY_DATADIR "/gdb-cmd",
			 NULL, NULL, NULL };
	args[0] = gnome_is_program_in_path ("gdb");
	args[5] = (char *)extra;

	if (!args[0]) {
		d(g_message ("Path: %s", getenv ("PATH")));
		gnome_dialog_run_and_close (
			GNOME_DIALOG (
				gnome_error_dialog (_("GDB could not be found on your system.\n"
						      "Debugging information will not be obtained."))));
		return;
	}

	if (!app || !extra || !*app || !*extra)
		return;
	
	/* FIXME: we should probably be fully expanding the link to
	   see if it is a directory, but unix sucks and i am lazy */
	if (g_file_test (app, G_FILE_TEST_ISFILE | G_FILE_TEST_ISLINK))
		app2 = g_strdup (app);
	else
		app2 = gnome_is_program_in_path (app);

	if (!app2) {
		g_free (args[0]);
		return;
	}

	args[4] = app2;

	d(g_message ("About to debug '%s'", app2));
	
	if (!g_file_exists (BUDDY_DATADIR "/gdb-cmd")) {
		d = gnome_error_dialog (_("Could not find the gdb-cmd file.\n"
					  "Please try reinstalling Bug Buddy."));
		gnome_dialog_run_and_close (GNOME_DIALOG (d));
		g_free (app2);
		return;
	}

	druid_data.gdb_pid = start_commandv ((const char **)args, &fd);
	if (druid_data.gdb_pid == -1) {
		d = gnome_error_dialog (_("Error on fork()."));
		gnome_dialog_run_and_close (GNOME_DIALOG (d));
		g_free (app2);
		return;
	}
	
	druid_data.fd = fd;
	druid_data.ioc = g_io_channel_unix_new (fd);
	g_io_add_watch (druid_data.ioc, G_IO_IN | G_IO_HUP, 
			handle_gdb_input, NULL);
	g_io_channel_unref (druid_data.ioc);
	gtk_editable_delete_text (GTK_EDITABLE (GET_WIDGET ("gdb-text")), 0, -1);

	druid_set_sensitive (FALSE, FALSE, TRUE);
	start_animation ();

	gtk_widget_set_sensitive (GTK_WIDGET (GET_WIDGET ("gdb-stop")), TRUE);
	gtk_widget_set_sensitive (GTK_WIDGET (GET_WIDGET ("gdb-go")), FALSE);

	druid_data.explicit_dirty = FALSE;

	g_free (app2);
}






