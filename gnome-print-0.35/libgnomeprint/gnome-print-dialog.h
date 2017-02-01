/*
 *  Copyright (C) 2000 Helix Code Inc.
 *
 *  Authors: Michael Zucchi <notzed@helixcode.com>
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

#ifndef _GNOME_PRINT_DIALOG
#define _GNOME_PRINT_DIALOG

#include <gtk/gtk.h>
#include <libgnome/gnome-defs.h>
#include <libgnomeui/gnome-dialog.h>
#include <sys/types.h>

BEGIN_GNOME_DECLS

#define GNOME_PRINT_DIALOG(obj)         GTK_CHECK_CAST (obj, gnome_print_dialog_get_type (), GnomePrintDialog)
#define GNOME_PRINT_DIALOG_CLASS(klass) GTK_CHECK_CLASS_CAST (klass, gnome_print_dialog_get_type (), GnomePrintDialogClass)
#define GNOME_IS_PRINT_DIALOG(obj)      GTK_CHECK_TYPE (obj, gnome_print_dialog_get_type ())

typedef struct _GnomePrintDialog      GnomePrintDialog;
typedef struct _GnomePrintDialogClass GnomePrintDialogClass;

/* used to track what type of range selector has been setup */
typedef enum {
	GNOME_PRINT_RANGETYPE_NONE,
	GNOME_PRINT_RANGETYPE_CUSTOM,
	GNOME_PRINT_RANGETYPE_PAGES,
	GNOME_PRINT_RANGETYPE_DATES
} GnomePrintRangeType;

/* what options on the range selector do we want? */
enum GnomePrintRangeFlags {
	GNOME_PRINT_RANGE_CURRENT = 1<<0,
	GNOME_PRINT_RANGE_ALL = 1<<1,
	GNOME_PRINT_RANGE_RANGE = 1<<2,
	GNOME_PRINT_RANGE_SELECTION = 1<<3,
	GNOME_PRINT_RANGE_SELECTION_UNSENSITIVE  = 1<<4,

	/* options specific to dates */
	GNOME_PRINT_RANGE_DATE_TIME = 1<<8,
	GNOME_PRINT_RANGE_DATE_24HR = 1<<9,
	GNOME_PRINT_RANGE_DATE_MONDAY = 1<<10,
};

/* used to new call */
enum GnomePrintFlags {
	GNOME_PRINT_DIALOG_RANGE = 1<<0,
	GNOME_PRINT_DIALOG_COPIES = 1<<1
};

/**
 * The button numbers corresponding to the standard buttons
 * Used with the GnomeDialog "clicked" signal.
 */
enum GnomePrintButtons {
	GNOME_PRINT_PRINT = 0,
	GNOME_PRINT_PREVIEW,
	GNOME_PRINT_CANCEL
};

guint gnome_print_dialog_get_type (void);

GtkWidget *gnome_print_dialog_new (const char *title, int flags);

/* for language bindings */
void gnome_print_dialog_construct (GtkWidget *gpd, const char *title, int flags);

void gnome_print_dialog_construct_range_custom (GnomePrintDialog *gpd, GtkWidget *custom);
void gnome_print_dialog_construct_range_any (GnomePrintDialog *gpd, int flags, GtkWidget *range_widget, char *currentlabel, char *rangelabel);
void gnome_print_dialog_construct_range_date (GnomePrintDialog *gpd, int flags, time_t start, time_t end, char *currentlabel, char *rangelabel);
void gnome_print_dialog_construct_range_page (GnomePrintDialog *gpd, int flags, int start, int end, char *currentlabel, char *rangelabel);

GnomePrintRangeType gnome_print_dialog_get_range (GnomePrintDialog *gpd);
int gnome_print_dialog_get_range_page (GnomePrintDialog *gpd, int *start, int *end);
int gnome_print_dialog_get_range_date (GnomePrintDialog *gpd, time_t *start, time_t *end);

void gnome_print_dialog_get_copies (GnomePrintDialog *gpd, int *copies, int *collate);
void gnome_print_dialog_set_copies (GnomePrintDialog *gpd, int copies, int collate);

GnomePrinter *gnome_print_dialog_get_printer (GnomePrintDialog *gpd);

END_GNOME_DECLS

#endif /* ! _GNOME_PRINT_DIALOG */
