
/*  ----------------------------------------------------------------------

    Copyright (C) 1998  Cesar Miquel  (miquel@df.uba.ar)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    ---------------------------------------------------------------------- */


#include <config.h>
#include <gnome.h>
#include "logview.h"

extern ConfigData *cfg;

/* ----------------------------------------------------------------------
   NAME:          AddMenu
   DESCRIPTION:   Add a menu
   ---------------------------------------------------------------------- */

GtkWidget
* AddMenu (MenuItem * items)
{
   GtkWidget *menu;
   GtkWidget *menuitem;

   menu = gtk_menu_new ();
   gtk_widget_set_style (menu, cfg->main_style);

   while (items->name != NULL)
   {
      /*  Create item */
      menuitem = gtk_menu_item_new_with_label (items->name);
      gtk_menu_append (GTK_MENU (menu), menuitem);
      gtk_widget_set_style (menuitem, cfg->main_style);

      /*  Add callback  */
      if (items->callback != NULL)
	 gtk_signal_connect (GTK_OBJECT (menuitem), "activate",
			     (GtkSignalFunc) items->callback, NULL);
      /*  If this has submenu add those. */
      if (items->submenu != NULL)
	 gtk_menu_item_set_submenu (GTK_MENU_ITEM (menuitem),
				    AddMenu (items->submenu));
      gtk_widget_show (menuitem);
      items++;
   }

   return menu;
}

/* ----------------------------------------------------------------------
   NAME:          StubCall
   DESCRIPTION:   This is a stub routine for a menu item. It is used
   as testing.
   ---------------------------------------------------------------------- */

void
StubCall (GtkWidget * widget, gpointer user_data)
{
}
