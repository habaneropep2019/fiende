/* gturing.c - a Turing machine simulator.
 * Copyright (C) 1998 The Free Software Foundation
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <config.h>
#include <gnome.h>
#include "turing.h"

#include "power.xpm"
#include "play.xpm"
#include "step.xpm"
#include "stop.xpm"

#define SCROLL_WIDTH 10000

#define STATE 0
#define READ 1
#define WRITE 2
#define MOVE 3
#define NEW_STATE 4
#define ENTRIES_MAX 5

/* A few globals. */
char *tape_font[] = 
{
	"-bitstream-courier-bold-r-*-*-25-*-*-*-*-*-*-*",
	"-adobe-courier-bold-r-normal-*-24-*-*-*-m-*-iso8859-1",
	"-adobe-courier-bold-r-normal-*-25-*-*-*-m-*-iso8859-1",
	"fixed",
	NULL
};

static char *progname;
static char *prog_message = NULL;
static char tape_string[1024] = "";
static char states_fname[1024] = "";

static turing *tm;
static long speed = 0;
static int stop_flag = 1;

static GtkWidget *rootw;
static GtkWidget *dialog = NULL;
static GtkWidget *state_clist = NULL;
static GtkWidget *entry;

static GtkWidget *tapelabel;
static GtkWidget *headlabel;
static GtkWidget *power;
static GtkWidget *stop;
static GtkWidget *play;
static GtkWidget *step;
static GtkWidget *statusline;
static GtkWidget *helpline;

static GtkWidget *save;
static GtkWidget *edit_save = NULL;

void power_do (void);
void power_callback(GtkWidget *power_button, gpointer data);
void state_clist_load_states(void);
void state_clist_select_state(turing *t);
void save_call(GtkWidget *w, gpointer data);

void set_save_sens(gboolean saves)
{
	gtk_widget_set_sensitive (save, saves);
	
	if (edit_save)
		gtk_widget_set_sensitive (edit_save, saves);
}

void set_toolbar_sens(gboolean powers, gboolean stops,
											gboolean plays, gboolean steps)
{
	gtk_widget_set_sensitive(power, powers);
	gtk_widget_set_sensitive(stop, stops);
	gtk_widget_set_sensitive(play, plays);
	gtk_widget_set_sensitive(step, steps);
}

void set_tape(char *str)
{
	int len, i;
	char *tmp;
	
	len = strlen(str);
	tmp = malloc(len + 1);
	
	for (i = 0; i < len; i++)
		if (i == tm->pos)
			tmp[i] = '^';
		else
			tmp[i] = ' ';
	
	tmp[len] = 0;
		
  gtk_label_set(GTK_LABEL(tapelabel), str);
  gtk_label_set(GTK_LABEL(headlabel), tmp);
	
	free(tmp);
}

void view_comment (void)
{
	GtkWidget *w;
	char *mess;
	
	if (prog_message && *prog_message && *prog_message != '\n')
		mess = prog_message;
	else
		mess = _("No comment for this program.");
	
	w = gnome_message_box_new(mess, GNOME_MESSAGE_BOX_INFO,
														GNOME_STOCK_BUTTON_OK, NULL);
	GTK_WINDOW(w)->position = GTK_WIN_POS_MOUSE;
	gtk_widget_show(w);
}

void set_states_fname(char *str)
{
	char *c;
	
	if ((*states_fname != 0) && (*tape_string != 0))
		power_callback(power, NULL);
	else
		set_toolbar_sens(FALSE, FALSE, FALSE, FALSE);
	
	if ((c = strrchr(states_fname, '/')) != NULL)
		c++;
	else
		c = states_fname;

	state_clist_load_states();
	state_clist_select_state(tm);
  gtk_label_set(GTK_LABEL(helpline), c);
}

void states_fname_examples_callback (GtkWidget *ok_button, gpointer data)
{
	GtkFileSelection *fsel;
	
	fsel = GTK_FILE_SELECTION (data);
	gtk_file_selection_set_filename (fsel, GTURING_EXAMPLES_DIR);
}

