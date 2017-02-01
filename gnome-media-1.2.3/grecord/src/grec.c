/*
 * GNOME sound-recorder: a soundrecorder and soundplayer for GNOME.
 *
 * Copyright (C) 2000 :
 * Andreas Hyden <a.hyden@cyberpoint.se>
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
*/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <audiofile.h>
#include <esd.h>
#include <fcntl.h>
#include <errno.h>

#include "grec.h"
#include "gui.h"
#include "preferences.h"
#include "sound.h"

#include "prog.h"

#define READ 0
#define WRITE 1

#define SAVE 0
#define DONTSAVE 1
#define CANCEL 2

const gchar* maintopic = N_("GNOME Soundrecorder:");

const gchar* temp_filename_record = "untitled.raw";
const gchar* temp_filename_play = "untitled.wav";
const gchar* temp_filename_backup = "untitled_backup.wav";
const gchar* temp_filename = "grecord_temp.wav";

gint repeat_counter = 0;

gchar* active_file = NULL;
gboolean default_file = FALSE;
gboolean file_changed = FALSE;

static guint play_id;
static guint record_id;

/* ------------------- Callbacks ------------------------------- */
void
on_record_activate_cb (GtkWidget* widget, gpointer data)
{
	/* Check if the sounddevice is ready */
	if (!check_if_sounddevice_ready ())
		return;

	grecord_set_sensitive_progress ();
	file_changed = TRUE;

	/* Reset record-time and stuff */
	UpdateStatusbarRecord (TRUE);

	RecEng.pid = fork ();
	if (RecEng.pid == 0) {
		
		/* Record */
		record_sound ();

		_exit (0);
	}
	else if (RecEng.pid == -1)
		g_error (_("Could not fork child process"));

	gnome_appbar_push (GNOME_APPBAR (grecord_widgets.appbar), _("Recording..."));
	RecEng.is_running = TRUE;

	record_id = gtk_timeout_add (1000,
				     (GtkFunction) UpdateStatusbarRecord,
				     FALSE);
}

void
on_play_activate_cb (GtkWidget* widget, gpointer data)
{
	/* Check if the sounddevice is ready */
	if (!check_if_sounddevice_ready ())
		return;

	grecord_set_sensitive_progress ();

	/* Show play-time and stuff */
	UpdateStatusbarPlay (TRUE);

	PlayEng.pid = fork ();
	if (PlayEng.pid == 0) {
		/* Play file */
		play_sound (active_file);

		_exit (0);
	}
	else if (PlayEng.pid == -1)
		g_error (_("Could not fork child process"));
	
	gnome_appbar_push (GNOME_APPBAR (grecord_widgets.appbar), _("Playing..."));
	PlayEng.is_running = TRUE;
	
	play_id = gtk_timeout_add (1000,
				   (GtkFunction) UpdateStatusbarPlay,
				   (gpointer) FALSE);
}

void
on_stop_activate_cb (GtkWidget* widget, gpointer data)
{
	gchar* temp_string1 = NULL;
	gchar* temp_string2 = NULL;

	gnome_appbar_pop (GNOME_APPBAR (grecord_widgets.appbar));

	if (RecEng.is_running) {
		temp_string1 = g_strconcat ("-r ", samplerate, NULL);
		
		if (channels)
			temp_string2 = g_strdup ("-c 1");
		else
			temp_string2 = g_strdup ("-c 2");

	}

	if (PlayEng.is_running) {

		gtk_timeout_remove (play_id);

		kill (PlayEng.pid, SIGKILL);
		waitpid (PlayEng.pid, NULL, WUNTRACED);

		PlayEng.is_running = FALSE;

		grecord_set_sensitive_file ();
	}

	if (RecEng.is_running) {
		gchar* command;
		gchar* temp_string;
		gchar* tfile1 = g_concat_dir_and_file (temp_dir, temp_filename_record);
		gchar* tfile2 = g_concat_dir_and_file (temp_dir, temp_filename_play);
		
		command = g_strconcat (sox_command, " ", temp_string1, " ", temp_string2, " ",
				       "-w ", "-s ", tfile1, " ",
				       tfile2, NULL);
		
		kill (RecEng.pid, SIGKILL);
		waitpid (RecEng.pid, NULL, WUNTRACED);

		RecEng.is_running = FALSE;

		run_command (command, _("Converting file..."));

		temp_string = g_concat_dir_and_file (temp_dir, temp_filename_play);

		g_free (temp_string);
		g_free (command);
		g_free (tfile1);
		g_free (tfile2);
	}

	g_free (temp_string1);
	g_free (temp_string2);
}

