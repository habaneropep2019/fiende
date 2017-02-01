/*
 * GMIX 3.0
 *
 * A total rewrite of gmix 2.5...
 *
 * This version includes most features of gnome-hello, except DND and SM. That
 * will follow.
 *
 * Supported sound-drivers:
 * - OSS (only lite checked)
 * - ALSA driver >0.4.1f, lib >0.4.1d
 *
 * TODO:
 * - /dev/audio (if the admins install GTK and GNOME on our Sparcs...)
 * - other sound systems
 * - you name it...
 * - Animated ramping of mixer sliders to preset values
 * - Load Settings button, several for presets ?
 * - more configuration
 * - move the sliders in accordance to the information received from
 *   the ALSA driver when another application adjusts a mixer setting
 *   (yes ALSA does this, without having to poll the mixer!!)
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

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/errno.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#ifdef ALSA
#include <sys/asoundlib.h>
#else
#ifdef HAVE_LINUX_SOUNDCARD_H 
#include <linux/soundcard.h>
#else 
#include <machine/soundcard.h>
#endif
#endif

#include <gnome.h>
#include <libgnomeui/gnome-window-icon.h>

#include "gmix.h"
#include "prefs.h"

static void config_cb (GtkWidget *widget, gpointer data);

device_info *open_device(int num);
GList *make_channels(device_info *device);
void scan_devices(void);
void free_one_device(gpointer a, gpointer b);
void free_devices(void);
void init_one_device(gpointer a, gpointer b);
void init_devices(void);
void get_one_device_config(gpointer a, gpointer b);
static void get_device_config(void);
void put_one_device_config(gpointer a, gpointer b);
static void put_device_config(void);
GtkWidget *make_slider_mixer(channel_info *ci);
void open_dialog(void);

static void gmix_restore_window_size (void);
static void gmix_save_window_size (void);

void scan_devices(void);
void free_devices(void);
void init_devices(void);

static void quit_cb (GtkWidget *widget, gpointer data);
static void lock_cb (GtkWidget *widget, channel_info *data);
static void mute_cb (GtkWidget *widget, channel_info *data);
static void rec_cb (GtkWidget *widget, channel_info *data);
static void adj_left_cb (GtkAdjustment *widget, channel_info *data);
static void adj_right_cb (GtkAdjustment *widget, channel_info *data);
static void help(GtkWidget *widget, gpointer data);
static void about_cb (GtkWidget *widget, gpointer data);

/*
 * Gnome info:
 */
GtkWidget *app;
GtkWidget  *slidernotebook;

/* Menus */
static GnomeUIInfo help_menu[] = {
	GNOMEUIINFO_ITEM_STOCK(N_("Help"), NULL, help, GNOME_STOCK_PIXMAP_HELP),
	GNOMEUIINFO_MENU_ABOUT_ITEM (about_cb, NULL),
	GNOMEUIINFO_END
};

static GnomeUIInfo settings_menu[] = {
	GNOMEUIINFO_MENU_PREFERENCES_ITEM (config_cb, NULL),
	GNOMEUIINFO_END
};

static GnomeUIInfo file_menu[] = {
	GNOMEUIINFO_MENU_EXIT_ITEM(quit_cb, NULL),
	GNOMEUIINFO_END
};      

static GnomeUIInfo main_menu[] = {
        GNOMEUIINFO_MENU_FILE_TREE (file_menu),
	GNOMEUIINFO_MENU_SETTINGS_TREE (settings_menu),
        GNOMEUIINFO_MENU_HELP_TREE (help_menu),
        GNOMEUIINFO_END
};
/* End of menus */ 

GList *devices;

int mode=0, mode_norestore=0, mode_initonly=0, mode_nosave=0, num_mixers, mode_restore=0;
#define M_NORESTORE	1
#define M_INITONLY 	2
#define M_NOSAVE	4

static const struct poptOption options[] = {
	{"norestore", 'r', POPT_ARG_NONE, &mode_norestore, 0, N_("don't restore mixer-settings from configuration"), NULL},
	{"restore", 'R', POPT_ARG_NONE, &mode_restore, 0, N_("restore mixer-settings from configuration"), NULL},
	{"initonly", 'i', POPT_ARG_NONE, &mode_initonly, 0, N_("initialise the mixer(s) from stored configuration and exit"), NULL},
	{"nosave", 's', POPT_ARG_NONE, &mode_nosave, 0, N_("don't save (modified) mixer-settings into configuration"), NULL},
	{NULL, '\0', 0, NULL, 0}
};

/*
 * Names for the mixer-channels. device_labels are the initial labels for the
 * sliders, device_names are used in the setup-file.
 *
 * i18n note: These names are defined in the soundcard.h file, but they are
 * only used in the initial configuration.
 * Don't translate the "device_names", because they're used for configuration.
 */
#ifdef ALSA
#ifndef GNOME_STOCK_PIXMAP_BLANK
#define GNOME_STOCK_PIXMAP_BLANK NULL
#endif
struct pixmap device_pixmap[] = {
	"Input Gain", GNOME_STOCK_PIXMAP_BLANK,
	"PC Speaker", GNOME_STOCK_PIXMAP_VOLUME,
	"MIC", GNOME_STOCK_PIXMAP_MIC,
	"Line", GNOME_STOCK_PIXMAP_LINE_IN,
	"CD", GNOME_STOCK_PIXMAP_CDROM,
	"Synth", GNOME_STOCK_PIXMAP_BLANK,
	"PCM", GNOME_STOCK_PIXMAP_BLANK,
	"Output Gain", GNOME_STOCK_PIXMAP_BLANK,
	"Treble", GNOME_STOCK_PIXMAP_BLANK,
	"Bass", GNOME_STOCK_PIXMAP_BLANK,
	"Master", GNOME_STOCK_PIXMAP_VOLUME,
	"default", GNOME_STOCK_PIXMAP_BLANK,
};
#else
const char *device_labels[] = SOUND_DEVICE_LABELS;
const char *device_names[]  = SOUND_DEVICE_NAMES;
#ifndef GNOME_STOCK_PIXMAP_BLANK
#define GNOME_STOCK_PIXMAP_BLANK NULL
#endif
const char *device_pixmap[] = {
	GNOME_STOCK_PIXMAP_VOLUME,               /* Master Volume */
	GNOME_STOCK_PIXMAP_BLANK,                /* Bass */
	GNOME_STOCK_PIXMAP_BLANK,                /* Treble */
	GNOME_STOCK_PIXMAP_BLANK,                /* Synth */
	GNOME_STOCK_PIXMAP_BLANK,                /* PCM */
	GNOME_STOCK_PIXMAP_VOLUME,               /* Speaker */
	GNOME_STOCK_PIXMAP_LINE_IN,              /* Line In */
	GNOME_STOCK_PIXMAP_MIC,                  /* Microphone */
	GNOME_STOCK_PIXMAP_CDROM,                /* CD-Rom */
	GNOME_STOCK_PIXMAP_BLANK,                /* Recording monitor ? */
	GNOME_STOCK_PIXMAP_BLANK,                /* ALT PCM */
	GNOME_STOCK_PIXMAP_BLANK,                /* Rec Level? */
	GNOME_STOCK_PIXMAP_BLANK,                /* In Gain */
	GNOME_STOCK_PIXMAP_BLANK,                /* Out Gain */
	GNOME_STOCK_PIXMAP_LINE_IN,              /* Aux 1 */
	GNOME_STOCK_PIXMAP_LINE_IN,              /* Aux 2 */
	GNOME_STOCK_PIXMAP_LINE_IN,              /* Line */
	GNOME_STOCK_PIXMAP_BLANK,                /* Digital 1 ? */
	GNOME_STOCK_PIXMAP_BLANK,                /* Digital 2 ? */
	GNOME_STOCK_PIXMAP_BLANK,                /* Digital 3 ? */
	GNOME_STOCK_PIXMAP_BLANK,                /* Phone in */
	GNOME_STOCK_PIXMAP_BLANK,                /* Phone Out */
	GNOME_STOCK_PIXMAP_BLANK,                /* Video */
	GNOME_STOCK_PIXMAP_BLANK,                /* Radio */
	GNOME_STOCK_PIXMAP_BLANK,                /* Monitor (usually mic) vol */
	GNOME_STOCK_PIXMAP_BLANK,                /* 3d Depth/space param */
	GNOME_STOCK_PIXMAP_BLANK,                /* 3d center param */
	GNOME_STOCK_PIXMAP_BLANK                 /* Midi */
};
#endif

