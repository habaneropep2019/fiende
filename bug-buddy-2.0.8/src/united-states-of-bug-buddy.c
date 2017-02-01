/* bug-buddy bug submitting program
 *
 * Copyright (C) 2000 Jacob Berkman
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

#include "libglade-buddy.h"

static char *help_pages[] = {
	"index.html",
	"debug-info.html",
	"description.html",
	"updating.html",
	"product.html",
	"component.html",
	"system-config.html",
	"submit-report.html",
	"summary.html",
	NULL
};

static char *state_title[] = {
	N_("Welcome to Bug Buddy"),
	N_("Debugging Information"),
	N_("Bug Description"),
	N_("Updating Product Listing"),
	N_("Product"),
	N_("Component"),
	N_("System Configuration"),
	N_("Submitting the Report"),
	N_("Finished!"),
	NULL
};

#define d(x)

void
druid_set_sensitive (gboolean prev, gboolean next, gboolean cancel)
{
	gtk_widget_set_sensitive (GET_WIDGET ("druid-prev"), prev);
	gtk_widget_set_sensitive (GET_WIDGET ("druid-next"), next);
	gtk_widget_set_sensitive (GET_WIDGET ("druid-cancel"), cancel);
}

void
on_druid_help_clicked (GtkWidget *w, gpointer data)
{
	GnomeHelpMenuEntry help_entry = { "bug-buddy", NULL };
	help_entry.path = help_pages[druid_data.state];
	gnome_help_display (NULL, &help_entry);
}

void
on_druid_about_clicked (GtkWidget *button, gpointer data)
{
	static GtkWidget *about, *href;
	static const char *authors[] = {
		"Jacob Berkman  <jacob@bug-buddy.org>",
		NULL
	};

	if (about) {
		gdk_window_show (about->window);
		gdk_window_raise (about->window);
		return;
	}
		
	about = gnome_about_new (_(PACKAGE), VERSION,
				 _("The graphical bug reporting tool for GNOME."),
				 authors,
				 "Copyright (C) 1999 - 2001 Jacob Berkman\n"
				 "Copyright 2000, 2001 Ximian, Inc.",
				 BUDDY_ICONDIR"/bug-buddy.png");
	gtk_signal_connect (GTK_OBJECT (about), "destroy",
			    GTK_SIGNAL_FUNC (gtk_widget_destroyed),
			    &about);

	href = gnome_href_new ("http://bug-buddy.org/",
			       _("The lame Bug Buddy web page"));
	gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (about)->vbox),
			    href, FALSE, FALSE, 0);

	gtk_widget_show (href);
	gtk_widget_show (about);
}

void
druid_set_state (BuddyState state)
{
	static gboolean been_here = FALSE;
	BuddyState oldstate;
	GtkWidget *w;
	char *s;
	int pos = 0;

	g_return_if_fail (state >= 0);
	g_return_if_fail (state < STATE_LAST);

	if (druid_data.state == state && been_here)
		return;

	been_here = TRUE;

	oldstate = druid_data.state;
	druid_data.state = state;

	gtk_widget_set_sensitive (GET_WIDGET ("druid-prev"),
				  (state > 0));

	gtk_widget_set_sensitive (GET_WIDGET ("druid-next"),
				  (state < STATE_FINISHED));

	gnome_canvas_item_set (druid_data.banner,
			       "text", _(state_title[state]),
			       NULL);

	gtk_notebook_set_page (GTK_NOTEBOOK (GET_WIDGET ("druid-notebook")),
			       state);	

	switch (druid_data.state) {
	case STATE_INTRO:
		/* check what gdb'ing we are going to do, and print
		 * the right message */
		break;
	case STATE_GDB:
		start_gdb ();
		break;
	case STATE_DESC:
		/* nothing to do */
		break;
	case STATE_UPDATE:
		if (oldstate == STATE_PRODUCT) {
			druid_set_state (state - 1);
			break;
		}
		if (druid_data.all_products) {
			druid_set_state (state + 1);
			return;
		}
		druid_set_sensitive (FALSE, FALSE, FALSE);
		load_bugzillas ();
		break;
	case STATE_PRODUCT:
#if 0
		if (!druid_data.package_name)
			determine_our_package ();
		gtk_entry_set_text (GTK_ENTRY (GET_WIDGET ("bts-package-entry")),
				    druid_data.package_name
				    ? druid_data.package_name
				    : "");
