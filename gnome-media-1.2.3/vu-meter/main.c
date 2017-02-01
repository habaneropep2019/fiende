/*
 * vu-meter -- A volume units meter for esd
 * Copyright (C) 1998 Gregory McLean
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Cambridge, MA 
 * 02139, USA.
 *
 * Modified April 9 1999, by Dave J. Andruczyk to fix time sync problems
 * now it should be pretty much perfectly in sync with the audio.  Adjust
 * the "lag" variable below to taste if you don't like it...  Utilized
 * code from "extace" to get the desired effects..
 * 
 * Small additions April 14th 1999, by Dave J. Andruczyk to fix the missing 
 * peaks problem that happened with quick transients not showin on the vu-meter.
 * Now it will catch pretty much all of them.
 * 
 * Eye candy!
 */
#include <config.h>
#include <gnome.h>
#include <esd.h>
#include <gtkledbar.h>
#include <sys/time.h>
#include <signal.h>
#include <fcntl.h>

gint sound = -1;
#define RATE   44100
typedef struct _vumeter {
    gdouble      l_level;
    gdouble      r_level;
    gint         esd_stat;
    gint         meter_stat;
}vumeter;

/* A fairly good size buffer to keep resource (cpu) down */
#define NSAMP  2048
#define BUFS  8
short        aubuf[BUFS][NSAMP];
GtkWidget   *dial[2];
GtkWidget   *window;
gchar       *esd_host = NULL;
gint 	    curbuf = 0;
gint 	    lag = 2;
gint 	    locount = 0;
gint 	    plevel_l = 0;
gint 	    plevel_r = 0;

/* function prototypes to make gcc happy: */
void update (void);
char *itoa (int i);
void open_sound (void);
void update_levels (gpointer data);
gint  update_display (gpointer data);
void  handle_read (gpointer data, gint source, GdkInputCondition condition);

void sig_alarm (int sig)
{
    led_bar_light_percent (dial[0], (0.0));
    led_bar_light_percent (dial[1], (0.0));
}

char 
*itoa (int i)
{
    static char ret[ 30 ];
    sprintf (ret, "%d", i);
    return ret;
}


static gint
save_state (GnomeClient *client, gint phase, GnomeRestartStyle save_style,
	    gint shutdown, GnomeInteractStyle inter_style, gint fast,
	    gpointer client_data)
{
    gchar *prefix= gnome_client_get_config_prefix (client);
    gchar *argv[]= { "rm", "-r", NULL };  
    gint   xpos;
    gint   ypos;

    gnome_config_push_prefix (prefix);

    gdk_window_get_geometry (window->window, &xpos, &ypos, NULL, NULL, NULL);
    gnome_config_set_int ("Geometry/x", xpos);
    gnome_config_set_int ("Geometry/y", ypos);

    gnome_config_pop_prefix ();
    gnome_config_sync();

    argv[2]= gnome_config_get_real_path (prefix);
    gnome_client_set_discard_command (client, 3, argv);

    return TRUE;
}


void 
open_sound (void)
{
    sound = esd_monitor_stream (ESD_BITS16|ESD_STEREO|ESD_STREAM|ESD_PLAY,
				RATE, esd_host, "volume_meter");
    if (sound < 0)
    { /* TPG: Make a friendly error dialog if we can't open sound */
	GtkWidget *box;
	box = gnome_error_dialog(_("Cannot connect to sound daemon.\nPlease run 'esd' at a command prompt."));
	gnome_dialog_run(GNOME_DIALOG(box));
	exit(1);
    }
}

void
update_levels(gpointer data)
{
    vumeter  *meter;
    gint buf;
    register gint i;
    register short val_l, val_r;
    static unsigned short bigl, bigr;

    meter = (vumeter *)data;
    meter->meter_stat = FALSE;

    curbuf++;
    if(curbuf >= BUFS)
	curbuf = 0;
    buf = ((BUFS*2)+curbuf-lag)%BUFS;
    if((curbuf%2)>0)
	return; 
    
    bigl = bigr = 0;
    for (i = 0; i < NSAMP/2;i++)
    {
	val_l = abs (aubuf[curbuf][i]);
	i++;
	val_r = abs (aubuf[curbuf][i]);
	bigl = (val_l > bigl) ? val_l : bigl;
	bigr = (val_r > bigr) ? val_r : bigr;
    }
    bigl /= (NSAMP/8);
    bigr /= (NSAMP/8);
    meter->l_level = bigl / 100.0;
    meter->r_level = bigr / 100.0;
    led_bar_light_percent (dial[0], meter->l_level);
    led_bar_light_percent (dial[1], meter->r_level);
    meter->meter_stat = TRUE;

    /* updates display RIGHT AWAY if a new
       peak has arrived.  Fixes the "lost
       peaks" problem that happens with fast
       transient music. Also reduced the MAIN
       update rate to lower cpu use. Makes it 
       work a bit better too.. */

    if(plevel_l != meter->l_level)
    {
	update_display(meter);
	goto done;
    }

    if(plevel_r != meter->r_level)
    {
	update_display(meter);
    }
done:

    plevel_l = meter->l_level;
    plevel_r = meter->r_level;

}