void
on_new_activate_cb (GtkWidget* widget, gpointer data)
{
	gint choice;
	gchar* string = NULL;
	gchar* temp_string = NULL;
	gchar* file1 = g_concat_dir_and_file (temp_dir, temp_filename_record);
	gchar* file2 = g_concat_dir_and_file (temp_dir, temp_filename_play);
	gchar* file3 = g_concat_dir_and_file (temp_dir, temp_filename_backup);

	if (PlayEng.is_running || RecEng.is_running || convert_is_running)
		on_stop_activate_cb (widget, data);

	if (file_changed) {
		choice = save_dont_or_cancel ();
		if (choice == SAVE) {
			save_dialog ();
			return;
		}
		else if (choice == CANCEL)
			return;
	}

	/* Remove old files (if any) */
	remove (file1);
	remove (file2);
	remove (file3);

	g_free (file1);
	g_free (file2);
	g_free (file3);

	default_file = TRUE;

	active_file = g_concat_dir_and_file (temp_dir, temp_filename_play);
	file_changed = FALSE;

	set_min_sec_time (get_play_time (active_file), TRUE);

	grecord_set_sensitive_nofile ();

	gtk_range_set_adjustment (GTK_RANGE (grecord_widgets.Statusbar), GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 1000, 1, 1, 0)));
	gtk_range_slider_update (GTK_RANGE (grecord_widgets.Statusbar));

	/* Reload configuration */
	load_config_file ();

	if (!audioformat)
		string = g_strdup ("16bit pcm");
	else
		string = g_strdup ("8bit pcm");

	temp_string = g_strconcat (_("Audioformat: "), string, NULL);
	gtk_label_set_text (GTK_LABEL (grecord_widgets.audio_format_label), temp_string);
	g_free (temp_string);

	temp_string = g_strconcat (_("Sample rate: "), samplerate, NULL);
	gtk_label_set_text (GTK_LABEL (grecord_widgets.sample_rate_label), temp_string);
	g_free (temp_string);
	g_free (string);

	if (!channels)
		string = g_strdup ("stereo");
	else
		string = g_strdup ("mono");

	temp_string = g_strconcat (_("Channels: "), string, NULL);
	gtk_label_set_text (GTK_LABEL (grecord_widgets.nr_of_channels_label), temp_string);
	g_free (temp_string);
	g_free (string);
}

void
on_open_activate_cb (GtkWidget* widget, gpointer data)
{
	gint choice;
	static GtkWidget* filesel = NULL;
	if (filesel) {
		if (filesel->window == NULL)
			return;
		
		gdk_window_show (filesel->window);
		gdk_window_raise (filesel->window);
		return;
	}

	if (file_changed) {
		choice = save_dont_or_cancel ();
		if (choice == SAVE) {
			save_dialog ();
			return;
		}
		else if (choice == CANCEL)
			return;
	}

	filesel = gtk_file_selection_new (_("Select a sound file"));

	gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (filesel)->ok_button),
			    "clicked", 
			    GTK_SIGNAL_FUNC (store_filename),
			    filesel);

	gtk_signal_connect (GTK_OBJECT (filesel),
			    "destroy",
			    GTK_SIGNAL_FUNC (gtk_widget_destroyed),
			    &filesel);

	gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION (filesel)->cancel_button),
				   "clicked", 
				   GTK_SIGNAL_FUNC (gtk_widget_destroy),
				   (gpointer) filesel);

	gtk_widget_show (filesel);
}

