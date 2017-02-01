#include <config.h>
#include <gnome.h>
#include <libgnomeui/gnome-window-icon.h>

#include "gtcd_public.h"
#include "cddb.h"

GtkWidget *csw;			/* cddb status window */
guint timer=0;
static char *status_file;

static void destroy_window(GtkWidget *widget, gpointer data);
static int status_timer(GtkWidget *gl);

static void destroy_window(GtkWidget *widget, gpointer data)
{
    gtk_timeout_remove(timer);
    gtk_widget_destroy(csw);
    csw = NULL;
}

static void init_status_file (void)
{
	status_file = gnome_util_home_file (".cddbstatus");
}

static int status_timer(GtkWidget *gl)
{
    if (!status_file)
	    init_status_file ();
    
    if(gnome_less_show_file(GNOME_LESS(gl), status_file) == FALSE)
    	gnome_less_clear(GNOME_LESS(gl));
    return TRUE;
}

static void call_slave(GtkWidget *widget, gpointer data)
{
    tcd_call_cddb_slave(&cd, "TCD", "1.0");
}

void cddb_status_dialog(GtkWidget *widget, gpointer data)
{
    GtkWidget *main_box, *gl;
    GtkWidget *button;

    if(csw)
	return;
    
    csw = gtk_window_new(GTK_WINDOW_DIALOG);
    gnome_window_icon_set_from_default (GTK_WINDOW (csw));
    gtk_container_border_width(GTK_CONTAINER(csw), GNOME_PAD_SMALL);
    gtk_window_set_title(GTK_WINDOW(csw), _("CDDB Status"));
    gtk_window_set_wmclass(GTK_WINDOW(csw), "cddb_status","gtcd");
    
    gtk_signal_connect(GTK_OBJECT(csw), "delete_event",
		       GTK_SIGNAL_FUNC(destroy_window), NULL);
 
    main_box = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
    gtk_container_add(GTK_CONTAINER(csw), main_box);
		  
    /* status box */
    gl = gnome_less_new();
    if (!status_file)
	    init_status_file ();
    gnome_less_show_file(GNOME_LESS(gl), status_file);

    gtk_box_pack_start(GTK_BOX(main_box), gl, TRUE, TRUE, 0);

    /* grab button */
    button = gtk_button_new_with_label(_("Get CDDB Now"));
    gtk_box_pack_start_defaults(GTK_BOX(main_box), button);
    gtk_signal_connect(GTK_OBJECT(button), "clicked",
		       GTK_SIGNAL_FUNC(call_slave), NULL);

    /* close button */
    button = gnome_stock_button(GNOME_STOCK_BUTTON_CLOSE);
    gtk_box_pack_start_defaults(GTK_BOX(main_box), button);
    gtk_signal_connect(GTK_OBJECT(button), "clicked",
		       GTK_SIGNAL_FUNC(destroy_window), NULL);

    timer = gtk_timeout_add(500, (GtkFunction)status_timer, gl);
    gtk_widget_show_all(csw);
}