#ifdef ALSA
snd_mixer_callbacks_t read_cbs;

static void rebuild_cb(void *data) {
	fprintf(stderr, "gmix rebuild_cb()\n");
}
static void element_cb(void *data, int cmd, snd_mixer_eid_t *eid) {
	device_info *device=(device_info *)data;
	int err;
	snd_mixer_element_info_t info;
	snd_mixer_element_t element;

	if(cmd != SND_MIXER_READ_ELEMENT_VALUE)
		return;

	if (snd_mixer_element_has_info(eid) != 1) {
/*		fprintf(stderr, "no element info\n"); */
		return;
	}
	memset(&info, 0, sizeof(info));
	info.eid = *eid;
	if ((err = snd_mixer_element_info_build(device->handle, &info)) < 0) {
		fprintf(stderr, "element info build error: %s\n", snd_strerror(err));
		return;
	}
	memset(&element, 0, sizeof(element));
	element.eid = *eid;
	if ((err = snd_mixer_element_build(device->handle, &element)) < 0) {
		fprintf(stderr, "element build error: %s\n", snd_strerror(err));
		return;
	}
	if (element.eid.type == SND_MIXER_ETYPE_VOLUME1) {
#if 0
		/* XXX This code should make the slider actually change to the
		 value in element.data.volume1.pvoices[0] (left) and
		 element.data.volume1.pvoices[1] (right).  This way the slider
		 reflects the value of the item changed in whatever other
		 applications are manipulating the (hardware) mixer */
		int i;
		GList *channels=g_list_first(device->channels);
		for (i=0; i<element.data.volume1.voices_size;
			 i++, channels = g_list_next(channels)) {
			channel_info *ci=(channel_info *)channels->data;

			fprintf(stderr, "%d(%d%%) ", element.data.volume1.pvoices[i],
									   element.data.volume1.pvoices[i] * 100 /
									   (info.data.volume1.prange[i].max -
					 					info.data.volume1.prange[i].min));
			/* This did not seem to work.
			 * I don't know how "sliders" and "adjustments" and all that goo
			 * works in GTK+.
			 */
			gtk_adjustment_set_value(GTK_ADJUSTMENT(ci->left), (gfloat)(-(
					element.data.volume1.pvoices[i] * 100 /
									   (info.data.volume1.prange[i].max -
					 					info.data.volume1.prange[i].min))));
		}
		fprintf(stderr, "\n");
#endif
	} else /* XXX ignore silently */
		fprintf(stderr, "unsupported element.eid.type=%d for %s\n",
				element.eid.type, element.eid.name);
	snd_mixer_element_free(&element);

}
static void group_cb(void *data, int cmd, snd_mixer_gid_t *gid) {
	fprintf(stderr, "gmix group_cb()\n");
}

#if 0
void read_mixer (gpointer a, gpointer b)
#else
void read_mixer(gpointer a, gint source, GdkInputCondition condition) 
#endif
{
	device_info *info = (device_info *)a;
	int err;

	if ((err=snd_mixer_read(info->handle, &read_cbs))<0) {
		fprintf(stderr, "error reading group: %s\n", snd_strerror(err));
		exit (1);
	}
}

#if 0
gboolean read_mixers ()
{
	g_list_foreach(devices, read_mixer, NULL);
	return TRUE;
}
#endif
#endif

static void error_close_cb(void)
{
	exit(1);
}

void config_cb(GtkWidget *widget, gpointer data)
{
	prefs_make_window();
};

int main(int argc, char *argv[]) 
{
	mode=0;

	bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);
	gnome_init_with_popt_table("gmix", VERSION, argc, argv, options,
				   0, NULL);

	if (g_file_exists (GNOME_ICONDIR"/gnome-volume.png"))
		gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gnome-volume.png");
	else
		gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/gnome-mixer.png");

	if(mode_nosave) mode |= M_NOSAVE;
	if(mode_initonly) mode |= M_INITONLY;

	scan_devices();
	if (devices) {
		if (~mode & M_INITONLY) {
		        get_gui_config();
			/* Beware boolean bastardization */
			if (!prefs.set_mixer_on_start)
				mode |= M_NORESTORE;
			  
		}
		/* Command line always overrides */
		if(mode_norestore) mode |= M_NORESTORE;
		if(mode_restore) mode &= ~M_NORESTORE;

		if (~mode & M_NORESTORE) {
			get_device_config();
			init_devices();
		} 

		if (~mode & M_INITONLY) {

			open_dialog();

			put_gui_config();/* Do we need this*/
		}

		if (~mode & M_NOSAVE) {
			put_device_config();
		}
		gnome_config_sync();
		free_devices();
	} else {
		GtkWidget *box;
		box = gnome_error_dialog(
			"I was not able to open your audio device.\n"
			"Please check that you have permission to open /dev/mixer\n"
			"and make sure you have sound support compiled into your kernel.");

		gtk_signal_connect(GTK_OBJECT(box), "close",
				   GTK_SIGNAL_FUNC(error_close_cb), NULL);
		gtk_main();
	}
	return 0;
}