void states_fname_callback(GtkWidget *ok_button, gpointer data)
{
	gboolean action_save;
	char *fname, *comment;
	GtkWidget *w, *text;
	
	fname = g_strdup(gtk_file_selection_get_filename(GTK_FILE_SELECTION(dialog)));
	
	action_save = (gboolean) data;
	if (action_save)
	{
		text = gtk_object_get_data (GTK_OBJECT (dialog), "text");
		comment = gtk_editable_get_chars (GTK_EDITABLE (text), 0, -1);
		if (turing_fwrite_states (tm->statehead, fname, comment))
			*states_fname = 0; /*error*/
		else
			strncpy(states_fname, fname, 1024);
		
		if (prog_message)
			free (prog_message);
		
		prog_message = strdup (comment);
	}
	else
	{
		if (turing_fread_states(tm, fname))
			*states_fname = 0; /*error*/
		else
		{
			strncpy(states_fname, fname, 1024);
		
			if (prog_message)
				free(prog_message);
			
			prog_message = turing_fread_comments(fname);
			view_comment ();
			
			set_save_sens (TRUE);
		}
	}
	
	set_states_fname(fname);
	g_free(fname);

	gnome_config_set_string("/gTuring/Options/program", states_fname);
	gtk_widget_destroy(dialog);
	dialog = NULL;
}

void tape_string_callback(GtkWidget *ok_button, gpointer data)
{
	strncpy(tape_string, gtk_entry_get_text(GTK_ENTRY(entry)), 1024);
	turing_set_tape(tm, tape_string);
	set_tape(tape_string);
	
	gtk_widget_destroy(dialog);
	dialog = NULL;
	
	if (tm->statehead && (*tape_string != 0))
		power_callback(power, NULL);
	else
		set_toolbar_sens(FALSE, FALSE, FALSE, FALSE);
	
	gnome_config_set_string("/gTuring/Options/tape", tape_string);
}

void speed_callback(GtkWidget *ok_button, gpointer data)
{
	speed = atoi(gtk_entry_get_text(GTK_ENTRY(entry)));
							 
	gtk_widget_destroy(dialog);
	dialog = NULL;
	
	gnome_config_set_int("/gTuring/Options/speed", speed);
}

void cancel_callback(GtkWidget *widget, gpointer data)
{
	void *p;

	if ((p = gtk_object_get_user_data(GTK_OBJECT(widget))) != NULL)
		g_free(p);
	
	gtk_widget_destroy(dialog);
	dialog = NULL;
}

void states_view_edit_set_clicked_callback (GtkWidget *widget, gpointer data)
{
	GtkWidget **entries, *message;
	gchar *c[ENTRIES_MAX];
	turing_state state, *s;
	int i;
	
	entries = gtk_object_get_data (GTK_OBJECT (widget), "entries");
	
	for (i = 0; i < ENTRIES_MAX; i++)
		c[i] = gtk_entry_get_text (GTK_ENTRY (entries[i]));

	/* FIXME: states should be able to be greater than 9. */
	if (!(*c[STATE] >= '0' && *c[STATE] <= '9') ||
			(*c[MOVE] != 'l' && *c[MOVE] != 'r') ||
			!(*c[NEW_STATE] >= '0' && *c[NEW_STATE] <= '9'))
	{
		gtk_widget_grab_focus (entries[0]);
		message = gnome_message_box_new (_("States must be numbers. Move can only be 'r' or 'l'."),
																		 GNOME_MESSAGE_BOX_ERROR,
																		 GNOME_STOCK_BUTTON_OK,
																		 NULL);
		gtk_widget_show (message);
		return;
	}
	
	state.no = atoi (c[STATE]);
	state.read = *c[READ];
	state.write = *c[WRITE];
	state.move = *c[MOVE];
	state.new = atoi (c[NEW_STATE]);
	state.next = NULL;
	
	turing_set_state (tm, state);
	
	gtk_clist_freeze (GTK_CLIST (state_clist));
	state_clist_load_states ();
	gtk_clist_thaw (GTK_CLIST (state_clist));
	
	for (s = tm->statehead; s; s = s->next)
		if ((s->no == state.no) && (s->read == state.read))
			break;
	
	if (s)
		for (i = 0; ; i++)
			if (gtk_clist_get_row_data (GTK_CLIST(state_clist), i) == s)
	    {
				gtk_clist_select_row(GTK_CLIST(state_clist), i, 0);
				break;
			}

	if (tm->tapehead)
		power_do ();

	set_save_sens (TRUE);
	
	gtk_widget_grab_focus (entries[0]);
}

