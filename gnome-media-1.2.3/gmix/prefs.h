/*
 * GMIX 3.0
 *
 * Copyright (C) 1998 Jens Ch. Restemeier <jchrr@hrz.uni-bielefeld.de>
 * Config dialog added by Matt Martin <Matt.Martin@ieee.org>, Sept 1999
 * ALSA driver by Brian J. Murrell <gnome-alsa@interlinx.bc.ca> Dec 1999
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

typedef struct {
	gboolean set_mixer_on_start;
	gboolean hide_menu;
	gboolean use_icons;
	gboolean use_labels;
} mixerprefs;

extern mixerprefs prefs;

#define PREFS_PAGE "Preferences Page"
#define PREFS_COPY "Preferences Data Copy"
#define LABELS_PAGE "Labels Page"

// Common options for the table of sound device labels.
#define LABEL_TABLE_OPTS GTK_FILL | GTK_EXPAND | GTK_SHRINK, \
			 GTK_FILL | GTK_EXPAND | GTK_SHRINK, \
			 0, 0

// Data structure used to pass information into the routine that
// builds a single row in the table of labels for a sound device.
typedef struct label_create_args label_create_args_t;
struct label_create_args {
	GSList *entry_list;
	GtkWidget *table;
	gint row;
};


void prefs_make_window(void);
void get_gui_config(void);
void put_gui_config(void);