/*
 * Open the device and query configuration
 */
device_info *open_device(int num)
{
#ifdef ALSA
	device_info *new_device;
	int device = 0, err; 
	snd_mixer_info_t info; 
	snd_mixer_groups_t groups;
	snd_mixer_gid_t *group;
	int cnt, chn;
	char *card_name;

	/*
	 * create new device configureation
	 */
	new_device = g_new0(device_info, 1);
	/*
	 * open the mixer-device
	 */
	if ((err=snd_mixer_open(&new_device->handle, num, device)) < 0) { 
		g_free(new_device);
		return NULL;
	} 
	if ((err=snd_mixer_info(new_device->handle, &info)) < 0) { 
		fprintf(stderr, "info failed: %s\n", snd_strerror(err));  
		snd_mixer_close(new_device->handle); 
		g_free(new_device);
		return NULL; 
	} 
	/*
	 * mixer-name
	 */
	if (snd_card_get_name(num, &card_name) == 0) {
		strcpy(new_device->info.name, card_name);
	} else {
		strcpy(new_device->info.name, info.name);
	}
	if(!isalpha(new_device->info.name[0]))
		g_snprintf(new_device->info.name, 31, "Card %d", num+1);
	/* and id */
	strcpy(new_device->info.id, info.id);
	/* 
	 * several bitmasks describing the mixer
	 */
	memset(&groups, 0, sizeof(groups));
	if ((err=snd_mixer_groups(new_device->handle, &groups))<0) {
		g_free(new_device);
		return NULL;
	}
	groups.pgroups=
		(snd_mixer_gid_t *)malloc(groups.groups_over*sizeof(snd_mixer_gid_t));
	if (!groups.pgroups) {
		g_free(new_device);
		return NULL;
	}
	groups.groups_size=groups.groups_over;
	groups.groups_over=groups.groups=0;
	if ((err=snd_mixer_groups(new_device->handle, &groups))<0) {
		g_free(new_device);
		free(groups.pgroups);
		return NULL;
	}
	for (cnt=0; cnt<groups.groups; cnt++) {
		snd_mixer_group_t g;

		group=&groups.pgroups[cnt];
		new_device->devmask|=(1<<cnt);
		memset(&g, 0, sizeof(g));
		g.gid=*group;
		err=snd_mixer_group_read(new_device->handle, &g);
		if (g.mute)
			/* XXX muting is actually done on a channel by channel basis on a
			 * given group
			 * gmix is not that sophisticated though... yet
			 */
			new_device->mute_bitmask|=(1<<cnt);
		if (g.capture)
			/* XXX same for recording sources */
			new_device->recsrc|=(1<<cnt);
			
		if (g.caps & SND_MIXER_GRPCAP_CAPTURE)
			new_device->recmask|=(1<<cnt);
		if (g.channels == SND_MIXER_CHN_MASK_STEREO)
			new_device->stereodevs|=(1<<cnt);
    	/*
    	 * get current volume
    	 */
		for (chn = 0; chn <= SND_MIXER_CHN_LAST; chn++) {
			if (!(g.channels & (1<<chn)))
				continue;
			if ((1<<chn) & SND_MIXER_CHN_MASK_FRONT_LEFT)
				new_device->volume_left[cnt]=
					(g.volume.values[chn] * 100 / (g.max - g.min));
			else if ((1<<chn) & SND_MIXER_CHN_MASK_FRONT_RIGHT)
				new_device->volume_right[cnt]=
					(g.volume.values[chn] * 100 / (g.max - g.min));
			else
				fprintf(stderr, "Unhandled channel on soundcard: %s\n",
						snd_mixer_channel_name(chn));
		}
	}
	free(groups.pgroups);
	new_device->lock_bitmask=new_device->stereodevs;	/* all locked */
	new_device->enabled_bitmask=new_device->devmask;	/* all enabled */
	
	memset(&read_cbs, 0, sizeof(read_cbs));
	read_cbs.private_data=new_device;
	read_cbs.rebuild=rebuild_cb;
	read_cbs.element=element_cb;
	read_cbs.group=group_cb;
	gdk_input_add(snd_mixer_file_descriptor(new_device->handle), GDK_INPUT_READ,
				  read_mixer, new_device);
#else
	device_info *new_device;
	char device_name[255];
	int res, ver, cnt;
	/*
	 * create new device configureation
	 */
	new_device = g_new0(device_info, 1);
	/*
	 * open the mixer-device
	 */
	if (num==0) {
		sprintf(device_name, "/dev/mixer");
	} else {
		sprintf(device_name, "/dev/mixer%i", num);
	}
	new_device->fd=open(device_name, O_RDWR, 0);
	if (new_device->fd<0) {
		g_free(new_device);
		return NULL;
	}
	/*
	 * check driver-version
	 */
#ifdef OSS_GETVERSION
	res=ioctl(new_device->fd, OSS_GETVERSION, &ver);
	if ((res==EINVAL) || (ver!=SOUND_VERSION)) {
		if (res == EINVAL) {
			fprintf(stderr, 
				_("Warning: This version of gmix was compiled with\n"
				"OSS version %d.%d.%d, and your system is running\n"
				"a version prior to 3.6.0.\n"), 
				SOUND_VERSION >> 16, 
				(SOUND_VERSION >> 8) & 0xff, 
				SOUND_VERSION & 0xff);
		} else {
			fprintf(stderr, 
				_("Warning: This version of gmix was compiled with\n"
				"OSS version %d.%d.%d, and your system is running\n"
				"version %d.%d.%d.\n"), 
				SOUND_VERSION >> 16, 
				(SOUND_VERSION >> 8) & 0xff, 
				SOUND_VERSION & 0xff,
				ver >> 16, (ver >> 8) & 0xff, ver & 0xff);
		}			
	}
#endif
	/*
	 * mixer-name
	 */
	res=ioctl(new_device->fd, SOUND_MIXER_INFO, &new_device->info);
	if (res!=0) {
		g_free(new_device);
		return NULL;
	}
	if(!isalpha(new_device->info.name[0]))
		g_snprintf(new_device->info.name, 31, "Card %d", num+1);
	/* 
	 * several bitmasks describing the mixer
	 */
	res=ioctl(new_device->fd, SOUND_MIXER_READ_DEVMASK, &new_device->devmask);
        res|=ioctl(new_device->fd, SOUND_MIXER_READ_RECMASK, &new_device->recmask);
        res|=ioctl(new_device->fd, SOUND_MIXER_READ_RECSRC, &new_device->recsrc);
        res|=ioctl(new_device->fd, SOUND_MIXER_READ_STEREODEVS, &new_device->stereodevs);
        res|=ioctl(new_device->fd, SOUND_MIXER_READ_CAPS, &new_device->caps);
	if (res!=0) {
		g_free(new_device);
		return NULL;
	}

	/*
	 * get current volume
	 */
	new_device->mute_bitmask=0;				/* not muted */
	new_device->lock_bitmask=new_device->stereodevs;	/* all locked */
	new_device->enabled_bitmask=new_device->devmask;	/* all enabled */
	for (cnt=0; cnt<SOUND_MIXER_NRDEVICES; cnt++) {
		if (new_device->devmask & (1<<cnt)) {
			unsigned long vol; /* l: vol&0xff, r:(vol&0xff00)>>8 */
			res=ioctl(new_device->fd, MIXER_READ(cnt), &vol);
		                                                
			new_device->volume_left[cnt]=vol & 0xff;
			if (new_device->stereodevs & (1<<cnt)) {
				new_device->volume_right[cnt]=(vol&0xff00)>>8;
			} else {
				new_device->volume_right[cnt]=vol&0xff;
			}
		}
	}
#endif
#if 0
	/*
	 * print debug-information
	 */
	printf("%s\n", new_device->info.id);
	printf("%s\n", new_device->info.name);
	
	for (cnt=0; cnt<SOUND_MIXER_NRDEVICES; cnt++) {
		if ((new_device->devmask | new_device->recmask) & (1<<cnt)) {
			printf("%s:", device_labels[cnt]);
			if (new_device->recmask & (1<<cnt)) {
				printf("recmask ");
			}
			if (new_device->recsrc & (1<<cnt)) {
				printf("recsrc ");
			}
			if ((new_device->devmask) & (1<<cnt)) {
				if (new_device->stereodevs & (1<<cnt)) {
					printf("stereo %i/%i", new_device->volume_left[cnt], new_device->volume_right[cnt]);
				} else {
					printf("mono %i", new_device->volume_left[cnt]);
				}
			}
			printf("\n");
		}
	}
#endif
	return new_device;
}