void
on_save_activate_cb (GtkWidget* widget, gpointer data)
{
	/* Check if the current sample is a new (recorded) one */
	is_file_default ();

	if (default_file)
		save_dialog ();

	/* No saving is needed, because the changes goes directly to the file. */
	/* If you don't want to save the changes (when it ask you) or if you select */
	/* 'undo all', it will copy the backup file back to the active file. */
}

void
on_saveas_activate_cb (GtkWidget* widget, gpointer data)
{
	save_dialog ();
}

void
on_exit_activate_cb (GtkWidget* widget, gpointer data)
{
	gint choice;
	gchar* tfile;
	gchar* command;

	if (PlayEng.is_running || RecEng.is_running)
		on_stop_activate_cb (widget, data);

	if (file_changed) {
		choice = save_dont_or_cancel ();
		if (choice == SAVE) {
			save_dialog ();
			return;
		}
		else if (choice == CANCEL)
			return;
	}

	/* User didn't want to save; copy the backup file to the changed file (active file) */
	tfile = g_concat_dir_and_file (temp_dir, temp_filename_backup);
	
	if (g_file_exists (tfile)) {
		command = g_strconcat ("cp -f ", tfile, " ", active_file, NULL);
		system (command);
		g_free (command);
	}

	gtk_main_quit ();

	remove (tfile);
	g_free (tfile);

	tfile = g_concat_dir_and_file (temp_dir, temp_filename_record);
	remove (tfile);
	g_free (tfile);
	tfile = g_concat_dir_and_file (temp_dir, temp_filename_play);
	remove (tfile);
	g_free (tfile);
}

void
on_preferences_activate_cb (GtkWidget* widget, gpointer data)
{
	static GtkWidget* props = NULL;

	if (props) {
		if (!props->window)
			return;

		gdk_window_show (props->window);
		gdk_window_raise (props->window);
		return;
	}
	props = create_grecord_propertybox ();
	
	gtk_signal_connect (GTK_OBJECT (props),
			    "destroy",
			    GTK_SIGNAL_FUNC (gtk_widget_destroyed),
			    &props);
	gtk_widget_show (props);
}

void
on_about_activate_cb (GtkWidget* widget, gpointer data)
{
	static GtkWidget* about = NULL;

	if (about) {
		if (about->window == NULL)
			return;
		
		gdk_window_show (about->window);
		gdk_window_raise (about->window);
		return;
	}
	
	about = create_about ();

	gtk_signal_connect (GTK_OBJECT (about),
			    "destroy",
			    GTK_SIGNAL_FUNC (gtk_widget_destroyed),
			    &about);

	gtk_widget_show (about);
}

void
on_runmixer_activate_cb (GtkWidget* widget, gpointer data)
{
	gchar* temp_string = g_strconcat (mixer_command, " &", NULL);
	system (temp_string);
	g_free (temp_string);
}

void 
on_increase_speed_activate_cb (GtkWidget* widget, gpointer data)
{
}

void 
on_decrease_speed_activate_cb (GtkWidget* widget, gpointer data)
{
}

void
on_add_echo_activate_cb (GtkWidget* widget, gpointer data)
{
	if (!g_file_exists (active_file))
	    return;

	file_changed = TRUE;
	add_echo (active_file, TRUE);
}

void
on_show_time_activate_cb (GtkWidget* widget, gpointer data)
{
	GtkCheckMenuItem* item = GTK_CHECK_MENU_ITEM (widget);
	gboolean active = item->active;

	if (active) {
		gtk_widget_show (grecord_widgets.timespace_label);
		gtk_widget_show (grecord_widgets.timesec_label);
		gtk_widget_show (grecord_widgets.timemin_label);
	}
	else {
		gtk_widget_hide (grecord_widgets.timespace_label);
		gtk_widget_hide (grecord_widgets.timesec_label);
		gtk_widget_hide (grecord_widgets.timemin_label);
	}

	show_time = active;

	gnome_config_set_bool ("/grecord/GUI Options/showtime", show_time);
	gnome_config_sync ();
}

