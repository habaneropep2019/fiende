/*
 *  Copyright (C) 2000 Helix Code Inc.
 *
 *  Authors: Michael Zucchi <notzed@helixcode.com>
 *
 *  A system print-copies widget.
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public License
 *  as published by the Free Software Foundation; either version 2 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* Must include these two first */
#include "config.h"
#include <libgnomeprint/gnome-print-i18n.h>

#include <gtk/gtkobject.h>
#include <gtk/gtkwidget.h>
#include <gtk/gtkcontainer.h>
#include <libgnomeui/gnome-uidefs.h>
#include <libgnomeui/gnome-dialog.h>
#include <libgnomeui/gnome-pixmap.h>

#include <libgnomeprint/gnome-print-copies.h>

enum {COPIES_SET, LAST_SIGNAL};

struct _GnomePrintCopies {
	GtkVBox vbox;
  
	guint changing : 1;

	GtkWidget *copies;
	GtkWidget *collate;
	GtkWidget *collate_image;

	GtkAccelGroup *accel_group;
};

struct _GnomePrintCopiesClass {
	GtkVBoxClass parent_class;

	void (* copies_set) (GnomePrintCopies * gpc, gint copies, gboolean collate);
};

static void gnome_print_copies_class_init (GnomePrintCopiesClass *class);
static void gnome_print_copies_init       (GnomePrintCopies *gspaper);
static void gnome_print_copies_destroy	  (GtkObject *object);

/* again, these images may be here temporarily */

/* XPM */
static char * collate_xpm[] = {
"65 35 6 1",
" 	c None",
".	c #000000",
"+	c #020202",
"@	c #FFFFFF",
"#	c #010101",
"$	c #070707",
"           ..++++++++++++++++..              ..++++++++++++++++..",
"           ..++++++++++++++++..              ..++++++++++++++++..",
"           ..@@@@@@@@@@@@@@@@..              ..@@@@@@@@@@@@@@@@..",
"           ..@@@@@@@@@@@@@@@@..              ..@@@@@@@@@@@@@@@@..",
"           ++@@@@@@@@@@@@@@@@..              ++@@@@@@@@@@@@@@@@..",
"           ++@@@@@@@@@@@@@@@@..              ++@@@@@@@@@@@@@@@@..",
"           ++@@@@@@@@@@@@@@@@..              ++@@@@@@@@@@@@@@@@..",
"           ++@@@@@@@@@@@@@@@@..              ++@@@@@@@@@@@@@@@@..",
"           ++@@@@@@@@@@@@@@@@..              ++@@@@@@@@@@@@@@@@..",
"           ++@@@@@@@@@@@@@@@@..              ++@@@@@@@@@@@@@@@@..",
"..+++++++++##++++++$@@@@@@@@@..   ..+++++++++##++++++$@@@@@@@@@..",
"..+++++++++##+++++#+@@@@@@@@@..   ..+++++++++##+++++#+@@@@@@@@@..",
"..@@@@@@@@@@@@@@@@++@@@@@@@@@..   ..@@@@@@@@@@@@@@@@++@@@@@@@@@..",
"..@@@@@@@@@@@@@@@@++@@@..@@@@..   ..@@@@@@@@@@@@@@@@++@@@..@@@@..",
"..@@@@@@@@@@@@@@@@++@@.@@.@@@..   ..@@@@@@@@@@@@@@@@++@@.@@.@@@..",
"..@@@@@@@@@@@@@@@@++@@@@@.@@@..   ..@@@@@@@@@@@@@@@@++@@@@@.@@@..",
"..@@@@@@@@@@@@@@@@++@@@@.@@@@..   ..@@@@@@@@@@@@@@@@++@@@@.@@@@..",
"..@@@@@@@@@@@@@@@@++@@@.@@@@@..   ..@@@@@@@@@@@@@@@@++@@@.@@@@@..",
"..@@@@@@@@@@@@@@@@++@@.@@@@@@..   ..@@@@@@@@@@@@@@@@++@@.@@@@@@..",
"..@@@@@@@@@@@@@@@@++@@....@@@..   ..@@@@@@@@@@@@@@@@++@@....@@@..",
"..@@@@@@@@@@@@@@@@++@@@@@@@@@..   ..@@@@@@@@@@@@@@@@++@@@@@@@@@..",
"..@@@@@@@@@@@@@@@@++@@@@@@@@@..   ..@@@@@@@@@@@@@@@@++@@@@@@@@@..",
"..@@@@@@@@@@@@@@@@++@@@@@@@@@..   ..@@@@@@@@@@@@@@@@++@@@@@@@@@..",
"..@@@@@@@@@@@.@@@@.............   ..@@@@@@@@@@@.@@@@.............",
"..@@@@@@@@@@..@@@@.............   ..@@@@@@@@@@..@@@@.............",
"..@@@@@@@@@@@.@@@@..              ..@@@@@@@@@@@.@@@@..           ",
"..@@@@@@@@@@@.@@@@..              ..@@@@@@@@@@@.@@@@..           ",
"..@@@@@@@@@@@.@@@@..              ..@@@@@@@@@@@.@@@@..           ",
"..@@@@@@@@@@@.@@@@..              ..@@@@@@@@@@@.@@@@..           ",
"..@@@@@@@@@@...@@@..              ..@@@@@@@@@@...@@@..           ",
"..@@@@@@@@@@@@@@@@..              ..@@@@@@@@@@@@@@@@..           ",
"..@@@@@@@@@@@@@@@@..              ..@@@@@@@@@@@@@@@@..           ",
"..@@@@@@@@@@@@@@@@..              ..@@@@@@@@@@@@@@@@..           ",
"....................              ....................           ",
"....................              ....................           "};

