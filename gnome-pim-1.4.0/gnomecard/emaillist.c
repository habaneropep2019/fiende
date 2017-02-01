/* GnomeCard - a graphical contact manager.
 *
 * emaillist.c: This file is part of GnomeCard.
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
#include "emaillist.h"

gchar *email_type_name[] = 
	      { N_("America On-Line"), N_("Apple Link"), N_("AT&T"),
		N_("CIS"), N_("e-World"), N_("Internet"), N_("IBM"),
		N_("MCI"), N_("Power Share"), N_("Prodigy"), N_("TLX"),
		N_("X400"), NULL };

static void emaillist_checkboxes_to_email_prop (EMailList *p, CardEMail *email)
{
	int i;
	
	g_assert (card_check_prop (email->prop));

	for (i = 0; i < 12; i++)
	  if (GTK_TOGGLE_BUTTON(p->type[i])->active) {
		  email->type = i + 1;
		  return;
	  }
	
	email->type = 0;
}

static void emaillist_email_prop_to_checkboxes (EMailList *p, CardEMail *email)
{
	g_assert (card_check_prop (email->prop));
	
	if (email->type)
	  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON 
					(p->type[email->type - 1]), TRUE);
}

static void emaillist_form_clear (EMailList *p)
{
	int i;
	
	for (i = 0; i < 12; i++)
	  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (p->type[i]), FALSE);
	
	gtk_editable_delete_text(GTK_EDITABLE(p->data), 0, strlen
				 (gtk_entry_get_text(GTK_ENTRY(p->data))));
}

static void emaillist_set_checkboxes_sensitivity (EMailList *p, gboolean flag)
{
	int i;
	
	for (i = 0; i < 12; i++)
	  gtk_widget_set_sensitive (p->type[i], flag);
}

static void emaillist_set_add_edit_sensitivity (EMailList *p, gboolean flag)
{
	gtk_widget_set_sensitive (p->add, flag);
	
	if (p->row != -1 || !flag)
	  gtk_widget_set_sensitive (p->edit, flag);
}

static void emaillist_set_form_sensitivity (EMailList *p, gboolean flag)
{
	emaillist_set_add_edit_sensitivity (p, flag);
	emaillist_set_checkboxes_sensitivity (p, flag);
}

static void emaillist_unselect (EMailList *p)
{
	p->row = -1;
	gtk_clist_unselect_all (GTK_CLIST (p->clist));
	
	gtk_widget_set_sensitive (p->edit, FALSE);
	gtk_widget_set_sensitive (p->del, FALSE);
	
	gtk_widget_set_sensitive (p->up, FALSE);
	gtk_widget_set_sensitive (p->down, FALSE);
}

static void emaillist_unselect_call (GtkWidget *w, int row, int col, GdkEvent *event, 
				     gpointer data)
{
	emaillist_unselect (data);
}

static void emaillist_select (EMailList *p, int row)
{
	CardEMail *email;
	
	g_assert (row >= 0 && row < p->num_rows);
	
	p->row = row;
	email = g_ptr_array_index (p->email_array, p->row);
	gtk_widget_set_sensitive (p->edit, TRUE);
	gtk_widget_set_sensitive (p->del, TRUE);
	
	emaillist_email_prop_to_checkboxes (p, email);
	gtk_entry_set_text (GTK_ENTRY (p->data), email->data);
	
	if (p->row != 0)
	  gtk_widget_set_sensitive (p->up, TRUE);
	else
	  gtk_widget_set_sensitive (p->up, FALSE);
	
	if (p->row != p->num_rows - 1)
	  gtk_widget_set_sensitive (p->down, TRUE);
	else
	  gtk_widget_set_sensitive (p->down, FALSE);

	emaillist_set_add_edit_sensitivity (p, FALSE);
	emaillist_set_checkboxes_sensitivity (p, TRUE);
}

static void emaillist_select_force (EMailList *p, int row)
{
	g_assert (row >= 0 && row < p->num_rows);
	
	gtk_clist_select_row (GTK_CLIST (p->clist), row, 0);
}

static void emaillist_select_call (GtkWidget *w, int row, int col, GdkEvent *event, 
				   gpointer data)
{
	emaillist_select (data, row);
}

static void emaillist_check_change (EMailList *p)
{
	gboolean flag;
	char *text;
	
	flag = TRUE;
	
	text = gtk_entry_get_text(GTK_ENTRY(p->data));
	if (! *text)
	  flag = FALSE;
	
	emaillist_set_form_sensitivity (p, flag);
}

static void emaillist_data_changed (GtkWidget *w, gpointer data)
{
	emaillist_check_change (data);
}

static void emaillist_add_email (EMailList *p, CardEMail *email)
{
	g_assert (card_check_prop (email->prop));
	
	p->row = gtk_clist_append (GTK_CLIST (p->clist), &(email->data));
	g_ptr_array_add (p->email_array, email);
	p->num_rows++;
}

static void emaillist_add (GtkWidget *w, gpointer data)
{
	EMailList *p;
        CardEMail *email;
	char *text;
	
	p = (EMailList *) data;
	
	text = gtk_entry_get_text(GTK_ENTRY(p->data));
	if (*text == 0)
	  return;
	
	email = g_malloc(sizeof(CardEMail));
	email->data = g_strdup(text);
	email->type = 0;
	email->prop = card_prop_empty();
	email->prop.used = TRUE;
	emaillist_add_email (p, email);
	emaillist_checkboxes_to_email_prop (p, email);
	emaillist_unselect (p);
	
	emaillist_form_clear (p);
	gnome_property_box_changed(p->prop_box);
}

static void emaillist_edit (GtkWidget *w, gpointer data)
{
	EMailList *p;
        CardEMail *email;
	char *text;
	
	g_assert (p->row != -1);
	
	p = (EMailList *) data;
	
	text = gtk_entry_get_text(GTK_ENTRY(p->data));
	if (*text == 0)
	  return;
	
	email = g_ptr_array_index (p->email_array, p->row);
	g_free (email->data);
	email->data = g_strdup(text);
	emaillist_checkboxes_to_email_prop (p, email);
	gtk_clist_set_text (GTK_CLIST (p->clist), p->row, 0, text);
	
	emaillist_set_add_edit_sensitivity (p, FALSE);
	gnome_property_box_changed(p->prop_box);
}

static void emaillist_del (GtkWidget *w, gpointer data)
{
	EMailList *p;
        CardEMail *email;
	int row;
	
	p = (EMailList *) data;
	
	g_assert (p->row != -1);
	row = p->row;
	
	email = g_ptr_array_index (p->email_array, p->row);
	g_free (email->data);
	g_free (email);
	g_ptr_array_remove_index (p->email_array, p->row);
	gtk_clist_remove (GTK_CLIST (p->clist), p->row);
	
	p->num_rows --;
	
	/* This lets do consecutive removals, but that is not very common. It's better
	 * to retain the deleted entry's values and allow it to be added immediatly,
	 * if the user made a mistake. 
	if (row != 0)
	  row--;
	
	p->row = row;
	
	if (p->num_rows != 0)
	  emaillist_select_force (p, p->row);
	else*/
	  emaillist_unselect (p);
	
	gtk_widget_set_sensitive (p->add, TRUE);
	gnome_property_box_changed(p->prop_box);
}

