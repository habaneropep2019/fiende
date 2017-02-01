/* GnomeCard - a graphical contact manager.
 *
 * sort.h: This file is part of GnomeCard.
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

#ifndef __GNOMECARD_SORT
#define __GNOMECARD_SORT

#include <gnome.h>

#include "card.h"
#include "columnhdrs.h"

typedef int (*sort_func) (const void *, const void *);

extern gint gnomecard_sort_col;

void gnomecard_sort_card_list(gint sort_col);
void gnomecard_sort_card_list_by_default(void);
void gnomecard_sort_cards(GtkWidget *w, gpointer data);

#endif
