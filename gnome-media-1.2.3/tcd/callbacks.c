/* Various callbacks for gtcd. */

#include <config.h>
#include <gnome.h>
#include <libgnorba/gnorba.h>
#include <sys/types.h>

#include "linux-cdrom.h"
#include "gtcd_public.h"
#include "callbacks.h"

void play_cb(GtkWidget *widget, gpointer data)
{
    if(!cd.isplayable)
	return;
    if(cd.sc.cdsc_audiostatus==CDROM_AUDIO_PAUSED)
	tcd_pausecd(&cd);
    else
	tcd_playtracks(&cd, cd.first_t, cd.last_t, prefs->only_use_trkind);
    cd.repeat_track = cd.cur_t;
    
    draw_status();
    return;
}

void pause_cb(GtkWidget *widget, gpointer data)
{
    if(cd.isplayable) tcd_pausecd(&cd);
    
    draw_status();
    return;
}

void stop_cb(GtkWidget *widget, gpointer data)
{
    if(cd.isplayable) tcd_stopcd(&cd);

    draw_status();
    return;
}

void eject_cb(GtkWidget *widget, gpointer data)
{
    tcd_ejectcd(&cd);
    cd.play_method = NORMAL;
    cd.repeat_track = -1;
    /* SDH: Make sure play/pause state change is noticed */
    cd.sc.cdsc_audiostatus = -1;
    if(cd.isplayable)
    {
	make_goto_menu();
	update_editor();
    }
    draw_status();
    return;
}

void mixer_cb(GtkWidget *widget, gpointer data)
{
    gnome_execute_shell(NULL, prefs->mixer_cmd);
    return;
}

static GtkWidget *about;

void destroy_about(GtkWidget *widget, gpointer data);

void destroy_about(GtkWidget *widget, gpointer data)
{
    about=NULL;
}

void about_cb(GtkWidget *widget, gpointer data)
{
    gchar *authors[] = {
	"Tim P. Gerla",
	"Icons by Straylight and Tigert",
	NULL
    };  

    if(about)
	return;

    about = gnome_about_new ( 
	_("TCD - The Gnome CD Player"), 
	VERSION,
	"(C) 1997-98 Tim P. Gerla",
	(const gchar**)authors,
	_("GTCD is the Gnome CD player application. Includes CDDB support."
	  " Please see the \'Thanks\' file included with the"
	  " distribution for more credits."),
	NULL);

    gtk_signal_connect(GTK_OBJECT(about), "delete_event",
		       destroy_about, NULL);
    gtk_signal_connect(GTK_OBJECT(about), "destroy",
		       destroy_about, NULL);

    gtk_widget_show(about);
    
    return;
}

gint goto_track_cb(GtkWidget *widget, gpointer data)
{
    tcd_playtracks(&cd, GPOINTER_TO_INT(data), cd.last_t, prefs->only_use_trkind);
    cd.repeat_track = GPOINTER_TO_INT(data);
    return 1;
}

extern int time_display;
extern GdkGC *gc;
extern guint slow_timeout;
extern guint fast_timeout;
extern void exit_action(void);
void quit_cb(GtkWidget *widget, gpointer data)
{
	save_prefs(prefs);
	gnome_config_set_int("/gtcd/ui/time_display", time_display);
	gnome_config_sync();

	gtk_timeout_remove (slow_timeout);
	gtk_timeout_remove (fast_timeout);

	gtk_widget_set_sensitive (widget, FALSE);

	/* Let widgets redraw */
	while (g_main_iteration (FALSE))
		;

	exit_action();
	tcd_close_disc(&cd);
	
	{
		CORBA_Environment ev;
		CORBA_exception_init(&ev);
		goad_server_unregister(NULL, "GOAD:gtcd:19990918",
				       "object", &ev);
		CORBA_exception_free(&ev);
	}

	gdk_gc_destroy(gc);
	gtk_main_quit();
}

void changer_cb(GtkWidget *widget, gpointer data)
{
    tcd_change_disc(&cd, GPOINTER_TO_INT(data));
    tcd_post_init(&cd);
    if(cd.err)
    	perror("accessing cdrom drive");
    cd.play_method = NORMAL;
    if(cd.isplayable)
    {
        tcd_readdiskinfo(&cd);
	make_goto_menu();
	update_editor();
	draw_status();
    }
    return;
}


void help(GtkWidget *widget, gpointer data)
{
	GnomeHelpMenuEntry help_entry = { "gtcd",
                                          "index.html" };
        gnome_help_display(NULL, &help_entry);
}

