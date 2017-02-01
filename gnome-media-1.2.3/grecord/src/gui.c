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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <gnome.h>

#include "grec.h"
#include "gui.h"
#include "preferences.h"
#include "sound.h"
#include "prog.h"

#include "../pixmaps/gnome_mixer.xpm"

static GnomeUIInfo arkiv1_menu_uiinfo[] =
{
	GNOMEUIINFO_MENU_NEW_ITEM (N_("_New"),
				   N_("Create a new sample"),
				   on_new_activate_cb, NULL),
	GNOMEUIINFO_MENU_OPEN_ITEM (on_open_activate_cb, NULL),
	GNOMEUIINFO_MENU_SAVE_ITEM (on_save_activate_cb, NULL),
	GNOMEUIINFO_MENU_SAVE_AS_ITEM (on_saveas_activate_cb, NULL),
	GNOMEUIINFO_SEPARATOR,
	{
		GNOME_APP_UI_ITEM, N_("Run mixer..."),
		N_("Run GNOME mixer"),
		on_runmixer_activate_cb, NULL, NULL,
		GNOME_APP_PIXMAP_DATA, gnome_mixer_xpm,
		0, 0, NULL
	},
	GNOMEUIINFO_SEPARATOR,
	GNOMEUIINFO_MENU_EXIT_ITEM (on_exit_activate_cb, NULL),
	GNOMEUIINFO_END
};

static GnomeUIInfo echo_effect_menu_uiinfo [] =
{
	{
		GNOME_APP_UI_ITEM, N_("Add echo"),
		N_("Add echo to the current sample"),
		on_add_echo_activate_cb, NULL, NULL,
		GNOME_APP_PIXMAP_NONE, NULL,
		0, 0, NULL,
	},
	GNOMEUIINFO_END
};

/*
  static GnomeUIInfo speed_effect_menu_uiinfo [] =
  {
  {
  GNOME_APP_UI_ITEM, N_("Increase speed"),
  N_("Increase the speed of the current sample"),
  on_increase_speed_activate_cb, NULL, NULL,
  GNOME_APP_PIXMAP_NONE, NULL,
  0, 0, NULL,
  },
  {
  GNOME_APP_UI_ITEM, N_("Decrease speed"),
  N_("Decrease the speed of the current sample"),
  on_decrease_speed_activate_cb, NULL, NULL,
  GNOME_APP_PIXMAP_NONE, NULL,
  0, 0, NULL,
  },
  GNOMEUIINFO_END
  };
*/

static GnomeUIInfo effects_menu_uiinfo [] =
{
	GNOMEUIINFO_SUBTREE (N_("Echo"), echo_effect_menu_uiinfo),
	/* GNOMEUIINFO_SUBTREE (N_("Speed"), speed_effect_menu_uiinfo), */
	GNOMEUIINFO_END
};

static GnomeUIInfo edit_menu_uiinfo[] =
{
	/* GNOMEUIINFO_MENU_UNDO_ITEM (on_undo_activate_cb, NULL), */
	{
		GNOME_APP_UI_ITEM, N_("Undo all changes"),
		N_("Undo all changes made on the current sample"),
		on_undoall_activate_cb, NULL, NULL,
		GNOME_APP_PIXMAP_STOCK, GNOME_STOCK_MENU_UNDO,
		0, 0, NULL,
	},
	/* GNOMEUIINFO_MENU_REDO_ITEM (on_redo_activate_cb, NULL), */
	GNOMEUIINFO_SEPARATOR,
	GNOMEUIINFO_SUBTREE (N_("Effects"), effects_menu_uiinfo),
	GNOMEUIINFO_END
};

static GnomeUIInfo alternativ1_menu_uiinfo[] =
{
	GNOMEUIINFO_MENU_PREFERENCES_ITEM (on_preferences_activate_cb, NULL),
	GNOMEUIINFO_SEPARATOR,
	{
		GNOME_APP_UI_TOGGLEITEM, N_("Show time"),
		N_("Show time"),
		on_show_time_activate_cb, NULL, NULL,
		GNOME_APP_PIXMAP_NONE, NULL,
		0, 0, NULL,
	},
	{
		GNOME_APP_UI_TOGGLEITEM, N_("Show soundinfo"),
		N_("Show sound information"),
		on_show_soundinfo_activate_cb, NULL, NULL,
		GNOME_APP_PIXMAP_NONE, NULL,
		0, 0, NULL,
	},
	GNOMEUIINFO_END
};

static GnomeUIInfo hj_lp1_menu_uiinfo[] =
{
	/* GNOMEUIINFO_HELP ("grecord"), */
	GNOMEUIINFO_MENU_ABOUT_ITEM (on_about_activate_cb, NULL),
	GNOMEUIINFO_END
};

GnomeUIInfo menubar1_uiinfo[] =
{
	GNOMEUIINFO_MENU_FILE_TREE (arkiv1_menu_uiinfo),
	GNOMEUIINFO_MENU_EDIT_TREE (edit_menu_uiinfo),
	GNOMEUIINFO_MENU_SETTINGS_TREE (alternativ1_menu_uiinfo),
	GNOMEUIINFO_MENU_HELP_TREE (hj_lp1_menu_uiinfo),
	GNOMEUIINFO_END
};

gpointer main_menu = menubar1_uiinfo;