GList *make_channels(device_info *device)
{
	int i;
	GList *channels;
#ifdef ALSA
	int err;
	snd_mixer_groups_t groups;
	channels=NULL;
	memset(&groups, 0, sizeof(groups));
	if ((err=snd_mixer_groups(device->handle, &groups))<0)
		return NULL;
	groups.pgroups=
		(snd_mixer_gid_t *)malloc(groups.groups_over*sizeof(snd_mixer_gid_t));
	if (!groups.pgroups)
		return NULL;
	groups.groups_size=groups.groups_over;
	groups.groups_over=groups.groups=0;
	if ((err=snd_mixer_groups(device->handle, &groups))<0) {
		free(groups.pgroups);
		return NULL;
	}
	for (i=0; i<groups.groups; i++) {
		int j;
		channel_info *new_channel;
		snd_mixer_group_t *g;
		g=(snd_mixer_group_t *)calloc(1, sizeof(snd_mixer_group_t));
		g->gid=groups.pgroups[i];
		if ((err=snd_mixer_group_read(device->handle, g)<0)) {
			fprintf(stderr, "error reading group: %s\n", snd_strerror(err));
			free(groups.pgroups);
			return NULL;
		}
		new_channel=(channel_info *)malloc(sizeof(channel_info));
		new_channel->mixer_group=g;
		new_channel->device=device;
		new_channel->channel=i;
		/* find the pixmap for this group */
		for (j=0; strcmp(device_pixmap[j].name, "default")!=0 &&
				  strcmp(device_pixmap[j].name, g->gid.name)!=0; j++);
		new_channel->pixmap = g_strdup (device_pixmap[j].pixmap);
		new_channel->title= g_strdup(_(groups.pgroups[i].name));
		new_channel->user_title= g_strdup(_(groups.pgroups[i].name));
		new_channel->passive=0;
		channels=g_list_append(channels, new_channel);
	}
	free(groups.pgroups);
#else
	channels=NULL;
	for (i=0; i<SOUND_MIXER_NRDEVICES; i++) {
		if ((device->devmask | device->recmask) & (1<<i)) {
			channel_info *new_channel;
			new_channel=(channel_info *)malloc(sizeof(channel_info));
			new_channel->device=device;
			new_channel->channel=i;
	 		new_channel->pixmap = g_strdup (device_pixmap[i]);
			new_channel->title= g_strdup(_(device_labels[i]));
			new_channel->user_title= g_strdup(_(device_labels[i]));
			new_channel->passive=0;
			channels=g_list_append(channels, new_channel);
		}
	}
#endif
	return channels;
}

void scan_devices(void)
{
	int cnt;
	device_info *new_device;
	cnt=0; devices=NULL;
	do {
		new_device=open_device(cnt++);
		if (new_device) {
			new_device->channels=make_channels(new_device);
			devices=g_list_append(devices, new_device);
		}
	} while (new_device);
	num_mixers=cnt-1;
}

#ifdef ALSA
void free_one_channel(gpointer a, gpointer b)
{
	channel_info *chan = (channel_info *)a;
	free(chan->mixer_group);
}
#endif

void free_one_device(gpointer a, gpointer b)
{
	device_info *info = (device_info *)a;
#ifdef ALSA
	snd_mixer_close(info->handle);
	g_list_foreach(info->channels, free_one_channel, NULL);
#else
	close(info->fd);
#endif
	g_list_free(info->channels);
}

void free_devices(void)
{
	g_list_foreach(devices, free_one_device, NULL);
	g_list_free(devices);
}

void init_one_device(gpointer a, gpointer b)
{
	device_info *info = (device_info *)a;
#ifdef ALSA
	GList *p;
#else
	unsigned long vol;
#endif
	int c;
	
#ifdef ALSA
	for (p=g_list_first(info->channels); p; p=g_list_next(p)) {
		channel_info *channel = (channel_info *)p->data;
		snd_mixer_group_t *g=channel->mixer_group;
		c = channel->channel;
		if (info->devmask & (1<<c)) {
			int err;
			if (info->mute_bitmask & (1<<c))
				g->mute=SND_MIXER_CHN_MASK_FRONT_LEFT+
						SND_MIXER_CHN_MASK_FRONT_RIGHT;
			else
				g->mute=0;
			while ((err=snd_mixer_group_write(info->handle, g))<0 && err==-EBUSY)
				if ((err=snd_mixer_read(info->handle, &read_cbs))<0) {
					fprintf(stderr, "error reading group: %s\n",
							snd_strerror(err));
					exit (1);
				}
			if (err<0) {
				fprintf(stderr, "error writing group: %s\n", snd_strerror(err));
				exit (1);
			}
		}
	}
#else
	ioctl(info->fd, SOUND_MIXER_WRITE_RECSRC, &info->recsrc);

	for (c=0; c<SOUND_MIXER_NRDEVICES; c++) {
		if (info->devmask & (1<<c)) {
			if (info->mute_bitmask & (1<<c)) {
				vol=0;
			} else {
				vol = info->volume_left[c];
				vol |= info->volume_right[c]<<8;
			}
			ioctl(info->fd, MIXER_WRITE(c), &vol);
		}
	}
#endif
}