void states_view_entry_grab_focus_callback (GtkWidget *widget, gpointer data)
{
	GtkWidget **entries;
	int i;
	
	entries = gtk_object_get_data (GTK_OBJECT (widget), "entries");
	
	for (i = 0; i < ENTRIES_MAX; i++)
		gtk_editable_select_region (GTK_EDITABLE (entries[i]), 0, 0);
	
	gtk_editable_select_region (GTK_EDITABLE (widget), 0, -1);
}

void states_view_edit_callback (GtkWidget *widget, gpointer data)
{
	GtkWidget *w;
	GtkWidget *table, *label, *frame, *button;
	GtkWidget **entries;
	GtkWidget *message;
	gint i;
	gchar *labels[] = { N_("State"), N_("Read"), N_("Write"), N_("Move"), N_("New State"), NULL };
	
	message = gnome_message_box_new (_("Editing is still alpha. Come back later or hack."),
																	 GNOME_MESSAGE_BOX_WARNING,
																	 GNOME_STOCK_BUTTON_OK,
																	 NULL);
	gtk_widget_show (message);
	
	gtk_widget_set_sensitive (widget, FALSE);
	
	w = GTK_WIDGET (data);
	entries = g_new (GtkWidget *, ENTRIES_MAX);
	gtk_object_set_data (GTK_OBJECT (state_clist), "entries", entries);
	
	frame = gtk_frame_new (_("Edit Or Create Row"));
	gtk_box_pack_start(GTK_BOX(GNOME_DIALOG(w)->vbox), frame, FALSE, FALSE, 0);
	table = gtk_table_new (2, 2 * ENTRIES_MAX + 2, FALSE);
	gtk_container_add (GTK_CONTAINER (frame), table);
	gtk_container_set_border_width (GTK_CONTAINER (table), 8);
	gtk_table_set_row_spacings (GTK_TABLE (table), 4);
	gtk_table_set_col_spacings (GTK_TABLE (table), 8);

	edit_save = button = gnome_pixmap_button (gnome_stock_pixmap_widget (w, GNOME_STOCK_PIXMAP_SAVE_AS),
																						_("Save"));
	gtk_signal_connect (GTK_OBJECT (button), "clicked", 
											save_call, NULL);
	gtk_widget_set_sensitive (edit_save, GTK_WIDGET_IS_SENSITIVE (save));
	gtk_table_attach (GTK_TABLE (table), button, ENTRIES_MAX * 2 + 1, ENTRIES_MAX * 2 + 2, 1, 2, 0, 
										GTK_FILL | GTK_EXPAND, 0, 0);
	

	button = gnome_pixmap_button (gnome_stock_pixmap_widget (w, GNOME_STOCK_PIXMAP_ADD),
																_("Set"));
	gtk_object_set_data (GTK_OBJECT (button), "entries", entries);
	gtk_table_attach (GTK_TABLE (table), button, ENTRIES_MAX * 2, ENTRIES_MAX * 2 + 1, 1, 2, 0, 
										GTK_FILL | GTK_EXPAND, 0, 0);
	
	for (i = 0; labels[i]; i++)
	{
#ifdef ENABLE_NLS
		label = gtk_label_new (_(labels[i]));
#else
		label = gtk_label_new (labels[i]);
#endif
		entries[i] = gtk_entry_new_with_max_length (1);
		gtk_object_set_data (GTK_OBJECT (entries[i]), "entries", entries);
		gtk_widget_set_usize (entries[i], 20, 0);

		gtk_signal_connect (GTK_OBJECT (entries[i]), "grab_focus", 
												states_view_entry_grab_focus_callback, NULL);
		if (i != 0)
			gtk_signal_connect_object (GTK_OBJECT (entries[i - 1]), "activate", 
																 gtk_widget_grab_focus, GTK_OBJECT (entries[i]));
																			 
		gtk_table_attach (GTK_TABLE (table), label, i * 2, i * 2 + 1, 0, 1, 0, 0, 0, 0);
		gtk_table_attach (GTK_TABLE (table), entries[i], i * 2, i * 2 + 1, 1, 2, 0, 0, 0, 0);
		gtk_table_attach (GTK_TABLE (table), gtk_vseparator_new (), 
											i * 2 + 1, i * 2 + 2, 0, 2, 0, GTK_FILL | GTK_EXPAND, 0, 0);
	}
	gtk_signal_connect_object (GTK_OBJECT (entries[i - 1]), "activate", 
														 gtk_widget_grab_focus, GTK_OBJECT (button));
	gtk_signal_connect_object (GTK_OBJECT (button), "clicked", 
														 gtk_widget_grab_focus, GTK_OBJECT (entries[0]));
	gtk_signal_connect (GTK_OBJECT (button), "clicked", 
											states_view_edit_set_clicked_callback, NULL);

	gtk_widget_show_all (frame);
}

