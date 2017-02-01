/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/* Copyright (C) 1998, 1999 Red Hat, Inc., Tom Tromey
 * Authors: Jonathan Blandford <jrb@redhat.com>
 * Authors: Tom Tromey <tromey@cygnus.com>
 */
#include <config.h>
#include "capplet-widget.h"
#include <X11/Xlib.h>
#include <assert.h>
#include <locale.h>

#include <gdk/gdkx.h>

#include "gnome.h"

/* Maximum number of mouse buttons we handle.  */
#define MAX_BUTTONS 32

/* Half the number of acceleration levels we support.  */
#define MAX_ACCEL 5

/* Maximum threshold we support.  */
#define MAX_THRESH 7


/* True if buttons are mapped right-to-left.  */
static gboolean mouse_rtol;

/* Number of buttons.  */
static int mouse_nbuttons;

/* Our acceleration number.  This is just an integer between 0 and
   2*MAX_ACCEL+1, inclusive.  */
static int mouse_acceleration;

/* Acceleration threshold.  */
static int mouse_thresh;

/* Adjustments. */
GtkObject *thresh_adjust;
GtkObject *accel_adjust;
static GtkWidget *capplet;
static GtkWidget *lbutton, *rbutton;

static void
mouse_read (void)
{
        unsigned char buttons[MAX_BUTTONS];
        int acc_num, acc_den, thresh;
        gboolean rtol_default;

        mouse_nbuttons = XGetPointerMapping (GDK_DISPLAY (), buttons, MAX_BUTTONS);
        assert (mouse_nbuttons <= MAX_BUTTONS);

        /* Note that we only handle right-to-left and left-to-right.
           Most weird mappings are treated as l-to-r.
           We could handle this by showing the mouse buttons and letting the
           user drag-and-drop them to reorder.  But I'm not convinced this
           is worth it.  */
        /* FIXME: this ignores the fact that a mouse with the weird little
           roller generates B4 and B5 when the roller is moved.  That
           shouldn't change when we remap the other mouse buttons.  */
        mouse_rtol = gnome_config_get_bool_with_default ("/Desktop/Mouse/right-to-left=false",
                                                         &rtol_default);
        if (rtol_default)
                mouse_rtol = (buttons[mouse_nbuttons - 1] == 1);

        mouse_thresh = gnome_config_get_int ("/Desktop/Mouse/threshold=-1");
        mouse_acceleration = gnome_config_get_int ("/Desktop/Mouse/acceleration=-1");

        if (mouse_thresh == -1 || mouse_acceleration == -1) {
                XGetPointerControl (GDK_DISPLAY (), &acc_num, &acc_den, &thresh);

                if (mouse_thresh == -1)
                        mouse_thresh = thresh;
                if (mouse_acceleration == -1) {
                        /* Only support cases in our range.  If neither the numerator nor
                           denominator is 1, then rescale.  */
                        if (acc_num != 1 && acc_den != 1) {
                                if (acc_num > acc_den) {
                                        acc_num = (int) ((double) acc_num / acc_den);
                                        acc_den = 1;
                                } else {
                                        acc_den = (int) ((double) acc_den / acc_num);
                                        acc_num = 1;
                                }
                        }
                        if (acc_num > MAX_ACCEL)
                                acc_num = MAX_ACCEL;
                        if (acc_den > MAX_ACCEL)
                                acc_den = MAX_ACCEL;
                        if (acc_den == 1)
                                mouse_acceleration = acc_num + MAX_ACCEL - 1;
                        else
                                mouse_acceleration = MAX_ACCEL - acc_den;
                }
        }
}

static void
mouse_help (void)
{
    GnomeHelpMenuEntry help_entry= {"control-center",
    "peripherals.html#GCCMOUSE"};
    gnome_help_display (NULL, &help_entry);
}
static void
mouse_apply (void)
{
        unsigned char buttons[MAX_BUTTONS], i;
        int num, den, max;

        assert (mouse_nbuttons <= MAX_BUTTONS);


        /* Ignore buttons above 3 -- these are assumed to be a wheel.
           If we have a non-wheeled mouse, this may do weird things */
        XGetPointerMapping(GDK_DISPLAY (), buttons, MAX_BUTTONS);
	max = MIN (mouse_nbuttons, 3);
        for (i = 0; i < max; ++i)
                buttons[i] = mouse_rtol ? (max - i) : (i + 1);
        XSetPointerMapping (GDK_DISPLAY (), buttons, mouse_nbuttons);

        if (mouse_acceleration < MAX_ACCEL)
                {
                        num = 1;
                        den = MAX_ACCEL - mouse_acceleration;
                }
        else
                {
                        num = mouse_acceleration - MAX_ACCEL + 1;
                        den = 1;
                }

        XChangePointerControl (GDK_DISPLAY (), True, True, num, den, mouse_thresh);
}
static void
mouse_write (void)
{
        mouse_apply();
        gnome_config_set_int ("/Desktop/Mouse/acceleration", mouse_acceleration);
        gnome_config_set_int ("/Desktop/Mouse/threshold", mouse_thresh);
        gnome_config_set_bool ("/Desktop/Mouse/right-to-left", mouse_rtol);
        gnome_config_sync ();
}
static void
mouse_revert (void)
{
        mouse_read();
        mouse_apply();
        GTK_ADJUSTMENT (thresh_adjust)->value = mouse_thresh;
        GTK_ADJUSTMENT (accel_adjust)->value = mouse_acceleration;
        gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON ((mouse_rtol
                                                         ? lbutton
                                                         : rbutton)), TRUE);
        gtk_adjustment_changed (GTK_ADJUSTMENT (thresh_adjust));
        gtk_adjustment_changed (GTK_ADJUSTMENT (accel_adjust));
}