void init_devices(void)
{
	g_list_foreach(devices, init_one_device, NULL);
}

void get_one_device_config(gpointer a, gpointer b)
{
	device_info *info = (device_info *)a;
	int cc;
	/*
	 * restore mixer-configuration
	 */ 
	char name[255];
	GList *p;

	for (p=g_list_first(info->channels); p; p=g_list_next(p)) {
		channel_info *channel = (channel_info *)p->data;
		cc=channel->channel;
#ifdef ALSA
		if (info->devmask & (1<<cc)) {
			sprintf(name, "/gmix/%s_%s/title=%s", info->info.id,\
				channel->title,\
				channel->title);
			channel->user_title = gnome_config_get_string(name);
			sprintf(name, "/gmix/%s_%s/mute=%s", info->info.id,\
				channel->title,\
				(info->mute_bitmask & (1<<cc))?"true":"false");
			info->mute_bitmask&=~(1<<cc);
			info->mute_bitmask|= gnome_config_get_bool(name) ? (1<<cc) : 0;
			if (info->stereodevs & (1<<cc)) {
				sprintf(name, "/gmix/%s_%s/lock=%s", info->info.id, channel->title, (info->lock_bitmask & (1<<cc))?"true":"false");
				info->lock_bitmask&=~(1<<cc);
				info->lock_bitmask|=gnome_config_get_bool(name) ? (1<<cc) : 0;
				sprintf(name, "/gmix/%s_%s/volume_left=%i", info->info.id, channel->title, info->volume_left[cc]);
				info->volume_left[cc]=gnome_config_get_int(name);
				sprintf(name, "/gmix/%s_%s/volume_right=%i", info->info.id, channel->title, info->volume_right[cc]);
				info->volume_right[cc]=gnome_config_get_int(name);
			} else {
				sprintf(name, "/gmix/%s_%s/volume=%i", info->info.id, channel->title, info->volume_left[cc]);
				info->volume_left[cc]=gnome_config_get_int(name);
			}
		}
		if (info->recmask & (1<<cc)) {
			sprintf(name, "/gmix/%s_%s/recsrc=%s", info->info.id, channel->title, (info->recsrc & (1<<cc))?"true":"false");
			info->recsrc&=~(1<<cc);
			info->recsrc|=gnome_config_get_bool(name) ? (1<<cc) : 0;
		}
#else
		if (info->devmask & (1<<cc)) {
			sprintf(name, "/gmix/%s_%s/title=%s", info->info.id,\
				device_names[cc],\
				device_labels[cc]);
			channel->user_title = gnome_config_get_string(name);
			sprintf(name, "/gmix/%s_%s/mute=%s", info->info.id,\
				device_names[cc],\
				(info->mute_bitmask & (1<<cc))?"true":"false");
			info->mute_bitmask&=~(1<<cc);
			info->mute_bitmask|= gnome_config_get_bool(name) ? (1<<cc) : 0;
			if (info->stereodevs & (1<<cc)) {
				sprintf(name, "/gmix/%s_%s/lock=%s", info->info.id, device_names[cc], (info->lock_bitmask & (1<<cc))?"true":"false");
				info->lock_bitmask&=~(1<<cc);
				info->lock_bitmask|=gnome_config_get_bool(name) ? (1<<cc) : 0;
				sprintf(name, "/gmix/%s_%s/volume_left=%i", info->info.id, device_names[cc], info->volume_left[cc]);
				info->volume_left[cc]=gnome_config_get_int(name);
				sprintf(name, "/gmix/%s_%s/volume_right=%i", info->info.id, device_names[cc], info->volume_right[cc]);
				info->volume_right[cc]=gnome_config_get_int(name);
			} else {
				sprintf(name, "/gmix/%s_%s/volume=%i", info->info.id, device_names[cc], info->volume_left[cc]);
				info->volume_left[cc]=gnome_config_get_int(name);
			}
		}
		if (info->recmask & (1<<cc)) {
			sprintf(name, "/gmix/%s_%s/recsrc=%s", info->info.id, device_names[cc], (info->recsrc & (1<<cc))?"true":"false");
			info->recsrc&=~(1<<cc);
			info->recsrc|=gnome_config_get_bool(name) ? (1<<cc) : 0;
		}
#endif
	}
}

void get_device_config(void)
{
	g_list_foreach(devices, get_one_device_config, NULL);
}

void put_one_device_config(gpointer a, gpointer b)
{
	device_info *info = (device_info *)a;
	int cc;
	/*
	 * save mixer-configuration
	 */ 
	char name[255];
	GList *p;

	for (p=g_list_first(info->channels); p; p=g_list_next(p)) {
		channel_info *channel = (channel_info *)p->data;
		cc=channel->channel;
#ifdef ALSA
		if (info->devmask & (1<<cc)) {
			sprintf(name, "/gmix/%s_%s/title", info->info.id, channel->title);
			gnome_config_set_string(name, channel->user_title);
			sprintf(name, "/gmix/%s_%s/mute", info->info.id, channel->title);
			gnome_config_set_bool(name, (info->mute_bitmask & (1<<cc))!=0);
			if (info->stereodevs & (1<<cc)) {
				sprintf(name, "/gmix/%s_%s/lock", info->info.id, channel->title);
				gnome_config_set_bool(name, (info->lock_bitmask & (1<<cc))!=0);
				sprintf(name, "/gmix/%s_%s/volume_left", info->info.id, channel->title);
				gnome_config_set_int(name, info->volume_left[cc]);
				sprintf(name, "/gmix/%s_%s/volume_right", info->info.id, channel->title);
				gnome_config_set_int(name, info->volume_right[cc]);
			} else {
				sprintf(name, "/gmix/%s_%s/volume", info->info.id, channel->title);
				gnome_config_set_int(name, info->volume_left[cc]);
			}
		}
		if (info->recmask & (1<<cc)) {
			sprintf(name, "/gmix/%s_%s/recsrc", info->info.id, channel->title);
			gnome_config_set_bool(name, (info->recsrc & (1<<cc))!=0);
		}
#else
		if (info->devmask & (1<<cc)) {
			sprintf(name, "/gmix/%s_%s/title", info->info.id, device_names[cc]);
			gnome_config_set_string(name, channel->user_title);
			sprintf(name, "/gmix/%s_%s/mute", info->info.id, device_names[cc]);
			gnome_config_set_bool(name, (info->mute_bitmask & (1<<cc))!=0);
			if (info->stereodevs & (1<<cc)) {
				sprintf(name, "/gmix/%s_%s/lock", info->info.id, device_names[cc]);
				gnome_config_set_bool(name, (info->lock_bitmask & (1<<cc))!=0);
				sprintf(name, "/gmix/%s_%s/volume_left", info->info.id, device_names[cc]);
				gnome_config_set_int(name, info->volume_left[cc]);
				sprintf(name, "/gmix/%s_%s/volume_right", info->info.id, device_names[cc]);
				gnome_config_set_int(name, info->volume_right[cc]);
			} else {
				sprintf(name, "/gmix/%s_%s/volume", info->info.id, device_names[cc]);
				gnome_config_set_int(name, info->volume_left[cc]);
			}
		}
		if (info->recmask & (1<<cc)) {
			sprintf(name, "/gmix/%s_%s/recsrc", info->info.id, device_names[cc]);
			gnome_config_set_bool(name, (info->recsrc & (1<<cc))!=0);
		}
#endif
	}
}

