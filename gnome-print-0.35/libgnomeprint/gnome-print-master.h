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

#ifndef GNOME_PRINT_MASTER_H
#define GNOME_PRINT_MASTER_H

#include <gtk/gtk.h>
#include <libgnome/gnome-defs.h>
#include <libgnomeprint/gnome-print.h>
#include <libgnomeprint/gnome-print-dialog.h>

BEGIN_GNOME_DECLS

#define GNOME_PRINT_MASTER(obj)         GTK_CHECK_CAST (obj, gnome_print_master_get_type (), GnomePrintMaster)
#define GNOME_PRINT_MASTER_CLASS(klass) GTK_CHECK_CLASS_CAST (klass, gnome_print_master_get_type (), GnomePrintMasterClass)
#define GNOME_IS_PRINT_MASTER(obj)      GTK_CHECK_TYPE (obj, gnome_print_master_get_type ())

typedef struct _GnomePrintMaster      GnomePrintMaster;
typedef struct _GnomePrintMasterClass GnomePrintMasterClass;

guint		   gnome_print_master_get_type	(void);
GnomePrintMaster  *gnome_print_master_new	(void);
GnomePrintMaster  *gnome_print_master_new_from_dialog (GnomePrintDialog *dialog);
GnomePrintContext *gnome_print_master_get_context(GnomePrintMaster *gpm);

void               gnome_print_master_set_paper (GnomePrintMaster *gpm,
						 const GnomePaper *paper);
const GnomePaper * gnome_print_master_get_paper (const GnomePrintMaster *gpm);

void gnome_print_master_set_printer (GnomePrintMaster *gpm,
				     GnomePrinter *printer);
void gnome_print_master_set_copies  (GnomePrintMaster *gpm,
				     int copies, gboolean iscollate);
void gnome_print_master_close       (GnomePrintMaster *gpm);
int  gnome_print_master_get_pages   (GnomePrintMaster *gpm);
int  gnome_print_master_print       (GnomePrintMaster *gpm);

END_GNOME_DECLS

#endif /* GNOME_PRINT_MASTER_H */
