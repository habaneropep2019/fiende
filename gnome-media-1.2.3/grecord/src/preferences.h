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

struct gwidgets {
	/* Page 0 --------- */
	gpointer RecordTimeout_spinbutton_v;
	gpointer StopRecordOnTimeout_checkbox_v;
	gpointer PopupSaveOnTimeout_checkbox_v;
	gpointer PopupWarnMessSize_checkbox_v;
	gpointer WarningSize_spinbutton_v;
	gpointer StopRecordSize_checkbox_v;
	gpointer StopRecordSize_spinbutton_v;

	/* Page 1 --------- */
	gpointer playrepeat_checkbox_v;
	gpointer playrepeatforever_radiobutton_v;
	gpointer playxtimes_radiobutton_v;
	gpointer playxtimes_spinbutton_v;

	/* Page 2 --------- */
	gpointer Play_fileentry_v;
	gpointer Sox_fileentry_v;
	gpointer Mixer_fileentry_v;
	gpointer TempDir_fileentry_v;
	
	/* Page 3 --------- */
	gpointer Audioformat_combo_entry_v;
	gpointer Samplerate_combo_entry_v;
	gpointer NrChannel_combo_entry_v;

	/* Page 4 --------- */
	gpointer show_time_checkbutton_v;
	gpointer show_soundinfo_checkbutton_v;

} propertywidgets;

void
load_config_file           (void);

void
save_config_file           (void);

void
free_vars                  (void);

/* Callbacks ----------------------------- */

void widget_in_propertybox_changed (GtkWidget* widget, gpointer propertybox);
void on_checkbox_clicked_activate_cb (GtkWidget* widget, gpointer w);
void on_repeat_activate_cb (GtkWidget* widget, gpointer data);

void on_propertybox_apply_activate (GtkWidget* widget, gint page_num, gpointer widgets);

