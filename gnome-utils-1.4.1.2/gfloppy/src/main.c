/* main.c
 *
 * Copyright (C) 1999 Red Hat, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <fcntl.h>
#include <gnome.h>
#include <libgnomeui/gnome-window-icon.h>
#include <glade/glade.h>
#include <unistd.h>
#include "gfloppy.h"
#include <sys/types.h>
#include <errno.h>
#include "progress.h"
#define NUM_DEVICES_TO_CHECK 4

extern int errno;

static GFloppy floppy;
static GladeXML *xml;
static GtkWidget *toplevel = NULL;

/* Only needed if more then one device exists */
static GList *valid_devices = NULL;

static gint
option_menu_get_history (GtkOptionMenu *option_menu)
{
	GtkWidget *active_widget;

	g_return_val_if_fail (GTK_IS_OPTION_MENU (option_menu), -1);

	active_widget = gtk_menu_get_active (GTK_MENU (option_menu->menu));

	if (active_widget)
		return g_list_index (GTK_MENU_SHELL (option_menu->menu)->children,
				     active_widget);
	else
		return -1;
}

static void
devices_option_activated (GtkWidget *menu_item, gchar *device)
{
	g_free (floppy.device);
	g_free (floppy.mdevice);

	floppy.device = g_strdup (device);

	/* FIXME: what the heck is /dev/fd2 and /dev/fd3 ?? */
	if (strncmp (floppy.device, "/dev/fd1", strlen ("/dev/fd1")) == 0)
		floppy.mdevice = g_strdup ("b:");
	else
		floppy.mdevice = g_strdup ("a:");
}

static void
start_format (void)
{
	pipe (floppy.message);

	floppy.pid = fork ();
	if (floppy.pid < 0) {
		g_error ("unable to fork ().\nPlease free up some resources and try again.\n");
		_exit (1);
	}
	if (floppy.pid == 0) {
		/* child */
		close (floppy.message [0]);
		close (STDERR_FILENO);
		close (STDOUT_FILENO);
		format_floppy (&floppy);
		_exit (0);
	}
	close (floppy.message [1]);

	fcntl (floppy.message [0], F_SETFL, O_NONBLOCK);
	setup_progress_and_run (&floppy, toplevel);
}

#include <stdlib.h>

static void
init_commands (void)
{
	floppy.mke2fs_cmd = NULL;
	floppy.mformat_cmd = NULL;
	floppy.badblocks_cmd = NULL;

	putenv ("PATH=$PATH:/sbin:/usr/sbin:/usr:/usr/bin");
	floppy.mke2fs_cmd    = gnome_is_program_in_path ("mke2fs");
	floppy.mformat_cmd   = gnome_is_program_in_path ("mformat");
	floppy.badblocks_cmd = gnome_is_program_in_path ("mbadblocks");

	if (floppy.mke2fs_cmd == NULL) {
		g_print ("Warning:  Unable to locate mke2fs.  Please confirm it is installed and try again\n");
		exit (1);
	}

}