/* XPM */
static char * nocollate_xpm[] = {
"65 35 6 1",
" 	c None",
".	c #000000",
"+	c #FFFFFF",
"@	c #020202",
"#	c #010101",
"$	c #070707",
"           ....................              ....................",
"           ....................              ....................",
"           ..++++++++++++++++..              ..++++++++++++++++..",
"           ..++++++++++++++++..              ..++++++++++++++++..",
"           @@++++++++++++++++..              @@++++++++++++++++..",
"           @@++++++++++++++++..              @@++++++++++++++++..",
"           @@++++++++++++++++..              @@++++++++++++++++..",
"           @@++++++++++++++++..              @@++++++++++++++++..",
"           @@++++++++++++++++..              @@++++++++++++++++..",
"           @@++++++++++++++++..              @@++++++++++++++++..",
"..@@@@@@@@@##@@@@@@$+++++++++..   ..@@@@@@@@@##@@@@@@$+++++++++..",
"..@@@@@@@@@##@@@@@#@+++++++++..   ..@@@@@@@@@##@@@@@#@+++++++++..",
"..++++++++++++++++@@+++++++++..   ..++++++++++++++++@@+++++++++..",
"..++++++++++++++++@@++++.++++..   ..++++++++++++++++@@+++..++++..",
"..++++++++++++++++@@+++..++++..   ..++++++++++++++++@@++.++.+++..",
"..++++++++++++++++@@++++.++++..   ..++++++++++++++++@@+++++.+++..",
"..++++++++++++++++@@++++.++++..   ..++++++++++++++++@@++++.++++..",
"..++++++++++++++++@@++++.++++..   ..++++++++++++++++@@+++.+++++..",
"..++++++++++++++++@@++++.++++..   ..++++++++++++++++@@++.++++++..",
"..++++++++++++++++@@+++...+++..   ..++++++++++++++++@@++....+++..",
"..++++++++++++++++@@+++++++++..   ..++++++++++++++++@@+++++++++..",
"..++++++++++++++++@@+++++++++..   ..++++++++++++++++@@+++++++++..",
"..++++++++++++++++@@+++++++++..   ..++++++++++++++++@@+++++++++..",
"..+++++++++++.++++.............   ..++++++++++..++++.............",
"..++++++++++..++++.............   ..+++++++++.++.+++.............",
"..+++++++++++.++++..              ..++++++++++++.+++..           ",
"..+++++++++++.++++..              ..+++++++++++.++++..           ",
"..+++++++++++.++++..              ..++++++++++.+++++..           ",
"..+++++++++++.++++..              ..+++++++++.++++++..           ",
"..++++++++++...+++..              ..+++++++++....+++..           ",
"..++++++++++++++++..              ..++++++++++++++++..           ",
"..++++++++++++++++..              ..++++++++++++++++..           ",
"..++++++++++++++++..              ..++++++++++++++++..           ",
"....................              ....................           ",
"....................              ....................           "};


static GtkVBoxClass *parent_class;
static guint gpc_signals[LAST_SIGNAL] = {0};

