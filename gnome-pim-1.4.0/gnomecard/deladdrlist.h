/* GnomeCard - a graphical contact manager.
 *
 * deladdrlist.h: This file is part of GnomeCard.
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

#ifndef __GNOMECARD_DELADDRLIST_H
#define __GNOMECARD_DELADDRLIST_H

#include "card.h"

typedef struct
{
	GtkWidget *the_vbox;
	
	GtkWidget *type[6], *data[DELADDR_MAX], *clist;
	GtkWidget *add, *edit, *del;
	GtkWidget *up, *down;
	
	Card *crd;
	GPtrArray *deladdr_array;
	int row, num_rows;
	GnomePropertyBox *prop_box;
} DelAddrList;

extern DelAddrList *deladdrlist_create_edit_page(Card *crd, GnomePropertyBox *box);
extern void         deladdrlist_del_entries (DelAddrList *p);
extern GList       *deladdrlist_get_entries (DelAddrList *p);
extern GList       *deladdrlist_find_by_type (GList *l, gint type);

#endif

