/* testGNOME - program similar to testgtk which shows gnome lib functions.
 *
 * Author : Richard Hestilow <hestgray@ionet.net>
 *
 * Copyright (C) 1998 Free Software Foundation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include <config.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <gdk_imlib.h>

#include "testgnome.h"
#include "bomb.xpm"

static const gchar *authors[] = {
	"Richard Hestilow",
	"Federico Mena",
	"Eckehard Berns",
	"Havoc Pennington",
	"Miguel de Icaza",
	"Jonathan Blandford",
	NULL
};

static void
delete_event (GtkWidget *widget, gpointer data)
{
	gtk_widget_destroy(data);
}

static void
create_about (void)
{
        GtkWidget *about;
        about = gnome_about_new("GNOME Test Program", VERSION ,
                                "(C) 1998 The Free Software Foundation",
                                authors,
                                "Program to display GNOME functions.",
                                NULL);
        gtk_widget_show (about);
}

static void
create_date_edit (void)
{
	GtkWidget *datedit;
	GtkWidget *win;
	time_t curtime = time(NULL);

	datedit = gnome_date_edit_new(curtime,1,1);
	win = create_newwin(TRUE,"testGNOME","Date Edit");
	gnome_app_set_contents(GNOME_APP(win),datedit);
	gtk_widget_show(datedit);
	gtk_widget_show(win);
}

static void
quit_test (void)
{
        gtk_main_quit ();
}

static void
window_close (GtkWidget *widget, gpointer data)
{
        gtk_widget_destroy (GTK_WIDGET(data));
}

static GnomeUIInfo file_menu[] = {
        { GNOME_APP_UI_ITEM, "Test", NULL, gtk_main_quit, NULL, NULL,
	  GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_EXIT, 'A',
	  GDK_SHIFT_MASK, NULL },
        { GNOME_APP_UI_ITEM, "Exit", NULL, quit_test, NULL, NULL,
	  GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_EXIT, 'X',
	  GDK_CONTROL_MASK, NULL },
        { GNOME_APP_UI_ITEM, "Close", NULL, window_close, NULL, NULL,
	  GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_EXIT, 'X',
	  GDK_CONTROL_MASK, NULL },
        { GNOME_APP_UI_ENDOFINFO }
};

static GnomeUIInfo help_menu[] = {
        { GNOME_APP_UI_ITEM, "About...", NULL, create_about, NULL, NULL,
	  GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_ABOUT, 0, 0,
	  NULL },
        { GNOME_APP_UI_ENDOFINFO }
};

static GnomeUIInfo main_menu[] = {
        { GNOME_APP_UI_SUBTREE, ("File"), NULL, file_menu, NULL, NULL,
	  GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL },
        { GNOME_APP_UI_SUBTREE, ("Help"), NULL, help_menu, NULL, NULL,
	  GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL },
        { GNOME_APP_UI_ENDOFINFO }
};

GtkWidget *
create_newwin(gboolean normal, gchar *appname, gchar *title)
{
	GtkWidget *app;

	app = gnome_app_new (appname,title);
	if (!normal)
        {
                gtk_signal_connect(GTK_OBJECT(app), "delete_event",
				   GTK_SIGNAL_FUNC(quit_test), NULL);
        };

	gnome_app_create_menus_with_data (GNOME_APP(app), main_menu, app);
	return GTK_WIDGET(app);
}

static void
create_calc(void)
{
	GtkWidget *app,*calc;
	app = create_newwin(TRUE,"testGNOME","Calculator");
	calc = gnome_calculator_new();
	gnome_app_set_contents(GNOME_APP(app),calc);
	gtk_widget_show(calc);
	gtk_widget_show(app);
}

static void
create_clock(void)
{
	GtkWidget *app;
	GtkWidget *clock;

	app=create_newwin(TRUE,"testGNOME","Clock");
	clock=gtk_clock_new(0);
	gnome_app_set_contents(GNOME_APP(app),clock);
	gtk_clock_set_seconds(GTK_CLOCK(clock),0);
        gtk_clock_start(GTK_CLOCK(clock));
	gtk_widget_show(clock);
	gtk_widget_show(app);
}

/* Creates a color picker with the specified parameters */
static void
create_cp (GtkWidget *table, int dither, int use_alpha, int left, int right, int top, int bottom)
{
	GtkWidget *cp;

	cp = gnome_color_picker_new ();
	gnome_color_picker_set_dither (GNOME_COLOR_PICKER (cp), dither);
	gnome_color_picker_set_use_alpha (GNOME_COLOR_PICKER (cp), use_alpha);
	gnome_color_picker_set_d (GNOME_COLOR_PICKER (cp), 1.0, 0.0, 1.0, 0.5);

	gtk_table_attach (GTK_TABLE (table), cp,
			  left, right, top, bottom,
			  0, 0, 0, 0);
	gtk_widget_show (cp);
}

static void
create_color_picker (void)
{
	GtkWidget *app;
	GtkWidget *table;
	GtkWidget *w;

	app = create_newwin (TRUE, "testGNOME", "Color Picker");

	table = gtk_table_new (3, 3, FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (table), GNOME_PAD_SMALL);
	gtk_table_set_row_spacings (GTK_TABLE (table), GNOME_PAD_SMALL);
	gtk_table_set_col_spacings (GTK_TABLE (table), GNOME_PAD_SMALL);
	gnome_app_set_contents (GNOME_APP (app), table);
	gtk_widget_show (table);

	/* Labels */

	w = gtk_label_new ("Dither");
	gtk_table_attach (GTK_TABLE (table), w,
			  1, 2, 0, 1,
			  GTK_FILL,
			  GTK_FILL,
			  0, 0);
	gtk_widget_show (w);

	w = gtk_label_new ("No dither");
	gtk_table_attach (GTK_TABLE (table), w,
			  2, 3, 0, 1,
			  GTK_FILL,
			  GTK_FILL,
			  0, 0);
	gtk_widget_show (w);

	w = gtk_label_new ("No alpha");
	gtk_table_attach (GTK_TABLE (table), w,
			  0, 1, 1, 2,
			  GTK_FILL,
			  GTK_FILL,
			  0, 0);
	gtk_widget_show (w);

	w = gtk_label_new ("Alpha");
	gtk_table_attach (GTK_TABLE (table), w,
			  0, 1, 2, 3,
			  GTK_FILL,
			  GTK_FILL,
			  0, 0);
	gtk_widget_show (w);

	/* Color pickers */

	create_cp (table, TRUE,  FALSE, 1, 2, 1, 2);
	create_cp (table, FALSE, FALSE, 2, 3, 1, 2);
	create_cp (table, TRUE,  TRUE,  1, 2, 2, 3);
	create_cp (table, FALSE, TRUE,  2, 3, 2, 3);

	gtk_widget_show (app);
}

/*
 * GnomeDruid
 */


typedef struct druid_data
{
	GtkWidget *radio_button; /* if set, goto A, else goto b */
	GtkWidget *target_a;
	GtkWidget *target_b;
} druid_data;

static gboolean
simple_druid_next_callback (GnomeDruidPage *page, GnomeDruid *druid, GnomeDruidPage *next)
{
	gtk_object_set_data (GTK_OBJECT (next), "back", page);
	gnome_druid_set_page (druid,
			      next);
	return TRUE;
}
static gboolean
complex_druid_next_callback (GnomeDruidPage *page, GnomeDruid *druid, druid_data *data)
{
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (data->radio_button))) {
		gtk_object_set_data (GTK_OBJECT (data->target_a), "back", page);
		gnome_druid_set_page (druid,
				      GNOME_DRUID_PAGE (data->target_a));
	} else {
		gtk_object_set_data (GTK_OBJECT (data->target_b), "back", page);
		gnome_druid_set_page (druid,
				      GNOME_DRUID_PAGE (data->target_b));
	}
	return TRUE;
}
static gboolean
druid_back_callback (GnomeDruidPage *page, GnomeDruid *druid, gpointer data)
{
	GtkWidget *back_page = NULL;
	back_page = gtk_object_get_data (GTK_OBJECT (page), "back");
	if (back_page) {
		gtk_object_set_data (GTK_OBJECT (page), "back", NULL);
		gnome_druid_set_page (druid,
				      GNOME_DRUID_PAGE (back_page));
		return TRUE;
	}
	return FALSE;
}

/*
 * The Druid's control flow looks something like this:
 *
 * page_start -> page_a -> page_b -> page_d -> page_f -> page_finish
 *                      |          \                  /
 *                      |            page_e -> page_g
 *                       \                  /
 *                         page_c ----------
 */
