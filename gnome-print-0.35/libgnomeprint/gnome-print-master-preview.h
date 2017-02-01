/*
 *  Copyright (C) 2000 Helix Code Inc.
 *
 *  Authors: Michael Zucchi <notzed@helixcode.com>
 *           Miguel de Icaza (miguel@gnu.org)
 *
 *  A system print preview window.  Based on print-preview.c
 *  from gnumeric.
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

#ifndef GNOME_PRINT_MASTER_PREVIEW_H
#define GNOME_PRINT_MASTER_PREVIEW_H

#include <gtk/gtk.h>
#include <libgnome/gnome-defs.h>
#include <libgnomeprint/gnome-print.h>
#include <libgnomeui/gnome-app.h>
#include <libgnomeprint/gnome-print-master.h>

BEGIN_GNOME_DECLS

#define GNOME_PRINT_MASTER_PREVIEW(obj)         GTK_CHECK_CAST (obj, gnome_print_master_preview_get_type (), GnomePrintMasterPreview)
#define GNOME_PRINT_MASTER_PREVIEW_CLASS(klass) GTK_CHECK_CLASS_CAST (klass, gnome_print_master_preview_get_type (), GnomePrintMasterPreviewClass)
#define GNOME_IS_PRINT_MASTER_PREVIEW(obj)      GTK_CHECK_TYPE (obj, gnome_print_master_preview_get_type ())


typedef struct _GnomePrintMasterPreviewPrivate      GnomePrintMasterPreviewPrivate;

typedef struct {
	GnomeApp app;

	GnomePrintMasterPreviewPrivate *priv;

	/*
	 * TODO: expose a few of the internals here, I do not know when
	 * they were moved out, but I can definetly see an use for them.
	 */
} GnomePrintMasterPreview;

typedef struct {
	GnomeAppClass parent_class;
} GnomePrintMasterPreviewClass;

guint		gnome_print_master_preview_get_type	(void);

GnomePrintMasterPreview *
gnome_print_master_preview_new	(GnomePrintMaster *gpm,
				 const char *title);

GnomePrintMasterPreview *
gnome_print_master_preview_new_with_orientation (GnomePrintMaster *gpm,
						 const char *title,
						 gboolean landscape);

END_GNOME_DECLS

#endif /* GNOME_PRINT_MASTER_PREVIEW_H */

