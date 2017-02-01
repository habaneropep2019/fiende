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

#include <gnome.h>
#include "grec.h"

struct mainwindow {
	guint x;
	guint y;
	guint height;
	guint width;
} mwin;

struct grecwidgets {
	gpointer New_button;
	gpointer Record_button;
	gpointer Play_button;
	gpointer Stop_button;
	gpointer Exit_button;
	gpointer grecord_window;
	gpointer Statusbar;
	gpointer appbar;
	gpointer audio_format_label;
	gpointer sample_rate_label;
	gpointer nr_of_channels_label;
	
	gpointer timespace_label;
	gpointer timesec_label;
	gpointer timemin_label;
} grecord_widgets;

extern GnomeUIInfo menubar1_uiinfo[];

GtkWidget* create_grecord_window      (void);
GtkWidget* create_about               (void);
GtkWidget* create_grecord_propertybox (void);

