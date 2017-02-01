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

#include "gmix.h"
#include "prefs.h"

/*
 * Gnome info:
 */
GtkWidget *configwin;

/******************** General Preferences Page ********************/

mixerprefs prefs={FALSE,FALSE,TRUE,TRUE};

static void retrieve_prefs(GtkWidget *page)
{
	mixerprefs *prefs_copy;

	prefs_copy = gtk_object_get_data(GTK_OBJECT(page), PREFS_COPY);
	prefs = *prefs_copy;
}

static void destroy_prefs(GtkWidget *page)
{
	mixerprefs *prefs_copy;

	prefs_copy = gtk_object_get_data(GTK_OBJECT(page), PREFS_COPY);
	g_free(prefs_copy);
}

static void bool_changed_cb(GtkWidget *widget, gboolean *data)
{
	if( *data )
		*data = FALSE;
	else        
		*data = TRUE;
	gnome_property_box_changed(GNOME_PROPERTY_BOX(configwin));
}

static GtkWidget *general_page(void)
{
	GtkWidget *start_frame, *gui_frame;
	GtkWidget *ubervbox;
	GtkWidget *vbox, *init_start, *menu_hide, *temp;
	mixerprefs *prefs_copy;

	ubervbox = gtk_vbox_new(TRUE, 0);

	prefs_copy = g_malloc(sizeof(mixerprefs));
	g_assert(prefs_copy);
	*prefs_copy = prefs;
	gtk_object_set_data(GTK_OBJECT(ubervbox), PREFS_COPY, prefs_copy);

	start_frame = gtk_frame_new(_("On startup"));
	gtk_container_border_width(GTK_CONTAINER(start_frame), GNOME_PAD_SMALL);
    
	vbox = gtk_vbox_new(TRUE, 0);

	/* Set on start */
	init_start = gtk_check_button_new_with_label(_("Restore saved mixer levels on startup"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(init_start), \
				     prefs_copy->set_mixer_on_start);
	gtk_signal_connect(GTK_OBJECT(init_start), "clicked",
			   GTK_SIGNAL_FUNC(bool_changed_cb), &prefs_copy->set_mixer_on_start);

	menu_hide = gtk_check_button_new_with_label(_("Hide menu on start"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(menu_hide), \
				     prefs_copy->hide_menu);
	gtk_signal_connect(GTK_OBJECT(menu_hide), "clicked",
			   GTK_SIGNAL_FUNC(bool_changed_cb), &prefs_copy->hide_menu);

	gtk_box_pack_start_defaults(GTK_BOX(vbox), init_start);
	/*	gtk_box_pack_start_defaults(GTK_BOX(vbox), menu_hide);*/

	gtk_widget_show_all(vbox);


	gtk_container_add(GTK_CONTAINER(start_frame), vbox);
	gtk_container_add(GTK_CONTAINER(ubervbox), start_frame);

	gui_frame = gtk_frame_new(_("GUI"));
	gtk_container_border_width(GTK_CONTAINER(gui_frame), GNOME_PAD_SMALL);

	vbox = gtk_vbox_new(TRUE, 0);
	temp = gtk_check_button_new_with_label(_("Use mixer icons"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(temp), \
				     prefs_copy->use_icons);
	gtk_signal_connect(GTK_OBJECT(temp), "clicked",
			   GTK_SIGNAL_FUNC(bool_changed_cb), &prefs_copy->use_icons);
	gtk_box_pack_start_defaults(GTK_BOX(vbox), temp);
	
	
	temp = gtk_check_button_new_with_label(_("Use mixer labels"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(temp), \
				     prefs_copy->use_labels);
	gtk_signal_connect(GTK_OBJECT(temp), "clicked",
			   GTK_SIGNAL_FUNC(bool_changed_cb), &prefs_copy->use_labels);
	gtk_box_pack_start_defaults(GTK_BOX(vbox), temp);
	
	
	gtk_container_add(GTK_CONTAINER(gui_frame), vbox);
	
	gtk_container_add(GTK_CONTAINER(ubervbox), gui_frame);
	
	gtk_widget_show_all(ubervbox);

	return ubervbox;
}

/******************** Label Name Page ********************/

/*
 * This routine extracts the label for a single slider of a sound
 * device.  This routine simply extracts the label from the
 * corresponding GtkEntry widget and attaches it to the sound channel
 * data structure.
 *
 * @param entry A pointer to the GtkEntry widget.
 *
 * @param user_data Unused.
 */
static void labels_retrieve_one_slider(gpointer entry, gpointer user_data)
{
	channel_info *channel;

	channel = gtk_object_get_user_data(GTK_OBJECT(entry));
	channel->user_title =
		strdup(gtk_entry_get_text(GTK_ENTRY(entry)));
}

/*
 * This routine extracts all of the labels for a single sound device
 * (a single page in the labels notebook). This routine simply calls a
 * helper routine once for each label in the notebook page.
 *
 * @param page A pointer to the notebook label page for a single sound
 * device.
 *
 * @param user_data Unused.
 */
static void labels_retrieve_one_page(gpointer page, gpointer user_data)
{
	GSList *entry_list;
	
	entry_list =
		gtk_object_get_user_data(GTK_OBJECT(GTK_NOTEBOOK_PAGE(page)));
	g_assert(entry_list);
	g_slist_foreach(entry_list, labels_retrieve_one_slider, NULL);
}

/*
 * This routine extracts all of the labels from the preferences
 * dialog.  Labels are not written into the channel data structures as
 * they are changed to make it easy to cancel the preferences dialog.
 * Only when the apply signal is set are the labels extracted and the
 * channel data structures updated.  This routine simply calls a
 * helper routine once for each page in the label notebook.
 *
 * @param notebook A pointer to the notebook containing all the
 * labels.  Within this notebook is a page of labels for each sound
 * device.
 */
static void labels_retrieve_all(GtkWidget *notebook)
{
	g_list_foreach(GTK_NOTEBOOK(notebook)->children,
		       labels_retrieve_one_page, NULL);
}

/********************/

/*
 * This routine destroys all of the non-widget data attached to the
 * label page for a single sound device (a single page in the labels
 * notebook).  Each page has a GSList threading all of the GtkEntry
 * objects on it for easy access.  The list itself must be freed, but
 * all of the list data are widgets that are part of the page layout
 * and will be automatically freed when the page is freed.
 *
 * @param page A pointer to the notebook label page for a single sound
 * device.
 *
 * @param user_data Unused.
 */
static void labels_destroy_one_page(gpointer page, gpointer user_data)
{
	GSList *entry_list;
	
	entry_list =
		gtk_object_get_user_data(GTK_OBJECT(GTK_NOTEBOOK_PAGE(page)));
	g_assert(entry_list);
	g_slist_free(entry_list);
	/* No need to free the data.  All items are gtk widgets */
	/* that will be destroyed when the window is destroyed. */
}

/*
 * This routine destroys all non-widget data attached to the labels
 * page of the preferences dialog.  This routine simply calls a helper
 * routine once for each page in the label notebook.
 *
 * @param notebook A pointer to the notebook containing all the
 * labels.  Within this notebook is a page of labels for each sound
 * device.
 */
static void labels_destroy_all(GtkWidget *notebook)
{
	g_list_foreach(GTK_NOTEBOOK(notebook)->children,
		       labels_destroy_one_page, NULL);
}

/********************/

/*
 * This routine resets the label for a single slider of a sound
 * device.  This routine simply extracts the original label from the
 * sound channel data structure and overwrites anything in the
 * corresponding GtkEntry widget.  Since all the data is kept in the
 * GtkEntry widgets until OK or Apply is clicked, this is all that's
 * necessary.
 *
 * @param entry A pointer to the GtkEntry widget.
 *
 * @param user_data Unused.
 */
static void labels_reset_one_slider(gpointer entry, gpointer user_data)
{
	channel_info *channel;

	channel = gtk_object_get_user_data(GTK_OBJECT(entry));
	gtk_entry_set_text(GTK_ENTRY(entry), channel->title);
}

/*
 * This routine resets all of the labels for a single sound device (a
 * single page in the labels notebook).  This routine simply calls a
 * helper routine once for each label in the notebook page.
 *
 * @param notebook A pointer to the notebook containing all the
 * labels.  Within this notebook is a page of labels for each sound
 * device.
 *
 * @param user_data Unused.
 */
static void labels_reset_page_cb(GtkWidget *button, gpointer page)
{
	GSList *entry_list;

	g_assert(page);
	entry_list = gtk_object_get_user_data(GTK_OBJECT(page));
	g_assert(entry_list);
	g_slist_foreach(entry_list, labels_reset_one_slider, NULL);
}

/********************/

/*
 * This routine sets the property box "changed" flag any time a label
 * is edited.  This routine ignores all its arguments and simply grabs
 * the global variable for the preferences window and goes for it.
 *
 * @param entry Unused
 *
 * @param user_data Unused.
 */
static void labels_changed_cb(GtkWidget *entry, gboolean *user_data)
{
	gnome_property_box_changed(GNOME_PROPERTY_BOX(configwin));
}

/*
 * This routine creates the label for a single slider of a sound
 * device.  This routine creates one row in a table, adding a single
 * label containing the fixed system value, and an entry box
 * containing the current label.  It also strings the entry widget
 * onto a list for easy access later.
 *
 * @param channel_p A pointer to the channel information for a single
 * slider of a sound device.
 *
 * @param user_data A pointer to arguments passed in from the calling
 * routine.  These arguments contain pointers to the table, the list
 * of entries, and the current row number.
 *
 * Note: This table could have been built as a vbox containing one
 * hbox per row, where the hbox contained the label and the entry.  I
 * wasn't sure that all the labels would line up with this method and
 * I knew that they would line up with a table, this the table.  Its a
 * little harder having to pass in the row, but it looks good.
 */
static void labels_create_one_slider(gpointer channel_p, gpointer user_data)
{
	channel_info *channel = (channel_info *)channel_p;
	label_create_args_t *args = user_data;
	GtkWidget *label, *entry;
	gint cc;

	cc=channel->channel;
	if (!(channel->device->devmask & (1<<cc)))
		return;

	label = gtk_label_new(channel->title);
	gtk_table_attach(GTK_TABLE(args->table), label,
			 0, 1, args->row, args->row+1, LABEL_TABLE_OPTS);

	entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entry),
			   channel->user_title);
	gtk_table_attach(GTK_TABLE(args->table), entry,
			 1, 2, args->row, args->row+1, LABEL_TABLE_OPTS);
	gtk_signal_connect(GTK_OBJECT(entry), "changed",
			   GTK_SIGNAL_FUNC(labels_changed_cb), NULL);
	gtk_object_set_user_data(GTK_OBJECT(entry), channel);
	args->entry_list = g_slist_prepend(args->entry_list, entry);
	args->row++;
}

/*
 * Whip up a page showing all the labels for a single sound device.
 * This page is presented as a vertically scrolling pane containing a
 * table of channels and their labels, and button to reset all the
 * labels to their default values.  Each row in the table is a single
 * sound channel giving its "name" (gtk label) and its current value
 * (gtk entry).  All of the gtk entries are strung onto a list for
 * easy access later in the callback routines.  A clist would have
 * been convenient for this task except that it doesn't support
 * editable items.
 *
 * @param device A pointer to the device_info data structure for this
 * sound device.
 *
 * @param parent A pointer to the parent notebook that contains a
 * single page for each sound device.
 */
static void label_one_device_config(gpointer device, gpointer parent)
{
	GtkWidget *ubervbox, *label, *table, *bbox, *button, *scrlwin;
	device_info *info = (device_info *)device;
	label_create_args_t args;

	/* The page: A scrolled viewport (and a label) */
	label = gtk_label_new(info->info.name);
	scrlwin = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrlwin),
				       GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
	gtk_notebook_append_page(GTK_NOTEBOOK(parent), scrlwin, label);
	ubervbox = gtk_vbox_new(FALSE, 5);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrlwin),
					      ubervbox);

	/* Put a table into the window.  This will expand dynamically
	  if there are more than eight rows for any given sound card. */
	table = gtk_table_new(2, 8, FALSE);
	gtk_container_add(GTK_CONTAINER(ubervbox), table);

	/* Fill the table. */
	args.row = 0;
	args.table = table;
	args.entry_list = NULL;
	g_list_foreach(info->channels, labels_create_one_slider, &args);
	gtk_object_set_user_data(GTK_OBJECT(scrlwin), args.entry_list);

	/* Add the label reset button */
	bbox = gtk_hbutton_box_new();
	gtk_container_add(GTK_CONTAINER(ubervbox), bbox);
	button = gtk_button_new_with_label(_("Defaults"));
	gtk_container_add(GTK_CONTAINER(bbox), button);
	gtk_signal_connect(GTK_OBJECT(button), "clicked",
			   GTK_SIGNAL_FUNC(labels_reset_page_cb), scrlwin);

}

