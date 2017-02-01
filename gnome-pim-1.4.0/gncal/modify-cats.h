/* 
 * modify-cats.h
 * dialog for modifying categories for an item
 * part of gnome-cal 
 */


/*
** Copyright (C) 2000 Dirk-Jan C. Binnema <djcb@dds.nl>
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**  
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**  
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
**  
*/


#ifndef _MODIFY_CATS_H_
#define _MODIFY_CATS_H_

#include <config.h>
#include <gnome.h>
#include "gncal-todo.h"


/*
 *  this structures stores the data for category visual representation
 */
typedef struct _Category {
	gchar *name;                 /* name of the category */ 
	gchar *name_end;             /* NULL */
	/*gchar *icon; */                /* the pixmap */
	gboolean is_available;       /* is this category not yet chosen? */
	gboolean selected;           /* is thi category selected in the clist? */
} Category;


gint modify_cats_dialog ( GtkWidget *parent, GList **list );
gchar *ico_categories_as_string ( GList *ico_categories );
gchar *categories_as_string     ( GList *categories );
GList *categories_as_ico_categories ( GList *categories );
GList *ico_categories_as_categories ( GList *ico_categories );


#endif /* _MODIFY_CATS_H_ */











