/* bug-buddy bug submitting program
 *
 * Copyright (C) 1999 - 2000 Jacob Berkman
 * Copyright 2000, 2001 Ximian, Inc.
 *
 * Author:  Jacob Berkman  <jacob@bug-buddy.org>
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
#include <gnome.h>

#include "bug-buddy.h"

#define d(x)

typedef enum {
	CONFIG_DONE,
	CONFIG_TOGGLE,
	CONFIG_ENTRY,
	CONFIG_USER,
	CONFIG_MAILER
} ConfigType;

typedef struct {
	ConfigType t;
	char *w;
	char *path;
	char *w2;
} ConfigItem;

static ConfigItem configs[] = {
	{ CONFIG_TOGGLE, "intro-skip-toggle",          "/bug-buddy/last/skip-intro=1" },
	{ CONFIG_ENTRY,  "gdb-binary-gnome-entry" },
	{ CONFIG_ENTRY,  "gdb-core-gnome-entry" },
	{ CONFIG_TOGGLE, "gdb-continue-toggle",        "/bug-buddy/last/gdb-continue=1" },
	{ CONFIG_ENTRY,  "desc-file-gnome-entry" },
/*	{ CONFIG_ENTRY,  "bts-package-gnome-entry" },*/
/*	{ CONFIG_ENTRY,  "bts-module-gnome-entry" }, */
	{ CONFIG_TOGGLE, "version-toggle",             "/bug-buddy/last/skip_conf=1" },
	{ CONFIG_USER,   "email-name-gnome-entry",     "/bug-buddy/last/name",          "email-name-entry" },
	{ CONFIG_ENTRY,  "email-email-gnome-entry",    "/bug-buddy/last/email_address", "email-email-entry" },
	{ CONFIG_ENTRY,  "email-to-gnome-entry" },
	{ CONFIG_ENTRY,  "email-cc-gnome-entry" },
	{ CONFIG_MAILER, "email-sendmail-gnome-entry", "/bug-buddy/last/mailer",        "email-sendmail-entry" },
	{ CONFIG_ENTRY,  "email-file-gnome-entry",     "/bug-buddy/last/bugfile",       "email-file-entry" },
	{ CONFIG_TOGGLE, "email-cc-toggle",            "/bug-buddy/last/cc=0" },
	{ CONFIG_DONE }
};

void
save_config (void)
{
	ConfigItem *item;
	GtkWidget *w;
	gboolean b;
	char *s;

	d(g_print ("saving config...\n"));

	for (item = configs; item->t; item++) {
		if (item->t == CONFIG_TOGGLE) {
			b = gtk_toggle_button_get_active (
				GTK_TOGGLE_BUTTON (GET_WIDGET (item->w)));
			gnome_config_set_bool (item->path, b);
			continue;
		}

		if (item->path) {
			w = GET_WIDGET (item->w2);;
			s = gtk_entry_get_text (GTK_ENTRY (w));
			gnome_config_set_string (item->path, s);
		} else
			s = NULL;
		
		w = GET_WIDGET (item->w);
		if (GNOME_IS_FILE_ENTRY (w))
			w = gnome_file_entry_gnome_entry (GNOME_FILE_ENTRY (w));
		if (s && *s)
			gnome_entry_prepend_history (GNOME_ENTRY (w), TRUE, s);

		gnome_entry_save_history (GNOME_ENTRY (w));
	}

	gnome_config_set_int ("/bug-buddy/last/submittype", 
			      druid_data.submit_type);

	gnome_config_set_bool ("/bug-buddy/last/already_run", 1);
			       
	gnome_config_sync ();
}

void
load_config (void)
{
	ConfigItem *item;
	GtkWidget *w;
	char *def = NULL, *d2;
	
	d(g_print ("loading config...\n"));

	for (item = configs; item->t; item++) {
		switch (item->t) {
		case CONFIG_TOGGLE:
			gtk_toggle_button_set_active (
				GTK_TOGGLE_BUTTON (GET_WIDGET (item->w)),
				gnome_config_get_bool (item->path));
			continue;
		case CONFIG_USER:
			def = g_strdup (g_get_real_name ());
			break;
		case CONFIG_MAILER:
			def = gnome_is_program_in_path ("sendmail");
			if (!def) {
				if (g_file_exists ("/usr/sbin/sendmail"))
					def = g_strdup ("/usr/sbin/sendmail");
				else if (g_file_exists ("/usr/lib/sendmail"))
					def = g_strdup ("/usr/lib/sendmail");
			}
			break;
		default:
			break;
		}

		if (item->w2) {
			w = GET_WIDGET (item->w2);
			if (item->path) {
				d2 = gnome_config_get_string (item->path);
				if (d2) {
					g_free (def);
					def = d2;
				}
			}
			if (def)
				gtk_entry_set_text (GTK_ENTRY (w), def);
		}

		g_free (def);
		def = NULL;

		w = GET_WIDGET (item->w);
		if (GNOME_IS_FILE_ENTRY (w))
			w = gnome_file_entry_gnome_entry (GNOME_FILE_ENTRY (w));

		gnome_entry_load_history (GNOME_ENTRY (w));
	}

	druid_data.submit_type =
		gnome_config_get_int ("/bug-buddy/last/submittype");

	druid_data.already_run =
		gnome_config_get_bool ("/bug-buddy/last/already_run=0");

	druid_data.state = 0;
}
