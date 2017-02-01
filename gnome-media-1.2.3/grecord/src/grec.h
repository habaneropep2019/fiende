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

/* Callbacks ---------------------------------------------------------------- */
void on_record_activate_cb (GtkWidget* widget, gpointer data);
void on_play_activate_cb (GtkWidget* widget, gpointer data);
void on_stop_activate_cb (GtkWidget* widget, gpointer data);
void on_new_activate_cb (GtkWidget* widget, gpointer data);
void on_open_activate_cb (GtkWidget* widget, gpointer data);
void on_save_activate_cb (GtkWidget* widget, gpointer data);
void on_saveas_activate_cb (GtkWidget* widget, gpointer data);
void on_exit_activate_cb (GtkWidget* widget, gpointer data);
void on_preferences_activate_cb (GtkWidget* widget, gpointer data);
void on_about_activate_cb (GtkWidget* widget, gpointer data);
void on_runmixer_activate_cb (GtkWidget* widget, gpointer data);
void on_increase_speed_activate_cb (GtkWidget* widget, gpointer data);
void on_decrease_speed_activate_cb (GtkWidget* widget, gpointer data);
void on_add_echo_activate_cb (GtkWidget* widget, gpointer data);
void on_show_time_activate_cb (GtkWidget* widget, gpointer data);
void on_show_soundinfo_activate_cb (GtkWidget* widget, gpointer data);
void on_undo_activate_cb (GtkWidget* widget, gpointer data);
void on_redo_activate_cb (GtkWidget* widget, gpointer data);
void on_undoall_activate_cb (GtkWidget* widget, gpointer data);

/* Help functions ----------------------------------------------------------- */
void record_sound (void);
void play_sound (const gchar* filename);
void store_filename (GtkFileSelection* selector, gpointer file_selector);
void save_filename (GtkFileSelection* selector, gpointer file_selector);
gint save_dont_or_cancel (void);
gboolean save_sound_file (const gchar* filename);
guint UpdateStatusbarPlay (gboolean begin);
guint UpdateStatusbarRecord (gboolean begin);
void save_dialog (void);
void is_file_default (void);
void grecord_set_sensitive_file (void);
void grecord_set_sensitive_nofile (void);
void grecord_set_sensitive_progress (void);
void grecord_set_sensitive_loading (void);
void run_command (const gchar* command, const gchar* appbar_comment);
guint check_if_loading_finished (gint pid);
gboolean soundfile_supported (const gchar* filename);
gboolean check_if_sounddevice_ready (void);
