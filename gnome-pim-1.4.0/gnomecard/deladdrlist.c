/* GnomeCard - a graphical contact manager.
 *
 * deladdrlist.c: This file is part of GnomeCard.
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
#include "deladdrlist.h"

gchar *deladdr_label[] = { N_("Post Office:"), N_("Extended:"), N_("Street:"),
	                   N_("City:"), N_("Region:"), N_("Postal Code:"),
	                   N_("Country:"), NULL };

gchar *deladdr_type_name[] = { N_("Home"), N_("Work"), N_("Postal Box"), 
	                       N_("Parcel"), N_("Domestic"), N_("International"),
			       NULL };

static void deladdrlist_checkboxes_to_deladdr_prop (DelAddrList *p, CardDelAddr *deladdr)
{
	int i;
	
	g_assert (card_check_prop (deladdr->prop));
	
	deladdr->type = 0;
	for (i = 0; i < 6; i++)
	  if (GTK_TOGGLE_BUTTON(p->type[i])->active)
	    deladdr->type |= 1 << i;
}

static void deladdrlist_deladdr_prop_to_checkboxes (DelAddrList *p, CardDelAddr *deladdr)
{
	int i;
	
	g_assert (card_check_prop (deladdr->prop));
	
	for (i = 0; i < 6; i++)
	  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (p->type[i]),
					deladdr->type & 1 << i);
}

static void deladdrlist_form_clear (DelAddrList *p)
{
	int i;
	
	for (i = 0; i < 6; i++)
	  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (p->type[i]), FALSE);
	
	for (i = 0; i < DELADDR_MAX; i++)
	  gtk_editable_delete_text(GTK_EDITABLE(p->data[i]), 0, strlen
				   (gtk_entry_get_text(GTK_ENTRY(p->data[i]))));
}

static void deladdrlist_set_checkboxes_sensitivity (DelAddrList *p, gboolean flag)
{
	int i;
	
	for (i = 0; i < 6; i++)
	  gtk_widget_set_sensitive (p->type[i], flag);
}

static void deladdrlist_set_add_edit_sensitivity (DelAddrList *p, gboolean flag)
{
	gtk_widget_set_sensitive (p->add, flag);
	
	if (p->row != -1 || !flag)
	  gtk_widget_set_sensitive (p->edit, flag);
}

static void deladdrlist_set_form_sensitivity (DelAddrList *p, gboolean flag)
{
	deladdrlist_set_add_edit_sensitivity (p, flag);
	deladdrlist_set_checkboxes_sensitivity (p, flag);
}

static void deladdrlist_unselect (DelAddrList *p)
{
	p->row = -1;
	gtk_clist_unselect_all (GTK_CLIST (p->clist));
	
	gtk_widget_set_sensitive (p->edit, FALSE);
	gtk_widget_set_sensitive (p->del, FALSE);
	
	gtk_widget_set_sensitive (p->up, FALSE);
	gtk_widget_set_sensitive (p->down, FALSE);
}

static void deladdrlist_unselect_call (GtkWidget *w, int row, int col, GdkEvent *event, 
				     gpointer data)
{
	deladdrlist_unselect (data);
}

static void deladdrlist_select (DelAddrList *p, int row)
{
	CardDelAddr *deladdr;
	int i;
	
	g_assert (row >= 0 && row < p->num_rows);
	
	p->row = row;
	deladdr = g_ptr_array_index (p->deladdr_array, p->row);
	gtk_widget_set_sensitive (p->edit, TRUE);
	gtk_widget_set_sensitive (p->del, TRUE);
	
	deladdrlist_deladdr_prop_to_checkboxes (p, deladdr);
	
	for (i = 0; i < DELADDR_MAX; i++)
	  gtk_entry_set_text (GTK_ENTRY (p->data[i]), 
			      MY_STR(deladdr->data[i]));
	
	if (p->row != 0)
	  gtk_widget_set_sensitive (p->up, TRUE);
	else
	  gtk_widget_set_sensitive (p->up, FALSE);
	
	if (p->row != p->num_rows - 1)
	  gtk_widget_set_sensitive (p->down, TRUE);
	else
	  gtk_widget_set_sensitive (p->down, FALSE);

	deladdrlist_set_add_edit_sensitivity (p, FALSE);
	deladdrlist_set_checkboxes_sensitivity (p, TRUE);
}

static void deladdrlist_select_force (DelAddrList *p, int row)
{
	g_assert (row >= 0 && row < p->num_rows);
	
	gtk_clist_select_row (GTK_CLIST (p->clist), row, 0);
}

static void deladdrlist_select_call (GtkWidget *w, int row, int col, GdkEvent *event, 
				   gpointer data)
{
	deladdrlist_select (data, row);
}

static void deladdrlist_check_change (DelAddrList *p)
{
	gboolean flag;
	char *text;
	int i;
	
	flag = FALSE;
	
	for (i = 0; i < DELADDR_MAX; i++) {
		text = gtk_entry_get_text(GTK_ENTRY(p->data[i]));
		if (*text) {
			flag = TRUE;
			break;
		}
	}
	
	deladdrlist_set_form_sensitivity (p, flag);
}

static void deladdrlist_data_changed (GtkWidget *w, gpointer data)
{
	deladdrlist_check_change (data);
}

static void deladdrlist_add_deladdr (DelAddrList *p, CardDelAddr *deladdr)
{
	int i;
	
	g_assert (card_check_prop (deladdr->prop));
	
	for (i = 0; i < DELADDR_MAX; i++)
	  if (deladdr->data[i] && *deladdr->data[i])
	    break;
	
	p->row = gtk_clist_append (GTK_CLIST (p->clist), &(deladdr->data[i]));
	g_ptr_array_add (p->deladdr_array, deladdr);
	p->num_rows++;
}

static void deladdrlist_add (GtkWidget *w, gpointer data)
{
	DelAddrList *p;
        CardDelAddr *deladdr;
	char *text;
	int i;
	
	p = (DelAddrList *) data;
	
	for (i = 0; i < DELADDR_MAX; i++) {
		text = gtk_entry_get_text(GTK_ENTRY(p->data[i]));
		if (*text)
		  break;
	}
	
	if (i == DELADDR_MAX)
	  return;
	
	deladdr = g_malloc(sizeof(CardDelAddr));
	
	for (i = 0; i < DELADDR_MAX; i++)
	  deladdr->data[i] = 
	   g_strdup(gtk_entry_get_text(GTK_ENTRY(p->data[i])));
	
	deladdr->type = 0;
	deladdr->prop = card_prop_empty();
	deladdr->prop.used = TRUE;
	deladdrlist_add_deladdr (p, deladdr);
	deladdrlist_checkboxes_to_deladdr_prop (p, deladdr);
	deladdrlist_unselect (p);
	
	deladdrlist_form_clear (p);
	gnome_property_box_changed(p->prop_box);
}

static void deladdrlist_edit (GtkWidget *w, gpointer data)
{
	DelAddrList *p;
        CardDelAddr *deladdr;
	char *text;
	int i;
	
	g_assert (p->row != -1);
	
	p = (DelAddrList *) data;
	
	for (i = 0; i < DELADDR_MAX; i++) {
		text = gtk_entry_get_text(GTK_ENTRY(p->data[i]));
		if (*text)
		  break;
	}
	
	if (i == DELADDR_MAX)
	  return;
	
	deladdr = g_ptr_array_index (p->deladdr_array, p->row);
	
	for (i = 0; i < DELADDR_MAX; i++) {
		g_free (deladdr->data[i]);
		deladdr->data[i] = 
		  g_strdup(gtk_entry_get_text(GTK_ENTRY(p->data[i])));
	}
	
	deladdrlist_checkboxes_to_deladdr_prop (p, deladdr);
	gtk_clist_set_text (GTK_CLIST (p->clist), p->row, 0, text);
	
	deladdrlist_set_add_edit_sensitivity (p, FALSE);
	gnome_property_box_changed(p->prop_box);
}

static void deladdrlist_del (GtkWidget *w, gpointer data)
{
	DelAddrList *p;
        CardDelAddr *deladdr;
	int row, i;
	
	p = (DelAddrList *) data;
	
	g_assert (p->row != -1);
	row = p->row;
	
	deladdr = g_ptr_array_index (p->deladdr_array, p->row);
	for (i = 0; i < DELADDR_MAX; i++)
	  g_free (deladdr->data[i]);
	g_free (deladdr);
	g_ptr_array_remove_index (p->deladdr_array, p->row);
	gtk_clist_remove (GTK_CLIST (p->clist), p->row);
	
	p->num_rows --;
	
	/* This lets do consecutive removals, but that is not very common. It's better
	 * to retain the deleted entry's values and allow it to be added immediatly,
	 * if the user made a mistake. 
	if (row != 0)
	  row--;
	
	p->row = row;
	
	if (p->num_rows != 0)
	  deladdrlist_select_force (p, p->row);
	else*/
	  deladdrlist_unselect (p);
	
	gtk_widget_set_sensitive (p->add, TRUE);
	gnome_property_box_changed(p->prop_box);
}

