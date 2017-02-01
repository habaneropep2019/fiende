/*
 *  Copyright (C) 2000 Helix Code Inc.
 *
 *  Authors: Michael Zucchi <notzed@helixcode.com>
 *           Lauris Kaplinski <lauris@helixcode.com>
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

#ifndef _GNOME_PRINT_COPIES_H_
#define _GNOME_PRINT_COPIES_H_

#include <gtk/gtk.h>
#include <libgnome/gnome-defs.h>

BEGIN_GNOME_DECLS

#define GNOME_TYPE_PRINT_COPIES         (gnome_print_copies_get_type ())
#define GNOME_PRINT_COPIES(obj)         GTK_CHECK_CAST (obj, GNOME_TYPE_PRINT_COPIES, GnomePrintCopies)
#define GNOME_PRINT_COPIES_CLASS(klass) GTK_CHECK_CLASS_CAST (klass, GNOME_TYPE_PRINT_COPIES, GnomePrintCopiesClass)
#define GNOME_IS_PRINT_COPIES(obj)      GTK_CHECK_TYPE (obj, GNOME_TYPE_PRINT_COPIES)
#define GNOME_IS_PRINT_COPIES_CLASS(klass) GTK_CHECK_CLASS_TYPE (klass, GNOME_TYPE_PRINT_COPIES)

typedef struct _GnomePrintCopies      GnomePrintCopies;
typedef struct _GnomePrintCopiesClass GnomePrintCopiesClass;

/*
 * We implement single signal at moment:
 *
 * void (* copies_set) (GnomeprintCopies * gpc, gint copies, gboolean collate);
 *
 */

guint gnome_print_copies_get_type (void);

GtkWidget *gnome_print_copies_new (void);

void gnome_print_copies_bind_editable_enters (GnomePrintCopies * gpc, GnomeDialog * dialog);
void gnome_print_copies_bind_accel_group (GnomePrintCopies * gpc, GtkWindow * window);

void gnome_print_copies_set_copies (GnomePrintCopies *gpc, gint copies, gboolean collate);

gint gnome_print_copies_get_copies (GnomePrintCopies *gpc);
gboolean gnome_print_copies_get_collate (GnomePrintCopies *gpc);

END_GNOME_DECLS

#endif /* ! _GNOME_PRINT_COPIES_H */
