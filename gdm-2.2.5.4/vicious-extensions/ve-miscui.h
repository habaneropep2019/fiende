/* Misc UI routines
 *
 * (c) 2000 Eazel, Inc.
 * (c) 2001 George Lebl
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
#ifndef VE_MISCUI_H
#define VE_MISCUI_H

void		ve_entry_set_red (GtkWidget *w,
				  gboolean state);

GtkWidget *	ve_gtk_option_menu_get_item (GtkOptionMenu *option_menu,
					     int index);
int		ve_gtk_option_menu_get_history (GtkOptionMenu *option_menu);

/* from gnome-libs, not to require new gnome-libs */
void		ve_gnome_dialog_grab_focus (GnomeDialog *dialog, gint button);

#endif /* VE_MISCUI_H */
