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

#ifndef _PROG_H_
#define _PROG_H_

#include <gnome.h>
#include <config.h>

const extern gchar* maintopic;

extern const gchar* temp_filename_record;
extern const gchar* temp_filename_play;
extern const gchar* temp_filename_backup;
extern const gchar* temp_filename;

extern gint repeat_counter;

extern gchar* active_file;
extern gboolean default_file;
extern gboolean file_changed;

struct PlayEngine
{
	pid_t pid;
	gboolean is_running;
} PlayEng;

struct RecordEngine
{
	pid_t pid;
	gboolean is_running;
} RecEng;

gboolean convert_is_running;

struct EffectEngine
{
	pid_t pid;
	gboolean is_running;

	gint p[2];
	FILE* input;
	gint data_input_id;
} EffectEng;

/* Will be declared from the config file ------- */
gint record_timeout;
gboolean stop_on_timeout;
gboolean save_when_finished;
gboolean popup_warn_mess;
gboolean stop_record;
gint popup_warn_mess_v;
gint stop_record_v;

gboolean playrepeat;
gboolean playrepeatforever;
gint playxtimes;

gchar* sox_command;
gchar* mixer_command;
gchar* temp_dir;

gboolean audioformat;
gchar* samplerate;
gboolean channels;

gboolean show_time;
gboolean show_soundinfo;

#endif
