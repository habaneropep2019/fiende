/* GnomeCard - a graphical contact manager.
 *
 * popup-menu.c: This file is part of GnomeCard.
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

#include <config.h>
#include <gnome.h>
#include "popup-menu.h"


void
popup_menu (struct menu_item *items, int nitems, GdkEventButton *event)
{
	GtkWidget *menu;
	GtkWidget *item;
	int i;

	menu = gtk_menu_new (); /* FIXME: this baby is never freed */

	for (i = 0; i < nitems; i++) {
		if (items[i].text) {
			item = gtk_menu_item_new_with_label (_(items[i].text));
			gtk_signal_connect (GTK_OBJECT (item), "activate",
					    items[i].callback,
					    items[i].data);
			gtk_widget_set_sensitive (item, items[i].sensitive);
		} else
			item = gtk_menu_item_new ();

		gtk_widget_show (item);
		gtk_menu_append (GTK_MENU (menu), item);
	}

	gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL, event->button, event->time);
}