void
on_show_soundinfo_activate_cb (GtkWidget* widget, gpointer data)
{
	GtkCheckMenuItem* item = GTK_CHECK_MENU_ITEM (widget);
	gboolean active = item->active;

	if (active) {
		gtk_widget_show (grecord_widgets.audio_format_label);
		gtk_widget_show (grecord_widgets.sample_rate_label);
		gtk_widget_show (grecord_widgets.nr_of_channels_label);
	}
	else {
		gtk_widget_hide (grecord_widgets.audio_format_label);
		gtk_widget_hide (grecord_widgets.sample_rate_label);
		gtk_widget_hide (grecord_widgets.nr_of_channels_label);
	}

	show_soundinfo = active;

	gnome_config_set_bool ("/grecord/GUI Options/showsoundinfo", show_soundinfo);
	gnome_config_sync ();
}

void
on_undo_activate_cb (GtkWidget* widget, gpointer data)
{
}

void
on_redo_activate_cb (GtkWidget* widget, gpointer data)
{
}

void
on_undoall_activate_cb (GtkWidget* widget, gpointer data)
{
	gchar* temp_string = g_concat_dir_and_file (temp_dir, temp_filename_backup);

	if (g_file_exists (temp_string)) {
		gchar* t = g_strconcat ("cp -f ", temp_string, " ", active_file, NULL);
		run_command (t, _("Undoing all changes..."));
		g_free (t);
	}

	g_free (temp_string);
}

/* --------------------- Help functions -------------------- */
void
record_sound (void)
{
	gchar* tfile;
	gchar buf[ESD_BUF_SIZE];
	gint sock = -1;
	gint rate = 0;
	gint length = 0;
	
	gint bits = ESD_BITS16;
	gint esd_channels = ESD_STEREO;
	gint mode = ESD_STREAM;
	gint func = ESD_RECORD;
	esd_format_t format = 0;

	gchar* host = NULL;
	gchar* name = NULL;

	FILE *target = NULL;

	/* Open the file for writing */
	tfile = g_concat_dir_and_file (temp_dir, temp_filename_record);
	target = fopen (tfile, "w");
	g_free (tfile);

	/* Set up bits, channels etc after the preferences */
	if (esd_channels == 1)
		esd_channels = ESD_MONO;
	if (audioformat != 0)
		bits = ESD_BITS8;

	rate = atoi (samplerate);

	format = bits | esd_channels | mode | func;
	sock = esd_record_stream_fallback (format, rate, host, name);
	
	if (sock <= 0)
		return;

	/* Start recording */
	while ((length = read(sock, buf, ESD_BUF_SIZE)) > 0) {
		if (fwrite (buf, 1, length, target) <= 0)
			return;
	}
	close (sock);
       	return;
}

void
play_sound (const gchar* filename)
{
	/* Play the file */
	esd_play_file ("grecord", filename, 1);
}