/*
 * Whip up a notebook page for the preferences panel containing all
 * the current labels.  This is arranged as a sub-notebook (within the
 * preferences notebook) containing one page per device.  This routine
 * calls a helper routine once for each sound device, and that creates
 * the individual pages in the notebook.
 *
 * @return GtkWidget * A pointer to the newly created notebook of
 * editable labels.  This notebook should be installed within the
 * preferences notebook.
 */
static GtkWidget *labels_create_page(void)
{
	GtkWidget *notebook;

	notebook = gtk_notebook_new();
	gtk_container_set_border_width(GTK_CONTAINER(notebook), 5);
	g_list_foreach(devices, label_one_device_config, notebook);
	return(notebook);
}

/******************** Preferences Window ********************/

static void apply_cb(GtkWidget *widget, gint page_num, void *data)
{       
        GtkWidget *page;

	if (page_num != -1)
		return;

	/* Copy out the user's new preferences */
	page = gtk_object_get_data(GTK_OBJECT(configwin), PREFS_PAGE);
	retrieve_prefs(page);

	/* Copy out the user's new labels */
	page = gtk_object_get_data(GTK_OBJECT(configwin), LABELS_PAGE);
	labels_retrieve_all(page);

        put_gui_config();

	gmix_build_slidernotebook();
}