guint
gnome_print_copies_get_type (void)
{
	static guint copies_type = 0;
	if (!copies_type) {
		GtkTypeInfo copies_info = {
			"GnomePrintCopies",
			sizeof (GnomePrintCopies),
			sizeof (GnomePrintCopiesClass),
			(GtkClassInitFunc) gnome_print_copies_class_init,
			(GtkObjectInitFunc) gnome_print_copies_init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL,
			(GtkClassInitFunc) NULL
		};
		copies_type = gtk_type_unique (gtk_vbox_get_type (), &copies_info);
	}
	return copies_type;
}

static void
gnome_print_copies_class_init (GnomePrintCopiesClass *class)
{
	GtkObjectClass *object_class;
  
	object_class = (GtkObjectClass *) class;

	parent_class = gtk_type_class (gtk_vbox_get_type ());

	gpc_signals[COPIES_SET] = gtk_signal_new ("copies_set",
						GTK_RUN_LAST,
						object_class->type,
						GTK_SIGNAL_OFFSET (GnomePrintCopiesClass, copies_set),
						gtk_marshal_NONE__INT_INT,
						GTK_TYPE_NONE,
						2, GTK_TYPE_INT, GTK_TYPE_INT);
	gtk_object_class_add_signals (object_class, gpc_signals, LAST_SIGNAL);

	object_class->destroy = gnome_print_copies_destroy;
}

static void
collate_toggled (GtkWidget *widget, GnomePrintCopies *gpc)
{
	gint copies;
	gboolean collate;

	copies = gtk_spin_button_get_value_as_int ((GtkSpinButton *) gpc->copies);
	collate = ((GtkToggleButton *) gpc->collate)->active;

	gnome_pixmap_load_xpm_d(GNOME_PIXMAP (gpc->collate_image),
				(char **)(collate?collate_xpm:nocollate_xpm));

	if (gpc->changing) return;

	gtk_signal_emit ((GtkObject *) gpc, gpc_signals[COPIES_SET], copies, collate);
}

static void
copies_changed (GtkWidget *widget, GnomePrintCopies *gpc)
{
	gint copies;
	gboolean collate;

	copies = gtk_spin_button_get_value_as_int ((GtkSpinButton *) gpc->copies);
	collate = ((GtkToggleButton *) gpc->collate)->active;

	if (gpc->changing) return;

	gtk_signal_emit ((GtkObject *) gpc, gpc_signals[COPIES_SET], copies, collate);
}

static void
gnome_print_copies_init (GnomePrintCopies *gpc)
{
	GtkWidget *table, *label, *frame;
	GtkLabel *collate_label;
	GtkAdjustment *adj;
	guint label_key;
	guint collate_key;

	gpc->accel_group = gtk_accel_group_new ();
	frame = gtk_frame_new(_("Copies"));
	gtk_container_add(GTK_CONTAINER(gpc), frame);
	gtk_widget_show(frame);

	table = gtk_table_new(2, 2, FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(table), GNOME_PAD);
	gtk_container_add(GTK_CONTAINER(frame), GTK_WIDGET (table));
	gtk_widget_show(table);

	label = gtk_label_new("");
	label_key = gtk_label_parse_uline (GTK_LABEL (label),
					   _("N_umber of copies:"));
	gtk_widget_show(label);
	gtk_table_attach_defaults((GtkTable *)table, label, 0, 1, 0, 1);

	adj = (GtkAdjustment *)gtk_adjustment_new(1, 1, 1000, 1.0, 10.0, 10.0);
	gpc->copies = gtk_spin_button_new(adj, 1.0, 0);
	gtk_widget_show(gpc->copies);
	gtk_table_attach_defaults((GtkTable *)table, gpc->copies, 1, 2, 0, 1);
	if (label_key != GDK_VoidSymbol) {
		gtk_widget_add_accelerator
			(GTK_WIDGET (&GTK_SPIN_BUTTON (gpc->copies)->entry),
			 "grab_focus", gpc->accel_group, label_key,
			 GDK_MOD1_MASK, 0);
	}

	gpc->collate_image = gnome_pixmap_new_from_xpm_d(nocollate_xpm);
	gtk_widget_show(gpc->collate_image);
	gtk_table_attach_defaults((GtkTable *)table, gpc->collate_image, 0, 1, 1, 2);

	gpc->collate = gtk_check_button_new_with_label("");
	collate_label = GTK_LABEL (GTK_BIN (gpc->collate)->child);
	collate_key = gtk_label_parse_uline (collate_label, _("_Collate"));
	if (collate_key != GDK_VoidSymbol) {
		gtk_widget_add_accelerator (gpc->collate, "clicked",
					   gpc->accel_group, collate_key,
					   GDK_MOD1_MASK, 0);
	}

	gtk_widget_show(gpc->collate);
	gtk_table_attach_defaults((GtkTable *)table, gpc->collate, 1, 2, 1, 2);

	gtk_signal_connect (GTK_OBJECT (gpc->copies), "changed",
			    GTK_SIGNAL_FUNC (copies_changed), gpc);
	gtk_signal_connect (GTK_OBJECT (gpc->collate), "toggled", 
			    GTK_SIGNAL_FUNC (collate_toggled), gpc);
}