#endif
		load_bugzilla_xml ();
#if 0
		if (!druid_data.product)
			druid_set_sensitive (TRUE, FALSE,  TRUE);
#endif
		break;
	case STATE_COMPONENT:
#if 0
		if (!druid_data.component)
			druid_set_sensitive (TRUE, FALSE,  TRUE);
#endif
		break;
	case STATE_SYSTEM:
#if 0
		/* start the process of version checking if we haven't
		 * run anything or the list of thingies has changed */
		do_dependency_stuff ();
#endif
		druid_set_state (state - 1);
		break;
	case STATE_EMAIL:
		/* fill in the content text */
		s = generate_email_text ();
		w = GET_WIDGET ("email-text");
		gtk_editable_delete_text (GTK_EDITABLE (w), 0, -1);
		gtk_editable_insert_text (GTK_EDITABLE (w), s, strlen (s), &pos);
		g_free (s);
		break;
	case STATE_FINISHED:
		/* print a summary yo */
		gtk_widget_hide (GET_WIDGET ("druid-prev"));
		gtk_widget_hide (GET_WIDGET ("druid-next"));
		gtk_widget_show (GET_WIDGET ("druid-finish"));
		gtk_widget_hide (GET_WIDGET ("druid-cancel"));
		break;
	default:
		g_assert_not_reached ();
		break;
	}
}

void
on_druid_prev_clicked (GtkWidget *w, gpointer data)
{
	druid_set_state (druid_data.state - 1);
}


static gboolean
email_is_valid (const char *addy)
{
	char *rev;

	if (!addy || strlen (addy) < 4 || !strchr (addy, '@') || strstr (addy, "@."))
		return FALSE;

	g_strreverse (rev = g_strdup (addy));
	
	/* assume that the country thingies are ok */
	if (rev[2] == '.') {
		g_free (rev);
		return TRUE ;
	}

	if (g_strncasecmp (rev, "moc.", 4) &&
	    g_strncasecmp (rev, "gro.", 4) &&
	    g_strncasecmp (rev, "ten.", 4) &&
	    g_strncasecmp (rev, "ude.", 4) &&
	    g_strncasecmp (rev, "lim.", 4) &&
	    g_strncasecmp (rev, "vog.", 4) &&
	    g_strncasecmp (rev, "tni.", 4) &&
	    g_strncasecmp (rev, "apra.", 5) &&

	    /* In the year 2000, seven new toplevel domains were approved by ICANN. */
	    g_strncasecmp (rev, "orea.", 5) &&
	    g_strncasecmp (rev, "zib.", 4) &&
	    g_strncasecmp (rev, "pooc.", 5) &&
	    g_strncasecmp (rev, "ofni.", 5) &&
	    g_strncasecmp (rev, "muesum.", 5) &&
	    g_strncasecmp (rev, "eman.", 5) &&
	    g_strncasecmp (rev, "orp.", 5)) {
		g_free (rev);
		return FALSE;
	}

	g_free (rev);

	return TRUE;
}

/* return true if page is ok */
static gboolean
intro_page_ok (void)
{
	GtkWidget *w;
	gchar *s;

	w = GET_WIDGET ("email-name-entry");
	s = gtk_entry_get_text (GTK_ENTRY (w));
	if (! (s && strlen (s))) {
		w = gnome_message_box_new (_("Please enter your name."),
					   GNOME_MESSAGE_BOX_ERROR,
					   GNOME_STOCK_BUTTON_OK,
					   NULL);
		gnome_dialog_run_and_close (GNOME_DIALOG (w));
		return FALSE;
	}

	w = GET_WIDGET ("email-email-entry");
	s = gtk_entry_get_text (GTK_ENTRY (w));
	if (!email_is_valid (s)) {
		w = gnome_message_box_new (_("Please enter a valid email address."),
					   GNOME_MESSAGE_BOX_ERROR,
					   GNOME_STOCK_BUTTON_OK,
					   NULL);
		gnome_dialog_run_and_close (GNOME_DIALOG (w));
		return FALSE;
	}

	w = GET_WIDGET ("email-sendmail-entry");
	s = gtk_entry_get_text (GTK_ENTRY (w));
	if (! (s && strlen (s) && g_file_exists (s))) {
		if (druid_data.submit_type == SUBMIT_TO_SELF ||
		    druid_data.submit_type == SUBMIT_REPORT) {
			GtkWidget *d;
			char *m;
			m = g_strdup_printf (_("'%s' doesn't seem to exist.\n\n"
					       "You won't be able to actually "
					       "submit a bug report, but you will\n"
					       "be able to save it to a file.\n\n"
					       "Specify a new location for sendmail?"),
					     s);	
			d = gnome_question_dialog (m, NULL, NULL);
			g_free (m);
			if (GNOME_YES == gnome_dialog_run_and_close (GNOME_DIALOG (d)))
				return FALSE;
		}
	}

	return TRUE;
}