GtkWidget*
create_grecord_window (void)
{
	GtkWidget* vbox1;
	GtkWidget* menubar1;
	GtkWidget* toolbar1;
	GtkWidget* tmp_toolbar_icon;
	GtkWidget* vbox2;
	GtkWidget* vbox3;
	GtkWidget* frame1;
	GtkWidget* vbox4;
	GtkWidget* hbox1;
	GtkWidget* grecord_appbar;
	GtkWidget* mess;
	GtkWidget* info_hbox;
	GtkWidget* New_button;
	GtkWidget* Record_button;
	GtkWidget* Play_button;
	GtkWidget* Stop_button;
	GtkWidget* Exit_button;
	GtkWidget* grecord_window;
	GtkWidget* Statusbar;
	GtkWidget* audio_format_label;
	GtkWidget* sample_rate_label;
	GtkWidget* nr_of_channels_label;
	
	GtkWidget* timespace_label;
	GtkWidget* timesec_label;
	GtkWidget* timemin_label
;
	gboolean found_file = FALSE;
	gboolean unsupported_soundfile = FALSE;
	gchar* audioformat_string = NULL;
	gchar* channels_string = NULL;
	gchar* temp_string = NULL;

	grecord_window = gnome_app_new ("grecord", "grecord");

	if (!audioformat)
		audioformat_string = g_strdup ("16bit pcm");
	else
		audioformat_string = g_strdup ("8bit pcm");

	if (!channels)
		channels_string = g_strdup ("stereo");
	else
		channels_string = g_strdup ("mono");

	/* Set the size of the main window */
	gtk_widget_set_uposition (GTK_WIDGET (grecord_window), mwin.x, mwin.y);
	gtk_window_set_default_size (GTK_WINDOW (grecord_window), mwin.width, mwin.height); 

	menubar1 = gtk_menu_bar_new ();
	gtk_widget_show (menubar1);
	gnome_app_fill_menu (GTK_MENU_SHELL (menubar1), menubar1_uiinfo,
			     NULL, FALSE, 0);

	if (show_time) {
		GtkCheckMenuItem* menu_item = GTK_CHECK_MENU_ITEM (alternativ1_menu_uiinfo[2].widget);
		menu_item->active = TRUE;
	}
	if (show_soundinfo) {
		GtkCheckMenuItem* menu_item = GTK_CHECK_MENU_ITEM (alternativ1_menu_uiinfo[3].widget);
		menu_item->active = TRUE;
	}

	toolbar1 = gtk_toolbar_new (GTK_ORIENTATION_HORIZONTAL, GTK_TOOLBAR_BOTH);
	gtk_widget_show (toolbar1);
	gtk_toolbar_set_space_size (GTK_TOOLBAR (toolbar1), 16);
	gtk_toolbar_set_space_style (GTK_TOOLBAR (toolbar1), GTK_TOOLBAR_SPACE_LINE);
	gtk_toolbar_set_button_relief (GTK_TOOLBAR (toolbar1), GTK_RELIEF_NONE);

	tmp_toolbar_icon = gnome_stock_pixmap_widget (grecord_window, GNOME_STOCK_PIXMAP_NEW);
	New_button = gtk_toolbar_append_element (GTK_TOOLBAR (toolbar1),
						 GTK_TOOLBAR_CHILD_BUTTON,
						 NULL,
						 _("New"),
						 _("Create new sample"), NULL,
						 tmp_toolbar_icon, NULL, NULL);
	gtk_widget_show (New_button);

	gtk_toolbar_append_space (GTK_TOOLBAR (toolbar1));

	tmp_toolbar_icon = gnome_stock_pixmap_widget (grecord_window, GNOME_STOCK_PIXMAP_FORWARD);
	Play_button = gtk_toolbar_append_element (GTK_TOOLBAR (toolbar1),
						  GTK_TOOLBAR_CHILD_BUTTON,
						  NULL,
						  _("Play"),
						  _("Play current sample"), NULL,
						  tmp_toolbar_icon, NULL, NULL);
	gtk_widget_show (Play_button);

	tmp_toolbar_icon = gnome_stock_pixmap_widget (grecord_window, GNOME_STOCK_PIXMAP_STOP);
	Stop_button = gtk_toolbar_append_element (GTK_TOOLBAR (toolbar1),
						  GTK_TOOLBAR_CHILD_BUTTON,
						  NULL,
						  _("Stop"),
						  _("Stop playing/recording"), NULL,
						  tmp_toolbar_icon, NULL, NULL);
	gtk_widget_show (Stop_button);

	tmp_toolbar_icon = gnome_stock_pixmap_widget (grecord_window, GNOME_STOCK_PIXMAP_MIC);
	Record_button = gtk_toolbar_append_element (GTK_TOOLBAR (toolbar1),
						    GTK_TOOLBAR_CHILD_BUTTON,
						    NULL,
						    _("Record"),
						    _("Start recording"), NULL,
						    tmp_toolbar_icon, NULL, NULL);
	gtk_widget_show (Record_button);
      
	gtk_toolbar_append_space (GTK_TOOLBAR (toolbar1));

	tmp_toolbar_icon = gnome_stock_pixmap_widget (grecord_window, GNOME_STOCK_PIXMAP_EXIT);
	Exit_button = gtk_toolbar_append_element (GTK_TOOLBAR (toolbar1),
						  GTK_TOOLBAR_CHILD_BUTTON,
						  NULL,
						  _("Exit"),
						  _("Exit this program"), NULL,
						  tmp_toolbar_icon, NULL, NULL);
	
	vbox1 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox1);

	vbox2 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox2);
	gtk_box_pack_start (GTK_BOX (vbox1), vbox2, TRUE, TRUE, 0);
	
	vbox3 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox3);
	gtk_box_pack_start (GTK_BOX (vbox2), vbox3, TRUE, TRUE, 0);
	
	frame1 = gtk_frame_new (_("Info"));
	gtk_widget_show (frame1);
	gtk_box_pack_start (GTK_BOX (vbox3), frame1, FALSE, TRUE, 0);
	gtk_widget_set_usize (frame1, -2, 70);
	gtk_container_set_border_width (GTK_CONTAINER (frame1), 10);
	
	vbox4 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox4);
	gtk_container_add (GTK_CONTAINER (frame1), vbox4);
	
	hbox1 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox1);
	gtk_box_pack_start (GTK_BOX (vbox4), hbox1, TRUE, TRUE, 0);

	Statusbar = gtk_hscale_new (GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 1000, 1, 1, 0)));
	gtk_widget_show (Statusbar);
	gtk_box_pack_start (GTK_BOX (hbox1), Statusbar, TRUE, TRUE, 0);
	gtk_widget_set_usize (Statusbar, -2, 0);
	gtk_scale_set_draw_value (GTK_SCALE (Statusbar), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET (Statusbar), FALSE);

	timemin_label = gtk_label_new ("00");
	gtk_box_pack_start (GTK_BOX (hbox1), timemin_label, FALSE, TRUE, 4);

	timespace_label = gtk_label_new (":");
	gtk_box_pack_start (GTK_BOX (hbox1), timespace_label, FALSE, TRUE, 0);
	
	timesec_label = gtk_label_new ("00");
	gtk_box_pack_start (GTK_BOX (hbox1), timesec_label, FALSE, TRUE, 4);

	if (show_time) {
		gtk_widget_show (timemin_label);
		gtk_widget_show (timespace_label);
		gtk_widget_show (timesec_label);
	}

	info_hbox = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (info_hbox);
	gtk_box_pack_start (GTK_BOX (vbox4), info_hbox, TRUE, TRUE, 0);

	temp_string = g_strconcat (_("Audioformat: "), audioformat_string, NULL);
	audio_format_label = gtk_label_new (temp_string);
	g_free (temp_string);
	gtk_box_pack_start (GTK_BOX (info_hbox), audio_format_label, TRUE, FALSE, 0);
	gtk_label_set_justify (GTK_LABEL (audio_format_label), GTK_JUSTIFY_LEFT);
	gtk_label_set_line_wrap (GTK_LABEL (audio_format_label), TRUE);

	temp_string = g_strconcat (_("Sample rate: "), samplerate, NULL);
	sample_rate_label = gtk_label_new (temp_string);
	g_free (temp_string);
	gtk_box_pack_start (GTK_BOX (info_hbox), sample_rate_label, TRUE, FALSE, 0);
	gtk_label_set_justify (GTK_LABEL (sample_rate_label), GTK_JUSTIFY_LEFT);
	gtk_label_set_line_wrap (GTK_LABEL (sample_rate_label), TRUE);

	temp_string = g_strconcat (_("Channels: "), channels_string, NULL);
	nr_of_channels_label = gtk_label_new (temp_string);
	g_free (temp_string);
	gtk_box_pack_start (GTK_BOX (info_hbox), nr_of_channels_label, TRUE, FALSE, 0);
	gtk_label_set_justify (GTK_LABEL (nr_of_channels_label), GTK_JUSTIFY_LEFT);
	gtk_label_set_line_wrap (GTK_LABEL (nr_of_channels_label), TRUE);

	if (show_soundinfo) {
		gtk_widget_show (audio_format_label);
		gtk_widget_show (sample_rate_label);
		gtk_widget_show (nr_of_channels_label);
	}

	grecord_appbar = gnome_appbar_new (FALSE, TRUE, GNOME_PREFERENCES_NEVER);
	gtk_widget_show (grecord_appbar);	

	/* Initialize structure */
	grecord_widgets.New_button = New_button;
	grecord_widgets.Record_button = Record_button;
	grecord_widgets.Play_button = Play_button;
	grecord_widgets.Stop_button = Stop_button;
	grecord_widgets.Exit_button = Exit_button;
	grecord_widgets.grecord_window = grecord_window;
	grecord_widgets.Statusbar = Statusbar;
	grecord_widgets.appbar = grecord_appbar;
	grecord_widgets.audio_format_label = audio_format_label;
	grecord_widgets.sample_rate_label = sample_rate_label;
	grecord_widgets.nr_of_channels_label = nr_of_channels_label;
	grecord_widgets.timespace_label = timespace_label;
	grecord_widgets.timesec_label = timesec_label;
	grecord_widgets.timemin_label = timemin_label;

	/* Gnome stuff */
	gnome_app_set_statusbar (GNOME_APP (grecord_window),  GTK_WIDGET (grecord_appbar));
	gnome_app_set_menus (GNOME_APP (grecord_window), GTK_MENU_BAR (menubar1));
	gnome_app_set_toolbar (GNOME_APP (grecord_window), GTK_TOOLBAR (toolbar1));
	gnome_app_set_contents (GNOME_APP (grecord_window), GTK_WIDGET (vbox1));

	gnome_app_install_menu_hints (GNOME_APP (grecord_window), menubar1_uiinfo);
	
	/* Initiate mainwindow and set the topic */
	is_file_default ();
	found_file = g_file_exists (active_file);

	if (!found_file && !default_file) {
		gchar* show_mess = g_strdup_printf (_("File '%s' doesn't exist; using default."), active_file);
		mess = gnome_message_box_new (show_mess,
					      GNOME_MESSAGE_BOX_WARNING,
					      GNOME_STOCK_BUTTON_OK,
					      NULL);
		g_free (show_mess);
		gtk_window_set_modal (GTK_WINDOW (mess), TRUE);
		gtk_widget_show (mess);

		grecord_set_sensitive_nofile ();
	}
	else if (!found_file && default_file) {
		gtk_widget_set_sensitive (GTK_WIDGET (Play_button), FALSE);
		gtk_widget_set_sensitive (GTK_WIDGET (Stop_button), FALSE);
	}
	else if (found_file && !default_file) {
		if (!soundfile_supported (active_file))
			unsupported_soundfile = TRUE;
	}
	
	if ((found_file || default_file) && unsupported_soundfile) {
		gchar* show_mess = g_strdup_printf (_("File '%s' isn't a supported soundfile."), active_file);
		mess = gnome_message_box_new (show_mess,
					      GNOME_MESSAGE_BOX_WARNING,
					      GNOME_STOCK_BUTTON_OK,
					      NULL);
		g_free (show_mess);
		gtk_window_set_modal (GTK_WINDOW (mess), TRUE);
		gtk_widget_show (mess);
		
		grecord_set_sensitive_nofile ();
	}

	set_min_sec_time (get_play_time (active_file), TRUE);
 
	/* Setup some callbacks */
	gtk_signal_connect (GTK_OBJECT (grecord_window), "delete_event", GTK_SIGNAL_FUNC (on_exit_activate_cb), NULL);
	gtk_signal_connect (GTK_OBJECT (New_button), "clicked", GTK_SIGNAL_FUNC (on_new_activate_cb), NULL);
	gtk_signal_connect (GTK_OBJECT (Record_button), "clicked", GTK_SIGNAL_FUNC (on_record_activate_cb), NULL);
	gtk_signal_connect (GTK_OBJECT (Play_button), "clicked", GTK_SIGNAL_FUNC (on_play_activate_cb), NULL);
	gtk_signal_connect (GTK_OBJECT (Stop_button), "clicked", GTK_SIGNAL_FUNC (on_stop_activate_cb), NULL);
	gtk_signal_connect (GTK_OBJECT (Exit_button), "clicked", GTK_SIGNAL_FUNC (on_exit_activate_cb), NULL);
	
	return grecord_window;
}