static void
create_druid(void)
{
  GtkWidget *window;
  GtkWidget *druid;
  gchar *fname;
  GtkWidget *page_start, *page_finish;
  GtkWidget *page_a, *page_b, *page_c, *page_d, *page_e, *page_f, *page_g;
  GdkImlibImage *logo = NULL;
  GdkImlibImage *watermark = NULL;
  GtkWidget *check_a, *check_b;
  GSList *radio_group;
  druid_data *data;

  /* load the images */
  fname = gnome_pixmap_file ("gnome-logo-icon.png");
  if (fname)
    logo = gdk_imlib_load_image (fname);
  g_free (fname);

#if 0
  /* We really need a better image for this.  For now, it'll do */
  fname = gnome_pixmap_file ("gnome-logo-large.png");
  if (fname)
    watermark = gdk_imlib_load_image (fname);
  g_free (fname);
#endif

  /* The initial stuff */
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  druid = gnome_druid_new ();

  /* The druid pages. */
  page_start = gnome_druid_page_start_new_with_vals
    ("Beginning of the DRUID",
     "This is a Sample DRUID\nIt will walk you through absolutely nothing. (-:\n\nIt would be nice to have a watermark on the left.",
     logo,
     watermark);
  page_a = gnome_druid_page_standard_new_with_vals ("Page A", logo);
  page_b = gnome_druid_page_standard_new_with_vals ("Page B", logo);
  page_c = gnome_druid_page_standard_new_with_vals ("Page C", logo);
  page_d = gnome_druid_page_standard_new_with_vals ("Page D", logo);
  page_e = gnome_druid_page_standard_new_with_vals ("Page E", logo);
  page_f = gnome_druid_page_standard_new_with_vals ("Page F", logo);
  page_g = gnome_druid_page_standard_new_with_vals ("Page G", logo);
  page_finish = gnome_druid_page_finish_new_with_vals
    ("End of the DRUID",
     "I hope you found this demo informative.  You would\nnormally put a message here letting someone know\nthat they'd successfully installed something.",
     logo,
     watermark);

  /* set each one up. */
  /* page_a */
  data = g_new (druid_data, 1);
  radio_group = NULL;
  check_a = gtk_radio_button_new_with_label (NULL, "Go to page B");
  radio_group = gtk_radio_button_group (GTK_RADIO_BUTTON (check_a));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_a), TRUE);
  check_b = gtk_radio_button_new_with_label (radio_group, "Go to page C");
  gtk_box_pack_start (GTK_BOX (GNOME_DRUID_PAGE_STANDARD (page_a)->vbox),
		      check_a, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (GNOME_DRUID_PAGE_STANDARD (page_a)->vbox),
		      check_b, FALSE, FALSE, 0);
  data->radio_button = check_a;
  data->target_a = page_b;
  data->target_b = page_c;
  gtk_signal_connect (GTK_OBJECT (page_a), "next", (GtkSignalFunc) complex_druid_next_callback, (gpointer) data);
  gtk_signal_connect (GTK_OBJECT (page_a), "back", (GtkSignalFunc) druid_back_callback, (gpointer) NULL);

  /* page_b */
  data = g_new (druid_data, 1);
  radio_group = NULL;
  check_a = gtk_radio_button_new_with_label (NULL, "Go to page D");
  radio_group = gtk_radio_button_group (GTK_RADIO_BUTTON (check_a));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_a), TRUE);
  check_b = gtk_radio_button_new_with_label (radio_group, "Go to page E");
  gtk_box_pack_start (GTK_BOX (GNOME_DRUID_PAGE_STANDARD (page_b)->vbox),
		      check_a, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (GNOME_DRUID_PAGE_STANDARD (page_b)->vbox),
		      check_b, FALSE, FALSE, 0);
  data->radio_button = check_a;
  data->target_a = page_d;
  data->target_b = page_e;
  gtk_signal_connect (GTK_OBJECT (page_b), "next", (GtkSignalFunc) complex_druid_next_callback, (gpointer) data);
  gtk_signal_connect (GTK_OBJECT (page_b), "back", (GtkSignalFunc) druid_back_callback, (gpointer) NULL);

  /* page_c */
  gtk_box_pack_start (GTK_BOX (GNOME_DRUID_PAGE_STANDARD (page_c)->vbox),
		      gtk_label_new ("This page will take you to page G\nYou don't have any say in the matter. (-:"),
		      FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (page_c), "next", (GtkSignalFunc) simple_druid_next_callback, (gpointer) page_g);
  gtk_signal_connect (GTK_OBJECT (page_c), "back", (GtkSignalFunc) druid_back_callback, (gpointer) NULL);

  /* page_d */
  gtk_box_pack_start (GTK_BOX (GNOME_DRUID_PAGE_STANDARD (page_d)->vbox),
		      gtk_label_new ("This page will take you to page F"),
		      FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (page_d), "next", (GtkSignalFunc) simple_druid_next_callback, (gpointer) page_f);
    gtk_signal_connect (GTK_OBJECT (page_d), "back", (GtkSignalFunc) druid_back_callback, (gpointer) NULL);

  /* page_e */
  gtk_box_pack_start (GTK_BOX (GNOME_DRUID_PAGE_STANDARD (page_e)->vbox),
		      gtk_label_new ("This page will take you to page G\n\nShe sells sea shells by the sea shore."),
		      FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (page_e), "next", (GtkSignalFunc) simple_druid_next_callback, (gpointer) page_g);
  gtk_signal_connect (GTK_OBJECT (page_e), "back", (GtkSignalFunc) druid_back_callback, (gpointer) NULL);

  /* page_f */
  gtk_box_pack_start (GTK_BOX (GNOME_DRUID_PAGE_STANDARD (page_f)->vbox),
		      gtk_label_new ("This is a second to last page.\nIt isn't as nice as page G.\n\nClick Next to go to the last page\n"),
		      FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (page_f), "next", (GtkSignalFunc) simple_druid_next_callback, (gpointer) page_finish);
  gtk_signal_connect (GTK_OBJECT (page_f), "back", (GtkSignalFunc) druid_back_callback, (gpointer) NULL);

  /* page_g */
  gtk_box_pack_start (GTK_BOX (GNOME_DRUID_PAGE_STANDARD (page_g)->vbox),
		      gtk_label_new ("This is page G!!!!\n\nyay!!!!!!!"),
		      FALSE, FALSE, 0);
  gtk_signal_connect (GTK_OBJECT (page_g), "back", (GtkSignalFunc) druid_back_callback, (gpointer) NULL);


  /* page_finish */
  gtk_signal_connect (GTK_OBJECT (page_finish), "back", (GtkSignalFunc) druid_back_callback, (gpointer) NULL);

  /*Tie it together */
  gtk_container_add (GTK_CONTAINER (window), druid);
  gnome_druid_append_page (GNOME_DRUID (druid),
			   GNOME_DRUID_PAGE (page_start));
  gnome_druid_append_page (GNOME_DRUID (druid),
			   GNOME_DRUID_PAGE (page_a));
  gnome_druid_append_page (GNOME_DRUID (druid),
			   GNOME_DRUID_PAGE (page_b));
  gnome_druid_append_page (GNOME_DRUID (druid),
			   GNOME_DRUID_PAGE (page_c));
  gnome_druid_append_page (GNOME_DRUID (druid),
			   GNOME_DRUID_PAGE (page_d));
  gnome_druid_append_page (GNOME_DRUID (druid),
			   GNOME_DRUID_PAGE (page_e));
  gnome_druid_append_page (GNOME_DRUID (druid),
			   GNOME_DRUID_PAGE (page_f));
  gnome_druid_append_page (GNOME_DRUID (druid),
			   GNOME_DRUID_PAGE (page_g));
  gnome_druid_append_page (GNOME_DRUID (druid),
			   GNOME_DRUID_PAGE (page_finish));
  gnome_druid_set_page (GNOME_DRUID (druid), GNOME_DRUID_PAGE (page_start));
  gtk_widget_show_all (window);
}

/*
 * GnomeGuru
 */
static void
create_guru(void)
{
  gnome_ok_dialog("The Guru widget is deprecated.  Use the Druid instead.");
}

/*
 * GnomePaperSelector
 */

static void
create_papersel(void)
{
	GtkWidget *papersel;
	GtkWidget *app;

	app = create_newwin(TRUE,"testGNOME","Paper Selection");
	papersel = gnome_paper_selector_new( );
	gnome_app_set_contents(GNOME_APP(app),papersel);
	gtk_widget_show(papersel);
	gtk_widget_show(app);
}

/*
 * GnomeDialog
 */

enum {
  modal,
  just_hide,
  click_closes,
  editable_enters
};

static void
toggle_boolean(GtkWidget * toggle, gboolean * setme)
{
  gboolean current = *setme;
  *setme = !current;
}

static void
block_until_clicked(GtkWidget *widget, GtkWidget *dialog)
{
  gint button;
  button = gnome_dialog_run(GNOME_DIALOG(dialog));
  g_print("Modal run ended, button %d clicked\n", button);
}

static void
set_to_null(GtkWidget * ignore, GnomeDialog ** d)
{
  *d = NULL;
}

static void
create_test_dialog (GtkWidget * button, gboolean * settings)
{
  static GnomeDialog * dialog = NULL;
  GtkWidget * entry;
  GtkWidget * app;

  if (dialog) {
    g_print("Previous dialog was not destroyed, destroying...\n");
    gtk_widget_destroy(GTK_WIDGET(dialog));
    dialog = NULL;
  }

  app = gtk_object_get_user_data(GTK_OBJECT(button));

  g_return_if_fail(app != NULL);
  g_return_if_fail(GTK_IS_WINDOW(app));

  dialog = GNOME_DIALOG(gnome_dialog_new( "A Test Dialog",
					  GNOME_STOCK_BUTTON_OK,
					  "Not a stock button",
					  GNOME_STOCK_BUTTON_CANCEL, NULL ));

  gnome_dialog_set_parent(dialog,GTK_WINDOW(app));

  entry = gtk_entry_new();
  button = gtk_button_new_with_label("gnome_dialog_run");

  gtk_signal_connect(GTK_OBJECT(button), "clicked",
		     GTK_SIGNAL_FUNC(block_until_clicked),
		     dialog);

  gtk_signal_connect(GTK_OBJECT(dialog), "destroy",
		     GTK_SIGNAL_FUNC(set_to_null),
		     &dialog);

  gtk_box_pack_start(GTK_BOX(dialog->vbox), entry, TRUE, TRUE, GNOME_PAD);
  gtk_box_pack_start(GTK_BOX(dialog->vbox), button, FALSE, FALSE, GNOME_PAD);

  if (settings[modal]) {
    g_print("Modal... ");
    gtk_window_set_modal(GTK_WINDOW (dialog), TRUE);
  }
  if (settings[just_hide]) {
    g_print("Close hides... ");
    gnome_dialog_close_hides(dialog, TRUE);
  }
  if (settings[click_closes]) {
    g_print("Click closes... ");
    gnome_dialog_set_close(dialog, TRUE);
  }
  if (settings[editable_enters]) {
    g_print("Editable enters... ");
    gnome_dialog_editable_enters(dialog, GTK_EDITABLE(entry));
  }
  g_print("\n");

  gtk_widget_show_all(GTK_WIDGET(dialog));
}

