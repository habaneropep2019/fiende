/* GnomeCard - a graphical contact manager.
 *
 * phonelist.c: This file is part of GnomeCard.
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

#include "card.h"
#include "dialog.h"
#include "my.h"
#include "phonelist.h"

gchar *phone_type_name[]={ N_("Preferred"), N_("Work"), N_("Home"), 
			   N_("Voice"), N_("Fax"), N_("Message Recorder"),
			   N_("Cellular"), N_("Pager"), N_("Bulletin Board"),
			   N_("Modem"), N_("Car"), N_("ISDN"), N_("Video"), 
			   NULL };

static void phonelist_checkboxes_to_phone_prop (PhoneList *p, CardPhone *phone)
{
	int i;
	
	g_assert (card_check_prop (phone->prop));
	
	phone->type = 0;
	for (i = 0; i < 13; i++)
	  if (GTK_TOGGLE_BUTTON(p->type[i])->active)
	    phone->type |= 1 << i;
}

static void phonelist_phone_prop_to_checkboxes (PhoneList *p, CardPhone *phone)
{
	int i;
	
	g_assert (card_check_prop (phone->prop));
	
	for (i = 0; i < 13; i++)
	  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (p->type[i]),
					phone->type & 1 << i);
}

static void phonelist_form_clear (PhoneList *p)
{
	int i;
	
	for (i = 0; i < 13; i++)
	  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (p->type[i]), FALSE);
	
	gtk_editable_delete_text(GTK_EDITABLE(p->data), 0, strlen
				 (gtk_entry_get_text(GTK_ENTRY(p->data))));
}

static void phonelist_set_checkboxes_sensitivity (PhoneList *p, gboolean flag)
{
	int i;
	
	for (i = 0; i < 13; i++)
	  gtk_widget_set_sensitive (p->type[i], flag);
}

static void phonelist_set_add_edit_sensitivity (PhoneList *p, gboolean flag)
{
	gtk_widget_set_sensitive (p->add, flag);
	
	if (p->row != -1 || !flag)
	  gtk_widget_set_sensitive (p->edit, flag);
}

static void phonelist_set_form_sensitivity (PhoneList *p, gboolean flag)
{
	phonelist_set_add_edit_sensitivity (p, flag);
	phonelist_set_checkboxes_sensitivity (p, flag);
}

static void phonelist_unselect (PhoneList *p)
{
	p->row = -1;
	gtk_clist_unselect_all (GTK_CLIST (p->clist));
	
	gtk_widget_set_sensitive (p->edit, FALSE);
	gtk_widget_set_sensitive (p->del, FALSE);
	
	gtk_widget_set_sensitive (p->up, FALSE);
	gtk_widget_set_sensitive (p->down, FALSE);
}

static void phonelist_unselect_call (GtkWidget *w, int row, int col, GdkEvent *event, 
				     gpointer data)
{
	phonelist_unselect (data);
}

static void phonelist_select (PhoneList *p, int row)
{
	CardPhone *phone;
	
	g_assert (row >= 0 && row < p->num_rows);
	
	p->row = row;
	phone = g_ptr_array_index (p->phone_array, p->row);
	gtk_widget_set_sensitive (p->edit, TRUE);
	gtk_widget_set_sensitive (p->del, TRUE);
	
	phonelist_phone_prop_to_checkboxes (p, phone);
	gtk_entry_set_text (GTK_ENTRY (p->data), phone->data);
	
	if (p->row != 0)
	  gtk_widget_set_sensitive (p->up, TRUE);
	else
	  gtk_widget_set_sensitive (p->up, FALSE);
	
	if (p->row != p->num_rows - 1)
	  gtk_widget_set_sensitive (p->down, TRUE);
	else
	  gtk_widget_set_sensitive (p->down, FALSE);

	phonelist_set_add_edit_sensitivity (p, FALSE);
	phonelist_set_checkboxes_sensitivity (p, TRUE);
}

static void phonelist_select_force (PhoneList *p, int row)
{
	g_assert (row >= 0 && row < p->num_rows);
	
	gtk_clist_select_row (GTK_CLIST (p->clist), row, 0);
}

static void phonelist_select_call (GtkWidget *w, int row, int col, GdkEvent *event, 
				   gpointer data)
{
	phonelist_select (data, row);
}

static void phonelist_check_change (PhoneList *p)
{
	gboolean flag;
	char *text;
	
	flag = TRUE;
	
	text = gtk_entry_get_text(GTK_ENTRY(p->data));
	if (! *text)
	  flag = FALSE;
	
	phonelist_set_form_sensitivity (p, flag);
}

static void phonelist_data_changed (GtkWidget *w, gpointer data)
{
	phonelist_check_change (data);
}

static void phonelist_add_phone (PhoneList *p, CardPhone *phone)
{
	g_assert (card_check_prop (phone->prop));
	
	p->row = gtk_clist_append (GTK_CLIST (p->clist), &(phone->data));
	g_ptr_array_add (p->phone_array, phone);
	p->num_rows++;
}

static void phonelist_add (GtkWidget *w, gpointer data)
{
	PhoneList *p;
        CardPhone *phone;
	char *text;
	
	p = (PhoneList *) data;
	
	text = gtk_entry_get_text(GTK_ENTRY(p->data));
	if (*text == 0)
	  return;
	
	phone = g_malloc(sizeof(CardPhone));
	phone->data = g_strdup(text);
	phone->type = 0;
	phone->prop = card_prop_empty();
	phone->prop.used = TRUE;
	phonelist_add_phone (p, phone);
	phonelist_checkboxes_to_phone_prop (p, phone);
	phonelist_unselect (p);
	
	phonelist_form_clear (p);
	gnome_property_box_changed(p->prop_box);
}

static void phonelist_edit (GtkWidget *w, gpointer data)
{
	PhoneList *p;
        CardPhone *phone;
	char *text;
	
	g_assert (p->row != -1);
	
	p = (PhoneList *) data;
	
	text = gtk_entry_get_text(GTK_ENTRY(p->data));
	if (*text == 0)
	  return;
	
	phone = g_ptr_array_index (p->phone_array, p->row);
	g_free (phone->data);
	phone->data = g_strdup(text);
	phonelist_checkboxes_to_phone_prop (p, phone);
	gtk_clist_set_text (GTK_CLIST (p->clist), p->row, 0, text);
	
	phonelist_set_add_edit_sensitivity (p, FALSE);
	gnome_property_box_changed(p->prop_box);
}

static void phonelist_del (GtkWidget *w, gpointer data)
{
	PhoneList *p;
        CardPhone *phone;
	int row;
	
	p = (PhoneList *) data;
	
	g_assert (p->row != -1);
	row = p->row;
	
	phone = g_ptr_array_index (p->phone_array, p->row);
	g_free (phone->data);
	g_free (phone);
	g_ptr_array_remove_index (p->phone_array, p->row);
	gtk_clist_remove (GTK_CLIST (p->clist), p->row);
	
	p->num_rows --;
	
	/* This lets do consecutive removals, but that is not very common. It's better
	 * to retain the deleted entry's values and allow it to be added immediatly,
	 * if the user made a mistake. 
	if (row != 0)
	  row--;
	
	p->row = row;
	
	if (p->num_rows != 0)
	  phonelist_select_force (p, p->row);
	else*/
	  phonelist_unselect (p);
	
	gtk_widget_set_sensitive (p->add, TRUE);
	gnome_property_box_changed(p->prop_box);
}

