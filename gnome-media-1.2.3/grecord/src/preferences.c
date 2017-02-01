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
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#include "gui.h"
#include "preferences.h"

#include "prog.h"

void
load_config_file    (void)
{
	char *s;

	gnome_config_push_prefix ("/grecord/Recording/");
	record_timeout       = gnome_config_get_int ("recordtimeout=2");
	stop_on_timeout      = gnome_config_get_bool ("stopontimeout=TRUE");
	save_when_finished   = gnome_config_get_bool ("savewhenfinished=FALSE");
	popup_warn_mess      = gnome_config_get_bool ("popupwarnmess=TRUE");
	stop_record          = gnome_config_get_bool ("stoprecord=FALSE");
	popup_warn_mess_v    = gnome_config_get_int ("popupwarnmess_v=100");
	stop_record_v        = gnome_config_get_int ("stoprecord_v=200");
	gnome_config_pop_prefix ();

	gnome_config_push_prefix ("/grecord/Playing/");
	playrepeat           = gnome_config_get_bool ("playrepeat=FALSE");
	playrepeatforever    = gnome_config_get_bool ("playrepeatforever=TRUE");
	playxtimes           = gnome_config_get_int ("playxtimes=2");
	gnome_config_pop_prefix ();

	gnome_config_push_prefix ("/grecord/Paths/");

	s                    = gnome_config_get_string ("soxcommand=sox");
	sox_command          = gnome_is_program_in_path (s);
	if (sox_command == NULL) {
		sox_command = s;
	} else {
		g_free (s);
	}

	s                    = gnome_config_get_string ("mixercommand=gmix");
	mixer_command        = gnome_is_program_in_path (s);
	if (mixer_command == NULL) {
		mixer_command = s;
	} else {
		g_free (s);
	}

	temp_dir             = gnome_config_get_string ("tempdir=/tmp/");
	gnome_config_pop_prefix ();

	gnome_config_push_prefix ("/grecord/Soundoptions/");
	audioformat          = gnome_config_get_bool ("audioformat=FALSE");
	samplerate           = gnome_config_get_string ("samplerate=44100");
	channels             = gnome_config_get_bool ("channels=FALSE");
	gnome_config_pop_prefix ();

	gnome_config_push_prefix ("/grecord/GUI Options/");
	show_time            = gnome_config_get_bool ("showtime=TRUE");
	show_soundinfo       = gnome_config_get_bool ("showsoundinfo=FALSE");
	gnome_config_pop_prefix ();
}

void
save_config_file  (void)
{
	gnome_config_push_prefix ("/grecord/Recording/");
	gnome_config_set_int ("recordtimeout", record_timeout);
	gnome_config_set_bool ("stopontimeout", stop_on_timeout);
	gnome_config_set_bool ("savewhenfinished", save_when_finished);
	gnome_config_set_bool ("popupwarnmess", popup_warn_mess);
	gnome_config_set_bool ("stoprecord", stop_record);
	gnome_config_set_int ("popupwarnmess_v", popup_warn_mess_v);
	gnome_config_set_int ("stoprecord_v", stop_record_v);
	gnome_config_pop_prefix ();

	gnome_config_push_prefix ("/grecord/Playing/");
	gnome_config_set_bool ("playrepeat", playrepeat);
	gnome_config_set_bool ("playrepeatforever", playrepeatforever);
	gnome_config_set_int ("playxtimes", playxtimes);
	gnome_config_pop_prefix ();

	gnome_config_push_prefix ("/grecord/Paths/");
	gnome_config_set_string ("soxcommand", sox_command);
	gnome_config_set_string ("mixercommand", mixer_command);
	gnome_config_set_string ("tempdir", temp_dir);
	gnome_config_pop_prefix ();

	gnome_config_push_prefix ("/grecord/Soundoptions/");
	gnome_config_set_bool ("audioformat", audioformat);
	gnome_config_set_string ("samplerate", samplerate);
	gnome_config_set_bool ("channels", channels);
	gnome_config_pop_prefix ();

	gnome_config_push_prefix ("/grecord/GUI Options/");
	gnome_config_set_bool ("showtime", show_time);
	gnome_config_set_bool ("showsoundinfo", show_soundinfo);
	gnome_config_pop_prefix ();

	gnome_config_sync ();
}

void
free_vars                  (void)
{
	g_free (sox_command);
	g_free (mixer_command);
	g_free (temp_dir);
	g_free (samplerate);
}


/* Callbacks --------------------------------------------------------- */

void
widget_in_propertybox_changed (GtkWidget* widget,
			       gpointer propertybox)
{
	gnome_property_box_changed (GNOME_PROPERTY_BOX (propertybox));
}