/* Run when the left- or right-handed radiobutton is clicked.  */
static void
button_toggled (GtkWidget *widget, gpointer data)
{
        mouse_rtol = (int) data;
        capplet_widget_state_changed(CAPPLET_WIDGET (capplet), TRUE);
}

/* Run when a scale widget is manipulated.  */
static void
scale_moved (GtkAdjustment *adj, gpointer data)
{
        int *value = (int *) data;
        *value = adj->value;
        capplet_widget_state_changed(CAPPLET_WIDGET (capplet), TRUE);
}
static void
make_scale (char *title, char *max_title, char *min_title,
	    GtkObject *adjust, int *update_var, GtkWidget *table, int row)
{
        GtkWidget *scale, *low, *high, *ttl;

        ttl = gtk_label_new (title);

        gtk_misc_set_alignment (GTK_MISC (ttl), 0.0, 0.5);
        gtk_table_attach (GTK_TABLE (table), ttl,
                          0, 3, row, row + 1,
                          GTK_FILL | GTK_SHRINK,
                          GTK_FILL | GTK_SHRINK,
                          0, 0);
        gtk_widget_show (ttl);

        low = gtk_label_new (min_title);
        gtk_misc_set_alignment (GTK_MISC (low), 0.0, 0.5);
        gtk_table_attach (GTK_TABLE (table), low,
                          0, 1, row + 1, row + 2,
                          GTK_FILL | GTK_SHRINK,
                          GTK_FILL | GTK_SHRINK,
                          0, 0);
        gtk_widget_show (low);

        scale = gtk_hscale_new (GTK_ADJUSTMENT (adjust));
        gtk_range_set_update_policy (GTK_RANGE (scale), GTK_UPDATE_CONTINUOUS);
        gtk_scale_set_digits (GTK_SCALE (scale), 0);
        gtk_scale_set_draw_value (GTK_SCALE (scale), 0);
        gtk_signal_connect (GTK_OBJECT (adjust), "value_changed",
                            GTK_SIGNAL_FUNC (scale_moved),
                            (gpointer) update_var);
        /*   gtk_widget_set_usize (scale, 200, -1); */
        gtk_table_attach (GTK_TABLE (table), scale,
                          1, 2, row + 1, row + 2,
                          GTK_EXPAND | GTK_FILL | GTK_SHRINK,
                          GTK_FILL | GTK_SHRINK,
                          0, 0);
        gtk_widget_show (scale);

        high = gtk_label_new (max_title);
        gtk_misc_set_alignment (GTK_MISC (high), 0.0, 0.5);
        gtk_table_attach (GTK_TABLE (table), high,
                          2, 3, row + 1, row + 2,
                          GTK_FILL | GTK_SHRINK,
                          GTK_FILL | GTK_SHRINK,
                          0, 0);
        gtk_widget_show (high);
}