static void deladdrlist_up (GtkWidget *w, gpointer data)
{
	DelAddrList *p;
	CardDelAddr *temp;
	
	p = (DelAddrList *) data;
	
	g_assert (p->row > 0);
	
	gtk_clist_swap_rows (GTK_CLIST (p->clist), p->row, p->row - 1);
	temp = g_ptr_array_index (p->deladdr_array, p->row);
	g_ptr_array_index (p->deladdr_array, p->row) =
	  g_ptr_array_index (p->deladdr_array, p->row - 1);
	g_ptr_array_index (p->deladdr_array, p->row - 1) = temp;
	p->row --;
	deladdrlist_select_force (p, p->row);
	gnome_property_box_changed(p->prop_box);
}

static void deladdrlist_down (GtkWidget *w, gpointer data)
{
	DelAddrList *p;
	CardDelAddr *temp;
	
	p = (DelAddrList *) data;
	
	g_assert (p->row < p->num_rows - 1);
	
	gtk_clist_swap_rows (GTK_CLIST (p->clist), p->row, p->row + 1);
	temp = g_ptr_array_index (p->deladdr_array, p->row);
	g_ptr_array_index (p->deladdr_array, p->row) =
	  g_ptr_array_index (p->deladdr_array, p->row + 1);
	g_ptr_array_index (p->deladdr_array, p->row + 1) = temp;
	p->row ++;
	deladdrlist_select_force (p, p->row);
	gnome_property_box_changed(p->prop_box);
}

