/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/* Author: Jaka Mocnik <jaka.mocnik@kiss.uni-lj.si>
 * Integrated with keyboard bell applet by Justin Maurer <justin@helixcode.com>
 * (C) 2000 Helix Code, Inc.
 * Based on gnome-core/desktop-properties/property-keyboard.c
 */
#include <config.h>
#include "capplet-widget.h"
#include <stdio.h>
#include <stdarg.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <X11/X.h>
#include <locale.h>
#ifdef HAVE_X11_EXTENSIONS_XF86MISC_H
#include <X11/extensions/xf86misc.h>
#endif

#include "gnome.h"

static gint keyboard_rate;
static gint keyboard_delay;
static gint keyboard_repeat;
static gint click_volume;
static gint click_on_keypress;
static XKeyboardState kbdstate;
static XKeyboardControl kbdcontrol;
static GtkWidget *capplet;
static GtkWidget *rbutton, *cbutton, *rscale, *dscale, *vscale;
static GtkObject *vol_adjust, *rate_adjust, *del_adjust;

static gint bell_percent;
static gint bell_pitch;
static gint bell_duration;
static GtkObject *vadj, *padj, *dadj;
static GtkWidget *vscale, *pscale, *dscale;

#ifdef HAVE_X11_EXTENSIONS_XF86MISC_H
static XF86MiscKbdSettings kbdsettings;
#endif
static void bell_revert(void);
static void bell_write(void);

static void
keyboard_read(void)
{
	gboolean repeat_default, click_default;
        gint event_base_return, error_base_return;

	keyboard_rate = gnome_config_get_int("/Desktop/Keyboard/rate=-");
	keyboard_delay = gnome_config_get_int("/Desktop/Keyboard/delay=-1");
	keyboard_repeat = gnome_config_get_bool_with_default ("/Desktop/Keyboard/repeat=true", &repeat_default);
        click_volume = gnome_config_get_int("/Desktop/Keyboard/clickvolume=-1");
        click_on_keypress = gnome_config_get_bool_with_default("/Desktop/Keyboard/click=false", &click_default);

	XGetKeyboardControl(GDK_DISPLAY(), &kbdstate);

	if (repeat_default)
		keyboard_repeat = kbdstate.global_auto_repeat;

	if (keyboard_rate == -1 || keyboard_delay == -1) {
#ifdef HAVE_X11_EXTENSIONS_XF86MISC_H
		if (XF86MiscQueryExtension(GDK_DISPLAY(), &event_base_return, &error_base_return) == True) {
                        XF86MiscGetKbdSettings(GDK_DISPLAY(), &kbdsettings);
                        keyboard_rate = kbdsettings.rate;
                        keyboard_delay = kbdsettings.delay;
                } else {
                        keyboard_rate = 5;
                        keyboard_delay = 500;
                }
#else
		/* FIXME: how to get the keyboard speed on non-xf86? */
		keyboard_rate = 5;
		keyboard_delay = 500;
#endif
	}

	if (click_default)
		click_on_keypress =  (kbdstate.key_click_percent == 0);

	if (click_volume == -1)
		click_volume = kbdstate.key_click_percent;
}

static void
keyboard_help(void)
{
    GnomeHelpMenuEntry help_entry= {"control-center",
    "peripherals.html#GCCKEY"};
    gnome_help_display (NULL, &help_entry);
}
static void
keyboard_apply(void)
{
        int event_base_return, error_base_return;

	if (keyboard_repeat) {
		XAutoRepeatOn(GDK_DISPLAY());
#ifdef HAVE_X11_EXTENSIONS_XF86MISC_H
		if (XF86MiscQueryExtension(GDK_DISPLAY(), &event_base_return, &error_base_return) == True) {
                        kbdsettings.rate =  keyboard_rate;
                        kbdsettings.delay = keyboard_delay;
                        XF86MiscSetKbdSettings(GDK_DISPLAY(), &kbdsettings);
                } else {
                        XAutoRepeatOff(GDK_DISPLAY());
                }
#endif
	} else {
		XAutoRepeatOff(GDK_DISPLAY());
	}

	kbdcontrol.key_click_percent = click_on_keypress ? click_volume : 0;
	XChangeKeyboardControl(GDK_DISPLAY(), KBKeyClickPercent, &kbdcontrol);
}

static void
keyboard_write(void)
{
        bell_write ();
        keyboard_apply();
	gnome_config_set_bool("/Desktop/Keyboard/repeat", keyboard_repeat);
	gnome_config_set_int("/Desktop/Keyboard/delay", keyboard_delay);
	gnome_config_set_int("/Desktop/Keyboard/rate", keyboard_rate);
	gnome_config_set_bool("/Desktop/Keyboard/click", click_on_keypress);
	gnome_config_set_int("/Desktop/Keyboard/clickvolume", click_volume);
	gnome_config_sync ();
}

