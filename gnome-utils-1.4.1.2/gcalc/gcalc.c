/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* Simple Double precision calculator using the GnomeCalculator widget
   Copyright (C) 1998 Free Software Foundation

   Author: George Lebl <jirka@5z.com>
*/

#include <config.h>
#include <gnome.h>
#include <libgnomeui/gnome-window-icon.h>

#include "gnome-calc.h"
#include "sr.h"

/* values for selection info */
enum {
  TARGET_STRING,
  TARGET_TEXT,
  TARGET_COMPOUND_TEXT,
};

static GtkWidget *app;
static GtkWidget *calc;
static char copied_string[13]="";

static void
about_cb (GtkWidget *widget, gpointer data)
{
	static GtkWidget *about = NULL;
	gchar *authors[] = {
		"George Lebl <jirka@5z.com>",
		NULL
	};

	if (about != NULL)
	{
		gdk_window_show(about->window);
		gdk_window_raise(about->window);
		return;
	}
	about = gnome_about_new(_("The Gnome Calculator"), VERSION,
				"(C) 1998 the Free Software Foundation",
				(const char **)authors,
				_("Simple double precision calculator similiar "
				  "to xcalc"),
				NULL);
	gtk_signal_connect(GTK_OBJECT(about), "destroy",
			   GTK_SIGNAL_FUNC(gtk_widget_destroyed), &about);
	gtk_widget_show (about);
}

static void
quit_cb (GtkWidget *widget, gpointer data)
{
	gtk_main_quit ();
}

/* Callback when the user toggles the selection */
static void
copy_contents (GtkWidget *widget, gpointer data)
{
	strcpy(copied_string, gnome_calc_get_result_string(GNOME_CALC (calc)));
	g_strstrip(copied_string);
	gtk_selection_owner_set (app,
				 GDK_SELECTION_PRIMARY,
				 GDK_CURRENT_TIME);
}


static void
selection_handle (GtkWidget *widget, 
		  GtkSelectionData *selection_data,
		  int info, int time,
		  gpointer data)
{
	if (info == TARGET_STRING) {
		gtk_selection_data_set (selection_data,
					GDK_SELECTION_TYPE_STRING,
					8*sizeof(gchar),
					(guchar *)copied_string,
					strlen(copied_string));
	} else if ((info == TARGET_TEXT) || (info == TARGET_COMPOUND_TEXT)) {
		guchar *text;
		GdkAtom encoding;
		gint format;
		gint new_length;

		gdk_string_to_compound_text (copied_string, &encoding, &format,
					     &text, &new_length);
		gtk_selection_data_set (selection_data, encoding, format,
					text, new_length);
		gdk_free_compound_text (text);
	}
}

/* Menus */
static GnomeUIInfo file_menu[] = {
	GNOMEUIINFO_MENU_EXIT_ITEM(quit_cb,NULL),
	GNOMEUIINFO_END
};

static GnomeUIInfo edit_menu[] = {
	GNOMEUIINFO_MENU_COPY_ITEM(copy_contents,NULL),
	GNOMEUIINFO_END
};

static GnomeUIInfo help_menu[] = {
	GNOMEUIINFO_HELP("gcalc"),
	GNOMEUIINFO_MENU_ABOUT_ITEM(about_cb,NULL),
	GNOMEUIINFO_END
};

static GnomeUIInfo gcalc_menu[] = {
	GNOMEUIINFO_MENU_FILE_TREE(file_menu),
	GNOMEUIINFO_MENU_EDIT_TREE(edit_menu),
	GNOMEUIINFO_MENU_HELP_TREE(help_menu),
        GNOMEUIINFO_END
};

static gboolean analog = FALSE;

struct poptOption options [] = {
  { "analog", '\0', POPT_ARG_NONE|POPT_ARGFLAG_ONEDASH, &analog, 0,
    N_("Run in analog (slide rule) mode"), NULL },
  POPT_AUTOHELP
  { NULL, 0, 0, NULL, 0}
};


int
main(int argc, char *argv[])
{
	static GtkTargetEntry targets[] = {
		{ "STRING", TARGET_STRING },
		{ "TEXT",   TARGET_TEXT }, 
		{ "COMPOUND_TEXT", TARGET_COMPOUND_TEXT }
	};
	static gint n_targets = sizeof(targets) / sizeof(targets[0]);
	
	/* Initialize the i18n stuff */
	bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);

	gnome_init_with_popt_table ("gcalc", VERSION, argc, argv, options, 0, NULL);
	gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gnome-calc2.png");

	if (analog) {
		run_slide_rule ();
		return 0;
	}

        app = gnome_app_new("gcalc", _("Gnome Calculator"));
	gtk_window_set_wmclass (GTK_WINDOW (app), "gcalc", "gcalc");
	gtk_window_set_policy (GTK_WINDOW (app), FALSE, TRUE, FALSE);

        gtk_signal_connect(GTK_OBJECT(app), "delete_event",
			GTK_SIGNAL_FUNC(quit_cb), NULL);

	/*set up the menu*/
        gnome_app_create_menus(GNOME_APP(app), gcalc_menu);

	calc = gnome_calc_new();
	gnome_calc_bind_extra_keys (GNOME_CALC (calc), GTK_WIDGET (app));
	gtk_widget_show(calc);

	gtk_selection_add_targets (GTK_WIDGET (app), GDK_SELECTION_PRIMARY,
				   targets, n_targets);

        gtk_signal_connect(GTK_OBJECT(app), "selection_get",
			   GTK_SIGNAL_FUNC(selection_handle), NULL);

	gnome_app_set_contents(GNOME_APP(app), calc);

	/* add calculator accel table to our window*/
	gtk_window_add_accel_group(GTK_WINDOW(app),
				   gnome_calc_get_accel_group(GNOME_CALC(calc)));

	gtk_widget_show(app);

	gtk_main ();

	return 0;
}