static void emaillist_up (GtkWidget *w, gpointer data)
{
	EMailList *p;
	CardEMail *temp;
	
	p = (EMailList *) data;
	
	g_assert (p->row > 0);
	
	gtk_clist_swap_rows (GTK_CLIST (p->clist), p->row, p->row - 1);
	temp = g_ptr_array_index (p->email_array, p->row);
	g_ptr_array_index (p->email_array, p->row) =
	  g_ptr_array_index (p->email_array, p->row - 1);
	g_ptr_array_index (p->email_array, p->row - 1) = temp;
	p->row --;
	emaillist_select_force (p, p->row);
	gnome_property_box_changed(p->prop_box);
}

static void emaillist_down (GtkWidget *w, gpointer data)
{
	EMailList *p;
	CardEMail *temp;
	
	p = (EMailList *) data;
	
	g_assert (p->row < p->num_rows - 1);
	
	gtk_clist_swap_rows (GTK_CLIST (p->clist), p->row, p->row + 1);
	temp = g_ptr_array_index (p->email_array, p->row);
	g_ptr_array_index (p->email_array, p->row) =
	  g_ptr_array_index (p->email_array, p->row + 1);
	g_ptr_array_index (p->email_array, p->row + 1) = temp;
	p->row ++;
	emaillist_select_force (p, p->row);
	gnome_property_box_changed(p->prop_box);
}

