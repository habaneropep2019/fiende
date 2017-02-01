/* 
 * gnome-print-multipage.h
 * Copyright (C) 2000  Helix Code, Inc.
 * Author: Chris Lahey <clahey@helixcode.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __GNOME_PRINT_MULTIPAGE_H__
#define __GNOME_PRINT_MULTIPAGE_H__

#include <libgnomeprint/gnome-print.h>

BEGIN_GNOME_DECLS

#define GNOME_TYPE_PRINT_MULTIPAGE		 (gnome_print_multipage_get_type ())
#define GNOME_PRINT_MULTIPAGE(obj)		 (GTK_CHECK_CAST ((obj), GNOME_TYPE_PRINT_MULTIPAGE, GnomePrintMultipage))
#define GNOME_PRINT_MULTIPAGE_CLASS(klass)	 (GTK_CHECK_CLASS_CAST ((klass), GNOME_TYPE_PRINT_MULTIPAGE, GnomePrintMultipageClass))
#define GNOME_IS_PRINT_MULTIPAGE(obj)	 (GTK_CHECK_TYPE ((obj), GNOME_TYPE_PRINT_MULTIPAGE))
#define GNOME_IS_PRINT_MULTIPAGE_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GNOME_TYPE_PRINT_MULTIPAGE))



typedef struct _GnomePrintMultipage       GnomePrintMultipage;
typedef struct _GnomePrintMultipageClass  GnomePrintMultipageClass;

GtkType gnome_print_multipage_get_type (void);

GnomePrintMultipage *gnome_print_multipage_new (GnomePrintContext *subpc, GList *affines /* Of type double[6] */);
GnomePrintMultipage *gnome_print_multipage_new_from_sizes (GnomePrintContext *subpc, gdouble paper_width, gdouble paper_height, gdouble page_width, gdouble page_height);

END_GNOME_DECLS

#endif /* __GNOME_PRINT_MULTIPAGE_H__ */
