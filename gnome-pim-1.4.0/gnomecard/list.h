/* GnomeCard - a graphical contact manager.
 *
 * list.h: This file is part of GnomeCard.
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

#ifndef __GNOMECARD_LIST
#define __GNOMECARD_LIST

#include <gnome.h>
#include "columnhdrs.h"

void gnomecard_update_list(Card *crd);
void gnomecard_rebuild_list(void);
void gnomecard_scroll_list(GList *node);
void gnomecard_list_set_node_info(Card *crd);
void gnomecard_add_card_to_list(Card *crd);
void gnomecard_add_card_sections_to_list(Card *crd);
void gnomecard_list_set_sorted_pos(Card *crd);
void gnomecard_list_remove_card(Card *crd);

GList *gnomecardCreateColTitles(GList *col);
void gnomecardFreeColTitles(GList *titles);
void gnomecardClearCardListDisplay(GtkWidget *list);
GtkWidget *gnomecardCreateCardListDisplay(ColumnType *hdrs);
#endif