static void cancel_cb(GtkWidget *widget, void *data)
{       
        GtkWidget *page;

	/* Find and free the copy of the preferences data */
	page = gtk_object_get_data(GTK_OBJECT(configwin), PREFS_PAGE);
	destroy_prefs(page);

	page = gtk_object_get_data(GTK_OBJECT(configwin), LABELS_PAGE);
	labels_destroy_all(page);

	gtk_widget_destroy (configwin);
        configwin=NULL;
}


void prefs_make_window(void)
{
        GtkWidget *label, *page;

	if (!configwin) {
	        configwin=gnome_property_box_new();
		gtk_widget_realize(configwin);

		label = gtk_label_new(_("Preferences"));
		page = general_page();
		gtk_object_set_data(GTK_OBJECT(configwin), PREFS_PAGE, page);
		gnome_property_box_append_page(
			GNOME_PROPERTY_BOX(configwin), page, label);
		
		label = gtk_label_new(_("Labels"));
		page = labels_create_page();
		gtk_object_set_data(GTK_OBJECT(configwin), LABELS_PAGE, page);
		gnome_property_box_append_page(
			GNOME_PROPERTY_BOX(configwin), page, label);
		
		gtk_signal_connect(GTK_OBJECT(configwin), "apply",
				   GTK_SIGNAL_FUNC(apply_cb), NULL);
		gtk_signal_connect(GTK_OBJECT(configwin), "destroy",
				   GTK_SIGNAL_FUNC(cancel_cb), NULL);
		gtk_signal_connect(GTK_OBJECT(configwin), "delete_event",
				   GTK_SIGNAL_FUNC(cancel_cb), NULL);
		gtk_signal_connect(GTK_OBJECT(configwin), "help",
				   GTK_SIGNAL_FUNC(help_cb), NULL);

		gtk_widget_show_all(configwin);	
    
	};
};

void get_gui_config(void)
{
	prefs.set_mixer_on_start=gnome_config_get_bool("/gmix/startup/init=true");
	prefs.hide_menu=gnome_config_get_bool("/gmix/gui/menu=false");
	prefs.use_icons=gnome_config_get_bool("/gmix/gui/icons=true");
	prefs.use_labels=gnome_config_get_bool("/gmix/gui/labels=true");
}

void put_gui_config(void)
{
	gnome_config_set_bool("/gmix/startup/init",prefs.set_mixer_on_start);
	gnome_config_set_bool("/gmix/gui/menu",prefs.hide_menu);
	gnome_config_set_bool("/gmix/gui/icons",prefs.use_icons);
	gnome_config_set_bool("/gmix/gui/labels",prefs.use_labels);
}

