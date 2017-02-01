/* GnomeCard - a graphical contact manager.
 *
 * my.h: This file is part of GnomeCard.
 * 
 * Copyright (C) 1999 The Free Software Foundation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __GNOMECARD_MY
#define __GNOMECARD_MY

#include <gnome.h>

#include "card.h"
#include "pix.h"

#define MY_FREE(a) if (a) g_free (a)
#define MY_STRLEN(x) (x?strlen(x):0)
#define MY_STRDUP(x) (*x?g_strdup(x):NULL)
#define MY_STR(a) (a)? a : ""

extern char *my_cap (char *str);
		     
extern GtkCTreeNode *
my_gtk_ctree_insert(GtkCTreeNode *parent, GtkCTreeNode *sibling, 
		    char **text, pix *p, gpointer data);
extern GtkWidget *my_gtk_entry_new(gint len, char *init);
extern GtkWidget *my_hbox_entry(GtkWidget *parent, char *label, char *init);
extern GtkWidget *my_gtk_spin_button_new(GtkAdjustment *adj, gint width);
extern GtkWidget *my_gtk_vbox_new(void);
extern GtkWidget *my_gtk_table_new(int x, int y);
extern void my_connect(gpointer widget, char *sig, gpointer box, 
		       CardProperty *prop, enum PropertyType type);

#endif