void put_device_config(void)
{
	g_list_foreach(devices, put_one_device_config, NULL);
}

/*
 * dialogs:
 */
GtkWidget *make_slider_mixer(channel_info *ci)
{
	GtkWidget *hbox, *scale;
	device_info *di;
	di=ci->device;

	hbox=gtk_hbox_new(FALSE,1);

	if (di->devmask & (1<<ci->channel)) {
	/* Left channel, display only if we have mixable device */
#ifdef ALSA
		ci->left=gtk_adjustment_new (-ci->device->volume_left[ci->channel], -100.0, 0.0, 1.0, 1.0, 0.0);
#else
		ci->left=gtk_adjustment_new (-ci->device->volume_left[ci->channel], -101.0, 0.0, 1.0, 1.0, 0.0);
#endif
		gtk_signal_connect(GTK_OBJECT(ci->left), "value_changed", (GtkSignalFunc)adj_left_cb, (gpointer)ci);
		scale = gtk_vscale_new (GTK_ADJUSTMENT(ci->left));
		gtk_range_set_update_policy (GTK_RANGE(scale), GTK_UPDATE_CONTINUOUS);
		gtk_scale_set_draw_value(GTK_SCALE(scale), FALSE);
		gtk_box_pack_start (GTK_BOX (hbox), scale, TRUE, TRUE, 0);
		gtk_widget_show (scale);
		if (di->stereodevs & (1<<ci->channel)) {
			/* Right channel, display only if we have stereo */
#ifdef ALSA
			ci->right=gtk_adjustment_new (-ci->device->volume_right[ci->channel], -100.0, 0.0, 1.0, 1.0, 0.0);
#else
			ci->right=gtk_adjustment_new (-ci->device->volume_right[ci->channel], -101.0, 0.0, 1.0, 1.0, 0.0);
#endif
			gtk_signal_connect(GTK_OBJECT(ci->right), "value_changed", (GtkSignalFunc)adj_right_cb, (gpointer)ci);
			scale = gtk_vscale_new (GTK_ADJUSTMENT(ci->right));
			gtk_range_set_update_policy (GTK_RANGE(scale), GTK_UPDATE_CONTINUOUS);
			gtk_scale_set_draw_value (GTK_SCALE(scale), FALSE);
			gtk_box_pack_start (GTK_BOX(hbox), scale, TRUE, TRUE, 0);
			gtk_widget_show (scale);
		}
	}

	return hbox;
}

void fill_in_device_guis(GtkWidget *notebook){
	GList *d, *c;
	GtkWidget *table, *spixmap;
	int i,j;

	i=0;
	for (d=devices; d; d=d->next) {
		device_info *di;
		di=d->data;
		j=0;
		for (c=((device_info *)d->data)->channels;c;c=c->next) {
			j+=2;
		}

		/* 
		 * David: changed 7 to 8 for table rows (06/04/1999)
		 */
		table=gtk_table_new(i*2, 8, FALSE);
		gtk_notebook_append_page(GTK_NOTEBOOK(notebook),
					 table, 
					 gtk_label_new(di->info.name));
		gtk_table_set_row_spacings (GTK_TABLE (table), 0);
		gtk_table_set_col_spacings (GTK_TABLE (table), 0);
		gtk_container_border_width (GTK_CONTAINER (table), 0);
		gtk_widget_show (table);

		for (c=((device_info *)d->data)->channels;c;c=c->next) {
			GtkWidget *label, *mixer, *separator;
			channel_info *ci;
			ci=c->data;
			if ((ci->pixmap)&&(prefs.use_icons))
			{
				spixmap = gnome_stock_pixmap_widget (app, ci->pixmap);
				gtk_table_attach (GTK_TABLE (table), spixmap, i, i+1, 1, 2, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
				gtk_widget_show (spixmap);
			}
			if (prefs.use_labels) {
				label=gtk_label_new(ci->user_title);
				gtk_misc_set_alignment (GTK_MISC(label), 0.1, 0.5);
				gtk_table_attach (GTK_TABLE (table), label, i, i+1, 2, 3, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
				gtk_widget_show(label);
			}

			mixer=make_slider_mixer(ci);
			gtk_table_attach (GTK_TABLE (table), mixer, i, i+1,\
					  3, 4, GTK_EXPAND | GTK_FILL,\
					  GTK_EXPAND | GTK_FILL, 0, 0);
			gtk_widget_show (mixer);

			if (ci->device->stereodevs & (1<<ci->channel)) {
				/* lock-button, only useful for stereo */
				ci->lock = gtk_check_button_new_with_label(_("Lock"));
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ci->lock), (ci->device->lock_bitmask & (1<<ci->channel))!=0);
				gtk_signal_connect (GTK_OBJECT (ci->lock), "toggled", (GtkSignalFunc) lock_cb, (gpointer)ci);
				gtk_table_attach (GTK_TABLE (table), ci->lock, i, i+1, 4, 5, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
				gtk_widget_show (ci->lock);
			}
			/*
			 * recording sources
			 */
			if (ci->device->recmask & (1<<ci->channel)) {
				ci->rec = gtk_check_button_new_with_label (_("Rec."));
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ci->rec), (ci->device->recsrc & (1<<ci->channel))!=0);
				gtk_signal_connect (GTK_OBJECT (ci->rec), "toggled", (GtkSignalFunc) rec_cb, (gpointer)ci);
				/* David: changed TOP_ATTACH to 7 and
				 * BOTTOM_ATTACH to 8   06/04/1999
				 */
				gtk_table_attach (GTK_TABLE (table), ci->rec, i, i+1, 7, 8, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
				gtk_widget_show (ci->rec);
			} else { /* 
				  * David: need to init it to null
				  * otherwise we get a segfault when
				  * trying to toggle
				  * the buttons 
				  */
				ci->rec = NULL;
			}
	
			if (ci->device->devmask & (1<<ci->channel)) {
				ci->mute=gtk_check_button_new_with_label(_("Mute"));
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ci->mute), (ci->device->mute_bitmask & (1<<ci->channel))!=0);
				gtk_signal_connect (GTK_OBJECT (ci->mute), "toggled", (GtkSignalFunc) mute_cb, (gpointer)ci);
				gtk_table_attach (GTK_TABLE (table), ci->mute, i, i+1, 6, 7, GTK_EXPAND | GTK_FILL, GTK_FILL, 0, 0);
				gtk_widget_show (ci->mute);
			}

			separator = gtk_vseparator_new ();
			/* BOTTOM_ATTACH changed to 8 */
			gtk_table_attach (GTK_TABLE (table), separator, i+1, i+2, 1, 8, GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
			gtk_widget_show (separator);
			i+=2; 
		}
	}

}