void 
handle_read (gpointer data, gint source, GdkInputCondition condition)
{
    static gint pos = 0;
    static gint to_get = NSAMP*2;
    static gint count;

    count = read(source, aubuf[curbuf] + pos, to_get);
    if (count <0)
      exit(1);		/* no data */
    else
    {
	pos += count;
	to_get -= count;
    }
    if (to_get == 0)
    {
	to_get = NSAMP*2;
	pos = 0;
	update_levels(data);
    }

}
gint
update_display (gpointer data)
{
    vumeter  *meter;
    meter = (vumeter *)data;

    if(!meter->meter_stat)
    {
	meter->l_level = (meter->l_level >= 0.0) ? meter->l_level - 1.0 : 0.0;
	meter->r_level = (meter->r_level >= 0.0) ? meter->r_level - 1.0 : 0.0;
	led_bar_light_percent (dial[0], meter->l_level);
	led_bar_light_percent (dial[1], meter->r_level);
    }
    return TRUE;
}

int 
main (int argc, char *argv[])
{
    GnomeClient   *client;
    GtkWidget     *hbox;
    GtkWidget     *frame;
    vumeter       *meter;
    gint          time_id;
    gint          i;
    gint          session_xpos = -1;
    gint          session_ypos = -1;
    gint          orient = 0;

    struct poptOption options[] = 
    {
	{ NULL, 'x', POPT_ARG_INT, NULL, 0, 
	  N_("Specify the X position of the meter."), 
	  N_("X-Position") },
	{ NULL, 'y', POPT_ARG_INT, NULL, 0, 
	  N_("Specify the Y position of the meter."), 
	  N_("Y-Position") },
	{ NULL, 's', POPT_ARG_STRING, NULL, 0, 
	  N_("Connect to the esd server on this host."), 
	  N_("ESD Server Host") },
	{ NULL, 'v', POPT_ARG_NONE, NULL, 0, 
	  N_("Open a vertical version of the meter."), NULL },
	{ NULL, '\0', 0, NULL, 0 }
    };
    options[0].arg = &session_xpos;
    options[1].arg = &session_ypos;
    options[2].arg = &esd_host;
    options[3].arg = &orient;

    bindtextdomain (PACKAGE, GNOMELOCALEDIR);
    textdomain (PACKAGE);
    gnome_init_with_popt_table ("Volume Meter", "0.1", argc, argv, options, 
				0, NULL);
    if (esd_host)
	g_print (_("Host is %s\n"), esd_host);
    meter = g_malloc0 (sizeof (vumeter));
    client = gnome_master_client ();
    gtk_object_ref (GTK_OBJECT (client));
    gtk_object_sink (GTK_OBJECT (client));
    gtk_signal_connect (GTK_OBJECT (client), "save_yourself",
			GTK_SIGNAL_FUNC (save_state), argv[0]);
    gtk_signal_connect (GTK_OBJECT (client), "die",
			GTK_SIGNAL_FUNC (gtk_main_quit), argv[0]);

    if (gnome_client_get_flags (client) & GNOME_CLIENT_RESTORED)
    {
	gnome_config_push_prefix (gnome_client_get_config_prefix (client));

	session_xpos = gnome_config_get_int ("Geometry/x");
	session_ypos = gnome_config_get_int ("Geometry/y");

	gnome_config_pop_prefix ();
    }

    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gnome_window_icon_set_from_file (GTK_WINDOW (window),
				     GNOME_ICONDIR"/gnome-vumeter.png");
    gtk_window_set_title (GTK_WINDOW (window), _("Volume Meter"));
    if (session_xpos >=0 && session_ypos >= 0)
	gtk_widget_set_uposition (window, session_xpos, session_ypos);

    gtk_signal_connect (GTK_OBJECT (window), "destroy",
			GTK_SIGNAL_FUNC (gtk_main_quit), NULL);
    gtk_signal_connect (GTK_OBJECT (window), "delete_event",
			GTK_SIGNAL_FUNC (gtk_main_quit), NULL);

    frame = gtk_frame_new (NULL);
    gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
    gtk_container_border_width (GTK_CONTAINER (frame), 4);
    gtk_container_add (GTK_CONTAINER (window), frame);

    if ( !orient ) 
	hbox = gtk_vbox_new (FALSE, 5);
    else
	hbox = gtk_hbox_new (FALSE, 5);
    gtk_container_border_width (GTK_CONTAINER (hbox), 5);
    gtk_container_add (GTK_CONTAINER (frame), hbox);
    for (i = 0; i < 2; i++)
    {
	dial[i] = led_bar_new (25, orient);
	gtk_box_pack_start (GTK_BOX (hbox), dial[i], FALSE, FALSE, 0);
    }
    gtk_widget_show_all (window);
    open_sound();
    fcntl(sound, F_SETFL, O_NONBLOCK);

    if(sound > 0) /* TPG: Make sure we have a valid fd... */
	gdk_input_add (sound, GDK_INPUT_READ, handle_read, meter);
    /* time_id = gtk_timeout_add (50000, (GtkFunction)update_display, meter); */

    gtk_main ();
    gtk_object_unref (GTK_OBJECT (client));
    /*   gtk_timeout_remove (time_id); */
    g_free (meter);
    return 0;
}
