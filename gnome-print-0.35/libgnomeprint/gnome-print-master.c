/*
 *  Copyright (C) 2000 Helix Code Inc.
 *
 *  Authors: Michael Zucchi <notzed@helixcode.com>
 *
 *  A system print interface.
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

#include <config.h>

#include <sys/stat.h>
#include <ctype.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtkobject.h>
#include <libgnome/gnome-defs.h>
#include <libgnome/gnome-i18n.h>
#include <libgnome/gnome-util.h>
#include <libgnomeui/gnome-stock.h>
#include <libgnomeui/gnome-messagebox.h>

#include <libgnomeprint/gnome-printer.h>
#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-print-meta.h>
#include <libgnomeprint/gnome-print-preview.h>
#include <libgnomeprint/gnome-printer-dialog.h>

#include "gnome-print-master.h"
#include <libgnomeprint/gnome-print-master-private.h>

static void gnome_print_master_class_init (GnomePrintMasterClass *class);
static void gnome_print_master_init       (GnomePrintMaster      *gspaper);

static gboolean alwaysoverwrite = FALSE;

static GtkObjectClass *parent_class;

guint
gnome_print_master_get_type (void)
{
	static guint print_master_type = 0;
	
	if (!print_master_type) {
		GtkTypeInfo print_master_info = {
			"GnomePrintMaster",
			sizeof (GnomePrintMaster),
			sizeof (GnomePrintMasterClass),
			(GtkClassInitFunc) gnome_print_master_class_init,
			(GtkObjectInitFunc) gnome_print_master_init,
			(GtkArgSetFunc) NULL,
			(GtkArgGetFunc) NULL
		};
		
		print_master_type = gtk_type_unique (gtk_object_get_type (), &print_master_info);
	}
	
	return print_master_type;
}

/* REmove */
#include <libgnomeprint/gnome-printer-private.h>