/* Functions for handling deladdr numbers */
/* delete card list of deladdr numbers from src, freeing as we go */
extern void deladdrlist_del_entries (DelAddrList *p)
{
        CardDelAddr *deladdr;
	int i, j;

	for (i = 0; i < p->num_rows; i++) {
		deladdr = g_ptr_array_index (p->deladdr_array, i);
		g_assert (card_check_prop (deladdr->prop));
		for (j = 0; j < DELADDR_MAX; j++)
			g_free (deladdr->data[j]);
		g_free (deladdr);
	}
}

/* return card list of deladdr from src into dest, allocating as we go */
extern GList *deladdrlist_get_entries (DelAddrList *p)
{
	GList *list;
        CardDelAddr *deladdr, *deladdr2;
	int i, j;

	list = NULL;
	for (i = 0; i < p->num_rows; i++) {
		deladdr = g_ptr_array_index (p->deladdr_array, i);
		g_assert (card_check_prop (deladdr->prop));
		deladdr2 = g_malloc (sizeof (CardDelAddr));
		deladdr2->prop = deladdr->prop;
		deladdr2->type = deladdr->type;
		
		for (j = 0; j < DELADDR_MAX; j++)
		  deladdr2->data[j] = g_strdup (deladdr->data[j]);
		list = g_list_append (list, deladdr2);
	}
	
	return list;
}

static void deladdrlist_copy_crd_deladdrs (DelAddrList *p, Card *crd)
{
        CardDelAddr *deladdr, *deladdr2;
	GList *l;
	int i;

	for (l = crd->deladdr.l; l; l = l->next) {
		deladdr = (CardDelAddr *) l->data;
		g_assert (card_check_prop (deladdr->prop));
		deladdr2 = g_malloc (sizeof (CardDelAddr));
		deladdr2->prop = deladdr->prop;
		deladdr2->type = deladdr->type;
		
		for (i = 0; i < DELADDR_MAX; i++)
		  deladdr2->data[i] = g_strdup (deladdr->data[i]);
		deladdrlist_add_deladdr (p, deladdr2);
	}
}

/* try to find specific deladdr type in a list of deladdr numbers */
/* returns ptr to list item if found, otherwise return NULL   */
extern GList *deladdrlist_find_by_type (GList *l, gint type)
{
	for (; l; l = l->next) {
		CardDelAddr *deladdr = ((CardDelAddr *)l->data);
		
		if ( deladdr->type & type )
		  break;
	}

	return l;
}