GtkWidget*
create_about (void)
{
	const gchar *authors[] = {
		/* if your charset allows it, replace the "e" of "Hyden"
		 *  by an "eacute" (U00E9) */
		N_("Andreas Hyden <a.hyden@cyberpoint.se>"),
		NULL
	};
	GtkWidget* about;
	
#ifdef ENABLE_NLS
	authors[0]=_(authors[0]);
#endif
	about = gnome_about_new (_("GNOME Sound recorder"), VERSION,
				/* if your charset allows it, replace the
				   "e" of Hyden by an "eacute" (U00E9) */
				 _("Copyright (C)  2000 Andreas Hyden"),
				 authors,
				 _("A simple soundrecorder and soundplayer for GNOME.\nDedicated to my cat, Malte."),
				 NULL);
	
	return about;
}

GtkWidget*
create_grecord_propertybox (void)
{
	GtkWidget* grecord_propertybox;
	GtkWidget* notebook1;
	GtkWidget* vbox4;
	GtkWidget* frame3;
	GtkWidget* vbox6;
	GtkWidget* hbox4;
	GtkWidget* label6;
	GtkObject* spinbutton_adj;
	GtkWidget* RecordTimeout_spinbutton;
	GtkWidget* label7;
	GtkWidget* StopRecordOnTimeout_checkbox;
	GtkWidget* PopupSaveOnTimeout_checkbox;
	GtkWidget* vbox5;
	GtkWidget* frame4;
	GtkWidget* vbox7;
	GtkWidget* PopupWarnMessSize_checkbox;
	GtkWidget* WarningSize_spinbutton;
	GtkWidget* StopRecordSize_checkbox;
	GtkWidget* StopRecordSize_spinbutton;
	GtkWidget* label1;
	GtkWidget* vbox1;
	GtkWidget* frame1;
	GtkWidget* vbox2;
	GtkWidget* label3;
	GtkWidget* frame2;
	GtkWidget* vbox3;
	GtkWidget* hbox3;
	GtkWidget* hbox5;
	GtkWidget* label8;
	GtkWidget* Mixer_fileentry;
	GtkWidget* Mixer_combo_entry;
	GtkWidget* label5;
	GtkWidget* TempDir_fileentry;
	GtkWidget* TempDir_combo_entry;
	GtkWidget* label2;
	GtkWidget* svbox1;
	GtkWidget* sframe1;
	GtkWidget* svbox2;
	GtkWidget* shbox1;
	GtkWidget* Audioformat_label;
	GtkWidget* Audioformat_combo;
	GList* Audioformat_combo_items = NULL;
	GtkWidget* Audioformat_combo_entry;
	GtkWidget* shbox3;
	GtkWidget* Samplerate_label;
	GtkWidget* Samplerate_combo;
	GList* Samplerate_combo_items = NULL;
	GtkWidget* Samplerate_combo_entry;
	GtkWidget* shbox4;
	GtkWidget* NrChannels_label;
	GtkWidget* NrChannel_combo;
	GList* NrChannel_combo_items = NULL;
	GtkWidget* NrChannel_combo_entry;
	GtkWidget* path_to_sox_label;
	GtkWidget* path_to_sox_fileentry;
	GtkWidget* path_to_sox_combo_entry;
	GtkWidget* path_to_sox_hbox;
	GtkWidget* gui_vbox;
	GtkWidget* gui_label;
	GtkWidget* mainwindow_gui_frame;
	GtkWidget* mainwindow_gui_vbox;
	GtkWidget* show_time_checkbutton;
	GtkWidget* show_soundinfo_checkbutton;
	GtkWidget* playing_vbox;
	GtkWidget* playing_label;
	GtkWidget* playrepeat_frame;
	GtkWidget* playrepeat_vbox;
	GtkWidget* playrepeat_checkbox;
	GtkWidget* playrepeatforever_radiobutton;
	GtkWidget* playxtimes_hbox;
	GtkWidget* playxtimes_radiobutton;
	GtkWidget* playxtimes_spinbutton;

	grecord_propertybox = gnome_property_box_new ();	

	notebook1 = GNOME_PROPERTY_BOX (grecord_propertybox)->notebook;
	gtk_widget_show (notebook1);
	
	vbox4 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox4);
	gtk_container_add (GTK_CONTAINER (notebook1), vbox4);
	
	frame3 = gtk_frame_new (_("Time"));
	gtk_widget_show (frame3);
	gtk_box_pack_start (GTK_BOX (vbox4), frame3, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (frame3), 3);

	vbox6 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox6);
	gtk_container_add (GTK_CONTAINER (frame3), vbox6);
	
	hbox4 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox4);
	gtk_box_pack_start (GTK_BOX (vbox6), hbox4, TRUE, TRUE, 0);
	
	label6 = gtk_label_new (_("Recording timeout: "));
	gtk_widget_show (label6);
	gtk_box_pack_start (GTK_BOX (hbox4), label6, FALSE, FALSE, 0);
	gtk_misc_set_padding (GTK_MISC (label6), 5, 0);
	
	spinbutton_adj = gtk_adjustment_new (1, 0, 100, 1, 10, 10);
	RecordTimeout_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (spinbutton_adj), 1, 0);
	gtk_widget_show (RecordTimeout_spinbutton);
	gtk_container_add (GTK_CONTAINER (hbox4), RecordTimeout_spinbutton);
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (RecordTimeout_spinbutton), TRUE);
	
	label7 = gtk_label_new (_("minutes"));
	gtk_widget_show (label7);
	gtk_box_pack_start (GTK_BOX (hbox4), label7, FALSE, FALSE, 0);
	gtk_misc_set_padding (GTK_MISC (label7), 5, 0);
	
	StopRecordOnTimeout_checkbox = gtk_check_button_new_with_label (_("Stop recording on timeout"));
	gtk_widget_show (StopRecordOnTimeout_checkbox);
	gtk_box_pack_start (GTK_BOX (vbox6), StopRecordOnTimeout_checkbox, FALSE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (StopRecordOnTimeout_checkbox), 3);
	
	PopupSaveOnTimeout_checkbox = gtk_check_button_new_with_label (_("Popup save dialog when recording is finished"));
	gtk_widget_show (PopupSaveOnTimeout_checkbox);
	gtk_box_pack_start (GTK_BOX (vbox6), PopupSaveOnTimeout_checkbox, FALSE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (PopupSaveOnTimeout_checkbox), 3);
	
	vbox5 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox5);
	gtk_box_pack_start (GTK_BOX (vbox4), vbox5, TRUE, TRUE, 0);
	
	frame4 = gtk_frame_new (_("Size"));
	gtk_widget_show (frame4);
	gtk_box_pack_start (GTK_BOX (vbox5), frame4, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (frame4), 3);
	
	vbox7 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox7);
	gtk_container_add (GTK_CONTAINER (frame4), vbox7);
	
	PopupWarnMessSize_checkbox = gtk_check_button_new_with_label (_("Popup warning message if size (Mb) of sample becomes bigger than:"));
	gtk_widget_show (PopupWarnMessSize_checkbox);
	gtk_box_pack_start (GTK_BOX (vbox7), PopupWarnMessSize_checkbox, FALSE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (PopupWarnMessSize_checkbox), 3);

	spinbutton_adj = gtk_adjustment_new (1, 0, 1000, 1, 10, 10);
	WarningSize_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (spinbutton_adj), 1, 0);
	gtk_widget_show (WarningSize_spinbutton);
	gtk_container_add (GTK_CONTAINER (vbox7), WarningSize_spinbutton);
	
	StopRecordSize_checkbox = gtk_check_button_new_with_label (_("Stop recording if size (Mb) of sample becomes bigger than:"));
	gtk_widget_show (StopRecordSize_checkbox);
	gtk_box_pack_start (GTK_BOX (vbox7), StopRecordSize_checkbox, FALSE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (StopRecordSize_checkbox), 3);

	spinbutton_adj = gtk_adjustment_new (1, 0, 1000, 1, 10, 10);
	StopRecordSize_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (spinbutton_adj), 1, 0);
	gtk_widget_show (StopRecordSize_spinbutton);
	gtk_container_add (GTK_CONTAINER (vbox7), StopRecordSize_spinbutton);
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (StopRecordSize_spinbutton), TRUE);
	
	label1 = gtk_label_new (_("Recording"));
	gtk_widget_show (label1);
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 0), label1);


	playing_vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (playing_vbox);
	gtk_container_add (GTK_CONTAINER (notebook1), playing_vbox);

	playrepeat_frame = gtk_frame_new (_("Play-repeating"));
	gtk_widget_show (playrepeat_frame);
	gtk_box_pack_start (GTK_BOX (playing_vbox), playrepeat_frame, FALSE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (playrepeat_frame), 3);

	playrepeat_vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (playrepeat_vbox);
	gtk_container_add (GTK_CONTAINER (playrepeat_frame), playrepeat_vbox);

	playrepeat_checkbox = gtk_check_button_new_with_label (_("Repeat"));
	gtk_widget_show (playrepeat_checkbox);
	gtk_box_pack_start (GTK_BOX (playrepeat_vbox), playrepeat_checkbox, FALSE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (playrepeat_checkbox), 3);

	playxtimes_hbox = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (playxtimes_hbox);
	gtk_container_add (GTK_CONTAINER (playrepeat_vbox), playxtimes_hbox);

	playrepeatforever_radiobutton = gtk_radio_button_new_with_label (NULL, _("Forever"));
	gtk_widget_show (playrepeatforever_radiobutton);
	gtk_box_pack_start (GTK_BOX (playxtimes_hbox), playrepeatforever_radiobutton, FALSE, FALSE, 20);
	gtk_container_set_border_width (GTK_CONTAINER (playrepeatforever_radiobutton), 3);

	playxtimes_radiobutton = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (playrepeatforever_radiobutton), _("Nr of times:"));
	gtk_widget_show (playxtimes_radiobutton);
	gtk_box_pack_start (GTK_BOX (playxtimes_hbox), playxtimes_radiobutton, FALSE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (playxtimes_radiobutton), 3);

	spinbutton_adj = gtk_adjustment_new (1, 1, 1000, 1, 10, 10);
	playxtimes_spinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (spinbutton_adj), 1, 0);
	gtk_widget_show (playxtimes_spinbutton);
	gtk_box_pack_start (GTK_BOX (playxtimes_hbox), playxtimes_spinbutton, TRUE, TRUE, 0);
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (playxtimes_spinbutton), TRUE);

	playing_label = gtk_label_new (_("Playing"));
	gtk_widget_show (playing_label);
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 1), playing_label);


	vbox1 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox1);
	gtk_container_add (GTK_CONTAINER (notebook1), vbox1);
     
	frame1 = gtk_frame_new (_("Program files"));
	gtk_widget_show (frame1);
	gtk_box_pack_start (GTK_BOX (vbox1), frame1, FALSE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (frame1), 3);
	
	vbox2 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox2);
	gtk_container_add (GTK_CONTAINER (frame1), vbox2);

	path_to_sox_hbox = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (path_to_sox_hbox);
	gtk_box_pack_start (GTK_BOX (vbox2), path_to_sox_hbox, TRUE, TRUE, 0);

	hbox5 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox5);
	gtk_box_pack_start (GTK_BOX (vbox2), hbox5, TRUE, TRUE, 0);

	path_to_sox_label = gtk_label_new (_("Path to sox:"));
	gtk_widget_show (path_to_sox_label);
	gtk_box_pack_start (GTK_BOX (path_to_sox_hbox), path_to_sox_label, FALSE, TRUE, 0);
	gtk_widget_set_usize (path_to_sox_label, 140, -2);
	gtk_label_set_justify (GTK_LABEL (path_to_sox_label), GTK_JUSTIFY_LEFT);
	gtk_label_set_line_wrap (GTK_LABEL (path_to_sox_label), TRUE);
	gtk_misc_set_padding (GTK_MISC (path_to_sox_label), 5, 0);

	path_to_sox_fileentry = gnome_file_entry_new (NULL, NULL);
	gtk_widget_show (path_to_sox_fileentry);
	gtk_box_pack_start (GTK_BOX (path_to_sox_hbox), path_to_sox_fileentry, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (path_to_sox_fileentry), 7);

	path_to_sox_combo_entry = gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (path_to_sox_fileentry));
	gtk_widget_show (path_to_sox_combo_entry);

	label8 = gtk_label_new (_("Path to mixer:"));
	gtk_widget_show (label8);
	gtk_box_pack_start (GTK_BOX (hbox5),label8, FALSE, TRUE, 0);
	gtk_widget_set_usize (label8, 140, -2);
	gtk_label_set_justify (GTK_LABEL (label8), GTK_JUSTIFY_LEFT);
	gtk_label_set_line_wrap (GTK_LABEL (label8), TRUE);
	gtk_misc_set_padding (GTK_MISC (label8), 5, 0);

	Mixer_fileentry = gnome_file_entry_new (NULL, NULL);
	gtk_widget_show (Mixer_fileentry);
	gtk_box_pack_start (GTK_BOX (hbox5), Mixer_fileentry, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (Mixer_fileentry), 7);

	Mixer_combo_entry = gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (Mixer_fileentry));
	gtk_widget_show (Mixer_combo_entry);
	
	frame2 = gtk_frame_new (_("Directories"));
	gtk_widget_show (frame2);
	gtk_box_pack_start (GTK_BOX (vbox1), frame2, FALSE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (frame2), 3);
	
	vbox3 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox3);
	gtk_container_add (GTK_CONTAINER (frame2), vbox3);
	
	hbox3 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox3);
	gtk_box_pack_start (GTK_BOX (vbox3), hbox3, TRUE, TRUE, 0);
	
	label5 = gtk_label_new (_("Temp dir:"));
	gtk_widget_show (label5);
	gtk_box_pack_start (GTK_BOX (hbox3), label5, FALSE, FALSE, 0);
	gtk_widget_set_usize (label5, 140, -2);
	gtk_label_set_justify (GTK_LABEL (label5), GTK_JUSTIFY_LEFT);
	gtk_label_set_line_wrap (GTK_LABEL (label5), TRUE);
	gtk_misc_set_padding (GTK_MISC (label5), 5, 0);
	
	TempDir_fileentry = gnome_file_entry_new (NULL, NULL);
	gtk_widget_show (TempDir_fileentry);
	gtk_box_pack_start (GTK_BOX (hbox3), TempDir_fileentry, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (TempDir_fileentry), 7);
	
	TempDir_combo_entry = gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (TempDir_fileentry));
	gtk_widget_show (TempDir_combo_entry);
	
	label2 = gtk_label_new (_("Paths"));
	gtk_widget_show (label2);
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 2), label2);

	svbox1 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (svbox1);
	gtk_container_add (GTK_CONTAINER (notebook1), svbox1);
	
	sframe1 = gtk_frame_new (_("Sound options"));
	gtk_widget_show (sframe1);
	gtk_box_pack_start (GTK_BOX (svbox1), sframe1, FALSE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (sframe1), 3);
	
	svbox2 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (svbox2);
	gtk_container_add (GTK_CONTAINER (sframe1), svbox2);
	
	shbox1 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (shbox1);
	gtk_box_pack_start (GTK_BOX (svbox2), shbox1, TRUE, TRUE, 0);

	Audioformat_label = gtk_label_new (_("Audioformat:"));
	gtk_widget_show (Audioformat_label);
	gtk_box_pack_start (GTK_BOX (shbox1), Audioformat_label, FALSE, FALSE, 0);
	gtk_widget_set_usize (Audioformat_label, 100, -2);
	gtk_label_set_justify (GTK_LABEL (Audioformat_label), GTK_JUSTIFY_LEFT);
	gtk_label_set_line_wrap (GTK_LABEL (Audioformat_label), TRUE);
	gtk_misc_set_padding (GTK_MISC (Audioformat_label), 5, 0);
	
	Audioformat_combo = gtk_combo_new ();
	gtk_widget_show (Audioformat_combo);
	gtk_box_pack_start (GTK_BOX (shbox1), Audioformat_combo, FALSE, TRUE, 0);
	Audioformat_combo_items = g_list_append (Audioformat_combo_items, _("8bit pcm"));
	Audioformat_combo_items = g_list_append (Audioformat_combo_items, _("16bit pcm"));
	gtk_combo_set_popdown_strings (GTK_COMBO (Audioformat_combo), Audioformat_combo_items);
	g_list_free (Audioformat_combo_items);
	gtk_container_set_border_width (GTK_CONTAINER (Audioformat_combo), 7);
	
	Audioformat_combo_entry = GTK_COMBO (Audioformat_combo)->entry;
	gtk_widget_show (Audioformat_combo_entry);
	gtk_entry_set_editable (GTK_ENTRY (Audioformat_combo_entry), FALSE);
	if (audioformat)
		gtk_entry_set_text (GTK_ENTRY (Audioformat_combo_entry), _("8bit pcm"));
	else
		gtk_entry_set_text (GTK_ENTRY (Audioformat_combo_entry), _("16bit pcm"));
	
	shbox3 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (shbox3);
	gtk_box_pack_start (GTK_BOX (svbox2), shbox3, TRUE, TRUE, 0);
	
	Samplerate_label = gtk_label_new (_("Sample rate:"));
	gtk_widget_show (Samplerate_label);
	gtk_box_pack_start (GTK_BOX (shbox3), Samplerate_label, FALSE, FALSE, 0);
	gtk_widget_set_usize (Samplerate_label, 100, -2);
	gtk_label_set_justify (GTK_LABEL (Samplerate_label), GTK_JUSTIFY_LEFT);
	gtk_label_set_line_wrap (GTK_LABEL (Samplerate_label), TRUE);
	gtk_misc_set_padding (GTK_MISC (Samplerate_label), 5, 0);
	
	Samplerate_combo = gtk_combo_new ();
	gtk_widget_show (Samplerate_combo);
	gtk_box_pack_start (GTK_BOX (shbox3), Samplerate_combo, FALSE, TRUE, 0);
	Samplerate_combo_items = g_list_append (Samplerate_combo_items, "8000");
	Samplerate_combo_items = g_list_append (Samplerate_combo_items, "11025");
	Samplerate_combo_items = g_list_append (Samplerate_combo_items, "16000");
	Samplerate_combo_items = g_list_append (Samplerate_combo_items, "22050");
	Samplerate_combo_items = g_list_append (Samplerate_combo_items, "32000");
	Samplerate_combo_items = g_list_append (Samplerate_combo_items, "44100");
	Samplerate_combo_items = g_list_append (Samplerate_combo_items, "48000");
	gtk_combo_set_popdown_strings (GTK_COMBO (Samplerate_combo), Samplerate_combo_items);
	g_list_free (Samplerate_combo_items);
	gtk_container_set_border_width (GTK_CONTAINER (Samplerate_combo), 7);
	
	Samplerate_combo_entry = GTK_COMBO (Samplerate_combo)->entry;
	gtk_widget_show (Samplerate_combo_entry);
	gtk_entry_set_text (GTK_ENTRY (Samplerate_combo_entry), _(samplerate));
	
	shbox4 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (shbox4);
	gtk_box_pack_start (GTK_BOX (svbox2), shbox4, TRUE, TRUE, 0);
	
	NrChannels_label = gtk_label_new (_("mono/stereo"));
	gtk_widget_show (NrChannels_label);
	gtk_box_pack_start (GTK_BOX (shbox4), NrChannels_label, FALSE, FALSE, 0);
	gtk_widget_set_usize (NrChannels_label, 100, -2);
	gtk_label_set_justify (GTK_LABEL (NrChannels_label), GTK_JUSTIFY_LEFT);
	gtk_label_set_line_wrap (GTK_LABEL (NrChannels_label), TRUE);
	gtk_misc_set_padding (GTK_MISC (NrChannels_label), 5, 0);
	
	NrChannel_combo = gtk_combo_new ();
	gtk_widget_show (NrChannel_combo);
	gtk_box_pack_start (GTK_BOX (shbox4), NrChannel_combo, FALSE, FALSE, 0);
	NrChannel_combo_items = g_list_append (NrChannel_combo_items, _("mono"));
	NrChannel_combo_items = g_list_append (NrChannel_combo_items, _("stereo"));
	gtk_combo_set_popdown_strings (GTK_COMBO (NrChannel_combo), NrChannel_combo_items);
	g_list_free (NrChannel_combo_items);
	gtk_container_set_border_width (GTK_CONTAINER (NrChannel_combo), 7);
	
	NrChannel_combo_entry = GTK_COMBO (NrChannel_combo)->entry;
	gtk_widget_show (NrChannel_combo_entry);
	gtk_entry_set_editable (GTK_ENTRY (NrChannel_combo_entry), FALSE);
	if (channels)
		gtk_entry_set_text (GTK_ENTRY (NrChannel_combo_entry), _("mono"));
	else
		gtk_entry_set_text (GTK_ENTRY (NrChannel_combo_entry), _("stereo"));
	

	label3 = gtk_label_new (_("Sound"));
	gtk_widget_show (label3);
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 3), label3);

	gui_vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (gui_vbox);
	gtk_container_add (GTK_CONTAINER (notebook1), gui_vbox);

	mainwindow_gui_frame = gtk_frame_new (_("Main window"));
	gtk_widget_show (mainwindow_gui_frame);
	gtk_box_pack_start (GTK_BOX (gui_vbox), mainwindow_gui_frame, FALSE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (mainwindow_gui_frame), 3);

	mainwindow_gui_vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (mainwindow_gui_vbox);
	gtk_container_add (GTK_CONTAINER (mainwindow_gui_frame), mainwindow_gui_vbox);

	show_time_checkbutton = gtk_check_button_new_with_label (_("Show time"));
	gtk_widget_show (show_time_checkbutton);
	gtk_box_pack_start (GTK_BOX (mainwindow_gui_vbox), show_time_checkbutton, FALSE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (show_time_checkbutton), 3);

	show_soundinfo_checkbutton = gtk_check_button_new_with_label (_("Show sound information"));
	gtk_widget_show (show_soundinfo_checkbutton);
	gtk_box_pack_start (GTK_BOX (mainwindow_gui_vbox), show_soundinfo_checkbutton, FALSE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (show_soundinfo_checkbutton), 3);

	gui_label = gtk_label_new (_("User interface"));
	gtk_widget_show (gui_label);
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 4), gui_label);

	/* Define structure propertywidgets ---------------------------------------------------- */
	propertywidgets.RecordTimeout_spinbutton_v = RecordTimeout_spinbutton;
	propertywidgets.StopRecordOnTimeout_checkbox_v = StopRecordOnTimeout_checkbox;
	propertywidgets.PopupSaveOnTimeout_checkbox_v = PopupSaveOnTimeout_checkbox;
	propertywidgets.PopupWarnMessSize_checkbox_v = PopupWarnMessSize_checkbox;
	propertywidgets.WarningSize_spinbutton_v = WarningSize_spinbutton;
	propertywidgets.StopRecordSize_checkbox_v = StopRecordSize_checkbox;
	propertywidgets.StopRecordSize_spinbutton_v = StopRecordSize_spinbutton;

	propertywidgets.playrepeat_checkbox_v = playrepeat_checkbox;
	propertywidgets.playrepeatforever_radiobutton_v = playrepeatforever_radiobutton;
	propertywidgets.playxtimes_radiobutton_v = playxtimes_radiobutton;
	propertywidgets.playxtimes_spinbutton_v = playxtimes_spinbutton;

	propertywidgets.Sox_fileentry_v = path_to_sox_fileentry;
	propertywidgets.Mixer_fileentry_v = Mixer_fileentry;
	propertywidgets.TempDir_fileentry_v = TempDir_fileentry;

	propertywidgets.Audioformat_combo_entry_v = Audioformat_combo_entry;
	propertywidgets.Samplerate_combo_entry_v = Samplerate_combo_entry;
	propertywidgets.NrChannel_combo_entry_v = NrChannel_combo_entry;

	propertywidgets.show_time_checkbutton_v = show_time_checkbutton;
	propertywidgets.show_soundinfo_checkbutton_v = show_soundinfo_checkbutton;

	/* Set default vaules from config file ------------------------------------------------- */
	gtk_entry_set_text (GTK_ENTRY (path_to_sox_combo_entry), sox_command);
	gtk_entry_set_text (GTK_ENTRY (Mixer_combo_entry), mixer_command);
	gtk_entry_set_text (GTK_ENTRY (TempDir_combo_entry), temp_dir);	
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (RecordTimeout_spinbutton), record_timeout);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (WarningSize_spinbutton), popup_warn_mess_v);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (StopRecordSize_spinbutton), stop_record_v);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (playxtimes_spinbutton), playxtimes);

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (StopRecordOnTimeout_checkbox), stop_on_timeout);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (PopupSaveOnTimeout_checkbox), save_when_finished);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (PopupWarnMessSize_checkbox), popup_warn_mess);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (StopRecordSize_checkbox), stop_record);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (show_time_checkbutton), show_time);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (show_soundinfo_checkbutton), show_soundinfo);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (playrepeat_checkbox), playrepeat);

	if (playrepeatforever)
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (playrepeatforever_radiobutton), TRUE);
	else
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (playxtimes_radiobutton), TRUE);

	if (!popup_warn_mess)
		gtk_widget_set_sensitive (WarningSize_spinbutton, FALSE);
	if (!stop_record)
		gtk_widget_set_sensitive (StopRecordSize_spinbutton, FALSE);

	if (!playrepeat) {
		gtk_widget_set_sensitive (playrepeatforever_radiobutton, FALSE);
		gtk_widget_set_sensitive (playxtimes_radiobutton, FALSE);
		gtk_widget_set_sensitive (playxtimes_spinbutton, FALSE);
	}

	if (playrepeatforever)
		gtk_widget_set_sensitive (playxtimes_spinbutton, FALSE);

	/* Callbacks ---------------------------------------------------------------------------- */
	gtk_signal_connect (GTK_OBJECT (RecordTimeout_spinbutton), "changed", GTK_SIGNAL_FUNC (widget_in_propertybox_changed), grecord_propertybox);
	gtk_signal_connect (GTK_OBJECT (StopRecordOnTimeout_checkbox), "clicked", GTK_SIGNAL_FUNC (widget_in_propertybox_changed), grecord_propertybox);
	gtk_signal_connect (GTK_OBJECT (PopupSaveOnTimeout_checkbox), "clicked", GTK_SIGNAL_FUNC (widget_in_propertybox_changed), grecord_propertybox);
	gtk_signal_connect (GTK_OBJECT (PopupWarnMessSize_checkbox), "clicked", GTK_SIGNAL_FUNC (widget_in_propertybox_changed), grecord_propertybox);
	gtk_signal_connect (GTK_OBJECT (PopupWarnMessSize_checkbox), "clicked", GTK_SIGNAL_FUNC (on_checkbox_clicked_activate_cb), WarningSize_spinbutton);
	gtk_signal_connect (GTK_OBJECT (WarningSize_spinbutton), "changed", GTK_SIGNAL_FUNC (widget_in_propertybox_changed), grecord_propertybox);
	gtk_signal_connect (GTK_OBJECT (StopRecordSize_checkbox), "clicked", GTK_SIGNAL_FUNC (widget_in_propertybox_changed), grecord_propertybox);
	gtk_signal_connect (GTK_OBJECT (StopRecordSize_checkbox), "clicked", GTK_SIGNAL_FUNC (on_checkbox_clicked_activate_cb), StopRecordSize_spinbutton);
	gtk_signal_connect (GTK_OBJECT (StopRecordSize_spinbutton), "changed", GTK_SIGNAL_FUNC (widget_in_propertybox_changed), grecord_propertybox);
	gtk_signal_connect (GTK_OBJECT (path_to_sox_combo_entry), "changed", GTK_SIGNAL_FUNC (widget_in_propertybox_changed), grecord_propertybox);
	gtk_signal_connect (GTK_OBJECT (Mixer_combo_entry), "changed", GTK_SIGNAL_FUNC (widget_in_propertybox_changed), grecord_propertybox);
	gtk_signal_connect (GTK_OBJECT (TempDir_combo_entry), "changed", GTK_SIGNAL_FUNC (widget_in_propertybox_changed), grecord_propertybox);
	gtk_signal_connect (GTK_OBJECT (Audioformat_combo_entry), "changed", GTK_SIGNAL_FUNC (widget_in_propertybox_changed), grecord_propertybox);
	gtk_signal_connect (GTK_OBJECT (Samplerate_combo_entry), "changed", GTK_SIGNAL_FUNC (widget_in_propertybox_changed), grecord_propertybox);
	gtk_signal_connect (GTK_OBJECT (NrChannel_combo_entry), "changed", GTK_SIGNAL_FUNC (widget_in_propertybox_changed), grecord_propertybox);
	gtk_signal_connect (GTK_OBJECT (show_time_checkbutton), "clicked", GTK_SIGNAL_FUNC (widget_in_propertybox_changed), grecord_propertybox);
	gtk_signal_connect (GTK_OBJECT (show_soundinfo_checkbutton), "clicked", GTK_SIGNAL_FUNC (widget_in_propertybox_changed), grecord_propertybox);
	gtk_signal_connect (GTK_OBJECT (playrepeat_checkbox), "clicked", GTK_SIGNAL_FUNC (widget_in_propertybox_changed), grecord_propertybox);
	gtk_signal_connect (GTK_OBJECT (playrepeatforever_radiobutton), "toggled", GTK_SIGNAL_FUNC (widget_in_propertybox_changed), grecord_propertybox);
	gtk_signal_connect (GTK_OBJECT (playxtimes_radiobutton), "toggled", GTK_SIGNAL_FUNC (widget_in_propertybox_changed), grecord_propertybox);
	gtk_signal_connect (GTK_OBJECT (playxtimes_spinbutton), "changed", GTK_SIGNAL_FUNC (widget_in_propertybox_changed), grecord_propertybox);
	gtk_signal_connect (GTK_OBJECT (playrepeat_checkbox), "clicked", GTK_SIGNAL_FUNC (on_repeat_activate_cb), NULL);
	gtk_signal_connect (GTK_OBJECT (playxtimes_radiobutton), "toggled", GTK_SIGNAL_FUNC (on_checkbox_clicked_activate_cb), playxtimes_spinbutton);
	
	gtk_signal_connect (GTK_OBJECT (grecord_propertybox), "apply", GTK_SIGNAL_FUNC (on_propertybox_apply_activate), NULL);

	return grecord_propertybox;
}