static void phonelist_up (GtkWidget *w, gpointer data)
{
	PhoneList *p;
	CardPhone *temp;
	
	p = (PhoneList *) data;
	
	g_assert (p->row > 0);
	
	gtk_clist_swap_rows (GTK_CLIST (p->clist), p->row, p->row - 1);
	temp = g_ptr_array_index (p->phone_array, p->row);
	g_ptr_array_index (p->phone_array, p->row) =
	  g_ptr_array_index (p->phone_array, p->row - 1);
	g_ptr_array_index (p->phone_array, p->row - 1) = temp;
	p->row --;
	phonelist_select_force (p, p->row);
	gnome_property_box_changed(p->prop_box);
}

static void phonelist_down (GtkWidget *w, gpointer data)
{
	PhoneList *p;
	CardPhone *temp;
	
	p = (PhoneList *) data;
	
	g_assert (p->row < p->num_rows - 1);
	
	gtk_clist_swap_rows (GTK_CLIST (p->clist), p->row, p->row + 1);
	temp = g_ptr_array_index (p->phone_array, p->row);
	g_ptr_array_index (p->phone_array, p->row) =
	  g_ptr_array_index (p->phone_array, p->row + 1);
	g_ptr_array_index (p->phone_array, p->row + 1) = temp;
	p->row ++;
	phonelist_select_force (p, p->row);
	gnome_property_box_changed(p->prop_box);
}