static void
create_dialog(void)
{
  GtkWidget * app;
  GtkWidget * vbox;
  GtkWidget * hbox;
  GtkWidget * toggle;
  GtkWidget * button;
  static gboolean settings[4] = {FALSE, TRUE, FALSE, TRUE};

  app = create_newwin(TRUE,"testGNOME","Dialog Boxes");
  vbox = gtk_vbox_new(FALSE, GNOME_PAD);
  hbox = gtk_hbox_new(FALSE, GNOME_PAD);

  gnome_app_set_contents(GNOME_APP(app),vbox);

  gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, GNOME_PAD);

  toggle = gtk_toggle_button_new_with_label("Modal");
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle), settings[modal]);
  gtk_signal_connect(GTK_OBJECT(toggle), "toggled",
		     GTK_SIGNAL_FUNC(toggle_boolean), &settings[modal]);
  gtk_box_pack_start(GTK_BOX(hbox), toggle, FALSE, FALSE, GNOME_PAD);

  toggle = gtk_toggle_button_new_with_label("Hide don't destroy");
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle), settings[just_hide]);
  gtk_signal_connect(GTK_OBJECT(toggle), "toggled",
		     GTK_SIGNAL_FUNC(toggle_boolean), &settings[just_hide]);
  gtk_box_pack_start(GTK_BOX(hbox), toggle, FALSE, FALSE, GNOME_PAD);

  toggle = gtk_toggle_button_new_with_label("Close on click");
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle), settings[click_closes]);
  gtk_signal_connect(GTK_OBJECT(toggle), "toggled",
		     GTK_SIGNAL_FUNC(toggle_boolean), &settings[click_closes]);
  gtk_box_pack_start(GTK_BOX(hbox), toggle, FALSE, FALSE, GNOME_PAD);

  toggle = gtk_toggle_button_new_with_label("Editable enters");
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle), settings[editable_enters]);
  gtk_signal_connect(GTK_OBJECT(toggle), "toggled",
		     GTK_SIGNAL_FUNC(toggle_boolean), &settings[editable_enters]);
  gtk_box_pack_start(GTK_BOX(hbox), toggle, FALSE, FALSE, GNOME_PAD);

  button = gtk_button_new_with_label("Create the dialog");

  gtk_object_set_user_data(GTK_OBJECT(button),app);

  gtk_signal_connect(GTK_OBJECT(button), "clicked",
		     GTK_SIGNAL_FUNC(create_test_dialog),
		     &settings[0]);

  gtk_box_pack_end(GTK_BOX(vbox), button, FALSE, FALSE, GNOME_PAD);

  gtk_widget_show_all(app);
}

static void
file_entry_update_files(GtkWidget *w, GnomeFileEntry *fentry)
{
	char *p;
	char *pp;

	GtkLabel *l1 = gtk_object_get_data(GTK_OBJECT(w),"l1");
	GtkLabel *l2 = gtk_object_get_data(GTK_OBJECT(w),"l2");

	p = gnome_file_entry_get_full_path(fentry,FALSE);
	pp = g_strconcat("File name: ",p,NULL);
	gtk_label_set_text(l1,pp);
	g_free(pp);
	if(p) g_free(p);

	p = gnome_file_entry_get_full_path(fentry,TRUE);
	pp = g_strconcat("File name(if exists only): ",p,NULL);
	gtk_label_set_text(l2,pp);
	g_free(pp);
	if(p) g_free(p);
}

static void
file_entry_modal_toggle(GtkWidget *w, GnomeFileEntry *fentry)
{
	gnome_file_entry_set_modal(fentry,GTK_TOGGLE_BUTTON(w)->active);
}

static void
file_entry_directory_toggle(GtkWidget *w, GnomeFileEntry *fentry)
{
	gnome_file_entry_set_directory(fentry,GTK_TOGGLE_BUTTON(w)->active);
}

static void
create_file_entry(void)
{
	GtkWidget *app;
	GtkWidget *entry;
	GtkWidget *l1,*l2;
	GtkWidget *but;
	GtkWidget *box;

	app = create_newwin(TRUE,"testGNOME","File Entry");

	box = gtk_vbox_new(FALSE,5);

	entry = gnome_file_entry_new("Foo","Bar");
	gtk_box_pack_start(GTK_BOX(box),entry,FALSE,FALSE,0);

	l1 = gtk_label_new("File name: ");
	gtk_box_pack_start(GTK_BOX(box),l1,FALSE,FALSE,0);

	l2 = gtk_label_new("File name(if exists only): ");
	gtk_box_pack_start(GTK_BOX(box),l2,FALSE,FALSE,0);

	but = gtk_button_new_with_label("Update file labels");
	gtk_object_set_data(GTK_OBJECT(but),"l1",l1);
	gtk_object_set_data(GTK_OBJECT(but),"l2",l2);
	gtk_signal_connect(GTK_OBJECT(but),"clicked",
			   GTK_SIGNAL_FUNC(file_entry_update_files),
			   entry);
	gtk_box_pack_start(GTK_BOX(box),but,FALSE,FALSE,0);

	but = gtk_toggle_button_new_with_label("Make browse dialog modal");
	gtk_signal_connect(GTK_OBJECT(but),"toggled",
			   GTK_SIGNAL_FUNC(file_entry_modal_toggle),
			   entry);
	gtk_box_pack_start(GTK_BOX(box),but,FALSE,FALSE,0);

	but = gtk_toggle_button_new_with_label("Directory only picker");
	gtk_signal_connect(GTK_OBJECT(but),"toggled",
			   GTK_SIGNAL_FUNC(file_entry_directory_toggle),
			   entry);
	gtk_box_pack_start(GTK_BOX(box),but,FALSE,FALSE,0);

	gnome_app_set_contents(GNOME_APP(app),box);
	gtk_widget_show_all(app);
}

static void
create_pixmap_entry(void)
{
	GtkWidget *app;
	GtkWidget *entry;

	app = create_newwin(TRUE,"testGNOME","Pixmap Entry");
	entry = gnome_pixmap_entry_new("Foo","Pixmap",TRUE);
	gnome_app_set_contents(GNOME_APP(app),entry);
	gtk_widget_show(entry);
	gtk_widget_show(app);
}

static void
create_icon_entry(void)
{
	GtkWidget *app;
	GtkWidget *entry;

	app = create_newwin(TRUE,"testGNOME","Icon Entry");
	entry = gnome_icon_entry_new("Foo","Icon");
	gnome_app_set_contents(GNOME_APP(app),entry);
	gtk_widget_show(entry);
	gtk_widget_show(app);
}

static void
create_number_entry(void)
{
	GtkWidget *app;
	GtkWidget *entry;

	app = create_newwin(TRUE,"testGNOME","Number Entry");
	entry = gnome_number_entry_new("Foo","Calculator");
	gnome_app_set_contents(GNOME_APP(app),entry);
	gtk_widget_show(entry);
	gtk_widget_show(app);
}

/* CREATE FONT PICKER
 */
static void cfp_ck_UseFont(GtkWidget *widget,GnomeFontPicker *gfp)
{
    gboolean show;
    gint size;

    show=!gfp->use_font_in_label;
    size=gfp->use_font_in_label_size;

    gnome_font_picker_fi_set_use_font_in_label(gfp,show,size);

}
static void cfp_sp_value_changed(GtkAdjustment *adj,GnomeFontPicker *gfp)
{
    gboolean show;
    gint size;

    show=gfp->use_font_in_label;
    size=(gint)adj->value;

    gnome_font_picker_fi_set_use_font_in_label(gfp,show,size);

}

static void cfp_ck_ShowSize(GtkWidget *widget,GnomeFontPicker *gfp)
{
    GtkToggleButton *tb;

    tb=GTK_TOGGLE_BUTTON(widget);

    gnome_font_picker_fi_set_show_size(gfp,tb->active);
}

static void cfp_set_font(GnomeFontPicker *gfp, gchar *font_name, GtkLabel *label)
{
    g_print("Font name: %s\n",font_name);
    gtk_label_set_text(label,font_name);
}