static void
keyboard_revert (void)
{
        bell_revert ();
        keyboard_read();
        keyboard_apply();
        GTK_ADJUSTMENT (vol_adjust)->value = click_volume;
        GTK_ADJUSTMENT (rate_adjust)->value = keyboard_rate;
        GTK_ADJUSTMENT (del_adjust)->value = keyboard_delay;
        gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (rbutton), keyboard_repeat);
        gtk_toggle_button_set_state (GTK_TOGGLE_BUTTON (cbutton), click_on_keypress);
        gtk_adjustment_changed (GTK_ADJUSTMENT (vol_adjust));
        gtk_adjustment_changed (GTK_ADJUSTMENT (rate_adjust));
        gtk_adjustment_changed (GTK_ADJUSTMENT (del_adjust));
}

/* Run when a toggle button is clicked.  */
static void
button_toggled (GtkWidget *widget, gpointer data)
{
        gint sens;

        if(widget == cbutton) {
                if(GTK_TOGGLE_BUTTON(widget)->active) {
                        click_on_keypress = TRUE;
                        sens = TRUE;
                }
                else {
                        click_on_keypress = FALSE;
                        sens = FALSE;
                }
                gtk_widget_set_sensitive(vscale, sens);
        }
        else if(widget == rbutton) {
                if(GTK_TOGGLE_BUTTON(widget)->active) {
                        keyboard_repeat = TRUE;
                        sens = TRUE;
                }
                else {
                        keyboard_repeat = FALSE;
                        sens = FALSE;
                }
                gtk_widget_set_sensitive(rscale, sens);
                gtk_widget_set_sensitive(dscale, sens);
        }

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

static GtkWidget *
make_scale (char *title, GtkObject *adjust, int *update_var, GtkWidget *table, int row)
{
        GtkWidget *scale, *ttl;

        ttl = gtk_label_new (title);

        gtk_misc_set_alignment (GTK_MISC (ttl), 0.0, 0.5);
        gtk_table_attach (GTK_TABLE (table), ttl,
                          0, 1, row, row + 1,
                          GTK_FILL | GTK_SHRINK,
                          GTK_FILL | GTK_SHRINK,
                          0, 0);

        scale = gtk_hscale_new (GTK_ADJUSTMENT (adjust));
        gtk_range_set_update_policy (GTK_RANGE (scale), GTK_UPDATE_CONTINUOUS);
        gtk_scale_set_digits (GTK_SCALE (scale), 0);
        gtk_signal_connect (GTK_OBJECT (adjust), "value_changed",
                            GTK_SIGNAL_FUNC (scale_moved),
                            (gpointer) update_var);

        gtk_table_attach (GTK_TABLE (table), scale,
                          1, 2, row, row + 1,
                          GTK_EXPAND | GTK_FILL | GTK_SHRINK,
                          GTK_FILL | GTK_SHRINK,
                          0, 0);

        return scale;
}

static void
bell_test(GtkWidget *widget, void *data)
{
        gint save_percent;
        gint save_pitch;
        gint save_duration;

        XGetKeyboardControl(GDK_DISPLAY(), &kbdstate);

        save_percent = kbdstate.bell_percent;
        save_pitch = kbdstate.bell_pitch;
        save_duration = kbdstate.bell_duration;

        kbdcontrol.bell_percent = bell_percent;
        kbdcontrol.bell_pitch = bell_pitch;
        kbdcontrol.bell_duration = bell_duration;
        XChangeKeyboardControl(GDK_DISPLAY(), KBBellPercent | KBBellPitch | KBBellDuration, &kbdcontrol);

        XBell (gdk_display,0);

        kbdcontrol.bell_percent = save_percent;
        kbdcontrol.bell_pitch = save_pitch;
        kbdcontrol.bell_duration = save_duration;
        XChangeKeyboardControl(GDK_DISPLAY(), KBBellPercent | KBBellPitch | KBBellDuration, &kbdcontrol);
}

static GtkWidget *
bell_setup(void)
{
        GtkWidget *frame;
        GtkWidget *table;
        GtkWidget *test, *hbox, *inner_hbox, *volume;

        frame = gtk_frame_new(_("Keyboard bell"));
        table = gtk_table_new (4, 2, FALSE);
        gtk_container_set_border_width (GTK_CONTAINER (table), GNOME_PAD_SMALL);
        gtk_table_set_row_spacings (GTK_TABLE (table), GNOME_PAD_SMALL);
        gtk_table_set_col_spacings (GTK_TABLE (table), GNOME_PAD_SMALL);
        gtk_container_add (GTK_CONTAINER (frame), table);

        vadj = gtk_adjustment_new(bell_percent, 0, 100, 1, 1, 0);
        vscale = make_scale(_("Volume"), vadj, &bell_percent, table, 1);

        padj = gtk_adjustment_new(bell_pitch, 0, 2000, 1, 1, 0);
        pscale = make_scale (_("Pitch (Hz)"), padj, &bell_pitch, table, 2);

        dadj = gtk_adjustment_new(bell_duration, 0, 500, 1, 1, 0);
        dscale = make_scale(_("Duration (ms)"), dadj, &bell_duration, table, 3);        /* Finished */

        test = gtk_button_new ();
        hbox = gtk_hbox_new (FALSE, 0);
        gtk_box_pack_start (GTK_BOX (hbox), test, FALSE, FALSE, 0);
        gtk_table_attach_defaults (GTK_TABLE(table), hbox, 0, 2, 4, 5);

        gtk_signal_connect (GTK_OBJECT (test), "clicked",
                            GTK_SIGNAL_FUNC (bell_test), NULL);
        inner_hbox = gtk_hbox_new (FALSE, 0);
        volume = gnome_stock_pixmap_widget (test, GNOME_STOCK_PIXMAP_VOLUME);
        gtk_box_pack_start (GTK_BOX (inner_hbox), volume, FALSE, FALSE, 0);
        gtk_box_pack_start (GTK_BOX (inner_hbox), gtk_label_new (_("Test")), FALSE, FALSE, 8);
        gtk_container_add (GTK_CONTAINER (test), inner_hbox);

        return frame;
}


static void
keyboard_setup (void)
{
        GtkWidget *vbox;
        GtkWidget *frame, *table, *bell;
#if 0
        GtkWidget *label, *entry, *hbox;
#endif
        vbox = gtk_vbox_new (FALSE, GNOME_PAD_SMALL);

        capplet = capplet_widget_new();
        frame = gtk_frame_new (_("Keyboard repeat rate"));
        gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);

        table = gtk_table_new (4, 2, FALSE);
        gtk_container_set_border_width (GTK_CONTAINER (table), GNOME_PAD_SMALL);
        gtk_table_set_row_spacings (GTK_TABLE (table), GNOME_PAD_SMALL);
        gtk_table_set_col_spacings (GTK_TABLE (table), GNOME_PAD_SMALL);
        gtk_container_add (GTK_CONTAINER (frame), table);


        rbutton = gtk_check_button_new_with_label (_("Enable auto-repeat"));
        gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(rbutton), keyboard_repeat);
        gtk_table_attach (GTK_TABLE (table), rbutton,
                          0, 2, 0, 1,
                          GTK_FILL | GTK_SHRINK,
                          GTK_FILL | GTK_SHRINK,
                          0, 0);

        rate_adjust = gtk_adjustment_new (keyboard_rate, 0, 255, 1, 1, 0);
        rscale = make_scale (_("Repeat rate"), rate_adjust, &keyboard_rate, table, 1);

        del_adjust = gtk_adjustment_new (keyboard_delay, 0, 10000, 1, 1, 0);
        dscale = make_scale (_("Repeat Delay"), del_adjust, &keyboard_delay, table, 2);

        frame = gtk_frame_new (_("Keyboard click"));
        gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
        table = gtk_table_new (2, 2, FALSE);
        gtk_container_add (GTK_CONTAINER (frame), table);
        gtk_container_set_border_width (GTK_CONTAINER (table), GNOME_PAD_SMALL);
        gtk_table_set_row_spacings (GTK_TABLE (table), GNOME_PAD_SMALL);
        gtk_table_set_col_spacings (GTK_TABLE (table), GNOME_PAD_SMALL);


        cbutton = gtk_check_button_new_with_label (_("Click on keypress"));
        gtk_toggle_button_set_state(GTK_TOGGLE_BUTTON(cbutton), click_on_keypress);
        gtk_table_attach (GTK_TABLE (table), cbutton,
                          0, 2, 0, 1,
                          GTK_FILL | GTK_SHRINK,
                          GTK_FILL | GTK_SHRINK,
                          0, 0);

        vol_adjust = gtk_adjustment_new (click_volume, 0, 100, 1, 1, 0);
        vscale = make_scale (_("Click volume"), vol_adjust, &click_volume, table, 1);

	bell = bell_setup ();
        gtk_box_pack_start (GTK_BOX (vbox), bell, FALSE, FALSE, 0);