/* Functions for handling phone numbers */
/* delete card list of phone numbers from src, freeing as we go */
extern void phonelist_del_entries (PhoneList *p)
{
        CardPhone *phone;
	int i;

	for (i = 0; i < p->num_rows; i++) {
		phone = g_ptr_array_index (p->phone_array, i);
		g_assert (card_check_prop (phone->prop));
		g_free (phone->data);
		g_free (phone);
	}
}

/* return card list of phone from src into dest, allocating as we go */
extern GList *phonelist_get_entries (PhoneList *p)
{
	GList *list;
        CardPhone *phone, *phone2;
	int i;

	list = NULL;
	for (i = 0; i < p->num_rows; i++) {
		phone = g_ptr_array_index (p->phone_array, i);
		g_assert (card_check_prop (phone->prop));
		phone2 = g_malloc (sizeof (CardPhone));
		phone2->prop = phone->prop;
		phone2->type = phone->type;
		phone2->data = g_strdup (phone->data);
		list = g_list_append (list, phone2);
	}
	
	return list;
}

static void phonelist_copy_crd_phones (PhoneList *p, Card *crd)
{
        CardPhone *phone, *phone2;
	GList *l;

	for (l = crd->phone.l; l; l = l->next) {
		phone = (CardPhone *) l->data;
		g_assert (card_check_prop (phone->prop));
		phone2 = g_malloc (sizeof (CardPhone));
		phone2->prop = phone->prop;
		phone2->type = phone->type;
		phone2->data = g_strdup (phone->data);
		phonelist_add_phone (p, phone2);
	}
}

/* try to find specific phone type in a list of phone numbers */
/* returns ptr to list item if found, otherwise return NULL   */
extern GList *phonelist_find_by_type (GList *l, gint type)
{
	for (; l; l = l->next) {
		CardPhone *phone = ((CardPhone *)l->data);
		
		if ( phone->type & type )
		  break;
	}

	return l;
}