void states_view_close_callback(GtkWidget *w, gpointer data)
{
	GtkWidget **entries;
	
	entries = gtk_object_get_data (GTK_OBJECT (w), "entries");
	if (entries)
		g_free (entries);
	
	state_clist = NULL;
	edit_save = NULL;
	
	gtk_widget_destroy(w);
}

void state_clist_select_state(turing *t)
{
	gint i, tmp;
	char buff[20];
	GtkWidget **entries;

	if (t->actualstate) {
		snprintf(buff, 20, _("State: %d"), t->actualstate->no);
		gtk_label_set(GTK_LABEL(statusline), buff);
	} else 
		gtk_label_set(GTK_LABEL(statusline), _("Stopped"));
			
	if (state_clist) {
		tmp = i = GPOINTER_TO_INT (gtk_object_get_user_data(GTK_OBJECT(state_clist)));
		while (i--)
			if (t->actualstate == 
					(turing_state *) gtk_clist_get_row_data(GTK_CLIST(state_clist), i))
				break;
		
		if (i >= 0) {
			gtk_clist_select_row(GTK_CLIST(state_clist), i, 0);
			/* bug: moveto corrupts the clist if all the rows appear in the list */
/*			if (tmp > 11)*/
				gtk_clist_moveto(GTK_CLIST(state_clist), (i - 5 < 5)? 0 : i - 5, 0, 0, 0);
			
		}
	}
}

int next_state(turing *t)
{
	int ret;
	
	ret = turing_run_state(t);
	state_clist_select_state(t);
	
	return ret;
}

void power_do (void)
{
	stop_flag = 1;
	tm->state = 0;
	tm->pos = 0;

	turing_set_tape(tm, tape_string);
	set_tape(tape_string);

	next_state(tm);
	set_toolbar_sens(FALSE, FALSE, TRUE, TRUE);
}

void power_callback(GtkWidget *power_button, gpointer data)
{
	power_do ();
}

gint do_play(gpointer data)
{
	char *tmp;
	int cont;
	
	cont = FALSE;

	if (!stop_flag) {
		tmp = turing_get_tape(tm);
		set_tape(tmp);
		free (tmp);
		
		cont = next_state(tm);
		
		if (! cont)
			set_toolbar_sens(TRUE, FALSE, stop_flag, stop_flag);
	}
	
	return cont;
}

void play_callback(GtkWidget *play_button, gpointer data)
{
	set_toolbar_sens(TRUE, TRUE, FALSE, FALSE);
	
	stop_flag = 0;
	gtk_timeout_add(speed, do_play, NULL);
}

void step_callback(GtkWidget *step_buttton, gpointer data)
{
	char *tmp;
	
	tmp = turing_get_tape(tm);
	set_tape(tmp);
	free (tmp);
			
	if (! next_state(tm))	{
		set_toolbar_sens(TRUE, FALSE, FALSE, FALSE);
		gtk_label_set(GTK_LABEL(statusline), _("Stopped"));
	}
	
	gtk_widget_set_sensitive(power, TRUE);
}

void stop_callback(GtkWidget *stop_button, gpointer data)
{
	stop_flag = 1;
	
	set_toolbar_sens(TRUE, FALSE, TRUE, TRUE);
}

