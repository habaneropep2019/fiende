/* Grapevine: GNOME Notifications
 *   Glade helper routines
 * Author: George Lebl
 * (c) 2000 Eazel, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef GLADE_HELPER_H
#define GLADE_HELPER_H

#include <glade/glade.h>

GladeXML *	glade_helper_load		(const char *file,
						 const char *widget,
						 GtkType expected_type,
						 gboolean dump_on_destroy);
GtkWidget *	glade_helper_load_widget	(const char *file,
						 const char *widget,
						 GtkType expected_type);

/* error checking get */
GtkWidget *	glade_helper_get 		(GladeXML *xml,
						 const char *name,
						 GtkType expected_type);
GtkWidget *	glade_helper_get_clist 		(GladeXML *xml,
						 const char *name,
						 GtkType expected_type,
						 int expected_columns);

/* For finding glade files */
char *		glade_helper_find_glade_file	(const char *file);
void		glade_helper_add_glade_directory (const char *directory);

#endif /* GLADE_HELPER_H */