#if 0        
        frame = gtk_frame_new(_("Preview"));
        gtk_box_pack_start (GTK_BOX (vbox), frame, FALSE, FALSE, 0);
        hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
        gtk_container_add(GTK_CONTAINER(frame), hbox);

        label = gtk_label_new(_("Test settings"));
        gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
        gtk_box_pack_start(GTK_BOX (hbox), label, FALSE, FALSE, 0);

        entry = gtk_entry_new();
        gtk_box_pack_start(GTK_BOX (hbox), entry, TRUE, TRUE, 0);
#endif
        gtk_signal_connect (GTK_OBJECT (capplet), "help",
                            GTK_SIGNAL_FUNC (keyboard_help), NULL);
        gtk_signal_connect (GTK_OBJECT (capplet), "try",
                            GTK_SIGNAL_FUNC (keyboard_apply), NULL);
        gtk_signal_connect (GTK_OBJECT (capplet), "revert",
                            GTK_SIGNAL_FUNC (keyboard_revert), NULL);
        gtk_signal_connect (GTK_OBJECT (capplet), "ok",
                            GTK_SIGNAL_FUNC (keyboard_write), NULL);
        gtk_signal_connect (GTK_OBJECT (cbutton), "clicked",
                            GTK_SIGNAL_FUNC (button_toggled), NULL);
        gtk_signal_connect (GTK_OBJECT (rbutton), "clicked",
                            GTK_SIGNAL_FUNC (button_toggled), NULL);

        gtk_widget_show_all (vbox);
        gtk_container_add (GTK_CONTAINER (capplet), vbox);
        gtk_widget_show (capplet);
}

