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
#include <config.h>
#include <gnome.h>

#include "ve-miscui.h"

void
ve_entry_set_red (GtkWidget *w, gboolean state)
{
	if (state) {
		GtkStyle *ns;
		GdkColor red = { 0, 65535, 0, 0 };

		ns = gtk_style_copy (w->style);
		gtk_style_ref (ns);

		ns->fg[GTK_STATE_NORMAL] = red;
		ns->text[GTK_STATE_NORMAL] = red;

		gtk_widget_set_style (w, ns);
		gtk_style_unref (ns);

		gtk_widget_queue_draw (w);
	} else {
		gtk_widget_set_rc_style (w);
	}
}

GtkWidget *
ve_gtk_option_menu_get_item (GtkOptionMenu *option_menu, int index)
{
	g_return_val_if_fail (option_menu != NULL, NULL);
	g_return_val_if_fail (GTK_IS_OPTION_MENU (option_menu), NULL);

	if (option_menu->menu != NULL) {
		return g_list_nth_data (GTK_MENU_SHELL(option_menu->menu)->children,
					index);
	}
	return NULL;
}

int
ve_gtk_option_menu_get_history (GtkOptionMenu *option_menu)
{
	GtkWidget *menu_item;

	g_return_val_if_fail (option_menu != NULL, -1);
	g_return_val_if_fail (GTK_IS_OPTION_MENU (option_menu), -1);

	if (option_menu->menu != NULL) {
		menu_item = gtk_menu_get_active (GTK_MENU (option_menu->menu));
		return g_list_index (GTK_MENU_SHELL(option_menu->menu)->children,
				     menu_item);
	}
	return -1;
}

/* from gnome-libs, not to require new gnome-libs */
void
ve_gnome_dialog_grab_focus (GnomeDialog *dialog, gint button)
{
	GList *list;

	g_return_if_fail(dialog != NULL);
	g_return_if_fail(GNOME_IS_DIALOG(dialog));

	list = g_list_nth (dialog->buttons, button);

	if (list && list->data){
		gtk_widget_grab_focus (GTK_WIDGET (list->data));
		return;
	}
}