void prompt(char *title, char *msg, GtkSignalFunc callback, char *def)
{
	GtkWidget *vbox, *label;
	
	if (dialog != NULL)
		return;
	
	dialog = gnome_dialog_new(title, GNOME_STOCK_BUTTON_OK, 
														GNOME_STOCK_BUTTON_CANCEL, NULL);
	vbox = GNOME_DIALOG(dialog)->vbox;
	label = gtk_label_new(msg);
	gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);
	entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entry), def);
	gtk_box_pack_start(GTK_BOX(vbox), entry, TRUE, TRUE, 0);
	
	gtk_signal_connect(GTK_OBJECT(dialog), "destroy",
			   GTK_SIGNAL_FUNC(cancel_callback), NULL);
  gnome_dialog_button_connect(GNOME_DIALOG(dialog), 0,
															GTK_SIGNAL_FUNC(callback), NULL);
	gnome_dialog_button_connect_object(GNOME_DIALOG(dialog), 1,
																		 GTK_SIGNAL_FUNC(cancel_callback),
																		 GTK_OBJECT(dialog));	
	
	gtk_widget_show_all(vbox);
	gtk_widget_show(dialog);
}

void save_call(GtkWidget *w, gpointer data) 
{
	GtkWidget *fsel;
	GtkWidget *frame, *frame2, *scrolled, *text;
	int i = 0;
	char *txt;
	
	dialog = fsel = gtk_file_selection_new(_("Save gTuring Program File"));
	
	frame2 = gtk_frame_new (_("Comment"));
	gtk_box_pack_start (GTK_BOX (GTK_FILE_SELECTION (fsel)->main_vbox), frame2, TRUE, TRUE, 0);
	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_NONE);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 8);
	gtk_container_add (GTK_CONTAINER (frame2), frame);
	scrolled = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled), 
																	GTK_POLICY_AUTOMATIC,
																	GTK_POLICY_AUTOMATIC);
	gtk_container_add (GTK_CONTAINER (frame), scrolled);
	text = gtk_text_new (NULL, NULL);
	gtk_text_set_editable (GTK_TEXT (text), TRUE);
	if (prog_message)
		gtk_editable_insert_text (GTK_EDITABLE (text), prog_message, strlen (prog_message), &i);
	gtk_container_add (GTK_CONTAINER (scrolled), text);
	
	gtk_widget_show_all (frame2);

	gtk_file_selection_set_filename (GTK_FILE_SELECTION(fsel), states_fname);
	gtk_object_set_data (GTK_OBJECT (fsel), "text", text);
	gtk_signal_connect (GTK_OBJECT(GTK_FILE_SELECTION(fsel)->ok_button), "clicked",
											GTK_SIGNAL_FUNC(states_fname_callback), (gpointer) TRUE);
	gtk_signal_connect (GTK_OBJECT(GTK_FILE_SELECTION(fsel)->cancel_button), "clicked", 
											GTK_SIGNAL_FUNC(cancel_callback), NULL);
	gtk_widget_show(fsel);
}

void open_call(GtkWidget *w, gpointer data) 
{
	GtkWidget *fsel;
	GtkWidget *button;
	        
	dialog = fsel = gtk_file_selection_new(_("Open gTuring Program File"));
	
	button = gtk_button_new_with_label ("Examples");
	gtk_container_add (GTK_CONTAINER (GTK_FILE_SELECTION (fsel)->button_area), button);
	gtk_widget_show (button);
	
	gtk_signal_connect (GTK_OBJECT(button), "clicked", 
											GTK_SIGNAL_FUNC(states_fname_examples_callback), fsel);
	gtk_signal_connect (GTK_OBJECT(GTK_FILE_SELECTION(fsel)->ok_button), "clicked",
											GTK_SIGNAL_FUNC(states_fname_callback), (gpointer) FALSE);
	gtk_signal_connect (GTK_OBJECT(GTK_FILE_SELECTION(fsel)->cancel_button), "clicked", 
											GTK_SIGNAL_FUNC(cancel_callback), NULL);
	gtk_widget_show(fsel);
}

void tape_call(GtkWidget *w, gpointer data)
{
	prompt(_("Tape Setting"), _("Please enter the tape:"), 
				 GTK_SIGNAL_FUNC(tape_string_callback), tape_string);
}