void
on_checkbox_clicked_activate_cb (GtkWidget* widget, gpointer w)
{
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget)))
		gtk_widget_set_sensitive (GTK_WIDGET (w), TRUE);
	else
		gtk_widget_set_sensitive (GTK_WIDGET (w), FALSE);
}

void
on_repeat_activate_cb (GtkWidget* widget,
		       gpointer data)
{
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget))) {
		gtk_widget_set_sensitive (GTK_WIDGET (propertywidgets.playrepeatforever_radiobutton_v), TRUE);
		gtk_widget_set_sensitive (GTK_WIDGET (propertywidgets.playxtimes_radiobutton_v), TRUE);

		/* Don't make it sensitive if playxtimes_radiobutton is selected */
		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (propertywidgets.playxtimes_radiobutton_v)))
			gtk_widget_set_sensitive (GTK_WIDGET (propertywidgets.playxtimes_spinbutton_v), TRUE);
	}
	else {
		gtk_widget_set_sensitive (GTK_WIDGET (propertywidgets.playrepeatforever_radiobutton_v), FALSE);
		gtk_widget_set_sensitive (GTK_WIDGET (propertywidgets.playxtimes_radiobutton_v), FALSE);
		gtk_widget_set_sensitive (GTK_WIDGET (propertywidgets.playxtimes_spinbutton_v), FALSE);
	}
}