static void
init_devices (void)
{
	GtkWidget *device_option;
	GtkWidget *device_entry;
	gchar *msg = NULL;
	gint ok_devices_present = 0;

	/* First check --device arg */
	if (floppy.device != NULL) {
		GFloppyStatus status;
		status = test_floppy_device (floppy.device);
		switch (status) {
		case GFLOPPY_NO_DEVICE:
			msg = g_strdup_printf (_("Unable to open the device %s, formatting cannot continue."), floppy.device);
			break;
		case GFLOPPY_DEVICE_DISCONNECTED:
			msg = g_strdup_printf (_("The device %s is disconnected.\nPlease attach device to continue."), floppy.device);
			break;
		case GFLOPPY_INVALID_PERMISSIONS:
			msg = g_strdup_printf (_("You do not have the proper permissions to write to %s, formatting will not be possible.\nContact your system administrator about getting write permissions."), floppy.device);
			break;
		default:
			break;
		}
		if (msg) {
			GtkWidget *toplevel = glade_xml_get_widget (xml, "toplevel");
			gtk_widget_set_sensitive (toplevel, FALSE);
			gnome_dialog_run_and_close (GNOME_DIALOG (gnome_error_dialog (msg)));
			exit (1);
		}
		/* Device is okay */
		ok_devices_present = 1;
	} else {
		GFloppyStatus status[NUM_DEVICES_TO_CHECK];
		gint i;
		char device[32];

		for (i = 0; i < NUM_DEVICES_TO_CHECK; i++) {
			sprintf(device,"/dev/fd%d", i);
			status[i] = test_floppy_device (device);
			if (status[i] == GFLOPPY_DEVICE_OK)
				ok_devices_present++;
		}
		if (ok_devices_present == 0) {
			GtkWidget *toplevel;

			/* Lets assume that there's only a problem with /dev/fd0 */
			switch (status[0]) {
			case GFLOPPY_NO_DEVICE:
				msg = g_strdup_printf (_("Unable to open the device %s, formatting cannot continue."), "/dev/fd0");
				break;
			case GFLOPPY_DEVICE_DISCONNECTED:
				msg = g_strdup_printf (_("The device %s is disconnected.\nPlease attach device to continue."), "/dev/fd0");
				break;
			case GFLOPPY_INVALID_PERMISSIONS:
				msg = g_strdup_printf (_("You do not have the proper permissions to write to %s, formatting will not be possible.\nContact your system administrator about getting write permissions."), "/dev/fd0");
				break;
			default:
				g_assert_not_reached ();
			}
			toplevel = glade_xml_get_widget (xml, "toplevel");
			gtk_widget_set_sensitive (toplevel, FALSE);
			gnome_dialog_run_and_close (GNOME_DIALOG (gnome_error_dialog (msg)));
			exit (1);
		} else if (ok_devices_present == 1) {
			for (i = 0; i < NUM_DEVICES_TO_CHECK; i++) {
				if (status[i] == GFLOPPY_DEVICE_OK) {
					floppy.device = g_strdup_printf ("/dev/fd%d", i);
					break;
				}
			}
		} else {
			for (i = 0; i < NUM_DEVICES_TO_CHECK; i++) {
				if (status[i] == GFLOPPY_DEVICE_OK) {
					valid_devices = g_list_append (valid_devices, g_strdup_printf ("/dev/fd%d", i));
					if (floppy.device == NULL)
						floppy.device = g_strdup_printf ("/dev/fd%d", i);
				}
			}
		}
	}

	/* FIXME: what the heck is /dev/fd2 and /dev/fd3 ?? */
	if (strncmp (floppy.device, "/dev/fd1", strlen ("/dev/fd1")) == 0)
		floppy.mdevice = g_strdup ("b:");
	else
		floppy.mdevice = g_strdup ("a:");

	/* set up the UI */
	device_option = glade_xml_get_widget (xml, "device_option");
	device_entry = glade_xml_get_widget (xml, "device_entry");

	if (ok_devices_present == 1) {
		gtk_widget_hide (device_option);
		gtk_widget_show (device_entry);
		gtk_entry_set_text (GTK_ENTRY (device_entry), floppy.device);
	} else {
		GtkWidget *menu;
		GList *list;

		gtk_widget_show (device_option);
		gtk_widget_hide (device_entry);
		gtk_option_menu_remove_menu (GTK_OPTION_MENU (device_option));
		menu = gtk_menu_new ();
		for (list = valid_devices; list; list = list->next) {
			GtkWidget *menu_item;
			menu_item = gtk_menu_item_new_with_label ((gchar *)list->data);
			gtk_signal_connect (GTK_OBJECT (menu_item), "activate", GTK_SIGNAL_FUNC (devices_option_activated), list->data);
			gtk_widget_show (menu_item);
			gtk_menu_append (GTK_MENU (menu), menu_item);
		}
		gtk_widget_show_all (menu);
		gtk_option_menu_set_menu (GTK_OPTION_MENU (device_option), menu);
		gtk_option_menu_set_history (GTK_OPTION_MENU (device_option), 0);
	}
}

static void
set_floppy_extended_device (void)
{
	switch (floppy.size) {
	case 0:
		floppy.extended_device = g_strdup_printf ("%sH1440", floppy.device);
		break;
	case 1:
		floppy.extended_device = g_strdup_printf ("%sh1200", floppy.device);
		break;
	case 2:
		floppy.extended_device = g_strdup_printf ("%sD720", floppy.device);
		break;
	case 3:
		floppy.extended_device = g_strdup_printf ("%sd360", floppy.device);
		break;
	default:
		g_assert_not_reached ();
	}
}

gint
on_toplevel_delete_event (GtkWidget *w, GdkEventAny *e, gpointer data)
{
	toplevel = NULL;
	return TRUE;
}