void state_clist_load_states(void)
{
	char *text[5];
	turing_state *s;
	gint i;

	if (state_clist) {
		gtk_clist_clear(GTK_CLIST(state_clist));
		
		for (i = 0; i < 5; i++)
			text[i] = malloc(6);
		
		i = 0;
		for (s = tm->statehead; s; s = s->next) {
			snprintf(text[0], 6, "%d", s->no);
			text[1][0] = s->read; text[1][1] = 0;
			text[2][0] = s->write; text[2][1] = 0;
			text[3][0] = s->move; text[3][1] = 0;
			snprintf(text[4], 6, "%d", s->new);
			i++;
			gtk_clist_insert(GTK_CLIST(state_clist), 0, text);
		}
		
		gtk_object_set_user_data(GTK_OBJECT(state_clist), GINT_TO_POINTER (i));
		
		for (s = tm->statehead; i--; s = s->next)
			gtk_clist_set_row_data(GTK_CLIST(state_clist), i, s);
		
		for (i = 0; i < 5; i++)
			free(text[i]);
	}
}

void state_clist_select_row_callback (GtkCList *clist, gint row, gint column, GdkEventButton *event,
																			gpointer user_data)
{
	GtkWidget **entries;
	
	entries = gtk_object_get_data (GTK_OBJECT (clist), "entries");
	if (entries)
	{
		int i;
		gchar *txt;
				
		for (i = 0; i < ENTRIES_MAX; i++)
		{
			gtk_clist_get_text (GTK_CLIST (state_clist), row, i, &txt);
			gtk_entry_set_text (GTK_ENTRY (entries[i]), txt);
		}
	}
}

void view_states_call(GtkWidget *widget, gpointer data)
{
	char *text[5] = { N_("State"), N_("Read"), N_("Write"), N_("Move"), 
		                N_("New State")	};
	GtkWidget *w, *vbox;
	GtkWidget *scrolled;
	int i;
	
	if (state_clist)
		return;
	
	w = gnome_dialog_new(_("Machine's states"), NULL);
	gnome_dialog_append_button_with_pixmap (GNOME_DIALOG (w), "  Edit >>", GNOME_STOCK_PIXMAP_PROPERTIES);
	gnome_dialog_append_button (GNOME_DIALOG (w), GNOME_STOCK_BUTTON_CLOSE);
	gtk_window_set_policy (GTK_WINDOW (w), TRUE, TRUE, FALSE);
	gnome_dialog_button_connect (GNOME_DIALOG(w),	0,
															 GTK_SIGNAL_FUNC(states_view_edit_callback), w);
	gnome_dialog_button_connect_object(GNOME_DIALOG(w),	1,
																		 GTK_SIGNAL_FUNC(states_view_close_callback),
																		 GTK_OBJECT(w));

#ifdef ENABLE_NLS
	{
		int i=0;
		for (i=0;i<5;i++) text[i]=_(text[i]);
	}
#endif
	
	state_clist = gtk_clist_new_with_titles(5, text);
	gtk_clist_set_selection_mode(GTK_CLIST(state_clist), GTK_SELECTION_SINGLE);
	gtk_clist_column_titles_passive(GTK_CLIST(state_clist));
	for (i = 0; i < 5; i++)
		gtk_clist_set_column_width(GTK_CLIST(state_clist), i, 60);
	gtk_widget_set_usize(state_clist, 360, 200);
	
	state_clist_load_states();
	state_clist_select_state(tm);
	
	gtk_signal_connect (GTK_OBJECT (state_clist), "select_row", state_clist_select_row_callback, NULL);

	scrolled = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled), 
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	gtk_container_add (GTK_CONTAINER (scrolled), state_clist);

	gtk_box_pack_start(GTK_BOX(GNOME_DIALOG(w)->vbox), scrolled, TRUE, TRUE, 0);
  gtk_widget_show_all(GNOME_DIALOG(w)->vbox);
	gtk_widget_show(w);
}

void view_comment_call(GtkWidget *w, gpointer data)
{
	view_comment ();
}

void exit_call(GtkWidget *w, gpointer data)
{
	gtk_main_quit();
	exit (0);
}

void playspeed_call(GtkWidget *w, gpointer data)
{
	char buff[1024];
					
	sprintf(buff, "%ld", speed);
	prompt(_("Animation Speed"), _("Miliseconds between steps:"), 
				 GTK_SIGNAL_FUNC(speed_callback), buff);
}