void
on_propertybox_apply_activate (GtkWidget* widget,
			       gint page_num,
			       gpointer widgets)
{
	GtkWidget* mess;
	gint choice;
	struct stat file_info;
	gchar* show_mess = NULL;
	gchar* audioformat_string = NULL;
	gchar* channels_string = NULL;
	gchar* temp_file = NULL;
	gboolean fullpath = FALSE;
	gboolean tempdir = FALSE;
	gint temp_count;

	if (page_num == -1)
		return;

	/* Get configuration from the propertybox ------------------------------------------- */
	record_timeout     = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (propertywidgets.RecordTimeout_spinbutton_v));
	stop_on_timeout    = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (propertywidgets.StopRecordOnTimeout_checkbox_v));
	save_when_finished = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (propertywidgets.PopupSaveOnTimeout_checkbox_v));
	popup_warn_mess    = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (propertywidgets.PopupWarnMessSize_checkbox_v));
	popup_warn_mess_v  = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (propertywidgets.WarningSize_spinbutton_v));
	stop_record        = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (propertywidgets.StopRecordSize_checkbox_v));
	stop_record_v      = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (propertywidgets.StopRecordSize_spinbutton_v));
    
	playrepeat         = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (propertywidgets.playrepeat_checkbox_v));
	playrepeatforever  = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (propertywidgets.playrepeatforever_radiobutton_v));
	playxtimes         = gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (propertywidgets.playxtimes_spinbutton_v));
       
	
	temp_file   = g_strdup (gtk_entry_get_text (GTK_ENTRY (gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (propertywidgets.Sox_fileentry_v)))));
	
	/* Check if the given sox command exists --------------------- */
	if (!g_strcasecmp (temp_file, "")) {
		show_mess = g_strdup_printf (_("You havn't entered a sox command; you will not be able to record.\nDo you want to use it anyway?"));
		mess = gnome_message_box_new (show_mess,
					      GNOME_MESSAGE_BOX_QUESTION,
					      GNOME_STOCK_BUTTON_YES,
					      GNOME_STOCK_BUTTON_NO,
					      NULL);
		g_free (show_mess);
		choice = gnome_dialog_run (GNOME_DIALOG (mess));
		if (choice == 0) 
			sox_command = g_strdup (temp_file);
		else
			gtk_entry_set_text (GTK_ENTRY (gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (propertywidgets.Sox_fileentry_v))), sox_command);
	}
	
	/* Check whetever the specified command is a full path or just the name (check for a '/') */
	temp_count = 0;
	while (temp_file[temp_count] != '\0') {
		if (temp_file[temp_count] == '/')
			fullpath = TRUE;
		temp_count++;
	}

	if (fullpath) {
		/* Check if the given mixer command exists -------------------- */
		if (!g_file_exists (temp_file)) {
			show_mess = g_strdup_printf (_("File %s in 'sox command' does not exist.\nDo you want to use it anyway?"), temp_file);
			mess = gnome_message_box_new (show_mess,
						      GNOME_MESSAGE_BOX_QUESTION,
						      GNOME_STOCK_BUTTON_YES,
						      GNOME_STOCK_BUTTON_NO,
						      NULL);
			g_free (show_mess);
			choice = gnome_dialog_run (GNOME_DIALOG (mess));
			if (choice == 0)
				sox_command = g_strdup (temp_file);
			else
				gtk_entry_set_text (GTK_ENTRY (gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (propertywidgets.Sox_fileentry_v))), sox_command);
		}
		else
			sox_command = g_strdup (temp_file);
	}
	else
		sox_command = g_strdup (temp_file);
	
	fullpath = FALSE;
	g_free (temp_file);
	temp_file   = g_strdup (gtk_entry_get_text (GTK_ENTRY (gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (propertywidgets.Mixer_fileentry_v)))));
	
	/* Check if the given mix command exists --------------------- */
	if (!g_strcasecmp (temp_file, "")) {
		show_mess = g_strdup_printf (_("You havn't entered a mixer command; you will not be able to start the mixer.\nDo you want to use it anyway?"));
		mess = gnome_message_box_new (show_mess,
					      GNOME_MESSAGE_BOX_QUESTION,
					      GNOME_STOCK_BUTTON_YES,
					      GNOME_STOCK_BUTTON_NO,
					      NULL);
		g_free (show_mess);
		choice = gnome_dialog_run (GNOME_DIALOG (mess));
		if (choice == 0) 
			mixer_command = g_strdup (temp_file);
		else
			gtk_entry_set_text (GTK_ENTRY (gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (propertywidgets.Mixer_fileentry_v))), mixer_command);
	}
	
	/* Check whetever the specified command is a full path or just the name (check for a '/') */
	temp_count = 0;
	while (temp_file[temp_count] != '\0') {
		if (temp_file[temp_count] == '/')
			fullpath = TRUE;
		temp_count++;
	}

	if (fullpath) {
		/* Check if the given mixer command exists -------------------- */
		if (!g_file_exists (temp_file)) {
			show_mess = g_strdup_printf (_("File %s in 'mixer command' does not exist.\nDo you want to use it anyway?"), temp_file);
			mess = gnome_message_box_new (show_mess,
						      GNOME_MESSAGE_BOX_QUESTION,
						      GNOME_STOCK_BUTTON_YES,
						      GNOME_STOCK_BUTTON_NO,
						      NULL);
			g_free (show_mess);
			choice = gnome_dialog_run (GNOME_DIALOG (mess));
			if (choice == 0)
				mixer_command = g_strdup (temp_file);
			else
				gtk_entry_set_text (GTK_ENTRY (gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (propertywidgets.Mixer_fileentry_v))), mixer_command);
		}
		else
			mixer_command = g_strdup (temp_file);
	}
	else
		mixer_command = g_strdup (temp_file);
	
	g_free (temp_file);
	temp_file = g_strdup (gtk_entry_get_text (GTK_ENTRY (gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (propertywidgets.TempDir_fileentry_v)))));

	if (!g_strcasecmp (temp_file, "")) {
		show_mess = g_strdup_printf (_("You havn't entered a temp dir. You will not be able to record.\nDo you want to use it anyway?"));
		mess = gnome_message_box_new (show_mess,
					      GNOME_MESSAGE_BOX_QUESTION,
					      GNOME_STOCK_BUTTON_YES,
					      GNOME_STOCK_BUTTON_NO,
					      NULL);
		g_free (show_mess);
		choice = gnome_dialog_run (GNOME_DIALOG (mess));
		if (choice == 0) 
			temp_dir = g_strdup (temp_file);
		else
			gtk_entry_set_text (GTK_ENTRY (gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (propertywidgets.TempDir_fileentry_v))), temp_dir);
		tempdir = TRUE;
	}

	if (!tempdir) {
		/* Check if the given temp directory exists -------------------- */
		if (!stat (temp_file, &file_info)) {         /* Exists */
			if (!S_ISDIR (file_info.st_mode)) {   /* Not a directory */
				show_mess = g_strdup_printf (_("Temp directory %s is a file.\nDo you want to use it anyway?"), temp_file);
				mess = gnome_message_box_new (show_mess,
							      GNOME_MESSAGE_BOX_QUESTION,
							      GNOME_STOCK_BUTTON_YES,
							      GNOME_STOCK_BUTTON_NO,
							      NULL);
				g_free (show_mess);
				choice = gnome_dialog_run (GNOME_DIALOG (mess));
				if (choice == 0)
					temp_dir = g_strdup (temp_file);
				else
					gtk_entry_set_text (GTK_ENTRY (gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (propertywidgets.TempDir_fileentry_v))), temp_dir);
			}
			else  { /* Exists and is a directory */
				gint ok;

				/* Check if you have rw permissions */
				ok = access (temp_file, W_OK | R_OK);

				if (ok == -1) {
					show_mess = g_strdup_printf (_("You don't have the correct permissions (read & write) for temp directory %s.\nDo you want to use it anyway?"), temp_file);
					mess = gnome_message_box_new (show_mess,
								      GNOME_MESSAGE_BOX_QUESTION,
								      GNOME_STOCK_BUTTON_YES,
								      GNOME_STOCK_BUTTON_NO,
								      NULL);
					g_free (show_mess);
					choice = gnome_dialog_run (GNOME_DIALOG (mess));
					if (choice == 0)
						temp_dir = g_strdup (temp_file);
					else
						gtk_entry_set_text (GTK_ENTRY (gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (propertywidgets.TempDir_fileentry_v))), temp_dir);
				}
				else
					temp_dir = g_strdup (temp_file);
			}
		}
		else if (errno == ENOENT) {                  /* Does not exist */
			show_mess = g_strdup_printf (_("Directory %s does not exist.\nDo want to use it anyway?"), temp_file);
			mess = gnome_message_box_new (show_mess,
						      GNOME_MESSAGE_BOX_QUESTION,
						      GNOME_STOCK_BUTTON_YES,
						      GNOME_STOCK_BUTTON_NO,
						      NULL);
			g_free (show_mess);
			choice = gnome_dialog_run (GNOME_DIALOG (mess));
			if (choice == 0)
				temp_dir = g_strdup (temp_file);
			else
				gtk_entry_set_text (GTK_ENTRY (gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (propertywidgets.TempDir_fileentry_v))), temp_dir);
		}
		else
			temp_dir = g_strdup (temp_file);
	}

	g_free (temp_file);

	if (g_strcasecmp (gtk_entry_get_text (GTK_ENTRY (propertywidgets.Audioformat_combo_entry_v)), "16bit pcm"))
		audioformat = TRUE;
	else
		audioformat = FALSE;
	samplerate  = g_strdup (gtk_entry_get_text (GTK_ENTRY (propertywidgets.Samplerate_combo_entry_v)));
	if (g_strcasecmp (gtk_entry_get_text (GTK_ENTRY (propertywidgets.NrChannel_combo_entry_v)), "stereo"))
		channels = TRUE;
	else
		channels = FALSE;
  
	show_time      = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (propertywidgets.show_time_checkbutton_v));
	show_soundinfo = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (propertywidgets.show_soundinfo_checkbutton_v));
	
	if (!audioformat)
		audioformat_string = g_strdup ("16bit pcm");
	else
		audioformat_string = g_strdup ("8bit pcm");

	if (!channels)
		channels_string = g_strdup ("stereo");
	else
		channels_string = g_strdup ("mono");

	/* Update some of the new information on the mainwindow */
	if (active_file == NULL) {
		gchar* temp_string = g_strconcat (_("Audioformat: "), audioformat_string, NULL);
		gtk_label_set_text (GTK_LABEL (grecord_widgets.audio_format_label), temp_string);
		g_free (temp_string);

		temp_string = g_strconcat (_("Sample rate: "), samplerate, NULL);
		gtk_label_set_text (GTK_LABEL (grecord_widgets.sample_rate_label), temp_string);
		g_free (temp_string);
		
		temp_string = g_strconcat (_("Channels: "), channels_string, NULL);
		gtk_label_set_text (GTK_LABEL (grecord_widgets.nr_of_channels_label), temp_string);
		g_free (temp_string);
	}

	if (show_time) {
		gtk_widget_show (GTK_WIDGET (grecord_widgets.timespace_label));
		gtk_widget_show (GTK_WIDGET (grecord_widgets.timemin_label));
		gtk_widget_show (GTK_WIDGET (grecord_widgets.timesec_label));
	}
	else {
		gtk_widget_hide (GTK_WIDGET (grecord_widgets.timespace_label));
		gtk_widget_hide (GTK_WIDGET (grecord_widgets.timemin_label));
		gtk_widget_hide (GTK_WIDGET (grecord_widgets.timesec_label));
	}

	if (show_soundinfo) {
		gtk_widget_show (GTK_WIDGET (grecord_widgets.audio_format_label));
		gtk_widget_show (GTK_WIDGET (grecord_widgets.sample_rate_label));
		gtk_widget_show (GTK_WIDGET (grecord_widgets.nr_of_channels_label));
	}
	else {
		gtk_widget_hide (GTK_WIDGET (grecord_widgets.audio_format_label));
		gtk_widget_hide (GTK_WIDGET (grecord_widgets.sample_rate_label));
		gtk_widget_hide (GTK_WIDGET (grecord_widgets.nr_of_channels_label));
	}

	g_free (audioformat_string);
	g_free (channels_string);

	/* Save the configuration */
	save_config_file ();
}

