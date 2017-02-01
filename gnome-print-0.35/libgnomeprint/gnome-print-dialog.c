/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 *  Copyright (C) 2000 Helix Code Inc.
 *
 *  Authors: Michael Zucchi <notzed@helixcode.com>
 *           Chema Celorio <chema@celorio.com>
 *
 *  A system print dialogue.
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

#include <time.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtkobject.h>
#include <gtk/gtkwidget.h>
#include <libgnomeui/gnome-dialog.h>
#include <libgnomeui/gnome-stock.h>
#include <libgnomeui/gnome-dateedit.h>

#include <libgnome/gnome-defs.h>
#include <libgnomeprint/gnome-printer.h>
#include <libgnomeprint/gnome-printer-dialog.h>
#include <libgnomeprint/gnome-print-copies.h>

#include "gnome-print-dialog.h"

struct _GnomePrintDialog {
	GnomeDialog dialog;

	void *priv;

	GtkWidget *printer;
	GtkWidget *copies;
	GnomePrintRangeType range_type;
	GtkWidget *range;
	GtkWidget *range_container;
	GtkAccelGroup *range_accel_group;
};

struct _GnomePrintDialogClass {
	GnomeDialogClass parent_class;
};

static void gnome_print_dialog_class_init (GnomePrintDialogClass *class);
static void gnome_print_dialog_init       (GnomePrintDialog      *gspaper);

static GnomeDialog *parent_class;

typedef struct _GnomePrintDialoguePrivate {
	int flags;

	GtkWidget *current;
	GtkWidget *all;
	GtkWidget *range;
	GtkWidget *selection;
	GtkWidget *range_start;
	GtkWidget *range_end;
} Private;

guint
gnome_print_dialog_get_type (void)
{
	static guint select_paper_type = 0;
  
	if (!select_paper_type) {
		GtkTypeInfo select_paper_info = {
			"GnomePrintDialog",
			sizeof (GnomePrintDialog),
			sizeof (GnomePrintDialogClass),
			(GtkClassInitFunc) gnome_print_dialog_class_init,
			(GtkObjectInitFunc) gnome_print_dialog_init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL
		};
		select_paper_type = gtk_type_unique (gnome_dialog_get_type (), &select_paper_info);
	}
  
	return select_paper_type;
}