static gboolean
text_is_sensical (const gchar *text, int sensitivity)
{
        /* If there are less than eight unique characters, 
	   it is probably nonsenical.  Also require a space */
	int chars[256] = { 0 };
	guint uniq = 0;

	if (!text || !*text)
		return FALSE;
	
	for ( ; *text; text++)
		if (!chars[(guchar)*text])
			chars[(guchar)*text] = ++uniq;
	
	d(g_message ("%d", uniq));

	return chars[' '] && uniq >= sensitivity;
 }

static gboolean
desc_page_ok (void)
{
	GtkWidget *w;

	char *s = gtk_entry_get_text (GTK_ENTRY (GET_WIDGET ("desc-file-entry")));

	if (getenv ("BUG_ME_HARDER"))
		return TRUE;

	if (s && *s) {
		const char *mime_type;
		if (!g_file_exists (s)) {
			w = gnome_error_dialog (
				_("The specified file does not exist."));
			gnome_dialog_run_and_close (GNOME_DIALOG (w));
			return FALSE;
		}

		mime_type = gnome_mime_type_of_file (s);
		d(g_message (_("File is of type: %s"), mime_type));
		
		if (!mime_type || strncmp ("text/", mime_type, 5)) {
			char *msg = g_strdup_printf (_("'%s' does not look like a text file."), s);
			gnome_dialog_run_and_close (
				GNOME_DIALOG (gnome_error_dialog (msg)));
			g_free (msg);
			return FALSE;
		}
	}
	
	s = gtk_entry_get_text (GTK_ENTRY (GET_WIDGET ("desc-subject")));
	if (!text_is_sensical (s, 6)) {
		w = gnome_error_dialog (
			_("You must include a comprehensible subject line in your bug report."));
		gnome_dialog_run_and_close (GNOME_DIALOG (w));
		return FALSE;
	}
	
	s = gtk_editable_get_chars (GTK_EDITABLE (GET_WIDGET ("desc-text")), 0, -1);
	if (!text_is_sensical (s, 8)) {
		w = gnome_error_dialog (
			_("You must include a comprehensible description in your bug report."));		
		gnome_dialog_run_and_close (GNOME_DIALOG (w));
		g_free (s);
		return FALSE;
	}

	return TRUE;
}