static void
gnome_print_master_finalize (GtkObject *object)
{
	GnomePrintMaster *gpm = GNOME_PRINT_MASTER(object);

	if (gpm->context != NULL) {
		gtk_object_unref (GTK_OBJECT (gpm->context));
	}

	if (gpm->printer) {
		gtk_object_unref (GTK_OBJECT (gpm->printer));
	}

	GTK_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gnome_print_master_class_init (GnomePrintMasterClass *class)
{
	GtkObjectClass *object_class;
	
	object_class = (GtkObjectClass *) class;
	parent_class = gtk_type_class (gtk_object_get_type ());

	object_class->finalize = gnome_print_master_finalize;
}

static void
gnome_print_master_init (GnomePrintMaster *gpm)
{
	gpm->copies = 1;
	gpm->iscollate = FALSE;
	gpm->paper = gnome_paper_with_name(gnome_paper_name_default());
	gpm->context = GNOME_PRINT_CONTEXT (gnome_print_meta_new ());
}

/**
 * gnome_print_master_new:
 * 
 * Create a new GnomePrintMaster.  All values are initialised
 * to sensible defaults.
 * 
 * Return value: A new GnomePrintMaster.
 **/
GnomePrintMaster *
gnome_print_master_new (void)
{
	GnomePrintMaster *gpm;

	gpm = GNOME_PRINT_MASTER ( gtk_type_new (gnome_print_master_get_type ()));

	return gpm;
}

/**
 * gnome_print_master_new_from_dialog:
 * @dialog: %A GnomePrintDialog
 * 
 * Create a new GnomePrintMaster based on the values in the
 * %GnomePrintDialog.  Range values are initialised to sensible
 * defaults.  Other values are initialised from the given dialog.
 * 
 * Return value: A new GnomePrintMaster.
 **/
GnomePrintMaster *
gnome_print_master_new_from_dialog (GnomePrintDialog *dialog)
{
	GnomePrintMaster *gpm;
	gint copies;
	gint collate;
	
	g_return_val_if_fail (dialog != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_DIALOG (dialog), NULL);

	gpm = GNOME_PRINT_MASTER ( gtk_type_new (gnome_print_master_get_type ()));

	gnome_print_master_set_printer (gpm, gnome_print_dialog_get_printer(dialog));
	gnome_print_dialog_get_copies (dialog, &copies, &collate);
	gnome_print_master_set_copies (gpm, copies, collate);

	return gpm;
}

/**
 * gnome_print_master_get_context:
 * @gpm: An initialised GnomePrintMaster.
 * 
 * Retrieve the GnomePrintContext which applications
 * print to.
 * 
 * Return value: The printing context.
 **/
GnomePrintContext *
gnome_print_master_get_context(GnomePrintMaster *gpm)
{
	g_return_val_if_fail (GNOME_IS_PRINT_MASTER (gpm), NULL);

	return gpm->context;
}

/**
 * gnome_print_master_get_pages:
 * @gpm: An initialised and closed GnomePrintMaster.
 * 
 * Find the number of pages stored in a completed printout.
 * 
 * Return value: If @gpm has not been closed using
 * gnome_print_master_close(), then 0, otherwise the number
 * of pages created by the application.
 **/
int
gnome_print_master_get_pages(GnomePrintMaster *gpm)
{
	g_return_val_if_fail (GNOME_IS_PRINT_MASTER (gpm), 0);

	return gnome_print_meta_pages(GNOME_PRINT_META(gpm->context));
}

/**
 * gnome_print_master_set_paper:
 * @gpm: An initialised GnomePrintMaster.
 * @paper: Paper size required.
 * 
 * Set the paper size for the print.
 **/
void
gnome_print_master_set_paper(GnomePrintMaster *gpm, const GnomePaper *paper)
{
	g_return_if_fail (GNOME_IS_PRINT_MASTER (gpm));

	gpm->paper = paper;
}

const GnomePaper *
gnome_print_master_get_paper (const GnomePrintMaster *gpm)
{
	g_return_val_if_fail (GNOME_IS_PRINT_MASTER (gpm), NULL);

	return gpm->paper;
}

/**
 * gnome_print_master_set_printer:
 * @gpm: An initialised GnomePrintMaster.
 * @printer: Printer device to use.
 * 
 * Sets physical printer used to print to.  If this is not set,
 * then a dialogue will be presented on each print.
 *
 * This functions takes ownership of the printer reference.
 **/
void
gnome_print_master_set_printer(GnomePrintMaster *gpm, GnomePrinter *printer)
{
	g_return_if_fail (GNOME_IS_PRINT_MASTER (gpm));

	if (gpm->printer)
		gtk_object_unref (GTK_OBJECT (gpm->printer));

	gpm->printer = printer;
}

/**
 * gnome_print_master_set_copies:
 * @gpm: A GnomePrintContext.
 * @copies: Number of copies of each page to print.
 * @iscollate: Whether page copies are collated.
 * 
 * Set the number of copies to print, and whether pages are collated.
 * If @iscollate is set, then a multiple copy print to a physical
 * printer will order pages as "1,2,3,...1,2,3,..." otherwise they
 * will be ordered "1,1,...,2,2...".
 **/
void
gnome_print_master_set_copies(GnomePrintMaster *gpm, int copies, gboolean iscollate)
{
	g_return_if_fail (GNOME_IS_PRINT_MASTER (gpm));

	gpm->copies = copies;
	gpm->iscollate = iscollate;
}

/**
 * gnome_print_master_close:
 * @gpm: A GnomePrintMaster which has had printing performed
 * on it.
 * 
 * Closes the GnomePrintMaster @gpm, ready for printing
 * or previewing.
 **/
void
gnome_print_master_close(GnomePrintMaster *gpm)
{
	g_return_if_fail (GNOME_IS_PRINT_MASTER (gpm));

	gnome_print_context_close (gpm->context);
}

#if 0
/* quick show preview window */
int
gnome_print_master_preview(GnomePrintMaster *gpm, char *title)
{
	char *text;
	GnomePrintMasterPreview *pmp;

	g_return_val_if_fail (GNOME_IS_PRINT_MASTER (gpm), -1);

	pmp = gnome_print_master_preview_new(gpm, title);
	gtk_widget_show(pmp);

	return 0;
}
#endif

/**
 * gnome_print_master_print:
 * @gpm: A completed GnomePrintMaster.
 * 
 * Print the pages stored in the GnomePrintMaster to
 * the phyisical printing device.
 *
 * If no printer has been set, then a dialogue is presented,
 * asking the user for the printer to print to.
 * 
 * Return value: Returns -1 on error.
 **/
int
gnome_print_master_print (GnomePrintMaster *gpm)
{
	int repeat, loop, repeati, loopi, result;
	int page, pagecount;
	GnomePrintContext *output;
	GnomePrinter *printer;

	g_return_val_if_fail (GNOME_IS_PRINT_MASTER (gpm), -1);

	printer = gpm->printer;

	/* I hate do {} while loops (Lauris) */
	do {
		if (!printer) {
			printer = gnome_printer_dialog_new_modal ();
			if (!printer) return -1;
		} else {
			gtk_object_ref (GTK_OBJECT (printer));
		}
		/* Check whether file exists */
		if (printer && !alwaysoverwrite) {
			const gchar *t;
			t = printer->filename;
			while (t && isspace (*t)) t += 1;
			if (t && t[0] && (t[0] != '|') && (t[0] != '*')) {
				struct stat s;
				gchar *fn;
				/* Regular file */
				if ((t[0] == '~') && (t[1] == '/')) {
					fn = g_concat_dir_and_file (g_get_home_dir (), t + 2);
				} else if ((t[0] != '/') && (t[0] != '.')) {
					fn = g_concat_dir_and_file (g_get_home_dir (), t);
				} else {
					fn = g_strdup (t);
				}
				if (!stat (fn, &s)) {
					GtkWidget *mb;
					gchar *msg;
					gint b;
					/* Present dialog */
					msg = g_strdup_printf (_("File %s already exists.\nIs it OK to overwrite its contents?"), fn);
					mb = gnome_message_box_new (msg,
								    GNOME_MESSAGE_BOX_QUESTION,
								    GNOME_STOCK_BUTTON_YES,
								    GNOME_STOCK_BUTTON_NO,
								    NULL);
					b = gnome_dialog_run_and_close (GNOME_DIALOG (mb));
					if (b == 1) {
						/* Shouldn't overwrite */
						gtk_object_unref (GTK_OBJECT (printer));
						printer = NULL;
					}
					g_free (msg);
				}
				g_free (fn);
			}
		}
	} while (!printer);

	output = gnome_print_context_new_with_paper_size (printer, gpm->paper ? gnome_paper_name (gpm->paper) : "A4");

	if (output == NULL) {
		gtk_object_unref (GTK_OBJECT (printer));
		return -1;
	}

	if (gpm->iscollate) {
		repeat = 1;
		loop = gpm->copies;
	} else {
		repeat = gpm->copies;
		loop = 1;
	}
	
	pagecount = gnome_print_master_get_pages(gpm);
	for (loopi=0;loopi<loop;loopi++) {
		for (page=0;page<pagecount;page++) {
			for (repeati=0;repeati<repeat;repeati++) {
				gnome_print_meta_render_from_object_page (output, GNOME_PRINT_META (gpm->context), page);
			}
		}
	}

	gtk_object_unref (GTK_OBJECT (printer));
	result = gnome_print_context_close(output);
	gtk_object_unref (GTK_OBJECT (output));

	return result;
}