/**
 * gnome_print_copies_new:
 *
 * Create a new GnomePrintCopies widget.
 * 
 * Return value: A new GnomePrintCopies widget.
 **/

GtkWidget *
gnome_print_copies_new (void)
{
  return GTK_WIDGET (gtk_type_new (gnome_print_copies_get_type ()));
}

void
gnome_print_copies_bind_editable_enters (GnomePrintCopies * gpc, GnomeDialog * dialog)
{
	g_return_if_fail (gpc != NULL);
	g_return_if_fail (GNOME_IS_PRINT_COPIES (gpc));
	g_return_if_fail (dialog != NULL);
	g_return_if_fail (GNOME_IS_DIALOG (dialog));

	gnome_dialog_editable_enters (dialog, GTK_EDITABLE (gpc->copies));
}

void
gnome_print_copies_bind_accel_group (GnomePrintCopies * gpc, GtkWindow * window)
{
	g_return_if_fail (gpc != NULL);
	g_return_if_fail (GNOME_IS_PRINT_COPIES (gpc));
	g_return_if_fail (window != NULL);
	g_return_if_fail (GTK_IS_WINDOW (window));

	gtk_window_add_accel_group (window, gpc->accel_group);
}

static void
gnome_print_copies_destroy (GtkObject *object)
{
	GnomePrintCopies *gpc;
	
	g_return_if_fail (object != NULL);
	g_return_if_fail (GNOME_IS_PRINT_COPIES (object));
	
	gpc = GNOME_PRINT_COPIES (object);
	if (gpc->accel_group)
		gtk_accel_group_unref (gpc->accel_group);
  
	if (GTK_OBJECT_CLASS (parent_class)->destroy)
		(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

/**
 * gnome_print_copies_set_copies:
 * @gpc: An initialised GnomePrintCopies widget.
 * @copies: New number of copies.
 * @collate: New collation status.
 * 
 * Set the number of copies and collation sequence to be displayed.
 **/

void
gnome_print_copies_set_copies (GnomePrintCopies *gpc, gint copies, gint collate)
{
	g_return_if_fail (gpc != NULL);
	g_return_if_fail (GNOME_IS_PRINT_COPIES (gpc));

	gpc->changing = TRUE;

	gtk_toggle_button_set_active ((GtkToggleButton *) gpc->collate, collate);

	gpc->changing = FALSE;

	gtk_spin_button_set_value ((GtkSpinButton *) gpc->copies, copies);
}

/**
 * gnome_print_copies_get_copies:
 * @gpc: An initialised GnomePrintCopies widget.
 * 
 * Retrieve the number of copies set
 *
 * Return value: Number of copies set
 **/

gint
gnome_print_copies_get_copies (GnomePrintCopies *gpc)
{
	g_return_val_if_fail (gpc != NULL, 0);
	g_return_val_if_fail (GNOME_IS_PRINT_COPIES (gpc), 0);

	return gtk_spin_button_get_value_as_int ((GtkSpinButton *) gpc->copies);
}

/**
 * gnome_print_copies_get_collate:
 * @gpc: An initialised GnomePrintCopies widget.
 * 
 * Retrieve the collation status
 *
 * Return value: Collation status
 **/

gboolean
gnome_print_copies_get_collate (GnomePrintCopies *gpc)
{
	g_return_val_if_fail (gpc != NULL, FALSE);
	g_return_val_if_fail (GNOME_IS_PRINT_COPIES (gpc), FALSE);

	return GTK_TOGGLE_BUTTON (gpc->collate)->active?TRUE:FALSE;
}