extern PhoneList *phonelist_create_edit_page (Card *crd, GnomePropertyBox *box)
{
	GtkWidget *sw, *w, *hbox, *frame, *vbox, *align;
	GtkWidget *button, *list, *the_hbox;
	PhoneList *p;
	int i;
	
	g_assert (card_check_prop (crd->prop));
	
	p = g_malloc(sizeof(PhoneList));
	p->crd = crd;
	p->num_rows = 0;
	p->row = -1;
	p->prop_box = box;
	p->phone_array = g_ptr_array_new ();
	
	/* Phone number page */
	p->the_vbox = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	the_hbox = w = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(p->the_vbox), the_hbox, FALSE, FALSE, 
			   GNOME_PAD_SMALL);
	gtk_object_set_user_data(GTK_OBJECT(w), p);
    
	frame = gtk_frame_new(_("Telephone data:"));
	gtk_box_pack_start(GTK_BOX(the_hbox), frame, FALSE, FALSE, GNOME_PAD_SMALL);
	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_container_add(GTK_CONTAINER(frame), hbox);
	vbox = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, GNOME_PAD_SMALL);
	p->data = my_hbox_entry(vbox, _("Number:"), NULL);
 	gtk_signal_connect(GTK_OBJECT(p->data), "changed",
			   GTK_SIGNAL_FUNC(phonelist_data_changed), p);

	frame = gtk_frame_new(_("Type:"));
	gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, GNOME_PAD_SMALL);
	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_container_add(GTK_CONTAINER(frame), hbox);
	
	vbox = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, GNOME_PAD_SMALL);

	for (i = 0; i < 13; i++) {
		p->type[i] = gtk_check_button_new_with_label(_(phone_type_name[i]));
		gtk_signal_connect(GTK_OBJECT(p->type[i]), "toggled",
				   GTK_SIGNAL_FUNC(phonelist_data_changed), p);
		gtk_box_pack_start(GTK_BOX(vbox), p->type[i], FALSE, FALSE, 0);
		
		if (i == 6) {
			vbox = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
			gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, GNOME_PAD_SMALL);
		}
	}
		
	vbox = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(the_hbox), vbox, FALSE, FALSE, GNOME_PAD_SMALL);
	align = gtk_alignment_new (0.5, 0.5, 0, 0);
	gtk_box_pack_start(GTK_BOX(vbox), align, TRUE, TRUE, 0);
	vbox = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_container_add(GTK_CONTAINER(align), vbox);

	p->add = button = gnome_pixmap_button 
	  (gnome_stock_pixmap_widget (vbox, GNOME_STOCK_PIXMAP_ADD), _("Add"));
 	gtk_signal_connect(GTK_OBJECT(button), "clicked",
			   GTK_SIGNAL_FUNC(phonelist_add), p);
	gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, GNOME_PAD_SMALL);
	
	p->edit = button = gnome_pixmap_button 
	  (gnome_stock_pixmap_widget (vbox, GNOME_STOCK_PIXMAP_PREFERENCES), _("Modify"));
 	gtk_signal_connect(GTK_OBJECT(button), "clicked",
			   GTK_SIGNAL_FUNC(phonelist_edit), p);
	gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, GNOME_PAD_SMALL);

	p->del = button = gnome_pixmap_button 
	  (gnome_stock_pixmap_widget (vbox, GNOME_STOCK_PIXMAP_REMOVE), _("Remove"));
 	gtk_signal_connect(GTK_OBJECT(button), "clicked",
			   GTK_SIGNAL_FUNC(phonelist_del), p);
	gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, GNOME_PAD_SMALL);
	
	vbox = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(the_hbox), vbox, TRUE, TRUE, GNOME_PAD_SMALL);
	frame = gtk_frame_new(_("Phone List:"));
	gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 0);
	vbox = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_container_add(GTK_CONTAINER(frame), vbox);
        sw = gtk_scrolled_window_new (NULL, NULL);
        gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
					GTK_POLICY_AUTOMATIC, 
					GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start(GTK_BOX(vbox), sw, TRUE, TRUE, GNOME_PAD_SMALL);
	p->clist = list = gtk_clist_new(1);
	gtk_clist_set_selection_mode (GTK_CLIST (list), GTK_SELECTION_SINGLE);
 	gtk_signal_connect(GTK_OBJECT(list), "unselect_row",
			   GTK_SIGNAL_FUNC(phonelist_unselect_call), p);
 	gtk_signal_connect(GTK_OBJECT(list), "select_row",
			   GTK_SIGNAL_FUNC(phonelist_select_call), p);
	gtk_container_add (GTK_CONTAINER (sw), list);
	hbox = gtk_hbox_new(TRUE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, GNOME_PAD_SMALL);
				   
	p->up = button = gtk_button_new();
	gtk_container_add (GTK_CONTAINER (button),
			   gnome_stock_pixmap_widget 
			   (hbox, GNOME_STOCK_PIXMAP_UP));
 	gtk_signal_connect(GTK_OBJECT(button), "clicked",
			   GTK_SIGNAL_FUNC(phonelist_up), p);
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, GNOME_PAD_SMALL);

	p->down = button = gtk_button_new();
	gtk_container_add (GTK_CONTAINER (button),
			   gnome_stock_pixmap_widget 
			   (hbox, GNOME_STOCK_PIXMAP_DOWN));
 	gtk_signal_connect(GTK_OBJECT(button), "clicked",
			   GTK_SIGNAL_FUNC(phonelist_down), p);
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, GNOME_PAD_SMALL);
	
	phonelist_copy_crd_phones (p, crd);
	
	phonelist_unselect (p);
	phonelist_check_change (p);
	
	return p;
}