void gmix_build_slidernotebook(void)
{
        /* This is a sloppy way to re-draw the mixers */
        GList *d;

	if (slidernotebook)
	{
		gtk_widget_hide(slidernotebook);
		/* Assumes that the number of devices is static... */
		for (d=devices; d; d=d->next) 
			gtk_notebook_remove_page(GTK_NOTEBOOK(slidernotebook),0);
	}
	else
	{
		slidernotebook = gtk_notebook_new();
	}
	gtk_widget_show(slidernotebook);
	fill_in_device_guis(slidernotebook);
}

void open_dialog(void)
{
	GList *d, *c;
	
	int i;

	app = gnome_app_new ("gmix", _("GMIX 3.0") );
	gtk_widget_realize (app);
	gtk_signal_connect (GTK_OBJECT (app), "delete_event",
			    GTK_SIGNAL_FUNC (quit_cb),
			    NULL);


	gmix_restore_window_size ();

	/*
	 * Build main menu
	 */
	gnome_app_create_menus(GNOME_APP(app), main_menu);

	/*
	 * count channels for table size;
	 */
	i=0;
	for (d=devices; d; d=d->next) {
		for (c=((device_info *)d->data)->channels; c; c=c->next) {
			i++;
		}
	}

	/*
	 * Build table with sliders
	 */	
	gmix_build_slidernotebook();
	gnome_app_set_contents(GNOME_APP (app), slidernotebook);

	gtk_widget_show(app);

	/*
	 * Go into gtk event-loop
	 */
	gtk_main();
}

static void gmix_restore_window_size (void)
{
	gint width = 0, height = 0;

	width = gnome_config_get_int ("/gmix/gui/width=0");
	height = gnome_config_get_int ("/gmix/gui/height=0");

	if (width!=0 && height!=0)
		gtk_window_set_default_size (GTK_WINDOW (app), width, height);
}

static void gmix_save_window_size (void)
{
	gint width, height;

	gdk_window_get_size (app->window, &width, &height);

	gnome_config_set_int ("/gmix/gui/width", width);
	gnome_config_set_int ("/gmix/gui/height", height);
}

/*
 * GTK Callbacks:
 */
static void quit_cb (GtkWidget *widget, gpointer data)
{
	gmix_save_window_size ();
	gtk_main_quit();
}

void lock_cb (GtkWidget *widget, channel_info *data)
{
	if (data==NULL) return;
	data->device->lock_bitmask&=~(1<<data->channel);
	if (GTK_TOGGLE_BUTTON (data->lock)->active) {
		data->device->lock_bitmask|=1<<data->channel;
		GTK_ADJUSTMENT(data->right)->value=GTK_ADJUSTMENT(data->left)->value = (GTK_ADJUSTMENT(data->right)->value+GTK_ADJUSTMENT(data->left)->value)/2.0;
		gtk_signal_emit_by_name(GTK_OBJECT(data->left),"value_changed");
		gtk_signal_emit_by_name(GTK_OBJECT(data->right),"value_changed");
	}
}

void mute_cb (GtkWidget *widget, channel_info *data)
{
	unsigned long vol;
#ifdef ALSA
	int err;
	snd_mixer_group_t *g=data->mixer_group;
#endif
	if (data==NULL) return;
	data->device->mute_bitmask&=~(1<<data->channel);
	if (GTK_TOGGLE_BUTTON (data->mute)->active) {
		data->device->mute_bitmask|=1<<data->channel;
		vol=0;
#ifdef ALSA
		g->mute=SND_MIXER_CHN_MASK_FRONT_LEFT+SND_MIXER_CHN_MASK_FRONT_RIGHT;
		while ((err=snd_mixer_group_write(data->device->handle, g))<0 &&
			   err==-EBUSY)
			if ((err=snd_mixer_read(data->device->handle, &read_cbs))<0) {
				fprintf(stderr, "error reading group: %s\n", snd_strerror(err));
				exit (1);
			}
		if (err<0) {
			fprintf(stderr, "error writing group: %s\n", snd_strerror(err));
			exit (1);
		}
#else
		ioctl(data->device->fd, MIXER_WRITE(data->channel), &vol);
#endif
	} else {
#ifdef ALSA
		g->mute=0;
		while ((err=snd_mixer_group_write(data->device->handle, g))<0 && err==-EBUSY)
			if ((err=snd_mixer_read(data->device->handle, &read_cbs))<0) {
				fprintf(stderr, "error reading group: %s\n", snd_strerror(err));
				exit (1);
			}
		if (err<0) {
			fprintf(stderr, "error writing group: %s\n", snd_strerror(err));
			exit (1);
		}
#else
		vol=data->device->volume_left[data->channel];
		vol|=data->device->volume_right[data->channel] << 8;
		ioctl(data->device->fd, MIXER_WRITE(data->channel), &vol);
#endif
	}
}