/* Functions for handling email numbers */
/* delete card list of email numbers from src, freeing as we go */
extern void emaillist_del_entries (EMailList *p)
{
        CardEMail *email;
	int i;

	for (i = 0; i < p->num_rows; i++) {
		email = g_ptr_array_index (p->email_array, i);
		g_assert (card_check_prop (email->prop));
		g_free (email->data);
		g_free (email);
	}
}

/* return card list of email from src into dest, allocating as we go */
extern GList *emaillist_get_entries (EMailList *p)
{
	GList *list;
        CardEMail *email, *email2;
	int i;

	list = NULL;
	for (i = 0; i < p->num_rows; i++) {
		email = g_ptr_array_index (p->email_array, i);
		g_assert (card_check_prop (email->prop));
		email2 = g_malloc (sizeof (CardEMail));
		email2->prop = email->prop;
		email2->type = email->type;
		email2->data = g_strdup (email->data);
		list = g_list_append (list, email2);
	}
	
	return list;
}

static void emaillist_copy_crd_emails (EMailList *p, Card *crd)
{
        CardEMail *email, *email2;
	GList *l;

	for (l = crd->email.l; l; l = l->next) {
		email = (CardEMail *) l->data;
		g_assert (card_check_prop (email->prop));
		email2 = g_malloc (sizeof (CardEMail));
		email2->prop = email->prop;
		email2->type = email->type;
		email2->data = g_strdup (email->data);
		emaillist_add_email (p, email2);
	}
}

/* try to find specific email type in a list of email numbers */
/* returns ptr to list item if found, otherwise return NULL   */
extern GList *emaillist_find_by_type (GList *l, gint type)
{
	for (; l; l = l->next) {
		CardEMail *email = ((CardEMail *)l->data);
		
		if ( email->type == type )
		  break;
	}

	return l;
}