static void
create_font_picker (void)
{
        GtkWidget *fontpicker1,*fontpicker2,*fontpicker3;

        GtkWidget *app;
        GtkWidget *vbox,*vbox1,*vbox2,*vbox3;
        GtkWidget *hbox1,*hbox3;
        GtkWidget *frPixmap,*frFontInfo,*frUser;
        GtkWidget *lbPixmap,*lbFontInfo,*lbUser;
        GtkWidget *ckUseFont,*spUseFont,*ckShowSize;
        GtkAdjustment *adj;

        app = create_newwin(TRUE,"testGNOME","Font Picker");


        vbox=gtk_vbox_new(FALSE,5);
        gtk_container_set_border_width(GTK_CONTAINER(vbox),5);
        gnome_app_set_contents(GNOME_APP(app),vbox);

        /* Pixmap */
        frPixmap=gtk_frame_new(_("Default Pixmap"));
        gtk_box_pack_start(GTK_BOX(vbox),frPixmap,TRUE,TRUE,0);
        vbox1=gtk_vbox_new(FALSE,FALSE);
        gtk_container_add(GTK_CONTAINER(frPixmap),vbox1);
        /* GnomeFontPicker with pixmap */
        fontpicker1 = gnome_font_picker_new();
        gtk_container_set_border_width(GTK_CONTAINER(fontpicker1),5);
        gtk_box_pack_start(GTK_BOX(vbox1),fontpicker1,TRUE,TRUE,0);
        lbPixmap=gtk_label_new(_("If you choose a font it will appear here"));
        gtk_box_pack_start(GTK_BOX(vbox1),lbPixmap,TRUE,TRUE,5);

        gtk_signal_connect(GTK_OBJECT(fontpicker1),"font_set",
                           GTK_SIGNAL_FUNC(cfp_set_font),lbPixmap);

        /* Font_Info */
        frFontInfo=gtk_frame_new(_("Font Info"));
        gtk_box_pack_start(GTK_BOX(vbox),frFontInfo,TRUE,TRUE,0);
        vbox2=gtk_vbox_new(FALSE,FALSE);
        gtk_container_set_border_width(GTK_CONTAINER(vbox2),5);
        gtk_container_add(GTK_CONTAINER(frFontInfo),vbox2);

        /* GnomeFontPicker with fontinfo */
        hbox1=gtk_hbox_new(FALSE,5);
        gtk_box_pack_start(GTK_BOX(vbox2),hbox1,FALSE,FALSE,0);

        ckUseFont=gtk_check_button_new_with_label(_("Use Font in button with size"));
        gtk_box_pack_start(GTK_BOX(hbox1),ckUseFont,TRUE,TRUE,0);

        adj=GTK_ADJUSTMENT(gtk_adjustment_new(14,5,150,1,1,1));
        spUseFont=gtk_spin_button_new(adj,1,0);
        gtk_box_pack_start(GTK_BOX(hbox1),spUseFont,FALSE,FALSE,0);

        ckShowSize=gtk_check_button_new_with_label(_("Show font size"));
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ckShowSize),TRUE);
        gtk_box_pack_start(GTK_BOX(vbox2),ckShowSize,FALSE,FALSE,5);

        fontpicker2 = gnome_font_picker_new();
        gnome_font_picker_set_mode(GNOME_FONT_PICKER(fontpicker2),GNOME_FONT_PICKER_MODE_FONT_INFO);
        gtk_box_pack_start(GTK_BOX(vbox2),fontpicker2,TRUE,TRUE,0);

        gtk_signal_connect(GTK_OBJECT(ckUseFont),"toggled",
                           (GtkSignalFunc)cfp_ck_UseFont,fontpicker2);

        gtk_signal_connect(GTK_OBJECT(ckShowSize),"toggled",
                           (GtkSignalFunc)cfp_ck_ShowSize,fontpicker2);

        gtk_signal_connect(GTK_OBJECT(adj),"value_changed",
                           (GtkSignalFunc)cfp_sp_value_changed,fontpicker2);

        lbFontInfo=gtk_label_new(_("If you choose a font it will appear here"));
        gtk_box_pack_start(GTK_BOX(vbox2),lbFontInfo,TRUE,TRUE,5);

        gtk_signal_connect(GTK_OBJECT(fontpicker2),"font_set",
                           GTK_SIGNAL_FUNC(cfp_set_font),lbFontInfo);


        /* User Widget */
        frUser=gtk_frame_new("User Widget");
        gtk_box_pack_start(GTK_BOX(vbox),frUser,TRUE,TRUE,0);
        vbox3=gtk_vbox_new(FALSE,FALSE);
        gtk_container_add(GTK_CONTAINER(frUser),vbox3);
        /* GnomeFontPicker with User Widget */
        fontpicker3 = gnome_font_picker_new();
        gnome_font_picker_set_mode(GNOME_FONT_PICKER(fontpicker3),GNOME_FONT_PICKER_MODE_USER_WIDGET);

        hbox3=gtk_hbox_new(FALSE,0);
        gtk_box_pack_start(GTK_BOX(hbox3),gnome_stock_new_with_icon(GNOME_STOCK_PIXMAP_SPELLCHECK),
                           FALSE,FALSE,5);
        gtk_box_pack_start(GTK_BOX(hbox3),gtk_label_new(_("This is an hbox with pixmap and text")),
            FALSE,FALSE,5);
        gnome_font_picker_uw_set_widget(GNOME_FONT_PICKER(fontpicker3),hbox3);
        gtk_container_set_border_width(GTK_CONTAINER(fontpicker3),5);
        gtk_box_pack_start(GTK_BOX(vbox3),fontpicker3,TRUE,TRUE,0);

        lbUser=gtk_label_new(_("If you choose a font it will appear here"));
        gtk_box_pack_start(GTK_BOX(vbox3),lbUser,TRUE,TRUE,5);

        gtk_signal_connect(GTK_OBJECT(fontpicker3),"font_set",
                           GTK_SIGNAL_FUNC(cfp_set_font),lbUser);

        gtk_widget_show_all(app);

}

static void
select_icon (GnomeIconList *gil, gint n, GdkEvent *event, gpointer data)
{
	printf ("Icon %d selected", n);

	if (event)
		printf (" with event type %d\n", (int) event->type);
	else
		printf ("\n");
}

static void
unselect_icon (GnomeIconList *gil, gint n, GdkEvent *event, gpointer data)
{
	printf ("Icon %d unselected", n);

	if (event)
		printf (" with event type %d\n", (int) event->type);
	else
		printf ("\n");
}

static void
create_icon_list(void)
{
	GtkWidget *app;
	GtkWidget *sw;
	GtkWidget *iconlist;
	GdkImlibImage *pix;
	int i;

	app = create_newwin(TRUE,"testGNOME","Icon List");

	sw = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	gnome_app_set_contents (GNOME_APP (app), sw);
	gtk_widget_set_usize (sw, 430, 300);
	gtk_widget_show (sw);

	iconlist = gnome_icon_list_new_flags (80, NULL, GNOME_ICON_LIST_IS_EDITABLE);
	gtk_container_add (GTK_CONTAINER (sw), iconlist);
	gtk_signal_connect (GTK_OBJECT (iconlist), "select_icon",
			    GTK_SIGNAL_FUNC (select_icon),
			    NULL);
	gtk_signal_connect (GTK_OBJECT (iconlist), "unselect_icon",
			    GTK_SIGNAL_FUNC (unselect_icon),
			    NULL);

	GTK_WIDGET_SET_FLAGS(iconlist, GTK_CAN_FOCUS);
	pix = gdk_imlib_create_image_from_xpm_data((gchar **)bomb_xpm);
	gdk_imlib_render (pix, pix->rgb_width, pix->rgb_height);

	gtk_widget_grab_focus (iconlist);

	gnome_icon_list_freeze (GNOME_ICON_LIST (iconlist));

	for (i = 0; i < 30; i++) {
		gnome_icon_list_append_imlib(GNOME_ICON_LIST(iconlist), pix, "Foo");
		gnome_icon_list_append_imlib(GNOME_ICON_LIST(iconlist), pix, "Bar");
		gnome_icon_list_append_imlib(GNOME_ICON_LIST(iconlist), pix, "LaLa");
	}

	gnome_icon_list_set_selection_mode (GNOME_ICON_LIST (iconlist), GTK_SELECTION_EXTENDED);
	gnome_icon_list_thaw (GNOME_ICON_LIST (iconlist));
	gtk_widget_show (iconlist);
	gtk_widget_show(app);
}


static void
create_less(void)
{
	GtkWidget *app;
	GtkWidget *less;

	app = create_newwin(TRUE,"testGNOME","Less");
	less = gnome_less_new();
	gnome_app_set_contents(GNOME_APP(app),less);
	gnome_less_set_fixed_font(GNOME_LESS(less), TRUE);
	gtk_widget_show(less);
	gtk_widget_show(app);
	gnome_less_show_command(GNOME_LESS(less),"fortune");
}

static void
create_pixmap(void)
{
	GtkWidget *app;
	GtkWidget *pixmap;
	app = create_newwin(TRUE,"testGNOME","Pixmap");
	pixmap = gnome_pixmap_new_from_xpm_d (bomb_xpm);

	gnome_app_set_contents(GNOME_APP(app),pixmap);
	gtk_widget_show(pixmap);
	gtk_widget_show(app);
}

static void
create_property_box(void)
{
/* this is broken, I dunno why. FIXME
   GtkWidget *vbox;
   GtkWidget *label;
   GtkWidget *check;
   GtkWidget *propbox;

   vbox=gtk_vbox_new(GNOME_PAD,FALSE);
   check=gtk_check_button_new_with_label("Option 1");
   gtk_box_pack_start(GTK_BOX(vbox),check,FALSE,FALSE,10);
   check=gtk_check_button_new_with_label("Another Option");
   gtk_box_pack_start(GTK_BOX(vbox),check,FALSE,FALSE,GNOME_PAD);


   propbox=gnome_property_box_new();
   label=gtk_label_new("foo");

   gnome_property_box_append_page(GNOME_PROPERTY_BOX(propbox),gtk_label_new("Bar"),label);
   gtk_widget_show_all(propbox); */
}

/* gnome-app-util */

static void
make_entry_hbox(GtkBox * box,
		gchar * buttontext, gchar * entrydefault,
		GtkSignalFunc callback, gpointer entrydata)
{
  GtkWidget * hbox;
  GtkWidget * entry;
  GtkWidget * button;

  hbox = gtk_hbox_new(TRUE, GNOME_PAD);
  entry = gtk_entry_new();
  gtk_entry_set_text(GTK_ENTRY(entry), entrydefault);
  gtk_object_set_user_data(GTK_OBJECT(entry), entrydata);
  gtk_box_pack_end(GTK_BOX(hbox), entry, TRUE, TRUE, GNOME_PAD);
  button = gtk_button_new_with_label(buttontext);
  gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, GNOME_PAD);
  gtk_signal_connect(GTK_OBJECT(button), "clicked",
		     callback, entry);

  gtk_box_pack_start(box, hbox, TRUE, TRUE, GNOME_PAD);
}