extern DelAddrList *deladdrlist_create_edit_page (Card *crd, GnomePropertyBox *box)
{
	GtkWidget *sw, *w, *hbox, *frame, *vbox, *table, *align;
	GtkWidget *label, *button, *list, *the_hbox;
	DelAddrList *p;
	int i;
	
	g_assert (card_check_prop (crd->prop));
	
	p = g_malloc(sizeof(DelAddrList));
	p->crd = crd;
	p->num_rows = 0;
	p->row = -1;
	p->prop_box = box;
	p->deladdr_array = g_ptr_array_new ();
	
	/* deladdr data page */
	p->the_vbox = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	the_hbox = w = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(p->the_vbox), the_hbox, FALSE, FALSE, 
			   GNOME_PAD_SMALL);
	gtk_object_set_user_data(GTK_OBJECT(w), p);
    
	frame = gtk_frame_new(_("Address data:"));
	gtk_box_pack_start(GTK_BOX(the_hbox), frame, FALSE, FALSE, GNOME_PAD_SMALL);
	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_container_add(GTK_CONTAINER(frame), hbox);
	vbox = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, GNOME_PAD_SMALL);
	table = gtk_table_new(2, DELADDR_MAX, FALSE);
	gtk_table_set_row_spacings(GTK_TABLE(table), 0);
	gtk_table_set_col_spacings(GTK_TABLE(table), GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, 0);
	
	for (i = 0; i < DELADDR_MAX; i++) {
		label = gtk_label_new(_(deladdr_label[i]));
		gtk_misc_set_alignment(GTK_MISC(label), 1, 0.5);
		gtk_table_attach(GTK_TABLE(table), label, 0, 1, i, i + 1,
				 GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 
				 0, 0);
		p->data[i] = my_gtk_entry_new(0, NULL);
		gtk_table_attach(GTK_TABLE(table), p->data[i], 1, 2, i, i + 1,
				 GTK_FILL | GTK_SHRINK | GTK_EXPAND, GTK_FILL | GTK_SHRINK,
				 GNOME_PAD_SMALL, GNOME_PAD_SMALL / 2);
		gtk_signal_connect(GTK_OBJECT(p->data[i]), "changed",
				   GTK_SIGNAL_FUNC(deladdrlist_data_changed), p);
	}

	frame = gtk_frame_new(_("Type:"));
	gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, GNOME_PAD_SMALL);
	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_container_add(GTK_CONTAINER(frame), hbox);
	
	vbox = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, GNOME_PAD_SMALL);

	for (i = 0; i < 6; i++) {
		p->type[i] = gtk_check_button_new_with_label(_(deladdr_type_name[i]));
		gtk_signal_connect(GTK_OBJECT(p->type[i]), "toggled",
				   GTK_SIGNAL_FUNC(deladdrlist_data_changed), p);
		gtk_box_pack_start(GTK_BOX(vbox), p->type[i], FALSE, FALSE, 0);
		
		if (i == 2) {
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
			   GTK_SIGNAL_FUNC(deladdrlist_add), p);
	gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, GNOME_PAD_SMALL);
	
	p->edit = button = gnome_pixmap_button 
	  (gnome_stock_pixmap_widget (vbox, GNOME_STOCK_PIXMAP_PREFERENCES), _("Modify"));
 	gtk_signal_connect(GTK_OBJECT(button), "clicked",
			   GTK_SIGNAL_FUNC(deladdrlist_edit), p);
	gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, GNOME_PAD_SMALL);

	p->del = button = gnome_pixmap_button 
	  (gnome_stock_pixmap_widget (vbox, GNOME_STOCK_PIXMAP_REMOVE), _("Remove"));
 	gtk_signal_connect(GTK_OBJECT(button), "clicked",
			   GTK_SIGNAL_FUNC(deladdrlist_del), p);
	gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, GNOME_PAD_SMALL);
	
	vbox = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(the_hbox), vbox, TRUE, TRUE, GNOME_PAD_SMALL);
	frame = gtk_frame_new(_("Address List:"));
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
			   GTK_SIGNAL_FUNC(deladdrlist_unselect_call), p);
 	gtk_signal_connect(GTK_OBJECT(list), "select_row",
			   GTK_SIGNAL_FUNC(deladdrlist_select_call), p);
	gtk_container_add (GTK_CONTAINER (sw), list);
	hbox = gtk_hbox_new(TRUE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, GNOME_PAD_SMALL);
				   
	p->up = button = gtk_button_new();
	gtk_container_add (GTK_CONTAINER (button),
			   gnome_stock_pixmap_widget 
			   (hbox, GNOME_STOCK_PIXMAP_UP));
 	gtk_signal_connect(GTK_OBJECT(button), "clicked",
			   GTK_SIGNAL_FUNC(deladdrlist_up), p);
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, GNOME_PAD_SMALL);

	p->down = button = gtk_button_new();
	gtk_container_add (GTK_CONTAINER (button),
			   gnome_stock_pixmap_widget 
			   (hbox, GNOME_STOCK_PIXMAP_DOWN));
 	gtk_signal_connect(GTK_OBJECT(button), "clicked",
			   GTK_SIGNAL_FUNC(deladdrlist_down), p);
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, GNOME_PAD_SMALL);
	
	deladdrlist_copy_crd_deladdrs (p, crd);
	
	deladdrlist_unselect (p);
	deladdrlist_check_change (p);
	
	return p;
}
