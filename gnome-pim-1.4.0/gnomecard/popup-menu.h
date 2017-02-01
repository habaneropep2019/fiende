/* GnomeCard - a graphical contact manager.
 *
 * popup-menu.h: This file is part of GnomeCard.
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

/* Popup menu utilities for gncal
 *
 * Copyright (C) 1998 The Free Software Foundation
 *
 * Author: Federico Mena <quartic@gimp.org>
 */

#ifndef POPUP_MENU_H
#define POPUP_MENU_H

#include <gdk/gdktypes.h>
#include <gtk/gtksignal.h>


struct menu_item {
	char *text;
	GtkSignalFunc callback;
	gpointer data;
	int sensitive;
};

void popup_menu (struct menu_item *items, int nitems, GdkEventButton *event);


#endif