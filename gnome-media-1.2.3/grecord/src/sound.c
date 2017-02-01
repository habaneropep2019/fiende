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
#include <audiofile.h>

#include "sound.h"
#include "gui.h"
#include "prog.h"

gdouble
get_play_time (const gchar* filename)
{
	gdouble play_time;
	gboolean nofile = FALSE;

	/* Check if there is an active file or not */
	if (!g_file_exists (filename))
		nofile = TRUE;
	
	if (!nofile) {
		AFframecount framecount;
		AFfilehandle file;
		AFfilesetup setup;
		gint samplerate;
		
		/* Audiofile setup */
		setup = afNewFileSetup ();
		file = afOpenFile (filename, "r", setup);
		
		/* Get some info from the soundfile */
		framecount  = afGetFrameCount (file, AF_DEFAULT_TRACK);
		samplerate  = afGetRate (file, AF_DEFAULT_TRACK);

		/* Play time */
		play_time = (int) framecount / samplerate;
		
		afFreeFileSetup (setup);
	}
	else
		play_time = 0;
       
	  return play_time;
}    

void
set_min_sec_time (gint sec, gboolean set_topic)
{
	gint minutes = 0;
	gint seconds = 0;
	
	gchar* temp_string = NULL;
	gchar* show_mess = NULL;

        /* Time in seconds -> time in seconds & minutes */
	minutes = (int) sec / 60;
	seconds = sec - (minutes * 60);

        /* Show it on the main window */
	if (minutes <= 0)
		temp_string = g_strdup ("00");
	else if (minutes < 10)
		temp_string = g_strdup_printf ("0%i", minutes);
	else
		temp_string = g_strdup_printf ("%i", minutes);

	gtk_label_set_text (GTK_LABEL (grecord_widgets.timemin_label), temp_string);

	if (sec != 0 && set_topic) {
		gchar* temp_string2;
		temp_string2 = g_strdup (strrchr (active_file, '/'));
		temp_string2[0] = ' ';
		show_mess = g_strconcat (_(maintopic), temp_string2, " - ", temp_string, NULL);
		g_free (temp_string2);
	}
	else if (sec == 0 && set_topic) {
		show_mess = g_strconcat (_(maintopic), _(" untitled.wav"),  " - ", "00", NULL);
	}

	g_free (temp_string);

	if (seconds <= 0)
		temp_string = g_strdup ("00");
	else if (seconds < 10)
		temp_string = g_strdup_printf ("0%i", seconds);
	else
		temp_string = g_strdup_printf ("%i", seconds);

	gtk_label_set_text (GTK_LABEL (grecord_widgets.timesec_label), temp_string);

	/* Set topic */
	if (set_topic) {
		gchar* temp;
		temp = g_strconcat (show_mess, ":", temp_string, NULL);
		gtk_window_set_title (GTK_WINDOW (grecord_widgets.grecord_window), temp);
		g_free (temp);
	}
	g_free (temp_string);
	g_free (show_mess);
}

/******************************/
/* ------ Effects ----------- */
/******************************/

void
add_echo (gchar* filename, gboolean overwrite)
{
	static gboolean first_time = TRUE;
	gchar* backup_file;

	gchar* command;
	gchar* command_plus_make_backup;

	backup_file = g_concat_dir_and_file (temp_dir, temp_filename_backup);

	command = g_strconcat (sox_command, " ", backup_file, " ", active_file, " echo 0.8 0.88 60.0 0.4", NULL);
	command_plus_make_backup = g_strconcat ("cp -f ", active_file, " ", backup_file, " ; ", command, NULL);

	/* Make a backup only the first time */
	if (first_time) {
		run_command (command_plus_make_backup, _("Adding echo to sample..."));
		first_time = FALSE;
	}
	else
		run_command (command, _("Adding echo to sample..."));

	g_free (backup_file);
	g_free (command);
	g_free (command_plus_make_backup);
}

void
increase_speed (gchar* filename, gboolean overwrite)
{
}

void
decrease_speed (gchar* filename, gboolean overwrite)
{
}