static void
gnome_print_dialog_destroy (GtkObject *object)
{
	GnomePrintDialog *gpd;
	
	g_return_if_fail (object != NULL);
	g_return_if_fail (GNOME_IS_PRINT_DIALOG (object));
	
	gpd = GNOME_PRINT_DIALOG (object);
	if (gpd->range_accel_group)
		gtk_accel_group_unref (gpd->range_accel_group);
  
	if (GTK_OBJECT_CLASS (parent_class)->destroy)
		(* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

static void
gnome_print_dialog_finalize (GtkObject *object)
{
	GnomePrintDialog *gpd = GNOME_PRINT_DIALOG(object);

	g_free(gpd->priv);

	/* all widgets should be free'd by their containers? */
	GTK_OBJECT_CLASS (parent_class)->finalize (object);
}


static void
gnome_print_dialog_class_init (GnomePrintDialogClass *class)
{
	GtkObjectClass *object_class;
	
	object_class = (GtkObjectClass *) class;
	parent_class = gtk_type_class (gnome_dialog_get_type ());

	object_class->destroy = gnome_print_dialog_destroy;
	object_class->finalize = gnome_print_dialog_finalize;
}

static void
gnome_print_dialog_init (GnomePrintDialog *gpd)
{
	gpd->printer = gnome_printer_widget_new ();

	if (gpd->printer) {
		gtk_box_pack_start (GTK_BOX(GNOME_DIALOG (gpd)->vbox), GTK_WIDGET (gpd->printer), FALSE, FALSE, 3);
		gtk_widget_show (gpd->printer);
	} else
		g_warning ("There aren't any printers installed\n");

	
	gpd->range_type = GNOME_PRINT_RANGETYPE_NONE;
	gpd->priv = g_malloc0(sizeof(Private));
	gpd->range_accel_group = NULL;
}

#define PAD 4
static void
init_body (GnomePrintDialog *gpd, int flags)
{
	GtkWidget *hbox;
	GnomePrinterWidget *gpw = GNOME_PRINTER_WIDGET (gpd->printer);

	if (gpd->copies || gpd->range_container)
		return;

	if (flags & (GNOME_PRINT_DIALOG_RANGE|GNOME_PRINT_DIALOG_COPIES)) {
		hbox = gtk_hbox_new(FALSE, 3);
		gtk_box_pack_start(GTK_BOX(GNOME_DIALOG (gpd)->vbox), GTK_WIDGET (hbox), FALSE, FALSE, 3);

		if (flags & GNOME_PRINT_DIALOG_COPIES) {
			GnomePrintCopies *gpc;
			
			gpd->copies = gnome_print_copies_new ();
			gtk_box_pack_end(GTK_BOX(hbox), gpd->copies, FALSE, TRUE, 3);
			gpc = GNOME_PRINT_COPIES (gpd->copies);
			gnome_print_copies_bind_editable_enters (gpc, GNOME_DIALOG (gpd));
			gnome_print_copies_bind_accel_group (gpc, GTK_WINDOW (gpd));
		}
		
		if (flags & GNOME_PRINT_DIALOG_RANGE) {
			gpd->range_container = gtk_frame_new(_("Print Range"));
			gtk_box_pack_start(GTK_BOX(hbox), gpd->range_container, FALSE, TRUE, 3);
			gtk_widget_show(gpd->range_container);
			gpd->range_accel_group = gtk_accel_group_new ();
			gtk_window_add_accel_group (GTK_WINDOW (gpd),
						    gpd->range_accel_group);
		}
		gtk_widget_show_all(hbox);
	}

	gnome_printer_widget_bind_editable_enters (gpw, GNOME_DIALOG (gpd));
	gnome_printer_widget_bind_accel_group (gpw, GTK_WINDOW (gpd));
#if 0
	gtk_widget_grab_focus (gpw->profile_selector);
#endif
}

static char const * *
print_buttons()
{
	static char const *values[4];
	values [0] = _("Print");
	values [1] = _("Preview");
	values [2] = GNOME_STOCK_BUTTON_CANCEL;
	values [3] = NULL;
	return values;
}

/**
 * gnome_print_dialog_new:
 * @title: Title of window.
 * @flags: Options for created widget.
 * 
 * Create a new gnome-print-dialog window.
 *
 * The following options flags are available:
 * GNOME_PRINT_DIALOG_RANGE: A range widget container will be created.
 * A range widget must be created separately, using one of the
 * gnome_print_dialog_construct_* functions.
 * GNOME_PRINT_DIALOG_COPIES: A copies widget will be created.
 * 
 * Return value: A newly created and initialised widget.
 **/
GtkWidget *
gnome_print_dialog_new (const char *title, int flags)
{
	GtkWidget *w;

	w = GTK_WIDGET ( gtk_type_new (gnome_print_dialog_get_type ()));
	if (GNOME_PRINT_DIALOG (w)->printer == NULL)
		return NULL;
	gnome_dialog_constructv(GNOME_DIALOG(w), title, print_buttons());
	init_body(GNOME_PRINT_DIALOG(w), flags);
	return w;
}

/**
 * gnome_print_dialog_construct:
 * @gpd: A created GnomePrintDialog.
 * @title: Title of the window.
 * @flags: Initialisation options, see gnome_print_dialog_new().
 * 
 * Used for language bindings to post-initialise an object instantiation.
 **/
void
gnome_print_dialog_construct (GtkWidget *gpd, const char *title, int flags)
{
	gnome_dialog_constructv(GNOME_DIALOG(gpd), title, print_buttons());
	init_body(GNOME_PRINT_DIALOG(gpd), flags);
}

/**
 * gnome_print_dialog_construct_range_custom:
 * @gpd: A GnomePrintDialog for which a range was requested.
 * @custom: A widget which will be placed in a "Range" frame in the
 * main display.
 * 
 * Install a custom range specification widget.
 **/
void
gnome_print_dialog_construct_range_custom (GnomePrintDialog *gpd, GtkWidget *custom)
{
	g_return_if_fail(gpd != NULL);
	g_return_if_fail(GNOME_IS_PRINT_DIALOG(gpd));
	g_return_if_fail(gpd->range_container != NULL);

	if (gpd->range) {
		gtk_container_remove(GTK_CONTAINER(gpd->range_container), gpd->range);
		gtk_object_unref(GTK_OBJECT(gpd->range));
		gpd->range = NULL;
	}

	gtk_container_add(GTK_CONTAINER(gpd->range_container), custom);
	gpd->range = custom;
}


static void
replace (GtkWidget **o, GtkWidget *n)
{
	if (*o) {
		gtk_object_unref ((GtkObject *)*o);
	}
	*o=n;
}

/**
 * gnome_print_dialog_construct_range_any:
 * @gpd: An initialise GnomePrintDialog, which can contain a range.
 * @flags: Options flags, which ranges are displayed.
 * @range_widget: Widget to display for the range option.
 * @currentlabel: Label to display next to the 'current page' button.
 * @rangelabel: Label to display next to the 'range' button.
 * 
 * Create a generic range area within the print range dialogue.  The flags
 * field contains a mask of which options you wish displayed:
 *
 * GNOME_PRINT_RANGE_CURRENT: A label @currentlabel will be displayed.
 * GNOME_PRINT_RANGE_ALL: A label "All" will be displayed.
 * GNOME_PRINT_RANGE_RANGE: A label @rangelabel will be displayed, next
 * to the range specification widget @range_widget.
 * GNOME_PRINT_RANGE_SELECTION: A label "Selection" will be displayed.
 * 
 **/
void
gnome_print_dialog_construct_range_any (GnomePrintDialog *gpd, int flags,
								GtkWidget *range_widget,
								gchar *currentlabel, gchar *rangelabel)
{
	GtkWidget *table;
	GtkWidget *current = NULL, *all = NULL, *range = NULL, *selection = NULL;
	int row;
	guint accel_key;
	Private *p;
	GSList *group = NULL;

	g_return_if_fail (gpd != NULL);
	g_return_if_fail (GNOME_IS_PRINT_DIALOG(gpd));
	g_return_if_fail (gpd->range_container != NULL);
	g_return_if_fail (!(flags&GNOME_PRINT_RANGE_SELECTION && flags&GNOME_PRINT_RANGE_SELECTION_UNSENSITIVE));
	p = gpd->priv;

	row=0;
	if (flags&GNOME_PRINT_RANGE_SELECTION)
		row++;
	if (flags&GNOME_PRINT_RANGE_SELECTION_UNSENSITIVE)
		row++;
	if (flags&GNOME_PRINT_RANGE_ALL)
		row++;
	if (flags&GNOME_PRINT_RANGE_RANGE)
		row++;
	if (flags&GNOME_PRINT_RANGE_CURRENT)
		row++;

	table = gtk_table_new(row, 2, FALSE);
	row=0;
	if (flags&GNOME_PRINT_RANGE_CURRENT) {
		current = gtk_radio_button_new_with_label(group, "");
		accel_key = gtk_label_parse_uline
			(GTK_LABEL (GTK_BIN (current)->child), currentlabel);
		gtk_table_attach(GTK_TABLE(table), current, 0, 1, row, row+1, GTK_FILL, GTK_FILL, 0, 0);
		group = gtk_radio_button_group((GtkRadioButton *)current);
		if (accel_key != GDK_VoidSymbol)
			gtk_widget_add_accelerator (current, "clicked",
						    gpd->range_accel_group,
						    accel_key,
						    GDK_MOD1_MASK, 0);
		row++;
	}

	if (flags&GNOME_PRINT_RANGE_ALL) {
		all = gtk_radio_button_new_with_label(group, "");
		accel_key = gtk_label_parse_uline
			(GTK_LABEL (GTK_BIN (all)->child), _("_All"));
		gtk_table_attach(GTK_TABLE(table), all, 0, 1, row, row+1, GTK_FILL, GTK_FILL, 0, 0);
		group = gtk_radio_button_group((GtkRadioButton *)all);
		if (accel_key != GDK_VoidSymbol)
			gtk_widget_add_accelerator (all, "clicked",
						    gpd->range_accel_group,
						    accel_key,
						    GDK_MOD1_MASK, 0);
		row++;
	}

	if (flags&GNOME_PRINT_RANGE_RANGE) {
		range = gtk_radio_button_new_with_label(group, "");
		accel_key = gtk_label_parse_uline
			(GTK_LABEL (GTK_BIN (range)->child), rangelabel);
		gtk_table_attach(GTK_TABLE(table), range, 0, 1, row, row+1, GTK_FILL, GTK_SHRINK, 0, 0);
		gtk_table_attach(GTK_TABLE(table), range_widget, 1, 2, row, row+1, GTK_FILL, 0, 0, 0);
		group = gtk_radio_button_group((GtkRadioButton *)range);
		if (accel_key != GDK_VoidSymbol)
			gtk_widget_add_accelerator (range, "clicked",
						    gpd->range_accel_group,
						    accel_key,
						    GDK_MOD1_MASK, 0);
		row++;
	}

	if (flags&GNOME_PRINT_RANGE_SELECTION) {
		selection = gtk_radio_button_new_with_label(group, "");
		accel_key = gtk_label_parse_uline
			(GTK_LABEL (GTK_BIN (selection)->child),
			 _("_Selection"));
		gtk_table_attach(GTK_TABLE(table), selection, 0, 1, row, row+1, GTK_FILL, GTK_FILL, 0, 0);
		group = gtk_radio_button_group((GtkRadioButton *)selection);
		if (accel_key != GDK_VoidSymbol)
			gtk_widget_add_accelerator (selection, "clicked",
						    gpd->range_accel_group,
						    accel_key,
						    GDK_MOD1_MASK, 0);
		row++;
	}
	if (flags&GNOME_PRINT_RANGE_SELECTION_UNSENSITIVE) {
		selection = gtk_radio_button_new_with_label(group, "");
		accel_key = gtk_label_parse_uline
			(GTK_LABEL (GTK_BIN (selection)->child),
			 _("_Selection"));
		gtk_widget_set_sensitive (GTK_WIDGET(selection), FALSE);
		gtk_table_attach(GTK_TABLE(table), selection, 0, 1, row, row+1, GTK_FILL, GTK_FILL, 0, 0);
		group = gtk_radio_button_group((GtkRadioButton *)selection);
		row++;
	}

	replace(&p->current, current);
	replace(&p->all, all);
	replace(&p->range, range);
	replace(&p->selection, selection);

	gtk_widget_show_all(table);

	gnome_print_dialog_construct_range_custom(gpd, table);
	gpd->range_type = GNOME_PRINT_RANGETYPE_CUSTOM;
}

/**
 * gnome_print_dialog_construct_range_date:
 * @gpd: An initialise GnomePrintDialog, which can contain a range.
 * @flags: Option flags.  See gnome_print_dialog_construct_any() and below
 * for date specific flags.
 * @start: Initial start date.
 * @end: Initial end date.
 * @currentlabel: Label text for the 'current' option.
 * @rangelabel: Label text for the 'range' option.
 * 
 * Create a generic date-range area.
 *
 * In addition to the generic range option flags, the following applies:
 *
 * GNOME_PRINT_RANGE_DATE_TIME: Allow a time to be specified/shown.
 * GNOME_PRINT_RANGE_DATE_24HR: Time is displayed in 24 hour format.
 * GNOME_PRINT_RANGE_DATE_MONDAY: Week starts on Monday for calendar display.
 *
 * Also, the GNOME_PRINT_RANGE_ALL flag cannot be specified (it is removed),
 * because it makes no sense!
 **/
void
gnome_print_dialog_construct_range_date (GnomePrintDialog *gpd, int flags,
					 time_t start, time_t end,
					 char *currentlabel, char *rangelabel)
{
	GtkWidget *table = NULL, *label;
	GtkWidget *range_start = NULL, *range_end = NULL;
	Private *p;

	g_return_if_fail(gpd != NULL);
	g_return_if_fail(GNOME_IS_PRINT_DIALOG(gpd));
	g_return_if_fail(gpd->range_container != NULL);

	/* cannot print all dates ... */
	flags &= ~GNOME_PRINT_RANGE_ALL;

	p = gpd->priv;

	if (flags&GNOME_PRINT_RANGE_RANGE) {
		int dateflags=0;

		table = gtk_table_new(2, 2, FALSE);
		label = gtk_label_new(_("from:"));
		gtk_table_attach((GtkTable *)table, label, 0, 1, 0, 1, GTK_FILL, GTK_FILL, 0, 0);

		/* remap flags */
		if (flags&GNOME_PRINT_RANGE_DATE_TIME)
			dateflags |= GNOME_DATE_EDIT_SHOW_TIME;
		if (flags&GNOME_PRINT_RANGE_DATE_24HR)
			dateflags |= GNOME_DATE_EDIT_24_HR;
		if (flags&GNOME_PRINT_RANGE_DATE_MONDAY)
			dateflags |= GNOME_DATE_EDIT_WEEK_STARTS_ON_MONDAY;

		range_start = gnome_date_edit_new_flags(start, dateflags);
		gtk_table_attach((GtkTable *)table, range_start, 1, 2, 0, 1, GTK_FILL, GTK_FILL, 0, 0);

		label = gtk_label_new(_("to:"));
		gtk_table_attach((GtkTable *)table, label, 0, 1, 1, 2, GTK_FILL, GTK_FILL, 0, 0);

		range_end = gnome_date_edit_new_flags(end, dateflags);
		gtk_table_attach((GtkTable *)table, range_end, 1, 2, 1, 2, GTK_FILL, GTK_FILL, 0, 0);
	}

	replace(&p->range_start, range_start);
	replace(&p->range_end, range_end);

	gnome_print_dialog_construct_range_any(gpd, flags, table, currentlabel, rangelabel);
	gpd->range_type = GNOME_PRINT_RANGETYPE_PAGES;
}

/**
 * gnome_print_dialog_construct_range_page:
 * @gpd: An initialise GnomePrintDialog, which can contain a range.
 * @flags: Option flags.  See gnome_print_dialog_construct_any().
 * @start: First page which may be printed.
 * @end: Last page which may be printed.
 * @currentlabel: Label text for current option.
 * @rangelabel: Label text for range option.
 * 
 * Construct a generic page/sheet range area.
 **/
void
gnome_print_dialog_construct_range_page (GnomePrintDialog *gpd, int flags, int start, int end,
					 char *currentlabel, char *rangelabel)
{
	GtkWidget *hbox = NULL, *label;
	GtkWidget *range_start = NULL, *range_end = NULL;
	GtkAdjustment *adj_start, *adj_end;
	GtkEntry *entry;
	guint label_key;
	Private *p;

	g_return_if_fail(gpd != NULL);
	g_return_if_fail(GNOME_IS_PRINT_DIALOG(gpd));
	g_return_if_fail(gpd->range_container != NULL);

	p = gpd->priv;

	if (flags&GNOME_PRINT_RANGE_RANGE) {
		hbox = gtk_hbox_new(FALSE, 3);
		label = gtk_label_new("");
		label_key = gtk_label_parse_uline (GTK_LABEL (label),
						   _("f_rom:"));
		gtk_box_pack_start((GtkBox *)hbox, label, FALSE, FALSE, 0);

		adj_start = (GtkAdjustment *)gtk_adjustment_new(start, start, end, 1, 10, 10);
		range_start = gtk_spin_button_new(adj_start, 1, 0.0);
		gtk_box_pack_start((GtkBox *)hbox, range_start, FALSE, FALSE, 0);
		if (label_key != GDK_VoidSymbol) {
			entry = &GTK_SPIN_BUTTON (range_start)->entry;
			gtk_widget_add_accelerator (GTK_WIDGET (entry),
						    "grab_focus",
						    gpd->range_accel_group,
						    label_key, GDK_MOD1_MASK,
						    0);
		}
	
		label = gtk_label_new("");
		label_key = gtk_label_parse_uline (GTK_LABEL (label),
						   _("_to:"));
		gtk_box_pack_start((GtkBox *)hbox, label, FALSE, FALSE, 0);

		adj_end = (GtkAdjustment *)gtk_adjustment_new(end, start, end, 1, 10, 10);
		range_end = gtk_spin_button_new(adj_end, 1, 0.0);
		gtk_box_pack_start((GtkBox *)hbox, range_end, FALSE, FALSE, 0);
		if (label_key != GDK_VoidSymbol) {
			entry = &GTK_SPIN_BUTTON (range_end)->entry;
			gtk_widget_add_accelerator (GTK_WIDGET (entry),
						    "grab_focus",
						    gpd->range_accel_group,
						    label_key, GDK_MOD1_MASK,
						    0);
		}

	}

	replace(&p->range_start, range_start);
	replace(&p->range_end, range_end);

	gnome_print_dialog_construct_range_any(gpd, flags, hbox, currentlabel, rangelabel);
	gpd->range_type = GNOME_PRINT_RANGETYPE_PAGES;
}

/**
 * gnome_print_dialog_get_range:
 * @gpd: A GnomePrintDialog with a range display.
 * 
 * Return the range option selected by the user.  This is a bitmask
 * with only 1 bit set, out of:
 *
 * GNOME_PRINT_RANGE_CURRENT: The current option selected.
 * GNOME_PRINT_RANGE_ALL: The all option selected.
 * GNOME_PRINT_RANGE_RANGE The range option selected.
 * GNOME_PRINT_RANGE_SELECTION: The selection option selected.
 * 
 * Return value: A bitmask with one option set.
 **/
GnomePrintRangeType
gnome_print_dialog_get_range (GnomePrintDialog *gpd)
{
	Private *p;

	g_return_val_if_fail(gpd != NULL, 0);
	g_return_val_if_fail(GNOME_IS_PRINT_DIALOG(gpd), 0);
	g_return_val_if_fail(gpd->range_container != NULL, 0);

	p = gpd->priv;

	if (p->current!=NULL)
	     if (GTK_TOGGLE_BUTTON(p->current)->active)
		  return GNOME_PRINT_RANGE_CURRENT;
	if (p->all!=NULL)
	     if (GTK_TOGGLE_BUTTON(p->all)->active)
		  return GNOME_PRINT_RANGE_ALL;
	if (p->range!=NULL)
	     if (GTK_TOGGLE_BUTTON(p->range)->active)
		  return GNOME_PRINT_RANGE_RANGE;
	if (p->selection!=NULL)
	     if (GTK_TOGGLE_BUTTON(p->selection)->active)
		  return GNOME_PRINT_RANGE_SELECTION;
	return 0;
}

/**
 * gnome_print_dialog_get_range_page:
 * @gpd: A GnomePrintDialog with a page range display.
 * @start: Return for the user-specified start page.
 * @end: Return for the user-specified end page.
 * 
 * Retrieves the user choice for range type and range, if the user
 * has requested a range of pages to print.
 * 
 * Return value: A bitmask with the user-selection set.  See
 * gnome_print_dialog_get_range().
 **/
int
gnome_print_dialog_get_range_page (GnomePrintDialog *gpd, int *start, int *end)
{
	int mask;
	Private *p;

	g_return_val_if_fail(gpd != NULL, 0);
	g_return_val_if_fail(GNOME_IS_PRINT_DIALOG(gpd), 0);
	g_return_val_if_fail(gpd->range_container != NULL, 0);
	/*	g_return_val_if_fail(gpd->range_type != GNOME_PRINT_RANGETYPE_PAGES, 0);*/

	p = gpd->priv;

	mask = gnome_print_dialog_get_range(gpd);
	if (mask & GNOME_PRINT_RANGE_RANGE) {
		*start = gtk_spin_button_get_value_as_int((GtkSpinButton *)p->range_start);
		*end = gtk_spin_button_get_value_as_int((GtkSpinButton *)p->range_end);
	}
	return mask;
}

/**
 * gnome_print_dialog_get_range_date:
 * @gpd: A GnomePrintDialog with a date range display.
 * @start: Return for the start of the time range, if the user
 * requested a range.
 * @end: Return for the end of the time range.
 * 
 * Get the user choice for range type and range values for
 * a date-range dialogue.
 * 
 * Return value: A bitmask with the user-selection set.  See
 * gnome_print_dialog_get_range().
 **/
int
gnome_print_dialog_get_range_date (GnomePrintDialog *gpd, time_t *start, time_t *end)
{
	int mask;
	Private *p;

	g_return_val_if_fail(gpd != NULL, 0);
	g_return_val_if_fail(GNOME_IS_PRINT_DIALOG(gpd), 0);
	g_return_val_if_fail(gpd->range_container != NULL, 0);
	g_return_val_if_fail(gpd->range_type != GNOME_PRINT_RANGETYPE_DATES, 0);

	p = gpd->priv;

	mask = gnome_print_dialog_get_range(gpd);
	if (mask & GNOME_PRINT_RANGE_RANGE) {
		*start = gnome_date_edit_get_date((GnomeDateEdit *)p->range_start);
		*end = gnome_date_edit_get_date((GnomeDateEdit *)p->range_end);
	}
	return mask;
}

/**
 * gnome_print_dialog_get_copies:
 * @gpd: A GnomePrintDialog with a copies display.
 * @copies: Return for the number of copies.
 * @collate: Return for collation flag.
 * 
 * Retrieves the number of copies and collation indicator from
 * the print dialogue.  If the print dialogue does not have a
 * copies indicator, then a default of 1 copy is returned.
 **/
void
gnome_print_dialog_get_copies (GnomePrintDialog *gpd, int *copies, int *collate)
{
	if (gpd->copies) {
		if (copies) *copies = gnome_print_copies_get_copies((GnomePrintCopies *) gpd->copies);
		if (collate) *collate = gnome_print_copies_get_collate ((GnomePrintCopies *) gpd->copies);
	} else {
		if (copies) *copies = 1;
		if (collate) *collate = FALSE;
	}
}

/**
 * gnome_print_dialog_set_copies:
 * @gpd: A GnomePrintDialog with a copies display.
 * @copies: New number of copies.
 * @collate: New collation status.
 * 
 * Sets the print copies and collation status in the print dialogue.
 **/
void
gnome_print_dialog_set_copies (GnomePrintDialog *gpd, int copies, int collate)
{
	if (gpd->copies)
		gnome_print_copies_set_copies((GnomePrintCopies *)gpd->copies, copies, collate);
}

/**
 * gnome_print_dialog_get_printer:
 * @gpd: An initialised GnomePrintDialog.
 * 
 * Retrieve the user-requested printer from the printer area of
 * the print dialogue.
 * 
 * Return value: The user-selected printer.
 **/
GnomePrinter *
gnome_print_dialog_get_printer (GnomePrintDialog *gpd)
{
	return gnome_printer_widget_get_printer((GnomePrinterWidget *)gpd->printer);
}