void
store_filename (GtkFileSelection* selector, gpointer file_selector)
{
	GtkWidget* mess;
	gchar* string = NULL;
	gchar* temp_string = NULL;
	AFfilehandle filename;
	gint in_audioformat, in_channels, in_rate, in_width;
	struct stat file;
	
	gchar* tempfile = gtk_file_selection_get_filename (GTK_FILE_SELECTION (file_selector));

	if (!stat(tempfile, &file)) {
		if (S_ISDIR (file.st_mode)) {
			gchar* show_mess = g_strdup_printf (_("'%s' is a directory.\nPlease select a soundfile to be opened."), tempfile);
			mess = gnome_message_box_new (show_mess,
						      GNOME_MESSAGE_BOX_ERROR,
						      GNOME_STOCK_BUTTON_OK,
					      NULL);
			g_free (show_mess);
			gtk_window_set_modal (GTK_WINDOW (mess), TRUE);
			gtk_widget_show (mess);
			return;
		}
	}
	else if (errno == ENOENT) {
		gchar* show_mess = g_strdup_printf (_("File '%s' doesn't exist.\nPlease select a existing soundfile to be opened."), tempfile);
		mess = gnome_message_box_new (show_mess,
					      GNOME_MESSAGE_BOX_ERROR,
					      GNOME_STOCK_BUTTON_OK,
					      NULL);
		g_free (show_mess);
		gtk_window_set_modal (GTK_WINDOW (mess), TRUE);
		gtk_widget_show (mess);
		return;
	}

	if (!soundfile_supported (tempfile)) {
		gchar* show_mess = g_strdup_printf (_("File '%s' isn't a valid soundfile."), tempfile);
		mess = gnome_message_box_new (show_mess,
					      GNOME_MESSAGE_BOX_ERROR,
					      GNOME_STOCK_BUTTON_OK,
					      NULL);
		g_free (show_mess);
		gtk_window_set_modal (GTK_WINDOW (mess), TRUE);
		gtk_widget_show (mess);
		return;
	}

	active_file = g_strdup (tempfile);
	file_changed = FALSE;
	gtk_widget_destroy (GTK_WIDGET (file_selector));
	
	grecord_set_sensitive_file ();
	
	/* Get info about the file */
	filename = afOpenFile (active_file, "r", NULL);
	if (!filename)
		return;

	in_channels = afGetChannels (filename, AF_DEFAULT_TRACK);
	in_rate     = afGetRate (filename, AF_DEFAULT_TRACK);
	afGetSampleFormat (filename, AF_DEFAULT_TRACK, &in_audioformat, &in_width);

	/* Update mainwindow with the new values and set topic */
	set_min_sec_time (get_play_time (active_file), TRUE);

	samplerate = g_strdup_printf ("%d", in_rate);
	if (in_channels == 2)
		string = g_strdup ("stereo");
	else
		string = g_strdup ("mono");

	temp_string = g_strconcat (_("Sample rate: "), samplerate, NULL);
	gtk_label_set_text (GTK_LABEL (grecord_widgets.sample_rate_label), temp_string);
	g_free (temp_string);

	temp_string =  g_strconcat (_("Channels: "), string, NULL);
	gtk_label_set_text (GTK_LABEL (grecord_widgets.nr_of_channels_label), temp_string);
	g_free (temp_string);
	g_free (string);

	if (in_audioformat)
		string = g_strdup ("16bit pcm");
	else
		string = g_strdup ("8bit pcm");
	
	temp_string = g_strconcat (_("Audioformat: "), string, NULL);
	gtk_label_set_text (GTK_LABEL (grecord_widgets.audio_format_label), temp_string);
	g_free (temp_string);
	g_free (string);

	default_file = FALSE;
}

void
save_filename (GtkFileSelection* selector, gpointer file_selector)
{
	gchar* temp_string;
	gchar* new_file;
	struct stat file;

	new_file = g_strdup (gtk_file_selection_get_filename (GTK_FILE_SELECTION (file_selector)));

	/* Check if the file already exists */
	if (!stat(new_file, &file)) {
		GtkWidget* mess;
		gchar* show_mess;
		gint choice;

		if (S_ISDIR (file.st_mode)) {
			show_mess = g_strdup_printf (_("'%s' is a directory.\nPlease enter another filename."), new_file);
			mess = gnome_message_box_new (show_mess,
						      GNOME_MESSAGE_BOX_ERROR,
						      GNOME_STOCK_BUTTON_OK,
						      NULL);
			g_free (show_mess);
			gtk_window_set_modal (GTK_WINDOW (mess), TRUE);
			gtk_widget_show (mess);
			return;
		}
		show_mess = g_strdup_printf (_("File '%s' already exists.\nDo you want to overwrite it?"), new_file);
		mess = gnome_message_box_new (show_mess,
					      GNOME_MESSAGE_BOX_WARNING,
					      GNOME_STOCK_BUTTON_YES,
					      GNOME_STOCK_BUTTON_NO,
					      GNOME_STOCK_BUTTON_CANCEL,
					      NULL);
		choice = gnome_dialog_run (GNOME_DIALOG (mess));
		g_free (show_mess);
		if (choice == 2 || choice == 1)
			return;
	}

	/* Check if the soundfile is supported */
	if (!save_sound_file (new_file)) {
		GtkWidget* mess = gnome_message_box_new (_("Error saving sound file"),
							 GNOME_MESSAGE_BOX_ERROR,
							 GNOME_STOCK_BUTTON_OK,
							 NULL);
		gtk_window_set_modal (GTK_WINDOW (mess), TRUE);
		gtk_widget_show (mess);
		g_free (new_file);
		return;
	}

	if (active_file[1] == '/') {
		gchar* tempstring;
		gchar* string = g_strdup ((char *) strrchr (active_file, '/'));
		string[1] = ' ';
		tempstring = g_strconcat (_(maintopic), string, NULL);
		gtk_window_set_title (GTK_WINDOW (grecord_widgets.grecord_window), tempstring);
		g_free (tempstring);
		g_free (string);
	}

	active_file = g_strdup (new_file);
	g_free (new_file);
	
	gtk_widget_destroy (GTK_WIDGET (file_selector));

	temp_string = g_strconcat ((maintopic), active_file, NULL);
	gtk_window_set_title (GTK_WINDOW (grecord_widgets.grecord_window), temp_string);
	g_free (temp_string);

	file_changed = FALSE;
	file_selector = NULL;
}