void rec_cb(GtkWidget *widget, channel_info *data)
{
	GList *c;
#ifdef ALSA
	snd_mixer_group_t *g=data->mixer_group;
	int err;
#endif
	if (data==NULL) return;
	if (data->passive) return;

	data->device->recsrc&=~(1<<data->channel);

	if (GTK_TOGGLE_BUTTON (data->rec)->active) {
#ifdef ALSA
		if (g->caps & SND_MIXER_GRPCAP_EXCL_CAPTURE) {
#else
		if (data->device->caps & SOUND_CAP_EXCL_INPUT) {
			/* XXX not sure if this is how this works because I don't have a
			 *     mixer that has "exclusive capture" devices
			 */
#endif
			data->device->recsrc=1<<data->channel;
		} else {
			data->device->recsrc|=(1<<data->channel);
		}
	}

#ifdef ALSA
	if ((data->device->recsrc & (1<<data->channel))!=0)
		g->capture = g->channels;
	else
		g->capture = 0;
	while ((err=snd_mixer_group_write(data->device->handle, g))<0 && err==-EBUSY)
		if ((err=snd_mixer_read(data->device->handle, &read_cbs))<0) {
			fprintf(stderr, "error reading group: %s\n", snd_strerror(err));
			exit (1);
		}
	if (err<0) {
		fprintf(stderr, "error writing group: %s\n", snd_strerror(err));
		exit (1);
	}
#else
	ioctl(data->device->fd,SOUND_MIXER_WRITE_RECSRC, &data->device->recsrc);
	
	ioctl(data->device->fd,SOUND_MIXER_READ_RECSRC, &data->device->recsrc);
#endif

	for (c=data->device->channels; c; c=c->next) {
		channel_info *info;
		info=c->data;
		info->passive=1;
		/* check to see if channel can record */
		if( info->rec != NULL )
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(info->rec), (data->device->recsrc & (1<<info->channel))!=0);
		info->passive=0;
	}
}

void adj_left_cb (GtkAdjustment *adjustment, channel_info *data)
{
	unsigned long vol;
#ifdef ALSA
	int err;
	snd_mixer_group_t *g=data->mixer_group;
#endif
	if (data==NULL) return;
	
	if (data->device->stereodevs & (1<<data->channel)) {
		if (data->device->lock_bitmask & (1<<data->channel)) {
			data->device->volume_right[data->channel]=data->device->volume_left[data->channel]=-GTK_ADJUSTMENT(data->left)->value;
			if (GTK_ADJUSTMENT(data->left)->value!=GTK_ADJUSTMENT(data->right)->value) {
				GTK_ADJUSTMENT(data->right)->value = GTK_ADJUSTMENT(data->left)->value;
				gtk_signal_emit_by_name(GTK_OBJECT(data->right),"value_changed");
			}
		} else {
			data->device->volume_left[data->channel]=-GTK_ADJUSTMENT(data->left)->value;
			data->device->volume_right[data->channel]=-GTK_ADJUSTMENT(data->right)->value;
		}
	} else {
		data->device->volume_left[data->channel]=-GTK_ADJUSTMENT(data->left)->value;
	}
#ifdef ALSA
	vol=g->max*data->device->volume_left[data->channel]/100;
	/* avoid writing if there is no change */
	if (g->volume.values[SND_MIXER_CHN_FRONT_LEFT] != vol) {
		g->volume.values[SND_MIXER_CHN_FRONT_LEFT] = vol;
		while ((err=snd_mixer_group_write(data->device->handle, g))<0 && err==-EBUSY)
			if ((err=snd_mixer_read(data->device->handle, &read_cbs))<0) {
				fprintf(stderr, "error reading group: %s\n", snd_strerror(err));
				exit (1);
			}
		if (err<0) {
			fprintf(stderr, "error writing group: %s\n", snd_strerror(err));
			exit (1);
		}
	}
#else
	vol=data->device->volume_left[data->channel];
	vol|=data->device->volume_right[data->channel] << 8;
	ioctl(data->device->fd, MIXER_WRITE(data->channel), &vol);
#endif

	if (GTK_TOGGLE_BUTTON (data->mute)->active) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->mute), FALSE);
	}
}

void adj_right_cb (GtkAdjustment *adjustment, channel_info *data)
{
	unsigned long vol;
#ifdef ALSA
	int err;
	snd_mixer_group_t *g=data->mixer_group;
#endif
	if (data==NULL) return;
	
	if (data->device->stereodevs & (1<<data->channel)) {
		if (data->device->lock_bitmask & (1<<data->channel)) {
			data->device->volume_right[data->channel]=data->device->volume_left[data->channel]=-GTK_ADJUSTMENT(data->right)->value;
			if (GTK_ADJUSTMENT(data->left)->value!=GTK_ADJUSTMENT(data->right)->value) {
				GTK_ADJUSTMENT(data->left)->value = GTK_ADJUSTMENT(data->right)->value;
				gtk_signal_emit_by_name(GTK_OBJECT(data->left),"value_changed");
			}
		} else {
			data->device->volume_left[data->channel]=-GTK_ADJUSTMENT(data->left)->value;
			data->device->volume_right[data->channel]=-GTK_ADJUSTMENT(data->right)->value;
		}
	} else {
		data->device->volume_left[data->channel]=-GTK_ADJUSTMENT(data->left)->value;
	}
#ifdef ALSA
	vol=g->max*data->device->volume_right[data->channel]/100;
	/* avoid writing if there is no change */
	if (g->volume.values[SND_MIXER_CHN_FRONT_RIGHT] != vol) {
		g->volume.values[SND_MIXER_CHN_FRONT_RIGHT] = vol;
		while ((err=snd_mixer_group_write(data->device->handle, g))<0 && err==-EBUSY)
			if ((err=snd_mixer_read(data->device->handle, &read_cbs))<0) {
				fprintf(stderr, "error reading group: %s\n", snd_strerror(err));
				exit (1);
			}
		if (err<0) {
			fprintf(stderr, "error writing group: %s\n", snd_strerror(err));
			exit (1);
		}
	}
#else
	vol=data->device->volume_left[data->channel];
	vol|=data->device->volume_right[data->channel] << 8;
	ioctl(data->device->fd, MIXER_WRITE(data->channel), &vol);
#endif

	if (GTK_TOGGLE_BUTTON (data->mute)->active) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data->lock), FALSE);
	}
}

void help(GtkWidget *widget, gpointer data)
{
        GnomeHelpMenuEntry help_entry = { "gmix",
                                          "index.html" };
        gnome_help_display(NULL, &help_entry);
}

void help_cb(GtkWidget *widget, gpointer data)
{
        GnomeHelpMenuEntry help_entry = { "gmix",
                                          "gmix-prefs.html" };
        gnome_help_display(NULL, &help_entry);
}

static void about_cb (GtkWidget *widget, gpointer data)
{
	static GtkWidget *about = NULL;

	static const char *authors[] = {
		"Jens Ch. Restemeier",
		NULL
	};

	if (about != NULL) {
		gdk_window_show (about->window);
		gdk_window_raise (about->window);
	}
	else
	{
		about = gnome_about_new ( _("GMIX - The Gnome Mixer"), VERSION,
					  "(C) 1998 Jens Ch. Restemeier",
					  authors,
					  _("This is a mixer for OSS sound-devices."),
					  NULL);

		gtk_signal_connect (GTK_OBJECT (about), "destroy",
				    GTK_SIGNAL_FUNC (gtk_widget_destroyed), &about);
		gtk_widget_show (about);
	}
}