void viewstates_call(GtkWidget *w, gpointer data)
{
}

void operation_call(GtkWidget *w, gpointer data)
{
}

void about_call(GtkWidget *w, gpointer data)
{
	GtkWidget *about;
	const gchar *authors[] = { "arturo@nuclecu.unam.mx", NULL };
	
	about = gnome_about_new (_("gturing"), NULL,
													 "(C) 1997-1998 the Free Software Fundation",
													 authors,
													 _("A Turing Machine for GNOME"),
													 NULL);
	gtk_widget_show (about);
}

GnomeUIInfo filemenu[] = {

	GNOMEUIINFO_MENU_OPEN_ITEM(open_call, NULL),
	GNOMEUIINFO_MENU_SAVE_AS_ITEM(save_call, NULL),

	GNOMEUIINFO_SEPARATOR,

	GNOMEUIINFO_MENU_EXIT_ITEM(exit_call, NULL),

	GNOMEUIINFO_END
};

GnomeUIInfo viewmenu[] = {
  
	GNOMEUIINFO_ITEM_NONE(N_("_Comment..."), N_("View the program's comment."),
			      view_comment_call),
	GNOMEUIINFO_ITEM_NONE(N_("_States..."),
			      N_("Open a table with the machine's states."),
			      view_states_call),
	GNOMEUIINFO_END
};

GnomeUIInfo settingsmenu[] = {
  
	GNOMEUIINFO_ITEM_NONE(N_("_Play Speed..."), N_("Set playing speed."),
			      playspeed_call),
	GNOMEUIINFO_ITEM_NONE(N_("_Tape..."), N_("Set the machine's tape."),
			      tape_call),
	GNOMEUIINFO_END
};

GnomeUIInfo helpmenu[] = {
	GNOMEUIINFO_HELP("gturing"),
	GNOMEUIINFO_MENU_ABOUT_ITEM(about_call, NULL),
	GNOMEUIINFO_END
};

GnomeUIInfo mainmenu[] = {
        GNOMEUIINFO_MENU_FILE_TREE(filemenu),
	GNOMEUIINFO_MENU_SETTINGS_TREE(settingsmenu),
	GNOMEUIINFO_MENU_VIEW_TREE(viewmenu),
	GNOMEUIINFO_MENU_HELP_TREE(helpmenu),
	GNOMEUIINFO_END
};

GnomeUIInfo toolbar[] = {
	GNOMEUIINFO_ITEM_STOCK(N_("Reset"), N_("Reset"), power_callback,
			       "gTuringReset"),
	GNOMEUIINFO_ITEM_STOCK(N_("Stop"), N_("Stop"), stop_callback,
			       "gTuringStop"),
	GNOMEUIINFO_ITEM_STOCK(N_("Run"), N_("Run"), play_callback,
			       "gTuringRun"),
	GNOMEUIINFO_ITEM_STOCK(N_("Step"), N_("Step"), step_callback,
			       "gTuringStep"),
	GNOMEUIINFO_END
};

char *try_font (void)
{
	GdkFont *font;
	int i;
	
	i = 0;
	font = NULL;
	while (tape_font[i] && !font)
		font = gdk_font_load (tape_font[i]);
	
	return tape_font[i];
}

void create_machine(void)
{
	GtkWidget *frame;
	GtkRcStyle *style;
	
	frame = rootw;
	tapelabel = gtk_label_new((*tape_string != 0)? tape_string : _("Welcome to gTuring."));
	/* This string is supposed to have the same length as the welcoming string. */
	headlabel = gtk_label_new(_("^                   "));
	
	style = gtk_rc_style_new ();
	style->font_name = try_font ();
	gtk_widget_modify_style (GTK_WIDGET (tapelabel), style);
	gtk_widget_modify_style (GTK_WIDGET (headlabel), style);
	
	gtk_box_pack_start(GTK_BOX(rootw), tapelabel, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(rootw), headlabel, TRUE, TRUE, 0);
}