static gboolean
submit_ok (void)
{
	gchar *to, *s, *s2, *file=NULL, *command;
	GtkWidget *w;
	FILE *fp;

	if (druid_data.submit_type != SUBMIT_FILE) {
		w = gnome_question_dialog (_("Submit this bug report now?"),
					   NULL, NULL);
		if (gnome_dialog_run_and_close (GNOME_DIALOG (w)))
			return FALSE;
	}

	w = GET_WIDGET ( (druid_data.submit_type == SUBMIT_TO_SELF)
			 ? "email-email-entry" : "email-to-entry");
	to = gtk_editable_get_chars (GTK_EDITABLE (w), 0, -1);

	if (druid_data.submit_type == SUBMIT_FILE) {
		file = gtk_entry_get_text (GTK_ENTRY (GET_WIDGET ("email-file-entry")));
		fp = fopen (file, "w");
		if (!fp) {
			s = g_strdup_printf (_("Unable to open file: '%s'"), file);
			w = gnome_error_dialog (s);
			g_free (s);
			g_free (to);
			gnome_dialog_run_and_close (GNOME_DIALOG (w));
			return FALSE;
		}
	} else {
		w = GET_WIDGET ("email-sendmail-entry");
		s = gtk_editable_get_chars (GTK_EDITABLE (w), 0, -1);
		command = g_strdup_printf ("%s -i -t", s);

		d(g_message (_("about to run '%s'"), command));
		fp =  popen (command, "w");
		g_free (command);
		if (!fp) {
			s2 = g_strdup_printf (_("Unable to start mail program: '%s'"), s);
			w = gnome_error_dialog (s2);
			g_free (s);
			g_free (s2);
			gnome_dialog_run_and_close (GNOME_DIALOG (w));
			return FALSE;
		}
		g_free (s);
	}

	{
		char *name, *from;
		
		w = GET_WIDGET ("email-name-entry");
		name = gtk_editable_get_chars (GTK_EDITABLE (w), 0, -1);


		w = GET_WIDGET ("email-email-entry");
		from = gtk_editable_get_chars (GTK_EDITABLE (w), 0, -1);

		fprintf (fp, "From: %s <%s>\n", name, from);
		g_free (from);
		g_free (name);
	}

	fprintf (fp, "To: %s\n", to);

	if (druid_data.submit_type == SUBMIT_REPORT &&
	    GTK_TOGGLE_BUTTON (GET_WIDGET ("email-cc-toggle"))->active) {
		w = GET_WIDGET ("email-email-entry");
		s = gtk_editable_get_chars (GTK_EDITABLE (w), 0, -1);
		fprintf (fp, "Cc: %s\n", s);
		g_free (s);
	}
	
	w = GET_WIDGET ("email-cc-entry");
	s = gtk_editable_get_chars (GTK_EDITABLE (w), 0, -1);
	if (*s) fprintf (fp, "Cc: %s\n", s);
	g_free (s);

	fprintf (fp, "X-Mailer: %s %s\n", PACKAGE, VERSION);

	w = GET_WIDGET ("email-text");
	s = gtk_editable_get_chars (GTK_EDITABLE (w), 0, -1);
	fprintf (fp, "%s", s);
	g_free (s);

	if (druid_data.submit_type == SUBMIT_FILE) {
		fclose (fp);
		s = g_strdup_printf (_("Your bug report was saved in '%s'"), file);
	} else {
		pclose (fp);
		s = g_strdup_printf (_("Your bug report has been submitted to:\n\n        <%s>\n\nThanks!"), to);
	}
	g_free (to);

	w = GET_WIDGET ("finished-label");
	gtk_label_set_text (GTK_LABEL (w), s);
	g_free (s);

	return TRUE;
}

void
on_druid_next_clicked (GtkWidget *w, gpointer data)
{
	BuddyState newstate;
	char *s;

	newstate = druid_data.state + 1;

	switch (druid_data.state) {
	case STATE_INTRO:
		/* validate email and sendmail */
		if (!intro_page_ok ())
			return;
		break;
	case STATE_GDB:
		/* nothing */
		break;
	case STATE_DESC:
		/* validate subject, description, and file name */
		if (!desc_page_ok ())
			return;
		break;
	case STATE_PRODUCT:
		/* check that the package is ok */
		if (!druid_data.product) {
			gnome_dialog_run_and_close (
				GNOME_DIALOG (gnome_error_dialog (
						      _("You must specify a product for your bug report."))));
			return;
		}
		bugzilla_product_add_components_to_clist (druid_data.product);
		break;
	case STATE_COMPONENT:
		if (!druid_data.component) {
			gnome_dialog_run_and_close (
				GNOME_DIALOG (gnome_error_dialog (
						      _("You must specify a component for your bug report."))));
			return;
		}
		s = gtk_editable_get_chars (GTK_EDITABLE (GET_WIDGET ("the-version-entry")), 0, -1);
		if (!s[0] && !getenv ("BUG_ME_HARDER")) {
			g_free (s);
			gnome_dialog_run_and_close (
				GNOME_DIALOG (gnome_error_dialog (
						      _("You must specify a version for your bug report."))));
			return;
		}
		g_free (s);
		newstate++;
		break;
	case STATE_SYSTEM:
		/* nothing */
		break;
	case STATE_EMAIL:
		/* validate included file.
		 * prompt that we should actually do anything  */
		if (!submit_ok ())
			return;
		break;
	default:
		g_assert_not_reached ();
		break;
	}

	druid_set_state (newstate);
}

void
on_druid_cancel_clicked (GtkWidget *w, gpointer data)
{
	GtkWidget *d;

	d = gnome_question_dialog (
		_("Are you sure you want to cancel\n"
		  "this bug report?"), NULL, NULL);
	if (gnome_dialog_run_and_close (GNOME_DIALOG (d)))
		return;

	save_config ();
	gtk_main_quit ();
}