gint
save_dont_or_cancel (void)
{
	GtkWidget* mess = gnome_message_box_new (_("File not saved. Do you want to save it?"),
						 GNOME_MESSAGE_BOX_QUESTION,
						 GNOME_STOCK_BUTTON_YES,
						 GNOME_STOCK_BUTTON_NO,
						 GNOME_STOCK_BUTTON_CANCEL,
						 NULL);

	return (gnome_dialog_run (GNOME_DIALOG (mess)));
}

gboolean
save_sound_file (const gchar* filename)
{
	/* Check if the file is default (if it's been recorded) */
	if (default_file) {
		gchar* tfile = g_concat_dir_and_file (temp_dir, temp_filename_play);
		gchar* command = g_strconcat ("cp -f ", tfile, " ", filename, NULL);
		
		/* Save the file */
		run_command (command, _("Saving..."));

		g_free (tfile);
		g_free (command);

		/* It's saved now */
		default_file = FALSE;
	}

	/* No saving is needed, because the changes go directly to the active file; don't worry, */
	/* the file is being saved first time it's edited, so you just have to do 'undo all' to restore it. */
	return TRUE;
}

guint
UpdateStatusbarPlay (gboolean begin)
{
	static gdouble length;
	static gdouble temp;
	static gfloat counter = 0.00;
	static gint countersec = 0;

	if (begin) {
		counter = 0.00;
		countersec = 0;

		length = get_play_time (active_file);
		temp = 1000 / length;
	}
	else {
		if (waitpid (PlayEng.pid, NULL, WNOHANG | WUNTRACED)) {
			if (playrepeat && playrepeatforever) {
				on_play_activate_cb (NULL, NULL);
				return FALSE;
			}
			else if (playrepeat && !playrepeatforever) {
				if (repeat_counter < playxtimes) {
					on_play_activate_cb (NULL, NULL);
					repeat_counter++;
					return FALSE;
				}
				repeat_counter = 0;
			}
			
			counter = 0;
			PlayEng.is_running = FALSE;
			grecord_set_sensitive_file ();
			on_stop_activate_cb (NULL, NULL);
			return FALSE;
		}
	}

	counter += (float) temp;

	countersec++;

	set_min_sec_time (countersec, FALSE);

	gtk_range_set_adjustment (GTK_RANGE (grecord_widgets.Statusbar), GTK_ADJUSTMENT (gtk_adjustment_new (counter, 0, 1000, 1, 1, 0)));
	gtk_range_slider_update (GTK_RANGE (grecord_widgets.Statusbar));
	
	return TRUE;
}