static void
make_button_hbox(GtkBox * box, gchar * buttontext,
		 GtkSignalFunc callback, gpointer data)
{
  GtkWidget * hbox;
  GtkWidget * button;

  hbox = gtk_hbox_new(TRUE, GNOME_PAD);
  button = gtk_button_new_with_label(buttontext);
  gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, GNOME_PAD);
  gtk_signal_connect(GTK_OBJECT(button), "clicked",
		     callback, data);

  gtk_box_pack_start(box, hbox, TRUE, TRUE, GNOME_PAD);
}

static void
message_cb(GtkWidget * button, GtkEntry * e)
{
  GnomeApp * app = gtk_object_get_user_data(GTK_OBJECT(e));
  gnome_app_message(app, gtk_entry_get_text(e));
}

static void
flash_cb(GtkWidget * b, GtkEntry * e)
{
  GnomeApp * app = gtk_object_get_user_data(GTK_OBJECT(e));
  gnome_app_flash(app, gtk_entry_get_text(e));
}

static void
error_cb(GtkWidget * b, GtkEntry * e)
{
  GnomeApp * app = gtk_object_get_user_data(GTK_OBJECT(e));
  gnome_app_error(app, gtk_entry_get_text(e));
}

static void
warning_cb(GtkWidget * b, GtkEntry * e)
{
  GnomeApp * app = gtk_object_get_user_data(GTK_OBJECT(e));
  gnome_app_warning(app, gtk_entry_get_text(e));
}

static void
reply_cb(gint reply, gpointer thedata)
{
  gchar * data = (gchar *)thedata;
  gchar * s = NULL;
  if (reply == GNOME_YES) {
    s = g_strconcat(_("The user chose Yes/OK with data:\n"),
		       data, NULL);
  }
  else if (reply == GNOME_NO) {
    s = g_strconcat(_("The user chose No/Cancel with data:\n"),
		       data, NULL);
  }

  if (s) {
    g_print(s);
    g_print("\n");
    gnome_ok_dialog(s);
    g_free(s);
  }
  else {
    gnome_error_dialog(_("Weird number in reply callback"));
  }
}

static gchar * data_string = "A test data string";

static void
question_cb(GtkWidget * b, GtkEntry * e)
{
  GnomeApp * app = gtk_object_get_user_data(GTK_OBJECT(e));
  gnome_app_question(app, gtk_entry_get_text(e),
		     reply_cb, data_string);
}

static void
question_modal_cb(GtkWidget * b, GtkEntry * e)
{
  GnomeApp * app = gtk_object_get_user_data(GTK_OBJECT(e));
  gnome_app_question_modal(app, gtk_entry_get_text(e),
			   reply_cb, data_string);
}

static void
ok_cancel_cb(GtkWidget * b, GtkEntry * e)
{
  GnomeApp * app = gtk_object_get_user_data(GTK_OBJECT(e));
  gnome_app_ok_cancel(app, gtk_entry_get_text(e),
		      reply_cb, data_string);
}

static void
ok_cancel_modal_cb(GtkWidget * b, GtkEntry * e)
{
  GnomeApp * app = gtk_object_get_user_data(GTK_OBJECT(e));
  gnome_app_ok_cancel_modal(app, gtk_entry_get_text(e),
			    reply_cb, data_string);
}


static void
string_cb(gchar * string, gpointer data)
{
  gchar * s = g_strconcat("Got string \"", string, "\" and data \"",
			     data, "\"", NULL);
  g_free(string);
  g_print(s);
  g_print("\n");
  gnome_ok_dialog(s);
  g_free(s);
}

static void
request_string_cb(GtkWidget * b, GtkEntry * e)
{
  GnomeApp * app = gtk_object_get_user_data(GTK_OBJECT(e));
  gnome_app_request_string(app, gtk_entry_get_text(e),
			   string_cb, data_string);
}

static gdouble
percent_cb(gpointer ignore)
{
  static gdouble progress = 0.0;
  progress += 0.05;
  if (progress > 1.0) progress = 0.0;
  return progress;
}

static void
cancel_cb(gpointer ignore)
{
  gnome_ok_dialog("Progress cancelled!");
}

static void
stop_progress_cb(GnomeDialog * d, gint button, GnomeAppProgressKey key)
{
  gnome_app_progress_done(key);
}

static void
progress_timeout_cb(GtkWidget * b, GnomeApp * app)
{
  GtkWidget * dialog;
  GnomeAppProgressKey key;

  dialog = gnome_dialog_new("Progress Timeout Test", "Stop test", NULL);
  gnome_dialog_set_close(GNOME_DIALOG(dialog), TRUE);

  key = gnome_app_progress_timeout(app, "Progress!", 200,
				   percent_cb,
				   cancel_cb,
				   dialog);

  gtk_signal_connect(GTK_OBJECT(dialog), "clicked",
		     GTK_SIGNAL_FUNC(stop_progress_cb), key);
  gtk_signal_connect_object(GTK_OBJECT(app), "destroy",
			    GTK_SIGNAL_FUNC(gnome_dialog_close),
			    GTK_OBJECT(dialog));

  gtk_widget_show(dialog);
}

static void
bar_push_cb(GtkWidget * b, GtkEntry * e)
{
  GnomeApp * app = gtk_object_get_user_data(GTK_OBJECT(e));
  gnome_appbar_push(GNOME_APPBAR(app->statusbar),
		    gtk_entry_get_text(e));
}

static void
bar_set_status_cb(GtkWidget * b, GtkEntry * e)
{
  GnomeApp * app = gtk_object_get_user_data(GTK_OBJECT(e));
  gnome_appbar_set_status(GNOME_APPBAR(app->statusbar),
			  gtk_entry_get_text(e));
}

static void
bar_set_default_cb(GtkWidget * b, GtkEntry * e)
{
  GnomeApp * app = gtk_object_get_user_data(GTK_OBJECT(e));
  gnome_appbar_set_default(GNOME_APPBAR(app->statusbar),
			   gtk_entry_get_text(e));
}

static void
bar_set_prompt_cb(GtkWidget * b, GtkEntry * e)
{
  GnomeApp * app = gtk_object_get_user_data(GTK_OBJECT(e));
  gnome_appbar_set_prompt(GNOME_APPBAR(app->statusbar),
			  gtk_entry_get_text(e), FALSE);
}

static void
bar_set_prompt_modal_cb(GtkWidget * b, GtkEntry * e)
{
  GnomeApp * app = gtk_object_get_user_data(GTK_OBJECT(e));
  gnome_appbar_set_prompt(GNOME_APPBAR(app->statusbar),
			  gtk_entry_get_text(e), TRUE);
}

static void
bar_pop_cb(GtkWidget * b, GnomeApp * app)
{
  gnome_appbar_pop(GNOME_APPBAR(app->statusbar));
}

static void
bar_clear_stack_cb(GtkWidget * b, GnomeApp * app)
{
  gnome_appbar_clear_stack(GNOME_APPBAR(app->statusbar));
}

static void
bar_refresh_cb(GtkWidget * b, GnomeApp * app)
{
  gnome_appbar_refresh(GNOME_APPBAR(app->statusbar));
}

static void
bar_clear_prompt_cb(GtkWidget * b, GnomeApp * app)
{
  gnome_appbar_clear_prompt(GNOME_APPBAR(app->statusbar));
}

static void
bar_progress_cb(GtkWidget * b, GtkSpinButton * sb)
{
  GnomeApp * app = gtk_object_get_user_data(GTK_OBJECT(sb));
  gdouble value = gtk_spin_button_get_value_as_float(sb);

  gnome_appbar_set_progress(GNOME_APPBAR(app->statusbar), value);
}

static void
dialog_ok_cb(GtkWidget * b, GtkEntry * e)
{
  gnome_ok_dialog (gtk_entry_get_text(e));
}

static void
dialog_error_cb(GtkWidget * b, GtkEntry * e)
{
  gnome_error_dialog (gtk_entry_get_text(e));
}

static void
dialog_warning_cb(GtkWidget * b, GtkEntry * e)
{
  gnome_warning_dialog (gtk_entry_get_text(e));
}

static void
dialog_question_cb(GtkWidget * b, GtkEntry * e)
{
  gnome_question_dialog (gtk_entry_get_text(e),
			 reply_cb, data_string);
}

static void
dialog_question_modal_cb(GtkWidget * b, GtkEntry * e)
{
  gnome_question_dialog_modal (gtk_entry_get_text(e),
			       reply_cb, data_string);
}

static void
dialog_ok_cancel_cb(GtkWidget * b, GtkEntry * e)
{
  gnome_ok_cancel_dialog (gtk_entry_get_text(e),
			  reply_cb, data_string);
}

static void
dialog_ok_cancel_modal_cb(GtkWidget * b, GtkEntry * e)
{
  gnome_ok_cancel_dialog_modal (gtk_entry_get_text(e),
				reply_cb, data_string);
}

static void
dialog_request_string_cb(GtkWidget * b, GtkEntry * e)
{
  gnome_request_string_dialog (gtk_entry_get_text(e),
			       string_cb, data_string);
}

static void
dialog_request_password_cb(GtkWidget * b, GtkEntry * e)
{
  gnome_request_password_dialog (gtk_entry_get_text(e),
				 string_cb, data_string);
}

