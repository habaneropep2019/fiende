/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*-

   eel-debug.c: Eel debugging aids.
 
   Copyright (C) 2000, 2001 Eazel, Inc.
  
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
  
   You should have received a copy of the GNU General Public
   License along with this program; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
  
   Author: Darin Adler <darin@eazel.com>
*/

#include <config.h>
#include "eel-debug.h"

#include "eel-glib-extensions.h"
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

/* Try and read the command line used to run the current process.
 * On Linux, the file /proc/self/cmdline contains this information.
 * In that case the function returns a constant static string with
 * the command line.
 *
 * On platform where this feature is not available, the result is
 * NULL.
 */
static const char *
get_process_name (void)
{
	static char name[_POSIX_PATH_MAX];
	static gboolean cmdline_read;
	FILE *stream;

	if (!cmdline_read) {
		cmdline_read = TRUE;

		stream = fopen ("/proc/self/cmdline", "r");
		if (stream != NULL) {
			if (fgets (name, sizeof (name), stream) == NULL) {
				name[0] = '\0';
			}
			fclose (stream);
		}
	}

	return name[0] != '\0' ? name : NULL;
}

/* Raise a SIGINT signal to get the attention of the debugger.
   When not running under the debugger, we don't want to stop,
   so we ignore the signal for just the moment that we raise it.
*/
void
eel_stop_in_debugger (void)
{
	void (* saved_handler) (int);

	saved_handler = signal (SIGINT, SIG_IGN);
	raise (SIGINT);
	signal (SIGINT, saved_handler);
}

static void
call_default_log_handler_with_better_message (const char *domain,
					      GLogLevelFlags level,
					      const char *message,
					      gpointer data)
{
	const char *name;
	char *better_message;

	/* FIXME: Need to make a version that doesn't allocate memory. */

	/* If the command line is known (non-zero length) then print
	 * that out along with the process id to make the warning or
	 * critical more useful.
	 *
	 * If the command line is not known, then just print the
	 * process id.
	 */
	name = get_process_name ();
	if (name != NULL) {
		better_message = g_strdup_printf ("%s(%lu): %s",
						  name,
						  (unsigned long) getpid (),
						  message);
	} else {
		better_message = g_strdup_printf ("%lu: %s",
						  (unsigned long) getpid (),
						  message);
	}
	g_log_default_handler (domain, level, better_message, data);
	g_free (better_message);
}

/* Stop in the debugger after running the default log handler.
   This makes certain kinds of messages stop in the debugger
   without making them fatal.
*/
static void
log_handler (const char *domain,
	     GLogLevelFlags level,
	     const char *message,
	     gpointer data)
{
	call_default_log_handler_with_better_message (domain, level, message, data);
	if ((level & (G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_WARNING)) != 0) {
		eel_stop_in_debugger ();
	}
}

static void
set_log_handler (const char *domain)
{
	g_log_set_handler (domain, G_LOG_LEVEL_MASK, log_handler, NULL);
}

void
eel_make_warnings_and_criticals_stop_in_debugger (const char *first_domain, ...)
{
	va_list domains;
	const char *domain;
	guint i;

	/* This is a workaround for the fact that there is not way to 
	 * make this useful debugging feature happen for ALL domains.
	 *
	 * What we did here is list all the ones we could think of that
	 * were interesting to us. It's ok to add more to the list.
	 */
	static const char * const standard_log_domains[] = {
		"",
		"Bonobo",
		"Eel",
		"Gdk",
		"Gdk-Pixbuf",
		"Gnome",
		"GnomeUI",
		"GnomeVFS",
		"GnomeVFS-pthread",
		"Gtk",
		"Nautilus",
		"Nautilus-Adapter",
		"Nautilus-Hardware"
		"Nautilus-Mozilla",
		"Nautilus-Music",
		"Nautilus-Notes",
		"Nautilus-Sample",
		"Nautilus-Test",
		"Nautilus-Text",
		"Nautilus-Throbber-Control",
		"Nautilus-Tree",
		"Nautilus-Tree",
		"NautilusErrorDialog",
		"ORBit"
	};

	set_log_handler (first_domain);
	va_start (domains, first_domain);
	for (;;) {
		domain = va_arg (domains, const char *);
		if (domain == NULL) {
			break;
		}
		set_log_handler (domain);
	}
	va_end (domains);

	set_log_handler (g_log_domain_glib);
	for (i = 0; i < EEL_N_ELEMENTS (standard_log_domains); i++) {
		set_log_handler (standard_log_domains[i]);
	}
}

int 
eel_get_available_file_descriptor_count (void)
{
	int count;
	GList *list;
	GList *p;
	FILE *file;

	list = NULL;
	for (count = 0; ; count++) {
		file = fopen("/dev/null", "r");
		if (file == NULL) {
			break;
		}
		list = g_list_prepend (list, file);
	}

	for (p = list; p != NULL; p = p->next) {
		fclose (p->data);
	}
	g_list_free (list);

	return count;
}