guint
UpdateStatusbarRecord (gboolean begin)
{
	static gint counter = 0;
	static gint timeout = 0;
	static gint countersec = 0;

	if (begin) {
		counter = 0.00;
		timeout = 0;
		countersec = 0;
		return TRUE;
	}

	/* Timeout */
	if (counter >= 1000) {
		if (stop_on_timeout) {
			on_stop_activate_cb (NULL, NULL);
			return TRUE;
		}

		counter /= 2;
	}
	
	if (waitpid (RecEng.pid, NULL, WNOHANG | WUNTRACED)) {
		if (convert_is_running)
			return FALSE;

		counter = 0;
		RecEng.is_running = FALSE;
		grecord_set_sensitive_file ();
		on_stop_activate_cb (NULL, NULL);

		if (save_when_finished)
			save_dialog ();

		return FALSE;
	}

	countersec++;
	
	set_min_sec_time (countersec, FALSE);
	
	if (popup_warn_mess) {
		struct stat fileinfo;
		static gint maxfilesize = -1;
		static gboolean show_message = TRUE;
		gchar* filename = g_concat_dir_and_file (temp_dir, temp_filename_record);
		
		if (maxfilesize == -1)
			maxfilesize = popup_warn_mess_v;
		
		stat (filename, &fileinfo);
		
		g_free (filename);
		
		if (fileinfo.st_size >= (maxfilesize * 1000000) && show_message) {
			gchar* message = g_strdup_printf (N_("The size of the current sample is more than %i Mb!"),
							  (int) (fileinfo.st_size / 1000000) /* In MB */);
			GtkWidget* mess = gnome_message_box_new (_(message),
								 GNOME_MESSAGE_BOX_WARNING,
								 GNOME_STOCK_BUTTON_OK,
								 NULL);
			gtk_widget_show (mess);
			g_free (message);
			show_message = FALSE;
	        }			
	}
	
	if (stop_record) {
		struct stat fileinfo;
		static gint maxfilesize = -1;
		gchar* filename = g_concat_dir_and_file (temp_dir, temp_filename_record);

		if (maxfilesize == -1)
			maxfilesize = stop_record_v;
		
		stat (filename, &fileinfo);

		g_free (filename);

		if (fileinfo.st_size >= (maxfilesize * 1000000 /* In MB */ )) {
		  	on_stop_activate_cb (NULL, NULL);
		        return FALSE;
		}
	}

	/* Get the timeout value and convert it to seconds (if it isn't allready done) */
	if (timeout == 0)
		timeout = record_timeout * 60;

	counter += (int) 1000 / timeout;

	gtk_range_set_adjustment (GTK_RANGE (grecord_widgets.Statusbar), GTK_ADJUSTMENT (gtk_adjustment_new (counter, 0, 1000, 1, 1, 0)));
	gtk_range_slider_update (GTK_RANGE (grecord_widgets.Statusbar));

	return TRUE;
}

void
save_dialog (void)
{
	GtkWidget* filesel = NULL;
	gchar* temp_file = g_concat_dir_and_file (temp_dir, temp_filename_play);

	filesel = gtk_file_selection_new (_("Save sound file"));

	if (!g_strcasecmp (active_file, temp_file)) {
		gchar* home_dir = g_strdup (getenv ("HOME"));
		gchar* default_file = g_concat_dir_and_file (home_dir, _(temp_filename_play));

		gtk_file_selection_set_filename (GTK_FILE_SELECTION (filesel), default_file);

		g_free (home_dir);
		g_free (default_file);
	}
	else
		gtk_file_selection_set_filename (GTK_FILE_SELECTION (filesel), active_file);
		
	gtk_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (filesel)->ok_button),
			    "clicked", 
			    GTK_SIGNAL_FUNC (save_filename),
			    filesel);

	gtk_signal_connect_object (GTK_OBJECT (GTK_FILE_SELECTION (filesel)->cancel_button),
				   "clicked", 
				   GTK_SIGNAL_FUNC (gtk_widget_destroy),
				   (gpointer) filesel);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
	
	gtk_widget_show (filesel);

	g_free (temp_file);
}

void
is_file_default (void)
{
	gchar* temp_string;
	temp_string = g_concat_dir_and_file (temp_dir, temp_filename_play);
	if (!g_strcasecmp (temp_string, active_file))
	        default_file = TRUE;
	else
		default_file = FALSE;

	g_free (temp_string);
}