static void
create_app_util(void)
{
  GnomeApp * app;
  GnomeAppBar * bar;
  GtkBox * vbox;
  GtkWidget * label;
  GtkWidget * sw;

  GtkWidget * hbox, * entry, * button;
  GtkAdjustment * adj;

  app =
    GNOME_APP(gnome_app_new("testGNOME",
			    "gnome-app-util/gnome-appbar/gnome-dialog-util test"));

  bar = GNOME_APPBAR(gnome_appbar_new(TRUE, TRUE, GNOME_PREFERENCES_USER));
  gnome_app_set_statusbar(app, GTK_WIDGET(bar));

  vbox = GTK_BOX(gtk_vbox_new(TRUE, GNOME_PAD));
  sw   = gtk_scrolled_window_new(NULL, NULL);

  gtk_container_set_focus_vadjustment (GTK_CONTAINER (vbox),
				       gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW(sw)));

  gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(sw), GTK_WIDGET(vbox));

  gnome_app_set_contents(app, sw);

  label = gtk_label_new("App Util Functions");
  gtk_box_pack_start(vbox, label, TRUE, TRUE, GNOME_PAD);

  label = gtk_label_new("Note: these functions should change behavior\n"
			"according to user preferences for (non)interactive\n"
			"appbar vs. dialogs.");
  gtk_box_pack_start(vbox, label, TRUE, TRUE, GNOME_PAD);

  make_entry_hbox(vbox, "Message", "This is a message",
		  GTK_SIGNAL_FUNC(message_cb), app);
  make_entry_hbox(vbox, "Flash", "Should disappear shortly",
		  GTK_SIGNAL_FUNC(flash_cb), app);
  make_entry_hbox(vbox, "Error", "an error",
		  GTK_SIGNAL_FUNC(error_cb), app);
  make_entry_hbox(vbox, "Warning", "Warning!",
		  GTK_SIGNAL_FUNC(warning_cb), app);
  make_entry_hbox(vbox, "Question", "Is this a question?",
		  GTK_SIGNAL_FUNC(question_cb), app);
  make_entry_hbox(vbox, "Modal Question", "This should be a modal question",
		  GTK_SIGNAL_FUNC(question_modal_cb), app);
  make_entry_hbox(vbox, "OK-Cancel", "An OK-Cancel",
		  GTK_SIGNAL_FUNC(ok_cancel_cb), app);
  make_entry_hbox(vbox, "Modal OK-Cancel", "Modal OK-Cancel",
		  GTK_SIGNAL_FUNC(ok_cancel_modal_cb), app);
  make_entry_hbox(vbox, "Request string", "Enter a string:",
		  GTK_SIGNAL_FUNC(request_string_cb), app);
  make_button_hbox(vbox, "Timeout Progress",
		   GTK_SIGNAL_FUNC(progress_timeout_cb), app);

  label = gtk_label_new("App Bar Functions");
  gtk_box_pack_start(vbox, label, TRUE, TRUE, GNOME_PAD);

  make_entry_hbox(vbox, "AppBar push", "This text was pushed",
		  GTK_SIGNAL_FUNC(bar_push_cb), app);
  make_entry_hbox(vbox, "AppBar set status", "This is a status",
		  GTK_SIGNAL_FUNC(bar_set_status_cb), app);
  make_entry_hbox(vbox, "AppBar set default", "Default text",
		  GTK_SIGNAL_FUNC(bar_set_default_cb), app);
  make_entry_hbox(vbox, "AppBar set prompt", "a prompt",
		  GTK_SIGNAL_FUNC(bar_set_prompt_cb), app);
  make_entry_hbox(vbox, "AppBar set modal prompt", "a modal prompt",
		  GTK_SIGNAL_FUNC(bar_set_prompt_modal_cb), app);

  make_button_hbox(vbox, "AppBar pop",
		   GTK_SIGNAL_FUNC(bar_pop_cb), app);
  make_button_hbox(vbox, "AppBar clear stack",
		   GTK_SIGNAL_FUNC(bar_clear_stack_cb), app);
  make_button_hbox(vbox, "AppBar refresh",
		   GTK_SIGNAL_FUNC(bar_refresh_cb), app);
  make_button_hbox(vbox, "AppBar clear prompt",
		   GTK_SIGNAL_FUNC(bar_clear_prompt_cb), app);

  adj = GTK_ADJUSTMENT(gtk_adjustment_new(0.5, 0.0, 1.0, 0.01, 0.05, 0.05));
  entry = gtk_spin_button_new(adj, 0.5, 3);
  gtk_object_set_user_data(GTK_OBJECT(entry), app);
  hbox = gtk_hbox_new(TRUE, GNOME_PAD);
  button = gtk_button_new_with_label("AppBar set progress");
  gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, TRUE, GNOME_PAD);
  gtk_box_pack_end(GTK_BOX(hbox), entry, TRUE, TRUE, GNOME_PAD);
  gtk_box_pack_start(vbox, hbox, TRUE, TRUE, GNOME_PAD);
  gtk_signal_connect(GTK_OBJECT(button), "clicked",
		     GTK_SIGNAL_FUNC(bar_progress_cb), entry);

  label = gtk_label_new("Dialog Util Functions");
  gtk_box_pack_start(vbox, label, TRUE, TRUE, GNOME_PAD);

  make_entry_hbox(vbox, "OK dialog", "Hi, is this OK?",
		  GTK_SIGNAL_FUNC(dialog_ok_cb), app);
  make_entry_hbox(vbox, "Error dialog", "An error! An error!",
		  GTK_SIGNAL_FUNC(dialog_error_cb), app);
  make_entry_hbox(vbox, "Warning dialog", "I'm warning you...",
		  GTK_SIGNAL_FUNC(dialog_warning_cb), app);
  make_entry_hbox(vbox, "OK-Cancel dialog", "OK or should I cancel?",
		  GTK_SIGNAL_FUNC(dialog_ok_cancel_cb), app);
  make_entry_hbox(vbox, "Modal OK-Cancel dialog", "Modally OK",
		  GTK_SIGNAL_FUNC(dialog_ok_cancel_modal_cb), app);
  make_entry_hbox(vbox, "Question dialog", "Are you sure?",
		  GTK_SIGNAL_FUNC(dialog_question_cb), app);
  make_entry_hbox(vbox, "Modal question dialog", "Modal - are you sure?",
		  GTK_SIGNAL_FUNC(dialog_question_modal_cb), app);
  make_entry_hbox(vbox, "Request string dialog", "Enter a string",
		  GTK_SIGNAL_FUNC(dialog_request_string_cb), app);
  make_entry_hbox(vbox, "Request password dialog", "Enter password",
		  GTK_SIGNAL_FUNC(dialog_request_password_cb), app);

  gtk_widget_set_usize(GTK_WIDGET(app), 640, 480);

  gtk_widget_show_all(GTK_WIDGET(app));
}

/* Used as a callback for menu items in the GnomeAppHelper test; just prints the string contents of
 * the data pointer.
 */
static void
item_activated (GtkWidget *widget, gpointer data)
{
	printf ("%s activated\n", (char *) data);
}

/* Menu definitions for the GnomeAppHelper test */

static GnomeUIInfo helper_file_menu[] = {
	{ GNOME_APP_UI_ITEM, "_New", "Create a new file", item_activated, "file/new", NULL,
	  GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_NEW, 'n', GDK_CONTROL_MASK, NULL },
	{ GNOME_APP_UI_ITEM, "_Open...", "Open an existing file", item_activated, "file/open", NULL,
	  GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_OPEN, 'o', GDK_CONTROL_MASK, NULL },
	{ GNOME_APP_UI_ITEM, "_Save", "Save the current file", item_activated, "file/save", NULL,
	  GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_SAVE, 's', GDK_CONTROL_MASK, NULL },
	{ GNOME_APP_UI_ITEM, "Save _as...", "Save the current file with a new name", item_activated, "file/save as", NULL,
	  GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_SAVE_AS, 0, 0, NULL },

	GNOMEUIINFO_SEPARATOR,

	{ GNOME_APP_UI_ITEM, "_Print...", "Print the current file", item_activated, "file/print", NULL,
	  GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_PRINT, 'p', GDK_CONTROL_MASK, NULL },

	GNOMEUIINFO_SEPARATOR,

	{ GNOME_APP_UI_ITEM, "_Close", "Close the current file", item_activated, "file/close", NULL,
	  GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_CLOSE, 0, 0, NULL },
	{ GNOME_APP_UI_ITEM, "E_xit", "Exit the program", item_activated, "file/exit", NULL,
	  GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_EXIT, 'q', GDK_CONTROL_MASK, NULL },
	GNOMEUIINFO_END
};

static GnomeUIInfo helper_edit_menu[] = {
	{ GNOME_APP_UI_ITEM, "_Undo", "Undo the last operation", item_activated, "edit/undo", NULL,
	  GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_UNDO, 'z', GDK_CONTROL_MASK, NULL },
	{ GNOME_APP_UI_ITEM, "_Redo", "Redo the last undo-ed operation", item_activated, "edit/redo", NULL,
	  GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_REDO, 0, 0, NULL },

	GNOMEUIINFO_SEPARATOR,

	{ GNOME_APP_UI_ITEM, "Cu_t", "Cut the selection to the clipboard", item_activated, "edit/cut", NULL,
	  GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_CUT, 'x', GDK_CONTROL_MASK, NULL },
	{ GNOME_APP_UI_ITEM, "_Copy", "Copy the selection to the clipboard", item_activated, "edit/copy", NULL,
	  GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_COPY, 'c', GDK_CONTROL_MASK, NULL },
	{ GNOME_APP_UI_ITEM, "_Paste", "Paste the contents of the clipboard", item_activated, "edit/paste", NULL,
	  GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_PASTE, 'v', GDK_CONTROL_MASK, NULL },
	GNOMEUIINFO_END
};