int
main (int argc, char *argv[])
{
	GtkWidget *ext2_entry;
	GtkWidget *type_option;
	GtkWidget *icon_frame;
	GtkWidget *icon;
	GtkWidget *quick_format_button;
	GtkWidget *device_combo;
	gint button;

	struct poptOption gfloppy_opts[] = {
		{"device", '\0', POPT_ARG_STRING, NULL, 0, NULL, NULL},
		{NULL, '\0', 0, NULL, 0, NULL, NULL}
	};

	floppy.device = NULL;
	gfloppy_opts[0].arg = &(floppy.device);
	gfloppy_opts[0].descrip = _("The device to format");
	gfloppy_opts[0].argDescrip = _("DEVICE");

	bindtextdomain (PACKAGE, GNOMELOCALEDIR);
	textdomain (PACKAGE);

	gnome_init_with_popt_table ("gfloppy", VERSION, argc, argv,
				    gfloppy_opts, 0, NULL);
	gnome_window_icon_set_default_from_file (GNOME_ICONDIR"/mc/i-floppy.png");
	init_commands ();

	/* Now we can set up glade */
	glade_gnome_init();
        xml = glade_xml_new (GLADEDIR "/gfloppy.glade", NULL);
	if (xml == NULL)
		xml = glade_xml_new ("gfloppy.glade", NULL);
	if (xml == NULL)
		g_error ("Cannot load/find floppy.glade");
	init_devices ();
	toplevel = glade_xml_get_widget (xml, "toplevel");
	quick_format_button = glade_xml_get_widget (xml, "quick_format_button");
	icon_frame = glade_xml_get_widget (xml, "icon_frame");
	type_option = glade_xml_get_widget (xml, "type_option");
	toplevel = glade_xml_get_widget (xml, "toplevel");
	device_combo = glade_xml_get_widget (xml, "device_combo");
	glade_xml_signal_autoconnect (xml);
	
	icon = gnome_pixmap_new_from_file (GNOME_ICONDIR"/mc/i-floppy.png");
	gtk_container_add (GTK_CONTAINER (icon_frame), icon);
	gtk_widget_show (icon);
	/* We do this to get around a bug in libglade.  Ideally we won't need
	 * to do this in the future. */
	gnome_dialog_append_button_with_pixmap (GNOME_DIALOG (toplevel), _("Format"), GNOME_STOCK_PIXMAP_SAVE);
	gnome_dialog_append_button (GNOME_DIALOG (toplevel), GNOME_STOCK_BUTTON_CLOSE);
	gnome_dialog_append_button (GNOME_DIALOG (toplevel), GNOME_STOCK_BUTTON_HELP);


	if (floppy.mformat_cmd == NULL) {
		/* We don't have mtools.  Allow ext2 only. */
		ext2_entry = glade_xml_get_widget (xml, "ext2_entry");

		gtk_widget_hide (type_option);
		gtk_widget_show (ext2_entry);
	}

	while ((button = gnome_dialog_run (GNOME_DIALOG (toplevel))) >= 0) {
		GtkWidget *density_option;
		if (button == 1 ) 
			break;

		if (button == 2) {
			GnomeHelpMenuEntry ref = {"gfloppy", "index.html"};
			gnome_help_display (NULL, &ref);
			continue;
		}
		density_option = glade_xml_get_widget (xml, "density_option");
		g_assert (density_option != NULL);
		g_assert (quick_format_button != NULL);

		gtk_widget_set_sensitive (toplevel, FALSE);

		if (floppy.mformat_cmd) {
			/* Check to see which one is selected. */
			floppy.type = option_menu_get_history (GTK_OPTION_MENU (type_option));
		} else {
			floppy.type = GFLOPPY_E2FS;
		}
		floppy.size = option_menu_get_history (GTK_OPTION_MENU (density_option));
		set_floppy_extended_device ();
		floppy.quick_format = GTK_TOGGLE_BUTTON (quick_format_button)->active;

		start_format ();
		if (!toplevel)
			break;
		gtk_widget_set_sensitive (toplevel, TRUE);
		if (gnome_dialog_run (GNOME_DIALOG (gnome_question_dialog_parented (_("Format another floppy?"),
										    NULL, NULL,
										    GTK_WINDOW (toplevel)))) == 1)
			break;
	}
	return 0;
}
