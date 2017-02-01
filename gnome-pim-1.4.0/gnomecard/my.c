/* GnomeCard - a graphical contact manager.
 *
 * my.c: This file is part of GnomeCard.
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

#include <config.h>
#include <gnome.h>
#include <ctype.h>

#include "gnomecard.h"
#include "my.h"

static void gnomecard_property_used(GtkWidget *w, gpointer data);

extern char *my_cap (char *str) 
{
	int i;
	
	str = g_strdup (str);
	
	str[0] = toupper (str[0]);
	
	for (i = 1; i < strlen (str); i++)
	  str[i] = tolower (str[i]);
	
	return str;
}

GtkWidget *
my_gtk_entry_new(gint len, char *init)
{
	GtkWidget *entry;
	
	entry = gtk_entry_new();
	if (len)
	  gtk_widget_set_usize (entry, 
				gdk_char_width (entry->style->font, 'M') * len, 0);
	if (init)
	  gtk_entry_set_text(GTK_ENTRY(entry), init);

	return entry;
}

extern GtkWidget *my_hbox_entry(GtkWidget *parent, char *label, char *init)
{
	GtkWidget *hbox, *w;

	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(parent), hbox, FALSE, FALSE, 0);
	w = gtk_label_new(label);
	gtk_box_pack_start(GTK_BOX(hbox), w, FALSE, FALSE, 0);
	w = my_gtk_entry_new(0, init);
	gtk_box_pack_start(GTK_BOX(hbox), w, TRUE, TRUE, 0);
	
	return w;
}

extern GtkWidget *my_gtk_spin_button_new(GtkAdjustment *adj, gint width)
{
	GtkWidget *spin;
	
	spin = gtk_spin_button_new(GTK_ADJUSTMENT(adj), 1, 0);
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON(spin), TRUE);			      
	gtk_widget_set_usize (spin, /*22 + 8 * width, 0);*/
		    gdk_char_width (spin->style->font, '-') * width + 22, 0);
			      
	return spin;
}

extern GtkWidget *my_gtk_vbox_new(void)
{
	GtkWidget *vbox;
	
	vbox = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_container_border_width(GTK_CONTAINER(vbox), GNOME_PAD_SMALL);

	return vbox;
}

extern GtkWidget *my_gtk_table_new(int x, int y)
{
	GtkWidget *table;
	
	table = gtk_table_new(x, y, FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table), GNOME_PAD_SMALL);
	gtk_table_set_col_spacings(GTK_TABLE(table), GNOME_PAD_SMALL);
	gtk_container_border_width(GTK_CONTAINER(table), GNOME_PAD_SMALL);
	
	return table;
}

static void
gnomecard_property_used(GtkWidget *w, gpointer data)
{
    CardProperty *prop;
    
    prop = (CardProperty *) data;
    prop->type = (int) gtk_object_get_user_data(GTK_OBJECT(w));
    prop->used = TRUE;
}

extern void my_connect(gpointer widget, char *sig, gpointer box, 
		       CardProperty *prop, enum PropertyType type)
{
 	gtk_signal_connect_object(GTK_OBJECT(widget), sig,
				  GTK_SIGNAL_FUNC(gnome_property_box_changed),
				  GTK_OBJECT(box));
	gtk_signal_connect(GTK_OBJECT(widget), sig,
			   GTK_SIGNAL_FUNC(gnomecard_property_used),
			   prop);
	gtk_object_set_user_data(GTK_OBJECT(widget), (gpointer) type);
}