static GnomeUIInfo helper_style_radio_items[] = {
	{ GNOME_APP_UI_ITEM, "_10 points", NULL, item_activated, "style/10 points", NULL,
	  GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL },
	{ GNOME_APP_UI_ITEM, "_20 points", NULL, item_activated, "style/20 points", NULL,
	  GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL },
	{ GNOME_APP_UI_ITEM, "_30 points", NULL, item_activated, "style/30 points", NULL,
	  GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL },
	{ GNOME_APP_UI_ITEM, "_40 points", NULL, item_activated, "style/40 points", NULL,
	  GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL },
	GNOMEUIINFO_END
};

static GnomeUIInfo helper_style_menu[] = {
	{ GNOME_APP_UI_TOGGLEITEM, "_Bold", "Make the selection bold", item_activated, "style/bold", NULL,
	  GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL },
	{ GNOME_APP_UI_TOGGLEITEM, "_Italic", "Make the selection italic", item_activated, "style/bold", NULL,
	  GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL },

	GNOMEUIINFO_SEPARATOR,

	GNOMEUIINFO_RADIOLIST (helper_style_radio_items),
	GNOMEUIINFO_END
};

static GnomeUIInfo helper_help_menu[] = {
	{ GNOME_APP_UI_ITEM, "_About...", "Displays information about the program", item_activated, "help/about", NULL,
	  GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_ABOUT, 0, 0, NULL },
	GNOMEUIINFO_END
};

static GnomeUIInfo helper_main_menu[] = {
	{ GNOME_APP_UI_SUBTREE, "_File", "File operations", helper_file_menu, NULL, NULL,
	  GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL },
	{ GNOME_APP_UI_SUBTREE, "_Edit", "Editing commands", helper_edit_menu, NULL, NULL,
	  GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL },
	{ GNOME_APP_UI_SUBTREE, "_Style", "Style settings", helper_style_menu, NULL, NULL,
	  GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL },
	{ GNOME_APP_UI_SUBTREE, "_Help", "Help on the program", helper_help_menu, NULL, NULL,
	  GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL },
	GNOMEUIINFO_END
};

/* Toolbar definition for the GnomeAppHelper test */

static GnomeUIInfo helper_toolbar_radio_items[] = {
	{ GNOME_APP_UI_ITEM, "Red", "Set red color", item_activated, "toolbar/red", NULL,
	  GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_PIXMAP_BOOK_RED, 0, 0, NULL },
	{ GNOME_APP_UI_ITEM, "Green", "Set green color", item_activated, "toolbar/green", NULL,
	  GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_PIXMAP_BOOK_GREEN, 0, 0, NULL },
	{ GNOME_APP_UI_ITEM, "Blue", "Set blue color", item_activated, "toolbar/blue", NULL,
	  GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_PIXMAP_BOOK_BLUE, 0, 0, NULL },
	{ GNOME_APP_UI_ITEM, "Yellow", "Set yellow color", item_activated, "toolbar/yellow", NULL,
	  GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_PIXMAP_BOOK_YELLOW, 0, 0, NULL },
	GNOMEUIINFO_END
};

static GnomeUIInfo helper_toolbar[] = {
	{ GNOME_APP_UI_ITEM, "New", "Create a new file", item_activated, "toolbar/new", NULL,
	  GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_PIXMAP_NEW, 0, 0, NULL },
	{ GNOME_APP_UI_ITEM, "Open", "Open an existing file", item_activated, "toolbar/open", NULL,
	  GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_PIXMAP_OPEN, 0, 0, NULL },
	{ GNOME_APP_UI_ITEM, "Save", "Save the current file", item_activated, "toolbar/save", NULL,
	  GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_PIXMAP_SAVE, 0, 0, NULL },
	{ GNOME_APP_UI_ITEM, "Print", "Print the current file", item_activated, "toolbar/print", NULL,
	  GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_PIXMAP_PRINT, 0, 0, NULL },

	GNOMEUIINFO_SEPARATOR,

	{ GNOME_APP_UI_ITEM, "Undo", "Undo the last operation", item_activated, "toolbar/undo", NULL,
	  GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_PIXMAP_UNDO, 0, 0, NULL },
	{ GNOME_APP_UI_ITEM, "Redo", "Redo the last undo-ed operation", item_activated, "toolbar/redo", NULL,
	  GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_PIXMAP_REDO, 0, 0, NULL },

	GNOMEUIINFO_SEPARATOR,

	{ GNOME_APP_UI_ITEM, "Cut", "Cut the selection to the clipboard", item_activated, "toolbar/cut", NULL,
	  GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_PIXMAP_CUT, 0, 0, NULL },
	{ GNOME_APP_UI_ITEM, "Copy", "Copy the selection to the clipboard", item_activated, "toolbar/copy", NULL,
	  GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_PIXMAP_COPY, 0, 0, NULL },
	{ GNOME_APP_UI_ITEM, "Paste", "Paste the contents of the clipboard", item_activated, "toolbar/paste", NULL,
	  GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_PIXMAP_PASTE, 0, 0, NULL },

	GNOMEUIINFO_SEPARATOR,

	GNOMEUIINFO_RADIOLIST (helper_toolbar_radio_items),
	GNOMEUIINFO_END
};

/* These three functions insert some silly text in the GtkEntry specified in the user data */

static void
insert_red (GtkWidget *widget, gpointer data)
{
	int pos;
	GtkWidget *entry;

	entry = GTK_WIDGET (data);

	pos = gtk_editable_get_position (GTK_EDITABLE (entry));
	gtk_editable_insert_text (GTK_EDITABLE (entry), "red book ", strlen ("red book "), &pos);
}

static void
insert_green (GtkWidget *widget, gpointer data)
{
	int pos;
	GtkWidget *entry;

	entry = GTK_WIDGET (data);

	pos = gtk_editable_get_position (GTK_EDITABLE (entry));
	gtk_editable_insert_text (GTK_EDITABLE (entry), "green book ", strlen ("green book "), &pos);
}

static void
insert_blue (GtkWidget *widget, gpointer data)
{
	int pos;
	GtkWidget *entry;

	entry = GTK_WIDGET (data);

	pos = gtk_editable_get_position (GTK_EDITABLE (entry));
	gtk_editable_insert_text (GTK_EDITABLE (entry), "blue book ", strlen ("blue book "), &pos);
}

/* Shared popup menu definition for the GnomeAppHelper test */

static GnomeUIInfo helper_shared_popup_dup[] = {
	{ GNOME_APP_UI_ITEM, "Insert a _red book", NULL, insert_red, NULL, NULL,
	  GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_BOOK_RED, 0, 0, NULL },
	{ GNOME_APP_UI_ITEM, "Insert a _green book", NULL, insert_green, NULL, NULL,
	  GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_BOOK_GREEN, 0, 0, NULL },
	{ GNOME_APP_UI_ITEM, "Insert a _blue book", NULL, insert_blue, NULL, NULL,
	  GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_BOOK_BLUE, 0, 0, NULL },
	GNOMEUIINFO_END
};

static GnomeUIInfo helper_shared_popup[] = {
	{ GNOME_APP_UI_ITEM, "Insert a _red book", NULL, insert_red, NULL, NULL,
	  GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_BOOK_RED, 0, 0, NULL },
	{ GNOME_APP_UI_ITEM, "Insert a _green book", NULL, insert_green, NULL, NULL,
	  GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_BOOK_GREEN, 0, 0, NULL },
	{ GNOME_APP_UI_ITEM, "Insert a _blue book", NULL, insert_blue, NULL, NULL,
	  GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_BOOK_BLUE, 0, 0, NULL },
        GNOMEUIINFO_SUBTREE("Subtree", helper_shared_popup_dup),
	GNOMEUIINFO_END
};

/* These function change the fill color of the canvas item specified in the user data */

static void
set_cyan (GtkWidget *widget, gpointer data)
{
	GnomeCanvasItem *item;

	item = GNOME_CANVAS_ITEM (data);

	gnome_canvas_item_set (item,
			       "fill_color", "cyan",
			       NULL);
}

static void
set_magenta (GtkWidget *widget, gpointer data)
{
	GnomeCanvasItem *item;

	item = GNOME_CANVAS_ITEM (data);

	gnome_canvas_item_set (item,
			       "fill_color", "magenta",
			       NULL);
}

static void
set_yellow (GtkWidget *widget, gpointer data)
{
	GnomeCanvasItem *item;

	item = GNOME_CANVAS_ITEM (data);

	gnome_canvas_item_set (item,
			       "fill_color", "yellow",
			       NULL);
}

/* Explicit popup menu definition for the GnomeAppHelper test */


static GnomeUIInfo helper_explicit_popup_dup[] = {
	{ GNOME_APP_UI_ITEM, "Set color to _cyan", NULL, set_cyan, NULL, NULL,
	  GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL },
	{ GNOME_APP_UI_ITEM, "Set color to _magenta", NULL, set_magenta, NULL, NULL,
	  GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL },
	{ GNOME_APP_UI_ITEM, "Set color to _yellow", NULL, set_yellow, NULL, NULL,
	  GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL },
	GNOMEUIINFO_END
};

static GnomeUIInfo helper_explicit_popup[] = {
	{ GNOME_APP_UI_ITEM, "Set color to _cyan", NULL, set_cyan, NULL, NULL,
	  GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL },
	{ GNOME_APP_UI_ITEM, "Set color to _magenta", NULL, set_magenta, NULL, NULL,
	  GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL },
	{ GNOME_APP_UI_ITEM, "Set color to _yellow", NULL, set_yellow, NULL, NULL,
	  GNOME_APP_PIXMAP_NONE, NULL, 0, 0, NULL },
        GNOMEUIINFO_SUBTREE("Subtree", helper_explicit_popup_dup),
	GNOMEUIINFO_END
};

