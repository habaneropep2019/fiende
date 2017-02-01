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

/*
 * GMIX version, for version-checking the config-file
 */
#define GMIX_VERSION 0x030000

#ifdef ALSA
/* stolen from OSS's soundcard.h */
typedef struct mixer_info
{
  char id[16];
  char name[32];
  int  modify_counter;
  int fillers[10];
} mixer_info;
#endif

/* 
 * All, that is known about a mixer-device
 */
typedef struct device_info {
#ifdef ALSA
	snd_mixer_t *handle;
#endif
	int fd;
	mixer_info info;
	int recsrc;	/* current recording-source(s) */
	int devmask;	/* devices supported by this driver */
	int recmask;	/* devices, that can be a recording-source */
	int stereodevs;	/* devices with stereo abilities */
	int caps;	/* features supported by the mixer */
	int volume_left[32], volume_right[32]; /* volume, mono only left */

	int mute_bitmask; /* which channels are muted */
	int lock_bitmask; /* which channels have the L/R sliders linked together */
	int enabled_bitmask; /* which channels should be visible in the GUI ? */
	GList *channels;
} device_info;

/*
 * All channels, that are visible in the mixer-window
 */
typedef struct channel_info {
	/* general info */
	device_info *device;	/* refferrence back to the device */
	int channel;		/* which channel of that device am I ? */
#ifdef ALSA
	snd_mixer_group_t *mixer_group;
#endif
	/* GUI info */
	char *title; /* or char *titel ? */
	char *user_title;
	char *pixmap;
	/* here are the widgets... */
	GtkObject *left, *right;
	GtkWidget *lock, *rec, *mute;

	int passive; /* avoid recursive calls to event handler */
} channel_info;

struct pixmap {
	char *name;
	const char *pixmap;
};

extern GList *devices;

void help_cb(GtkWidget *widget, gpointer data);
void fill_in_device_guis(GtkWidget *notebook);
void gmix_build_slidernotebook(void);