extern EMailList *emaillist_create_edit_page (Card *crd, GnomePropertyBox *box)
{
	GtkWidget *sw, *w, *hbox, *frame, *vbox, *align;
	GtkWidget *button, *list, *the_hbox;
	GSList *group;
	EMailList *p;
	int i;
	
	g_assert (card_check_prop (crd->prop));
	
	p = g_malloc(sizeof(EMailList));
	p->crd = crd;
	p->num_rows = 0;
	p->row = -1;
	p->prop_box = box;
	p->email_array = g_ptr_array_new ();
	
	/* Email number page */
	p->the_vbox = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	
	/* URL */
	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(p->the_vbox), hbox, 
			   FALSE, FALSE, GNOME_PAD_SMALL);
	w = gtk_label_new(_("Web Page:"));
	gtk_box_pack_start(GTK_BOX(hbox), w, FALSE, FALSE, GNOME_PAD_SMALL);
	p->url = w = my_gtk_entry_new(0, crd->url.str);
	gtk_box_pack_start(GTK_BOX(hbox), w, TRUE, TRUE, GNOME_PAD_SMALL);
	my_connect(w, "changed", box, &crd->url.prop, PROP_URL);
	
	/* E-mail stuff */

	the_hbox = w = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(p->the_vbox), the_hbox, FALSE, FALSE, 
			   GNOME_PAD_SMALL);
	gtk_object_set_user_data(GTK_OBJECT(w), p);
    
	frame = gtk_frame_new(_("E-mail data:"));
	gtk_box_pack_start(GTK_BOX(the_hbox), frame, FALSE, FALSE, GNOME_PAD_SMALL);
	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_container_add(GTK_CONTAINER(frame), hbox);
	vbox = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, GNOME_PAD_SMALL);
	p->data = my_hbox_entry(vbox, _("Address:"), NULL);
 	gtk_signal_connect(GTK_OBJECT(p->data), "changed",
			   GTK_SIGNAL_FUNC(emaillist_data_changed), p);

	frame = gtk_frame_new(_("Type:"));
	gtk_box_pack_start(GTK_BOX(vbox), frame, FALSE, FALSE, GNOME_PAD_SMALL);
	hbox = gtk_hbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_container_add(GTK_CONTAINER(frame), hbox);
	
	vbox = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, GNOME_PAD_SMALL);

	group = NULL;
	for (i = 0; i < 12; i++) {
		p->type[i] = 
		  gtk_radio_button_new_with_label(group, _(email_type_name[i]));
		
		gtk_signal_connect(GTK_OBJECT(p->type[i]), "toggled",
				   GTK_SIGNAL_FUNC(emaillist_data_changed), p);
		gtk_box_pack_start(GTK_BOX(vbox), p->type[i], FALSE, FALSE, 0);
		gtk_widget_show (p->type[i]);
		group = gtk_radio_button_group (GTK_RADIO_BUTTON (p->type[i]));
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (p->type[i]), FALSE);
		
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
			   GTK_SIGNAL_FUNC(emaillist_add), p);
	gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, GNOME_PAD_SMALL);
	
	p->edit = button = gnome_pixmap_button 
	  (gnome_stock_pixmap_widget (vbox, GNOME_STOCK_PIXMAP_PREFERENCES), _("Modify"));
 	gtk_signal_connect(GTK_OBJECT(button), "clicked",
			   GTK_SIGNAL_FUNC(emaillist_edit), p);
	gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, GNOME_PAD_SMALL);

	p->del = button = gnome_pixmap_button 
	  (gnome_stock_pixmap_widget (vbox, GNOME_STOCK_PIXMAP_REMOVE), _("Remove"));
 	gtk_signal_connect(GTK_OBJECT(button), "clicked",
			   GTK_SIGNAL_FUNC(emaillist_del), p);
	gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, GNOME_PAD_SMALL);
	
	vbox = gtk_vbox_new(FALSE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(the_hbox), vbox, TRUE, TRUE, GNOME_PAD_SMALL);
	frame = gtk_frame_new(_("Email List:"));
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
			   GTK_SIGNAL_FUNC(emaillist_unselect_call), p);
 	gtk_signal_connect(GTK_OBJECT(list), "select_row",
			   GTK_SIGNAL_FUNC(emaillist_select_call), p);
	gtk_container_add (GTK_CONTAINER (sw), list);
	hbox = gtk_hbox_new(TRUE, GNOME_PAD_SMALL);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, GNOME_PAD_SMALL);
				   
	p->up = button = gtk_button_new();
	gtk_container_add (GTK_CONTAINER (button),
			   gnome_stock_pixmap_widget 
			   (hbox, GNOME_STOCK_PIXMAP_UP));
 	gtk_signal_connect(GTK_OBJECT(button), "clicked",
			   GTK_SIGNAL_FUNC(emaillist_up), p);
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, GNOME_PAD_SMALL);

	p->down = button = gtk_button_new();
	gtk_container_add (GTK_CONTAINER (button),
			   gnome_stock_pixmap_widget 
			   (hbox, GNOME_STOCK_PIXMAP_DOWN));
 	gtk_signal_connect(GTK_OBJECT(button), "clicked",
			   GTK_SIGNAL_FUNC(emaillist_down), p);
	gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, GNOME_PAD_SMALL);
	
	emaillist_copy_crd_emails (p, crd);
	
	emaillist_unselect (p);
	emaillist_check_change (p);
	
	return p;
}