static void bell_read(void)
{
        bell_percent = gnome_config_get_int("/Desktop/Bell/percent=-1");
        bell_pitch = gnome_config_get_int("/Desktop/Bell/pitch=-1");
        bell_duration = gnome_config_get_int("/Desktop/Bell/duration=-1");

        XGetKeyboardControl(GDK_DISPLAY(), &kbdstate);

        if (bell_percent == -1) {
                bell_percent = kbdstate.bell_percent;
        }
        if (bell_pitch == -1) {
                bell_pitch = kbdstate.bell_pitch;
        }
        if (bell_duration == -1) {
                bell_duration = kbdstate.bell_duration;
        }
}

static void bell_apply(void)
{
        kbdcontrol.bell_percent = bell_percent;
        kbdcontrol.bell_pitch = bell_pitch;
        kbdcontrol.bell_duration = bell_duration;
        XChangeKeyboardControl(GDK_DISPLAY(), KBBellPercent | KBBellPitch | KBBellDuration, &kbdcontrol);
}

static void bell_help(void)
{
  gchar *tmp;

  tmp = gnome_help_file_find_file ("users-guide", "gccmm.html#GCC-KEYBELL");
  if (tmp) {
    gnome_help_goto(0, tmp);
    g_free(tmp);
  } else {
          GtkWidget *mbox;

          mbox = gnome_message_box_new(_("No help is available/installed for these settings. Please make sure you\nhave the GNOME User's Guide installed on your system."),
                                       GNOME_MESSAGE_BOX_ERROR,
                                       _("Close"), NULL);

          gtk_widget_show(mbox);
  }
}

static void bell_write(void)
{
        bell_apply();
        gnome_config_set_int("/Desktop/Bell/percent", bell_percent);
        gnome_config_set_int("/Desktop/Bell/pitch", bell_pitch);
        gnome_config_set_int("/Desktop/Bell/duration", bell_duration);
        gnome_config_sync ();
}

static void
bell_revert (void)
{
        bell_read();
        bell_apply();
        GTK_ADJUSTMENT (vadj)->value = bell_percent;
        GTK_ADJUSTMENT (padj)->value = bell_pitch;
        GTK_ADJUSTMENT (dadj)->value = bell_duration;
        gtk_adjustment_changed (GTK_ADJUSTMENT (vadj));
        gtk_adjustment_changed (GTK_ADJUSTMENT (padj));
        gtk_adjustment_changed (GTK_ADJUSTMENT (dadj));
}

int
main (int argc, char **argv)
{
        GnomeClient *client;
        GnomeClientFlags flags;
        gchar *session_args[3];
        int token, init_results;

				setlocale(LC_ALL, "");
        bindtextdomain (PACKAGE, GNOMELOCALEDIR);
        textdomain (PACKAGE);
	init_results = gnome_capplet_init("keyboard-properties", VERSION,
					  argc, argv, NULL, 0, NULL);
	if (init_results < 0) {
                g_warning (_("an initialization error occurred while "
			   "starting 'keyboard-properties-capplet'.\n"
                           "aborting...\n"));
                exit (1);
	}

	client = gnome_master_client ();
	flags = gnome_client_get_flags(client);

	if (flags & GNOME_CLIENT_IS_CONNECTED) {
		token = gnome_startup_acquire_token("GNOME_KEYBOARD_PROPERTIES",
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

        keyboard_read ();	
	bell_read ();

        if (token) {
                keyboard_apply ();
		bell_apply ();
        }
	if (init_results != 1) {
		keyboard_setup ();
	        capplet_gtk_main ();
	}
        return 0;
}