void grecord_set_sensitive_file (void)
{
	gtk_widget_set_sensitive (GTK_WIDGET (grecord_widgets.Record_button), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET (grecord_widgets.Play_button), TRUE);
	gtk_widget_set_sensitive (GTK_WIDGET (grecord_widgets.Stop_button), FALSE);

	/* Make the menu sensitive again */
	gtk_widget_set_sensitive (GTK_WIDGET (menubar1_uiinfo[0].widget), TRUE);
	gtk_widget_set_sensitive (GTK_WIDGET (menubar1_uiinfo[1].widget), TRUE);
	gtk_widget_set_sensitive (GTK_WIDGET (menubar1_uiinfo[2].widget), TRUE);
	gtk_widget_set_sensitive (GTK_WIDGET (menubar1_uiinfo[3].widget), TRUE);
}

void grecord_set_sensitive_nofile (void)
{
	gtk_widget_set_sensitive (GTK_WIDGET (grecord_widgets.Record_button), TRUE);
	gtk_widget_set_sensitive (GTK_WIDGET (grecord_widgets.Play_button), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET (grecord_widgets.Stop_button), FALSE);
}

void grecord_set_sensitive_progress (void)
{
	gtk_widget_set_sensitive (GTK_WIDGET (grecord_widgets.Record_button), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET (grecord_widgets.Play_button), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET (grecord_widgets.Stop_button), TRUE);
}

void
grecord_set_sensitive_loading (void)
{
	gtk_widget_set_sensitive (GTK_WIDGET (grecord_widgets.Record_button), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET (grecord_widgets.Play_button), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET (grecord_widgets.Stop_button), FALSE);

	/* Also make the menu insensitive, so you can't save or anything during the process */
	gtk_widget_set_sensitive (GTK_WIDGET (menubar1_uiinfo[0].widget), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET (menubar1_uiinfo[1].widget), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET (menubar1_uiinfo[2].widget), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET (menubar1_uiinfo[3].widget), FALSE);
}

void
run_command (const gchar* command, const gchar* appbar_comment)
{
	gint load_pid;

	/* Make the widgets insensitive */
	grecord_set_sensitive_loading ();

	/* Add a comment do the appbar about what is going on */
	gnome_appbar_push (GNOME_APPBAR (grecord_widgets.appbar), _(appbar_comment));

        load_pid = fork ();
	if (load_pid == 0) {
		/* Run the command */
		system (command);

		/* Finished, exit child process */
		_exit (0);
	}
	else if (load_pid == -1)
		g_error (_("Could not fork child process"));

	convert_is_running = TRUE;

	/* Add a function for checking when process has died */
	gtk_timeout_add (100, (GtkFunction) check_if_loading_finished, (gpointer) load_pid);
}

guint
check_if_loading_finished (gint pid)
{
	/* Check if process still alive */
	if (waitpid (pid, NULL, WNOHANG | WUNTRACED)) {
		
		/* Show the playtime of the file */
		set_min_sec_time (get_play_time (active_file), TRUE);
		
		/* Remove the comment from the appbar, because we're finished */
		gnome_appbar_pop (GNOME_APPBAR (grecord_widgets.appbar));

		/* Make widgets sensitive again */
		grecord_set_sensitive_file ();

		convert_is_running = FALSE;

		return FALSE;
	}

	return TRUE;
}

gboolean
soundfile_supported (const gchar* filename)
{
	AFfilehandle filetype  = afOpenFile (filename, "r", NULL);
	gint soundtype = afGetFileFormat (filetype, NULL);

	/* Check if the file exists */
	if (!g_file_exists (filename))
		return FALSE;

	/* Check if the file is a valid soundfile */
	if (soundtype == AF_FILE_UNKNOWN)
		return FALSE;

	return TRUE;
}

gboolean
check_if_sounddevice_ready ()
{
	/* Reset errno */
	errno = 0;

	/* Check if the sounddevice is ready */
	esd_audio_open ();
	 
	/* Sounddevice not ready, tell the user */
	if (errno != 0) {
		GtkWidget* mess;

		esd_audio_close ();

		mess = gnome_message_box_new (_("Sounddevice not ready! Please check that there isn't\nanother program running that's using the sounddevice."),
					      GNOME_MESSAGE_BOX_ERROR,
					      GNOME_STOCK_BUTTON_OK,
					      NULL);
		gnome_dialog_run (GNOME_DIALOG (mess));
		return FALSE;
	}

	esd_audio_close ();

	return TRUE;
}