void create_status(void)
{
	GtkWidget *hbox, *frame;

	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_end(GTK_BOX(rootw), hbox, FALSE, FALSE, 0);
	
	frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
	gtk_box_pack_start(GTK_BOX(hbox), frame, TRUE, TRUE, 0);
	helpline = gtk_label_new("");
	gtk_container_add(GTK_CONTAINER(frame), helpline);
	
	frame = gtk_frame_new(NULL);
	gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);
	gtk_box_pack_start(GTK_BOX(hbox), frame, TRUE, TRUE, 0);
	statusline = gtk_label_new("");
	gtk_container_add(GTK_CONTAINER(frame), statusline);
}

GnomeStockPixmapEntry *pentry_new(gchar **xpm_data, gint size)
{
	GnomeStockPixmapEntry *pentry;
	
	pentry = g_malloc(sizeof(GnomeStockPixmapEntry));
	pentry->data.type = GNOME_STOCK_PIXMAP_TYPE_DATA;
	pentry->data.width = size;
	pentry->data.height = size;
	pentry->data.label = NULL;
	pentry->data.xpm_data = xpm_data;
	
	return pentry;
}

static void init_stock(void)
{
	GnomeStockPixmapEntry *pentry;
	gchar **xpms[] = { power_xpm, play_xpm, stop_xpm, step_xpm, NULL };
	gchar *names[] = { "Reset", "Run", "Stop", "Step" };
	gchar stockname[22];
	int i;
	
	for (i = 0; xpms[i]; i++) {
		snprintf(stockname, 22, "gTuring%s", names[i]);
		pentry = pentry_new(xpms[i], 8);
		gnome_stock_pixmap_register(stockname, GNOME_STOCK_PIXMAP_REGULAR, pentry);
	}
}

void init_interface(int argc, char *argv[])
{
	GtkWidget *app;
	
	init_stock();						
	
	app = gnome_app_new("gturing", "gturing");
	gtk_window_set_wmclass(GTK_WINDOW(app), "gturing", "gturing");
	gtk_signal_connect(GTK_OBJECT(app), "delete_event",
			   GTK_SIGNAL_FUNC(exit_call), NULL);
	gtk_widget_show(app);

	rootw = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	gnome_app_set_contents(GNOME_APP(app), rootw);
	gnome_app_create_menus(GNOME_APP(app), mainmenu);
	gnome_app_create_toolbar(GNOME_APP(app), toolbar);

	save = filemenu[1].widget;
	
	power = toolbar[0].widget;
	stop = toolbar[1].widget;
	play = toolbar[2].widget;
	step = toolbar[3].widget;

	set_save_sens (FALSE);
	set_toolbar_sens(FALSE, FALSE, FALSE, FALSE);
	
	create_machine();
	create_status();
	
	gtk_widget_show_all(rootw);
}

void init_globals(void)
{
	tm = turing_new();
	
	strncpy(tape_string, gnome_config_get_string("/gTuring/Options/tape="), 1024);
	strncpy(states_fname, gnome_config_get_string("/gTuring/Options/program="), 1024);
	speed = gnome_config_get_int("/gTuring/Options/speed=50");
}

/* Want to add command-line options? This is the right place. */
void parse_args(int argc, char *argv[])
{
/*	int i;
	char help[] = N_("%s:\n\nUsage: %s [options] [states_file] [tape_string]\n\n\
-?  --help           Display this help and exit.\n");*/

	progname = argv[0];
/*	i = 1;
	
	while(i < argc)
		{
			if ((strcmp(argv[i], _("--help")) == 0) ||
							 (strcmp(argv[i], "-?") == 0))
				{
					fprintf(stderr, help, progname, progname);
					exit (1);
				}
			else if (*states_fname == 0)
				strcpy(states_fname, argv[i]);
			else if (*tape_string == 0)
				strcpy(tape_string, argv[i]);
			else
				{
					fprintf(stderr, help, progname, progname);
					exit (1);
				}
			
			i++;
		}*/
}

/* The main. */
int main (int argc, char *argv[])
{
	gnome_score_init("gnomecard");

        bindtextdomain(PACKAGE, GNOMELOCALEDIR);
        textdomain(PACKAGE);

	parse_args(argc, argv);
	gnome_init("gnomecard", VERSION, argc, argv);

	init_globals();
	init_interface(argc, argv);
	
	if (*states_fname != 0)
		set_states_fname(states_fname);

	gtk_main();
	
	return 0;
}