static void
mouse_setup (void)
{
        GtkWidget *vbox, *frame, *hbox, *table, *sep;
        
        GtkWidget *vbox_main;
        gchar *filename;

        vbox_main = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);



        /* Icon. */
        filename = gnome_pixmap_file ("gnome-mouse.png");
        if (filename) {
                GtkWidget *pixmap;

                hbox = gtk_hbox_new (FALSE, GNOME_PAD_SMALL);
                pixmap = gnome_pixmap_new_from_file (filename);
                gtk_box_pack_start (GTK_BOX (hbox), pixmap, FALSE, FALSE, 0);
                gtk_box_pack_start (GTK_BOX (vbox_main), hbox, FALSE, FALSE, 0);
        }   

        /* Mouse buttons */
        hbox = gtk_hbox_new (FALSE, GNOME_PAD_SMALL);
        gtk_container_set_border_width (GTK_CONTAINER (hbox), GNOME_PAD);
        gtk_box_pack_start (GTK_BOX (vbox_main), hbox, TRUE, TRUE, 0);
        capplet = capplet_widget_new();
        frame = gtk_frame_new (_("Mouse buttons"));
        gtk_box_pack_start (GTK_BOX (hbox), frame, FALSE, FALSE, 0);
        gtk_widget_show (frame);

        vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);
        gtk_container_set_border_width (GTK_CONTAINER (vbox), GNOME_PAD_SMALL);
        gtk_container_add (GTK_CONTAINER (frame), vbox);
        gtk_widget_show (vbox);

        lbutton = gtk_radio_button_new_with_label (NULL, _("Left handed"));
        rbutton = gtk_radio_button_new_with_label (gtk_radio_button_group (GTK_RADIO_BUTTON (lbutton)),
                                                   _("Right handed"));
        gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON ((mouse_rtol
                                                         ? lbutton
                                                         : rbutton)), TRUE);

        gtk_signal_connect (GTK_OBJECT (capplet), "help",
                            GTK_SIGNAL_FUNC (mouse_help), NULL);
        gtk_signal_connect (GTK_OBJECT (capplet), "try",
                            GTK_SIGNAL_FUNC (mouse_apply), NULL);
        gtk_signal_connect (GTK_OBJECT (capplet), "revert",
                            GTK_SIGNAL_FUNC (mouse_revert), NULL);
        gtk_signal_connect (GTK_OBJECT (capplet), "ok",
                            GTK_SIGNAL_FUNC (mouse_write), NULL);
        gtk_signal_connect (GTK_OBJECT (capplet), "cancel",
                            GTK_SIGNAL_FUNC (mouse_revert), NULL);
        gtk_signal_connect (GTK_OBJECT (lbutton), "clicked",
                            GTK_SIGNAL_FUNC (button_toggled),
                            (gpointer) 1);
        gtk_signal_connect (GTK_OBJECT (rbutton), "clicked",
                            GTK_SIGNAL_FUNC (button_toggled),
                            (gpointer) 0);
        gtk_box_pack_start (GTK_BOX (vbox), lbutton, FALSE, FALSE, 0);
        gtk_box_pack_start (GTK_BOX (vbox), rbutton, FALSE, FALSE, 0);

        /* Mouse motion */

        frame = gtk_frame_new (_("Mouse motion"));
        gtk_box_pack_start (GTK_BOX (hbox), frame, TRUE, TRUE, 0);

        table = gtk_table_new (5, 3, FALSE);
        gtk_container_set_border_width (GTK_CONTAINER (table), GNOME_PAD_SMALL);
        gtk_table_set_row_spacings (GTK_TABLE (table), GNOME_PAD_SMALL);
        gtk_table_set_col_spacings (GTK_TABLE (table), GNOME_PAD_SMALL);
        gtk_container_add (GTK_CONTAINER (frame), table);

        accel_adjust = gtk_adjustment_new (mouse_acceleration, 0, 2 * MAX_ACCEL + 1, 1, 1, 1);
        make_scale (_("Acceleration"), _("Fast"), _("Slow"),
                    accel_adjust, &mouse_acceleration, table, 0);

        sep = gtk_hseparator_new ();
        gtk_table_attach (GTK_TABLE (table), sep,
                          0, 3, 2, 3,
                          GTK_FILL | GTK_SHRINK,
                          GTK_FILL | GTK_SHRINK,
                          0, 0);

        thresh_adjust = gtk_adjustment_new (mouse_thresh, 0, MAX_THRESH, 1, 1, 1);
        make_scale (_("Threshold"), _("Large"), _("Small"),
                    thresh_adjust, &mouse_thresh, table, 3);

        /* Done */
  
        gtk_container_add (GTK_CONTAINER (capplet), vbox_main);
        gtk_widget_show_all (capplet);
}

int
main (int argc, char **argv)
{
        GnomeClient *client = NULL;
        GnomeClientFlags flags;
        gchar *session_args[3];
        int token, init_results;

				setlocale(LC_ALL, "");
        bindtextdomain (PACKAGE, GNOMELOCALEDIR);
        textdomain (PACKAGE);

        init_results = gnome_capplet_init("mouse-properties", VERSION,
                                          argc, argv, NULL, 0, NULL);

	if (init_results < 0) {
                g_warning (_("an initialization error occurred while "
			   "starting 'mouse-properties-capplet'.\n"
                           "aborting...\n"));
                exit (1);
	}

	client = gnome_master_client ();
	flags = gnome_client_get_flags(client);

	if (flags & GNOME_CLIENT_IS_CONNECTED) {
		token = gnome_startup_acquire_token("GNOME_MOUSE_PROPERTIES",
				                  gnome_client_get_id(client));

		if (token) {
			session_args[0] = argv[0];
			session_args[1] = "--init-session-settings";
			session_args[2] = NULL;
			gnome_client_set_priority (client, 20);
			gnome_client_set_restart_style (client, 
							GNOME_RESTART_ANYWAY);
			gnome_client_set_restart_command (client, 2, 
							  session_args);
		}
		else 
			gnome_client_set_restart_style (client, 
							GNOME_RESTART_NEVER);

                gnome_client_flush (client);
        }
	else
		token = 1;

        mouse_read ();	

        if(token) 
                mouse_apply ();

	if (init_results != 1) {
		mouse_setup ();
	        capplet_gtk_main ();
	}
        return 0;
}