/* Event handler for canvas items in the explicit popup menu demo */

static gint
item_event (GnomeCanvasItem *item, GdkEvent *event, gpointer data)
{
	if (!((event->type == GDK_BUTTON_PRESS) && (event->button.button == 3)))
		return FALSE;

	gnome_popup_menu_do_popup (GTK_WIDGET (data), NULL, NULL, (GdkEventButton *) event, item);

	return TRUE;
}

/* Test the GnomeAppHelper module */
static void
create_app_helper (GtkWidget *widget, gpointer data)
{
	GtkWidget *app;
	GtkWidget *vbox;
	GtkWidget *frame;
	GtkWidget *vbox2;
	GtkWidget *w;
	GtkWidget *popup;
        GnomeAppBar *bar;
	GnomeCanvasItem *item;

	app = gnome_app_new ("testGNOME", "GnomeAppHelper test");
	gnome_app_create_menus (GNOME_APP (app), helper_main_menu);
	gnome_app_create_toolbar (GNOME_APP (app), helper_toolbar);

        bar = GNOME_APPBAR(gnome_appbar_new(FALSE, TRUE, GNOME_PREFERENCES_USER));
        gnome_app_set_statusbar(GNOME_APP(app), GTK_WIDGET(bar));

        gnome_app_install_appbar_menu_hints(GNOME_APPBAR(bar), helper_main_menu);

	vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), GNOME_PAD_SMALL);

	/* Shared popup menu */

	popup = gnome_popup_menu_new (helper_shared_popup);

	frame = gtk_frame_new ("Shared popup menu");
	gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
	gtk_widget_show (frame);

	vbox2 = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
	gtk_container_set_border_width (GTK_CONTAINER (vbox2), GNOME_PAD_SMALL);
	gtk_container_add (GTK_CONTAINER (frame), vbox2);
	gtk_widget_show (vbox2);

	w = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (vbox2), w, FALSE, FALSE, 0);
	gtk_widget_show (w);
	gnome_popup_menu_attach (popup, w, w);

	w = gtk_entry_new ();
	gtk_box_pack_start (GTK_BOX (vbox2), w, FALSE, FALSE, 0);
	gtk_widget_show (w);
	gnome_popup_menu_attach (popup, w, w);

	/* Popup menu explicitly popped */

	popup = gnome_popup_menu_new (helper_explicit_popup);

	frame = gtk_frame_new ("Explicit popup menu");
	gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);
	gtk_widget_show (frame);

	w = gnome_canvas_new ();
	gtk_widget_set_usize ((w), 200, 100);
	gnome_canvas_set_scroll_region (GNOME_CANVAS (w), 0.0, 0.0, 200.0, 100.0);
	gtk_container_add (GTK_CONTAINER (frame), w);
	gtk_widget_show (w);

	gtk_signal_connect (GTK_OBJECT (w), "destroy",
			    (GtkSignalFunc) delete_event,
			    popup);

	item = gnome_canvas_item_new (gnome_canvas_root (GNOME_CANVAS (w)),
				      gnome_canvas_ellipse_get_type (),
				      "x1", 5.0,
				      "y1", 5.0,
				      "x2", 95.0,
				      "y2", 95.0,
				      "fill_color", "white",
				      "outline_color", "black",
				      NULL);
	gtk_signal_connect (GTK_OBJECT (item), "event",
			    (GtkSignalFunc) item_event,
			    popup);

	item = gnome_canvas_item_new (gnome_canvas_root (GNOME_CANVAS (w)),
				      gnome_canvas_ellipse_get_type (),
				      "x1", 105.0,
				      "y1", 0.0,
				      "x2", 195.0,
				      "y2", 95.0,
				      "fill_color", "white",
				      "outline_color", "black",
				      NULL);
	gtk_signal_connect (GTK_OBJECT (item), "event",
			    (GtkSignalFunc) item_event,
			    popup);

	gnome_app_set_contents (GNOME_APP (app), vbox);
	gtk_widget_show (app);
}

/*test for dentry edit*/
static void
create_dentry_edit(void)
{
	GtkWidget *app;
	GtkObject *edit;
	GtkWidget *nbook;

	nbook = gtk_notebook_new();

	app = create_newwin(TRUE,"testGNOME","DEntry Entry");
	edit = gnome_dentry_edit_new_notebook(GTK_NOTEBOOK(nbook));
	gnome_app_set_contents(GNOME_APP(app),nbook);
	gtk_widget_show_all(app);
}

static void
href_cb(GtkObject *button)
{
	GtkWidget *href = gtk_object_get_data(button, "href");
	GtkWidget *url_ent = gtk_object_get_data(button, "url");
	GtkWidget *label_ent = gtk_object_get_data(button, "label");
	gchar *url, *label;

	url = gtk_entry_get_text(GTK_ENTRY(url_ent));
	label = gtk_entry_get_text(GTK_ENTRY(label_ent));
	if (!label || ! label[0])
		label = url;
	gnome_href_set_url(GNOME_HREF(href), url);
	gnome_href_set_label(GNOME_HREF(href), label);
}

static void
create_href(void)
{
	GtkWidget *app, *vbox, *href, *ent1, *ent2, *wid;

	app = create_newwin(TRUE,"testGNOME","HRef test");
	vbox = gtk_vbox_new(FALSE, 5);
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 5);
	gnome_app_set_contents(GNOME_APP(app), vbox);

	href = gnome_href_new("http://www.gnome.org/", "Gnome Website");
	gtk_box_pack_start(GTK_BOX(vbox), href, FALSE, FALSE, 0);

	wid = gtk_hseparator_new();
	gtk_box_pack_start(GTK_BOX(vbox), wid, TRUE, FALSE, 0);

	wid = gtk_label_new("The launch behaviour of the\n"
			    "configured with the control center");
	gtk_box_pack_start(GTK_BOX(vbox), wid, TRUE, FALSE, 0);

	ent1 = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(ent1), "http://www.gnome.org/");
	gtk_box_pack_start(GTK_BOX(vbox), ent1, TRUE, TRUE, 0);

	ent2 = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(ent2), "Gnome Website");
	gtk_box_pack_start(GTK_BOX(vbox), ent2, TRUE, TRUE, 0);

	wid = gtk_button_new_with_label("set href props");
	gtk_object_set_data(GTK_OBJECT(wid), "href", href);
	gtk_object_set_data(GTK_OBJECT(wid), "url", ent1);
	gtk_object_set_data(GTK_OBJECT(wid), "label", ent2);
	gtk_signal_connect(GTK_OBJECT(wid), "clicked",
			   GTK_SIGNAL_FUNC(href_cb), NULL);
	gtk_box_pack_start(GTK_BOX(vbox), wid, TRUE, TRUE, 0);

	gtk_widget_show_all(app);
}

int
main (int argc, char *argv[])
{
	struct {
		char *label;
		void (*func) ();
	} buttons[] =
	  {
		  { "app-util/appbar/dialog-util", create_app_util },
		  { "app-helper", create_app_helper },
		  { "calculator", create_calc },
		  { "canvas", create_canvas },
		  { "clock",	create_clock },
		  { "color picker", create_color_picker },
		  { "druid", create_druid },
		  { "guru", create_guru },
		  { "paper-sel", create_papersel },
		  { "date edit", create_date_edit },
		  { "dialog", create_dialog },
		  { "file entry", create_file_entry },
                  { "pixmap entry", create_pixmap_entry },
                  { "icon entry", create_icon_entry },
                  { "number entry", create_number_entry },
                  { "font picker", create_font_picker },
		  { "icon list", create_icon_list },
		  { "less", create_less },
		  { "pixmap", create_pixmap },
		  { "dentry edit", create_dentry_edit },
		  { "href", create_href },
		  { "(Reload preferences)", gnome_preferences_load },
		  { "prop box", create_property_box },
	  };
	int nbuttons = sizeof (buttons) / sizeof (buttons[0]);
	GtkWidget *app;
	GtkWidget *box1;
	GtkWidget *box2;
	GtkWidget *button;
	GtkWidget *scrolled_window;
	int i;

	gnome_init ("testGNOME", VERSION, argc, argv);

	app = create_newwin(FALSE,"testGNOME", "testGNOME");
	gtk_widget_set_usize (app, 200,300);
	box1 = gtk_vbox_new (FALSE, 0);
	gnome_app_set_contents(GNOME_APP(app),box1);
	gtk_widget_show (box1);
	scrolled_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_container_set_border_width (GTK_CONTAINER (scrolled_window), 10);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	GTK_WIDGET_UNSET_FLAGS (GTK_SCROLLED_WINDOW (scrolled_window)->vscrollbar,
				GTK_CAN_FOCUS);
	gtk_box_pack_start (GTK_BOX (box1), scrolled_window, TRUE, TRUE, 0);
	gtk_widget_show (scrolled_window);
	box2 = gtk_vbox_new (FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (box2), 10);
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolled_window), box2);
	gtk_container_set_focus_vadjustment (GTK_CONTAINER (box2),gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW(scrolled_window)));
	gtk_widget_show (box2);
	for (i = 0; i < nbuttons; i++)
	{
		button = gtk_button_new_with_label (buttons[i].label);
		if (buttons[i].func)
			gtk_signal_connect (GTK_OBJECT (button),
					    "clicked",
					    GTK_SIGNAL_FUNC(buttons[i].func),
					    NULL);
		else
			gtk_widget_set_sensitive (button, FALSE);
		gtk_box_pack_start (GTK_BOX (box2), button, TRUE, TRUE, 0);
		gtk_widget_show (button);
	}

	gtk_widget_show (app);
	gtk_main();

	gtk_widget_destroy(app);

	return 0;
}
